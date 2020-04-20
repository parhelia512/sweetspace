#include "MainMenuMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"
#include "LevelConstants.h"
#include "Tween.h"

using namespace cugl;

/** Number of buttons for room ID entry */
constexpr unsigned int NUM_DIGITS = 10;

#pragma region Animation Constants
/** Duration of a standard transition */
constexpr int TRANSITION_DURATION = 30;
#pragma endregion

#pragma region Initialization Logic
bool MainMenuMode::init(const std::shared_ptr<AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}
	// Set network controller
	net = MagicInternetBox::getInstance();
	input = InputController::getInstance();

	screenHeight = dimen.height;
	// Initialize the scene to a locked width
	if (!Scene::init(dimen)) {
		return false;
	}

	// Acquire the scene built by the asset loader and resize it the scene
	auto scene = assets->get<Node>("matchmaking");
	scene->setContentSize(dimen);
	scene->doLayout(); // Repositions the HUD

#pragma region Scene Graph Components
	hostBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_hostbtn"));
	clientBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_home_btnwrap_clientbtn"));

	mainScreen = assets->get<Node>("matchmaking_home");
	hostScreen = assets->get<Node>("matchmaking_host");
	clientScreen = assets->get<Node>("matchmaking_client");
	connScreen = std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_connscreen"));

	hostLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_host_wrap_plate_room"));
	clientLabel =
		std::dynamic_pointer_cast<Label>(assets->get<Node>("matchmaking_client_wrap_plate_room"));

	hostBeginBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_host_wrap_startbtn"));
	hostNeedle = assets->get<Node>("matchmaking_host_dial_hand");

	clientJoinBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_wrap_joinbtn"));
	clientClearBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_client_buttons_btnclear"));

	levelSelect = assets->get<Node>("matchmaking_levelselect");
	easyBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_levelselect_easybtn"));
	medBtn = std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_levelselect_medbtn"));
	hardBtn =
		std::dynamic_pointer_cast<Button>(assets->get<Node>("matchmaking_levelselect_hardbtn"));

	buttonManager.registerButton(hostBtn);
	buttonManager.registerButton(clientBtn);
	buttonManager.registerButton(hostBeginBtn);
	buttonManager.registerButton(clientJoinBtn);
	buttonManager.registerButton(clientClearBtn);
	buttonManager.registerButton(easyBtn);
	buttonManager.registerButton(medBtn);
	buttonManager.registerButton(hardBtn);
	for (unsigned int i = 0; i < NUM_DIGITS; i++) {
		clientRoomBtns.push_back(std::dynamic_pointer_cast<Button>(
			assets->get<Node>("matchmaking_client_buttons_btn" + std::to_string(i))));
		buttonManager.registerButton(clientRoomBtns[i]);
	}
#pragma endregion

	transitionFrame = -1;

	updateClientLabel();

	addChild(scene);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MainMenuMode::dispose() {
	removeAllChildren();
	hostBtn = nullptr;
	clientBtn = nullptr;
	mainScreen = nullptr;
	hostScreen = nullptr;
	clientScreen = nullptr;
	connScreen = nullptr;
	hostLabel = nullptr;
	hostBeginBtn = nullptr;
	hostNeedle = nullptr;
	clientLabel = nullptr;
	clientJoinBtn = nullptr;
	clientClearBtn = nullptr;
	levelSelect = nullptr;
	easyBtn = nullptr;
	medBtn = nullptr;
	hardBtn = nullptr;
	clientRoomBtns.clear();
}
#pragma endregion

#pragma region Internal Helpers
void MainMenuMode::updateClientLabel() {
	std::vector<char> room;
	for (unsigned int i = 0; i < clientEnteredRoom.size(); i++) {
		room.push_back('0' + clientEnteredRoom[i]);
	}
	for (unsigned int i = clientEnteredRoom.size(); i < globals::ROOM_LENGTH; i++) {
		room.push_back('_');
	}

	std::ostringstream disp;
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		disp << room[i];
		if (i < globals::ROOM_LENGTH - 1) {
			disp << ' ';
		}
	}

	clientLabel->setText(disp.str());
}

void MainMenuMode::setRoomID() {
	if (roomID == net->getRoomID()) {
		return;
	}
	roomID = net->getRoomID();

	if (roomID == "") {
		hostLabel->setText("_ _ _ _ _");
		clientEnteredRoom.clear();
		updateClientLabel();
		return;
	}

	std::ostringstream disp;
	for (unsigned int i = 0; i < globals::ROOM_LENGTH; i++) {
		disp << roomID.at(i);
		if (i < globals::ROOM_LENGTH - 1) {
			disp << ' ';
		}
	}
	hostLabel->setText(disp.str());
}

void MainMenuMode::processTransition() {
	transitionFrame++;
	switch (currState) {
		case StartScreen: {
			if (transitionFrame >= TRANSITION_DURATION) {
				currState = transitionState;
				transitionState = NA;
				transitionFrame = -1;
				mainScreen->setVisible(false);
			} else {
				mainScreen->setColor(
					Tween::fade(Tween::linear(1.0f, 0.0f, transitionFrame, TRANSITION_DURATION)));
				if (transitionState == ClientScreen) {
					clientScreen->setPositionY(
						Tween::easeOut(-screenHeight, 0, transitionFrame, TRANSITION_DURATION));
				}
			}
			break;
		}
		case HostScreenWait: {
			if (transitionState == HostScreen) {
				if (transitionFrame >= TRANSITION_DURATION) {
					currState = HostScreen;
					transitionState = NA;
					transitionFrame = -1;
					hostScreen->setPositionY(0);
				} else {
					hostScreen->setPositionY(
						Tween::easeOut(-screenHeight, 0, transitionFrame, TRANSITION_DURATION));
				}
			}
			break;
		}
		default:
			break;
	}
}

void MainMenuMode::processUpdate() {
	switch (currState) {
		case HostScreenWait: {
			if (net->getRoomID() != "") {
				setRoomID();
				hostScreen->setVisible(true);
				hostScreen->setPositionY(-screenHeight);
				transitionState = HostScreen;
				connScreen->setVisible(false);
			} else {
				connScreen->setVisible(true);
			}
			if (net->matchStatus() == MagicInternetBox::MatchmakingStatus::HostError) {
				connScreen->setText("Error Connecting :(");
			}
			break;
		}
		case HostScreen: {
			float percentage = (float)(net->getNumPlayers() - 1) / (float)globals::MAX_PLAYERS;
			hostNeedle->setAngle(-percentage * globals::TWO_PI);
			break;
		}
		default: {
			break;
		}
	}
}

/**
 * Returns true iff a button was properly tapped (the tap event both started and ended on the
 * button)
 *
 * @param button The button
 * @param tapData The start and end locations provided by the input controller
 */
bool tappedButton(std::shared_ptr<Button> button, std::tuple<Vec2, Vec2> tapData) {
	return button->containsScreen(std::get<0>(tapData)) &&
		   button->containsScreen(std::get<1>(tapData));
}

void MainMenuMode::processButtons() {
	if (currState != ClientScreenDone) {
		buttonManager.process();
	}

	// Do not process inputs if a) nothing was pressed, or b) currently transitioning
	if (!InputController::getInstance()->isTapEndAvailable() || transitionState != NA) {
		return;
	}

	std::tuple<Vec2, Vec2> tapData = InputController::getInstance()->getTapEndLoc();

	switch (currState) {
		case StartScreen: {
			if (tappedButton(hostBtn, tapData)) {
				startHostThread = std::unique_ptr<std::thread>(new std::thread([]() {
					MagicInternetBox::getInstance()->initHost();
					CULog("SEPARATE THREAD FINISHED INIT HOST");
				}));
				transitionState = HostScreenWait;
			} else if (tappedButton(clientBtn, tapData)) {
				transitionState = ClientScreen;
				clientScreen->setPositionY(-screenHeight);
				clientScreen->setVisible(true);
			}
			break;
		}
		case HostScreen: {
			if (tappedButton(hostBeginBtn, tapData)) {
				if (net->getNumPlayers() >= globals::MIN_PLAYERS) {
					currState = HostLevelSelect;
					hostScreen->setVisible(false);
					levelSelect->setVisible(true);
				}
			}
			break;
		}
		case HostLevelSelect: {
			if (tappedButton(easyBtn, tapData)) {
				gameReady = true;
				net->startGame(1);
				return;
			}
			if (tappedButton(medBtn, tapData)) {
				gameReady = true;
				net->startGame(2);
				return;
			}
			if (tappedButton(hardBtn, tapData)) {
				gameReady = true;
				net->startGame(3);
				return;
			}
			break;
		}
		case ClientScreen: {
			if (tappedButton(clientJoinBtn, tapData)) {
				if (clientEnteredRoom.size() != globals::ROOM_LENGTH) {
					break;
				}

				std::ostringstream room;
				for (int i = 0; i < globals::ROOM_LENGTH; i++) {
					room << clientEnteredRoom[i];
				}

				currState = ClientScreenDone;
				clientJoinBtn->setDown(true);
				net->initClient(room.str());

				break;
			}

			for (unsigned int i = 0; i < NUM_DIGITS; i++) {
				if (tappedButton(clientRoomBtns[i], tapData)) {
					if (clientEnteredRoom.size() < globals::ROOM_LENGTH) {
						clientEnteredRoom.push_back(i);
						updateClientLabel();
					}
					break;
				}
			}

			if (tappedButton(clientClearBtn, tapData)) {
				if (clientEnteredRoom.size() > 0) {
					clientEnteredRoom.pop_back();
					clientJoinBtn->setDown(false);
					updateClientLabel();
				}
				break;
			}
		}
		default: {
			break;
		}
	}
}
#pragma endregion

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MainMenuMode::update(float timestep) {
	input->update(timestep);

	if (transitionState != NA) {
		processTransition();
	} else {
		processUpdate();
		processButtons();
	}

	switch (net->matchStatus()) {
		case MagicInternetBox::MatchmakingStatus::ClientRoomInvalid:
		case MagicInternetBox::MatchmakingStatus::ClientRoomFull:
			if (currState == ClientScreenDone) {
				clientEnteredRoom.clear();
				updateClientLabel();
				currState = ClientScreen;
				clientJoinBtn->setDown(false);
			}
			return;
		case MagicInternetBox::MatchmakingStatus::Uninitialized:
		case MagicInternetBox::MatchmakingStatus::HostError:
			return;
		case MagicInternetBox::MatchmakingStatus::GameStart:
			gameReady = true;
			return;
		default:
			net->update();
			break;
	}
}

/**
 * Draws the game.
 */
void MainMenuMode::draw(const std::shared_ptr<SpriteBatch>& batch) { render(batch); }