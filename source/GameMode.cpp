﻿#include "GameMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "PlayerDonutModel.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Level Layout
/** The Angle in degrees for fixing a breach*/
constexpr float EPSILON_ANGLE = 5.2f;
/** The Angle in degrees for which a door can be activated*/
constexpr float DOOR_ACTIVE_ANGLE = 15.0f;
/** Angles to adjust per frame to prevent door tunneling */
constexpr float ANGLE_ADJUST = 0.5f;

// Friction
/** The friction factor while fixing a breach */
constexpr float FIX_BREACH_FRICTION = 0.65f;
/** The friction factor applied when moving through other players breaches */
constexpr float OTHER_BREACH_FRICTION = 0.2f;

// Health
/** Grace period for a breach before it starts deducting health */
constexpr float BREACH_HEALTH_GRACE_PERIOD = 5.0f;
/** Amount of health to decrement each frame per breach */
constexpr float BREACH_HEALTH_PENALTY = 0.003f;
/** Some undocumented upper bound for challenge progress */
constexpr int CHALLENGE_PROGRESS_HIGH = 100;
/** Some undocumented lower bound for challenge progress */
constexpr int CHALLENGE_PROGRESS_LOW = 10;

#pragma mark -
#pragma mark Constructors

/**
 * Initializes the controller contents, and starts the game
 *
 * The constructor does not allocate any objects or memory.  This allows
 * us to have a non-pointer reference to this controller, reducing our
 * memory allocation.  Instead, allocation happens in this method.
 *
 * @param assets    The (loaded) assets for this game mode
 *
 * @return true if the controller is initialized properly, false otherwise.
 */
bool GameMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	isBackToMainMenu = false;

	// Music Initialization

	auto source = assets->get<Sound>("theme");
	if (AudioChannels::get()->currentMusic() == nullptr ||
		AudioChannels::get()->currentMusic()->getFile() != source->getFile()) {
		AudioChannels::get()->stopMusic(globals::MUSIC_FADE_OUT);
		AudioChannels::get()->queueMusic(source, true, source->getVolume(), globals::MUSIC_FADE_IN);
	}
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}

	// Input Initialization
	input = InputController::getInstance();
	input->clear();

	// Network Initialization
	net = MagicInternetBox::getInstance();
	playerID = net->getPlayerID();
	roomId = net->getRoomID();

	const char* levelName = LEVEL_NAMES.at(net->getLevelNum());

	CULog("Loading level %s b/c mib gave level num %d", levelName, net->getLevelNum());
	unsigned int shipNumPlayers = net->getMaxNumPlayers();

	std::shared_ptr<LevelModel> level = assets->get<LevelModel>(levelName);
	int maxEvents = (int)(level->getMaxBreaches() * shipNumPlayers / globals::MIN_PLAYERS);
	int maxDoors = std::min(level->getMaxDoors() * shipNumPlayers / globals::MIN_PLAYERS,
							shipNumPlayers * 2 - 1);
	int maxButtons = (int)(level->getMaxButtons() * shipNumPlayers / globals::MIN_PLAYERS);
	if (maxButtons % 2 != 0) maxButtons += 1;
	ship = ShipModel::alloc(shipNumPlayers, maxEvents, maxDoors, playerID,
							(float)level->getShipSize((int)shipNumPlayers),
							(int)(level->getInitHealth() * shipNumPlayers / globals::MIN_PLAYERS),
							maxButtons);
	gm.init(ship, level);

	donutModel = ship->getDonuts().at(static_cast<unsigned long>(playerID));
	ship->initTimer(level->getTime());
	ship->setLevelNum(net->getLevelNum());

	// Scene graph Initialization
	sgRoot.init(assets, ship, playerID);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void GameMode::dispose() {
	gm.dispose();
	sgRoot.dispose();
	donutModel = nullptr;
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void GameMode::update(float timestep) {
	// Check if need to go back to menu
	if (!isBackToMainMenu) {
		isBackToMainMenu = sgRoot.getIsBackToMainMenu();
		if (isBackToMainMenu) {
			AudioChannels::get()->stopMusic(1);
		}
	}
	// Set needle percentage in pause menu
	sgRoot.setNeedlePercentage((float)(net->getNumPlayers() - 1) / (float)globals::MAX_PLAYERS);

	// Connection Status Checks
	switch (net->matchStatus()) {
		case MagicInternetBox::Disconnected:
		case MagicInternetBox::ClientRoomInvalid:
		case MagicInternetBox::ReconnectError:
			if (net->reconnect(roomId)) {
				net->update();
			}
			sgRoot.setStatus(GameGraphRoot::Reconnecting);
			sgRoot.update(timestep);
			return;
		case MagicInternetBox::Reconnecting:
			// Still Reconnecting
			net->update();
			sgRoot.setStatus(GameGraphRoot::Reconnecting);
			sgRoot.update(timestep);
			return;
		case MagicInternetBox::ClientRoomFull:
		case MagicInternetBox::GameEnded:
			// Game Ended, Replace with Another Screen
			CULog("Game Ended");
			net->update(ship);
			sgRoot.update(timestep);
			return;
		case MagicInternetBox::GameStart:
			net->update(ship);
			sgRoot.setStatus(GameGraphRoot::Normal);
			break;
		default:
			CULog("ERROR: Uncaught MatchmakingStatus Value Occurred");
	}

	// Only process game logic if game is running
	input->update(timestep);

	// Check for loss
	if (ship->getHealth() < 1) {
		sgRoot.setStatus(GameGraphRoot::Loss);
		sgRoot.update(timestep);

		switch (sgRoot.getAndResetLastButtonPressed()) {
			case GameGraphRoot::GameButton::Restart:
				net->restartGame();
				break;
			case GameGraphRoot::GameButton::NextLevel:
				CULog("Next Level Pressed");
				net->nextLevel();
				break;
			default:
				break;
		}
		return;
	}

	// Jump Logic. Moved here for Later Win Screen Jump support (Demi's Request)
	if (input->hasJumped() && !donutModel->isJumping()) {
		donutModel->startJump();
		net->jump(playerID);
	}

	// Check for Win
	if (ship->timerEnded() && ship->getHealth() > 0) {
		sgRoot.setStatus(GameGraphRoot::Win);
		sgRoot.update(timestep);

		switch (sgRoot.getAndResetLastButtonPressed()) {
			case GameGraphRoot::GameButton::Restart:
				net->restartGame();
				break;
			case GameGraphRoot::GameButton::NextLevel:
				CULog("Next Level Pressed");
				net->nextLevel();
				break;
			default:
				break;
		}
		return;
	}

	if (!(ship->timerEnded())) {
		ship->updateTimer(timestep);
	}

	// Move the donut (MODEL ONLY)
	float thrust = input->getRoll();
	donutModel->applyForce(thrust);

	// Breach Checks
	for (int i = 0; i < ship->getBreaches().size(); i++) {
		std::shared_ptr<BreachModel> breach = ship->getBreaches().at(i);
		if (breach == nullptr || !breach->getIsActive()) {
			continue;
		}
		float diff = ship->getSize() / 2 -
					 abs(abs(donutModel->getAngle() - breach->getAngle()) - ship->getSize() / 2);
		if (!donutModel->isJumping() && playerID != breach->getPlayer() &&
			diff < globals::BREACH_WIDTH && breach->getHealth() != 0) {
			// Slow player by drag factor
			donutModel->setFriction(OTHER_BREACH_FRICTION);
		} else if (playerID == breach->getPlayer() && diff < EPSILON_ANGLE &&
				   donutModel->getJumpOffset() == 0.0f && breach->getHealth() > 0) {
			if (!breach->isPlayerOn()) {
				// Decrement Health
				breach->decHealth(1);
				breach->setIsPlayerOn(true);
				net->resolveBreach(i);
			}

			// Slow player by friction factor if not already slowed more
			if (donutModel->getFriction() > FIX_BREACH_FRICTION) {
				donutModel->setFriction(FIX_BREACH_FRICTION);
			}

		} else if (diff > EPSILON_ANGLE && breach->isPlayerOn()) {
			breach->setIsPlayerOn(false);
		}
	}

	// Door Checks
	for (int i = 0; i < ship->getDoors().size(); i++) {
		if (ship->getDoors().at(i) == nullptr || ship->getDoors().at(i)->halfOpen() ||
			!ship->getDoors().at(i)->getIsActive()) {
			continue;
		}
		float diff = donutModel->getAngle() - ship->getDoors().at(i)->getAngle();
		float a = diff + ship->getSize() / 2;
		diff = a - floor(a / ship->getSize()) * ship->getSize() - ship->getSize() / 2;

		if (abs(diff) < globals::DOOR_WIDTH) {
			// Stop donut and push it out if inside
			donutModel->setVelocity(0);
			if (diff < 0) {
				donutModel->setAngle(donutModel->getAngle() - ANGLE_ADJUST < 0.0f
										 ? ship->getSize()
										 : donutModel->getAngle() - ANGLE_ADJUST);
			} else {
				donutModel->setAngle(donutModel->getAngle() + ANGLE_ADJUST > ship->getSize()
										 ? 0.0f
										 : donutModel->getAngle() + ANGLE_ADJUST);
			}
		}
		if (abs(diff) < DOOR_ACTIVE_ANGLE) {
			ship->getDoors().at(i)->addPlayer(playerID);
			net->flagDualTask(i, playerID, 1);
		} else {
			if (ship->getDoors().at(i)->isPlayerOn(playerID)) {
				ship->getDoors().at(i)->removePlayer(playerID);
				net->flagDualTask(i, playerID, 0);
			}
		}
	}

	for (int i = 0; i < ship->getBreaches().size(); i++) {
		// this should be adjusted based on the level and number of players
		if (ship->getBreaches().at(i)->getIsActive() &&
			trunc(ship->getBreaches().at(i)->getTimeCreated()) - trunc(ship->timer) >
				BREACH_HEALTH_GRACE_PERIOD) {
			ship->decHealth(BREACH_HEALTH_PENALTY);
		}
	}

	gm.update(timestep);

	for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
		ship->getDonuts()[i]->update(timestep);
	}

	if (ship->getChallenge() && trunc(ship->timer) <= globals::ROLL_CHALLENGE_LENGTH) {
		ship->setChallenge(false);
	}

	if (ship->getChallenge() && trunc(ship->timer) > globals::ROLL_CHALLENGE_LENGTH) {
		bool allRoll = true;
		for (unsigned int i = 0; i < ship->getDonuts().size(); i++) {
			if (ship->getRollDir() == 0) {
				if (ship->getDonuts()[i]->getVelocity() >= 0) {
					allRoll = false;
					break;
				}
			} else {
				if (ship->getDonuts()[i]->getVelocity() <= 0) {
					allRoll = false;
					break;
				}
			}
		}
		if (allRoll) {
			ship->updateChallengeProg();
		}
		if (ship->getChallengeProg() > CHALLENGE_PROGRESS_HIGH ||
			trunc(ship->timer) == trunc(ship->getEndTime())) {
			if (ship->getChallengeProg() < CHALLENGE_PROGRESS_LOW) {
				gm.setChallengeFail(true);
				ship->failAllTask();
			}
			ship->setChallenge(false);
			ship->setChallengeProg(0);
		}
	}

	for (int i = 0; i < ship->getButtons().size(); i++) {
		auto button = ship->getButtons().at(i);

		if (button == nullptr || !button->getIsActive()) {
			continue;
		}

		float diff = donutModel->getAngle() - button->getAngle();
		float shipSize = ship->getSize();
		float a = diff + shipSize / 2;
		diff = a - floor(a / shipSize) * shipSize - shipSize / 2;

		int flag = (abs(diff) < globals::BUTTON_ACTIVE_ANGLE && donutModel->isJumping()) ? 1 : 0;
		ship->flagButton(i, playerID, flag);
		net->flagButton(i, playerID, flag);

		if (flag == 1) {
			if (button->getPair()->isJumpedOn()) {
				CULog("Resolving button");
				ship->resolveButton(i);
				net->resolveButton(i);
			}
		}
	}

	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void GameMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) { sgRoot.render(batch); }
