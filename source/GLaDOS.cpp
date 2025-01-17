﻿#include "GLaDOS.h"

#include <vector>

using namespace cugl;
using namespace std;

/** Time to wait until sending another stabilizer, in tutorial. */
constexpr float STABILIZER_TIMEOUT = 10.0f;

/** Time to wait until sending the first stabilizer, in tutorial. */
constexpr float STABILIZER_START = 2.0f;

/** Time to wait until sending another stabilizer, in tutorial. */
constexpr int MAX_ATTEMPTS = 120;
#pragma mark -
#pragma mark GM
/**
 * Creates a new GM controller.
 *
 * This constructor does NOT do any initialzation.  It simply allocates the
 * object. This makes it safe to use this class without a pointer.
 */
GLaDOS::GLaDOS()
	: active(false),
	  // NOLINTNEXTLINE This ain't the NSA; we don't need better security than this
	  rand(static_cast<unsigned int>(time(nullptr))),
	  mib(MagicInternetBox::getInstance()),
	  maxEvents(0),
	  levelNum(0),
	  customEventCtr(0),
	  sections(0),
	  maxDoors(0),
	  maxButtons(0),
	  stabilizerStart(0) {}

/**
 * Deactivates this input controller, releasing all listeners.
 *
 * This method will not dispose of the input controller. It can be reused
 * once it is reinitialized.
 */
void GLaDOS::dispose() {
	if (active) {
		active = false;
	}
}

/**
 * Initializes the GM
 *
 * This method works like a proper constructor, initializing the GM
 * controller and allocating memory.
 *
 * @return true if the controller was initialized successfully
 */
bool GLaDOS::init(const std::shared_ptr<ShipModel>& ship,
				  const std::shared_ptr<LevelModel>& level) {
	const bool success = true;
	this->ship = ship;
	levelNum = mib.getLevelNum().value();
	maxEvents = static_cast<int>(ship->getBreaches().size());
	maxDoors = static_cast<int>(ship->getDoors().size());
	maxButtons = static_cast<int>(ship->getButtons().size());
	blocks = level->getBlocks();
	events = level->getEvents();
	readyQueue.clear();
	std::queue<int> empty1;
	std::queue<int> empty2;
	std::queue<int> empty3;
	std::swap(breachFree, empty1);
	std::swap(doorFree, empty2);
	std::swap(buttonFree, empty3);

	for (int i = 0; i < maxEvents; i++) {
		breachFree.push(i);
	}
	for (int i = 0; i < maxDoors; i++) {
		doorFree.push(i);
	}
	for (int i = 0; i < maxButtons; i++) {
		buttonFree.push(i);
	}

	active = success;
	return success;
}

/**
 * Initializes the GM for the tutorial levels
 *
 * This method works like a proper constructor, initializing the GM
 * controller and allocating memory.
 *
 * @return true if the controller was initialized successfully
 */
bool GLaDOS::init(const std::shared_ptr<ShipModel>& ship, const int levelNum) {
	const bool success = true;
	readyQueue.clear();
	this->ship = ship;
	this->levelNum = levelNum;
	CULog("Starting level %d", levelNum);
	maxEvents = tutorial::MAX_BREACH.at(levelNum) * mib.getNumPlayers() / globals::MIN_PLAYERS;
	maxDoors = tutorial::MAX_DOOR.at(levelNum) * mib.getNumPlayers() / globals::MIN_PLAYERS;
	maxButtons = tutorial::MAX_BUTTON.at(levelNum) * mib.getNumPlayers() / globals::MIN_PLAYERS;
	const int unop =
		static_cast<int>(tutorial::SECTIONED.at(levelNum)) * static_cast<int>(mib.getNumPlayers());
	sections = unop;
	customEventCtr = static_cast<int>(tutorial::CUSTOM_EVENTS.at(levelNum));
	const float size = static_cast<float>(tutorial::SIZE_PER.at(levelNum)) *
					   static_cast<float>(mib.getNumPlayers());
	ship->init(mib.getMaxNumPlayers(), maxEvents, maxDoors, mib.getPlayerID().value(), size,
			   tutorial::HEALTH.at(levelNum), maxButtons, unop);
	ship->setTimeless(true);
	ship->initTimer(1);
	ship->setLevelNum(levelNum);
	std::queue<int> empty1;
	std::queue<int> empty2;
	std::queue<int> empty3;
	std::swap(breachFree, empty1);
	std::swap(doorFree, empty2);
	std::swap(buttonFree, empty3);
	for (int i = 0; i < maxEvents; i++) {
		breachFree.push(i);
	}
	for (int i = 0; i < maxDoors; i++) {
		doorFree.push(i);
	}
	for (int i = 0; i < maxButtons; i++) {
		buttonFree.push(i);
	}

	active = success;
	if (unop > 0 || levelNum == tutorial::DOOR_LEVEL) {
		ship->separateDonuts();
	}
	for (int i = 0; i < unop; i++) {
		const float angle = size / (static_cast<float>(unop) * 2) +
							(size * static_cast<float>(i)) / static_cast<float>(unop);
		ship->createUnopenable(angle, i);
	}
	switch (levelNum) {
		case tutorial::DOOR_LEVEL:
			for (int i = 0; i < maxDoors; i++) {
				const float angle = size / (static_cast<float>(maxDoors) * 2) +
									(size * static_cast<float>(i)) / static_cast<float>(maxDoors);
				const int j = doorFree.front();
				doorFree.pop();
				ship->createDoor(angle, j);
			}
			break;
		case tutorial::BUTTON_LEVEL:
			for (int i = 0; i < unop; i++) {
				const float angle = size / (static_cast<float>(unop) * 2) +
									(size * static_cast<float>(i)) / static_cast<float>(unop);
				// Find usable button IDs
				const int k = buttonFree.front();
				buttonFree.pop();
				const int j = buttonFree.front();
				buttonFree.pop();

				// Dispatch challenge creation
				ship->createButton(angle + tutorial::BUTTON_PADDING, k,
								   angle - tutorial::BUTTON_PADDING, j);
			}
			break;
		case tutorial::STABILIZER_LEVEL:
			stabilizerStart = -STABILIZER_TIMEOUT;
			break;
		default:
			break;
	}
	return success;
}

/**
 * Places an object in the game. Requires that enough resources are present.
 *
 * @param obj the object to place
 * @param zeroAngle the angle corresponding to the relative angle zero
 * @param ids a vector of relative ids, scrambled by the caller
 */
void GLaDOS::placeObject(BuildingBlockModel::Object obj, float zeroAngle, vector<int> ids) {
	const int p =
		obj.player == -1 ? static_cast<int>(rand() % ship->getDonuts().size()) : ids.at(obj.player);
	placeObject(obj, zeroAngle, p);
}

/**
 * Places an object in the game. Requires that enough resources are present.
 *
 * @param obj the object to place
 * @param zeroAngle the angle corresponding to the relative angle zero
 * @param p the id to use for the player
 */
void GLaDOS::placeObject(BuildingBlockModel::Object obj, float zeroAngle, int p) { // NOLINT
	int i = 0;
	float objAngle = static_cast<float>(obj.angle) + zeroAngle;
	if (objAngle < 0) {
		objAngle += ship->getSize();
	} else if (objAngle >= ship->getSize()) {
		objAngle -= ship->getSize();
	}
	switch (obj.type) {
		case BuildingBlockModel::Breach:
			i = breachFree.front();
			breachFree.pop();
			ship->createBreach(objAngle, p, i);
			mib.createBreach(objAngle, p, i);
			break;
		case BuildingBlockModel::Door:
			i = doorFree.front();
			doorFree.pop();
			ship->createDoor(objAngle, i);
			mib.createDualTask(objAngle, i);
			break;
		case BuildingBlockModel::Button: {
			// Roll for pair's angle
			const float origAngle = objAngle;
			float pairAngle = 0;
			int attempts = 0;
			bool goodAngle = false;
			do {
				pairAngle = static_cast<float>(rand() % static_cast<int>(ship->getSize()));
				goodAngle = ship->getAngleDifference(pairAngle, origAngle) >= globals::BUTTON_WIDTH;
				for (unsigned int k = 0; goodAngle && k < ship->getBreaches().size(); k++) {
					const float breachAngle = ship->getBreaches()[k]->getAngle();
					const float diff = ship->getAngleDifference(breachAngle, pairAngle);
					if (ship->getBreaches()[k]->getIsActive() &&
						diff < globals::BUTTON_ACTIVE_ANGLE) {
						goodAngle = false;
						break;
					}
				}

				for (unsigned int k = 0; goodAngle && k < ship->getDoors().size(); k++) {
					const float doorAngle = ship->getDoors()[k]->getAngle();
					const float diff = ship->getAngleDifference(doorAngle, pairAngle);
					if (ship->getDoors()[k]->getIsActive() && diff < globals::BUTTON_WIDTH) {
						goodAngle = false;
						break;
					}
				}
				for (unsigned int k = 0; goodAngle && k < ship->getButtons().size(); k++) {
					const float buttonAngle = ship->getButtons()[k]->getAngle();
					const float diff = ship->getAngleDifference(buttonAngle, pairAngle);
					if (ship->getButtons()[k]->getIsActive() && diff < globals::BUTTON_WIDTH) {
						goodAngle = false;
						break;
					}
				}
				attempts++;
			} while (attempts < MAX_ATTEMPTS && !goodAngle);
			if (goodAngle) {
				placeButtons(origAngle, pairAngle);
			}
			break;
		}
		case BuildingBlockModel::Roll: {
			auto& stabilizer = ship->getStabilizer();
			if (stabilizer.getIsActive()) {
				break;
			}
			if (p != mib.getPlayerID() && ship->getDonuts().at(p)->getIsActive()) {
				mib.createAllTask(p);
			} else {
				stabilizer.startChallenge(ship->timePassed());
			}
			break;
		}
	}
}

void GLaDOS::placeButtons(float angle1, float angle2) {
	// Find usable button IDs
	const int i = buttonFree.front();
	buttonFree.pop();
	const int j = buttonFree.front();
	buttonFree.pop();

	// Dispatch challenge creation
	ship->createButton(angle1, i, angle2, j);
	mib.createButtonTask(angle1, i, angle2, j);
}

/**
 * Processes the GM.
 *
 * This method is used to run the GM for generating and managing current ship events
 */
void GLaDOS::update(float dt) { // NOLINT

	// Check if this is the host
	if (mib.getPlayerID() != 0) {
		CULogError("WARNING: GLaDOS called from non-host");
		return;
	}

	// ============================================
	//  BELOW THIS LINE, ALL ACTIONS ARE HOST-ONLY
	// ============================================

	// Removing breaches that have 0 health left
	std::queue<int>().swap(breachFree);
	for (int i = 0; i < maxEvents; i++) {
		const std::shared_ptr<BreachModel>& breach = ship->getBreaches().at(i);
		if (breach == nullptr) {
			continue;
		}

		// check if the assigned player is inactive
		if (!ship->getDonuts().at(breach->getPlayer())->getIsActive()) {
			while (breach->getHealth() > 0) {
				breach->decHealth(1);
				mib.resolveBreach(i);
			}
		}

		if (!breach->getIsActive()) {
			breachFree.push(i);
		}
	}

	std::queue<int>().swap(doorFree);
	for (int i = 0; i < maxDoors; i++) {
		const std::shared_ptr<DoorModel> door = ship->getDoors().at(i);
		if (door == nullptr) {
			continue;
		}
		if (!door->getIsActive()) {
			doorFree.push(i);
		}
	}

	std::queue<int>().swap(buttonFree);
	for (int i = 0; i < maxButtons; i++) {
		auto& btn = ship->getButtons().at(i);
		if (btn == nullptr) {
			continue;
		}
		if (!btn->getIsActive()) {
			buttonFree.push(i);
		}
	}

	if (levelNum < globals::NUM_TUTORIAL_LEVELS &&
		std::find(std::begin(tutorial::REAL_LEVELS), std::end(tutorial::REAL_LEVELS),
				  mib.getLevelNum()) == std::end(tutorial::REAL_LEVELS)) {
		tutorialLevels(dt);
		return;
	}

	for (int i = 0; i < events.size(); i++) {
		const std::shared_ptr<EventModel> event = events.at(i);
		int spawnRate =
			static_cast<int>(globals::MIN_PLAYERS /
							 (event->getProbability() * static_cast<float>(mib.getNumPlayers())));
		if (spawnRate < 1) {
			spawnRate = 1;
		}
		if (event->isActive(static_cast<int>(ship->timePassedIgnoringFreeze())) &&
			rand() % spawnRate <= 1) {
			// ready up the event
			readyQueue.push_back(event);
			if (event->isOneTime()) {
				// If it's a one time event, we don't want to add it again next frame
				events.erase(events.begin() + i);
				i--;
			}
		}
	}
	for (int i = 0; i < readyQueue.size(); i++) {
		// assign the relative player ids
		vector<int> ids;
		ids.reserve(ship->getDonuts().size());
		for (int j = 0; j < ship->getDonuts().size(); j++) {
			ids.push_back(j);
		}
		// NOLINTNEXTLINE It's fine that this shuffle algorithm isn't perfect
		random_shuffle(ids.begin(), ids.end());
		const shared_ptr<EventModel> event = readyQueue.at(i);
		const shared_ptr<BuildingBlockModel> block = blocks.at(event->getBlock());
		const vector<BuildingBlockModel::Object> objects = block->getObjects();
		const int breachesNeeded = block->getBreachesNeeded();
		const int doorsNeeded = block->getDoorsNeeded();
		const int buttonsNeeded = block->getButtonsNeeded();

		// If we don't have enough resources for this event, they're probably already fucked
		if (doorsNeeded > doorFree.size() || breachesNeeded > breachFree.size() ||
			buttonsNeeded > buttonFree.size()) {
			readyQueue.erase(readyQueue.begin() + i);
			i--;
			continue;
		}
		// the ids we actually use
		vector<int> neededIds;
		for (const auto& object : objects) {
			const int id = object.player;
			if (id != -1) {
				ids.push_back(id);
			}
		}
		float angle = 0;
		int padding = 0;
		switch (block->getType()) {
			case BuildingBlockModel::MinDist: {
				angle = static_cast<float>(rand() % static_cast<int>(ship->getSize()));
				padding = block->getDistance();
				break;
			}
			case BuildingBlockModel::SpecificPlayer: {
				const int id = ids.at(block->getPlayer());
				angle =
					ship->getDonuts().at(id)->getAngle() + static_cast<float>(block->getDistance());
				break;
			}
			case BuildingBlockModel::Random: {
				angle = static_cast<float>(rand() % static_cast<int>(ship->getSize()));
				break;
			}
		}
		bool goodAngle = true;
		for (int j = 0; goodAngle && j < ship->getDonuts().size(); j++) {
			const float diff = ship->getAngleDifference(ship->getDonuts().at(j)->getAngle(), angle);
			const float dist = find(neededIds.begin(), neededIds.end(), j) != neededIds.end()
								   ? 0
								   : static_cast<float>(padding);
			if (diff < dist + static_cast<float>(block->getRange()) / 2) {
				goodAngle = false;
				break;
			}
		}
		// Make sure it's not too close to other breaches
		for (unsigned int k = 0; goodAngle && k < ship->getBreaches().size(); k++) {
			const float breachAngle = ship->getBreaches()[k]->getAngle();
			const float diff = ship->getAngleDifference(breachAngle, angle);
			if (ship->getBreaches()[k]->getIsActive() &&
				diff < static_cast<float>(block->getRange()) / 2) {
				goodAngle = false;
				break;
			}
		}

		for (unsigned int k = 0; goodAngle && k < ship->getDoors().size(); k++) {
			const float doorAngle = ship->getDoors()[k]->getAngle();
			const float diff = ship->getAngleDifference(doorAngle, angle);
			if (ship->getDoors()[k]->getIsActive() &&
				diff < static_cast<float>(block->getRange()) / 2) {
				goodAngle = false;
				break;
			}
		}

		for (unsigned int k = 0; goodAngle && k < ship->getButtons().size(); k++) {
			const float buttonAngle = ship->getButtons()[k]->getAngle();
			const float diff = ship->getAngleDifference(buttonAngle, angle);
			if (ship->getButtons()[k]->getIsActive() &&
				diff < static_cast<float>(block->getRange()) / 2) {
				goodAngle = false;
				break;
			}
		}
		if (!goodAngle) {
			continue;
		}
		// set angle to where zero is
		angle =
			angle - static_cast<float>(block->getRange()) / 2 - static_cast<float>(block->getMin());

		if (angle < 0) {
			angle += ship->getSize();
		} else if (angle >= ship->getSize()) {
			angle -= ship->getSize();
		}
		for (const auto& object : objects) {
			placeObject(object, angle, ids);
		}
		readyQueue.erase(readyQueue.begin() + i);
		break;
	}
}

void GLaDOS::tutorialLevels(float /*dt*/) {
	switch (levelNum) {
		case tutorial::BREACH_LEVEL:
			if (ship->timePassed() >= tutorial::B_L_PART1 && customEventCtr == 2) {
				const float actualWidth = ship->getSize() / static_cast<float>(sections);
				for (int i = 0; i < ship->getDonuts().size(); i++) {
					const float mid = actualWidth * static_cast<float>(i);
					float suggestedAngle1 = mid + tutorial::B_L_LOC1;
					float suggestedAngle2 = mid + tutorial::B_L_LOC2;
					if (suggestedAngle1 < 0) {
						suggestedAngle1 += ship->getSize();
					}
					if (suggestedAngle2 < 0) {
						suggestedAngle2 += ship->getSize();
					}
					const float diff1 = ship->getAngleDifference(
						suggestedAngle1, ship->getDonuts().at(i)->getAngle());
					const float diff2 = ship->getAngleDifference(
						suggestedAngle2, ship->getDonuts().at(i)->getAngle());
					if (diff1 > diff2) {
						placeObject({BuildingBlockModel::Breach, 0, -1}, suggestedAngle1,
									(i + 1) % static_cast<int>(ship->getDonuts().size()));
					} else {
						placeObject({BuildingBlockModel::Breach, 0, -1}, suggestedAngle2,
									(i + 1) % static_cast<int>(ship->getDonuts().size()));
					}
				}
				customEventCtr--;
			} else if (ship->timePassed() >= tutorial::B_L_PART2 && customEventCtr == 1) {
				const float actualWidth = ship->getSize() / static_cast<float>(sections);
				for (int i = 0; i < ship->getDonuts().size(); i++) {
					const float mid = actualWidth * static_cast<float>(i);
					float suggestedAngle1 = mid + tutorial::B_L_LOC3;
					float suggestedAngle2 = mid + tutorial::B_L_LOC4;
					if (suggestedAngle1 >= ship->getSize()) {
						suggestedAngle1 -= ship->getSize();
					}
					if (suggestedAngle2 >= ship->getSize()) {
						suggestedAngle2 -= ship->getSize();
					}
					const float diff1 = ship->getAngleDifference(
						suggestedAngle1, ship->getDonuts().at(i)->getAngle());
					const float diff2 = ship->getAngleDifference(
						suggestedAngle2, ship->getDonuts().at(i)->getAngle());
					if (diff1 > diff2) {
						placeObject({BuildingBlockModel::Breach, 0, -1}, suggestedAngle1, i);
					} else {
						placeObject({BuildingBlockModel::Breach, 0, -1}, suggestedAngle2, i);
					}
				}
				customEventCtr--;
			} else if (customEventCtr <= 0) {
				// Check if all breaches that can be resolved are resolved.
				if (ship->getBreaches().size() - breachFree.size() == mib.getNumPlayers()) {
					ship->setTimeless(false);
					mib.forceWinLevel();
					ship->initTimer(0);
				}
			}
			break;
		case tutorial::DOOR_LEVEL:
			if (ship->getDoors().size() - doorFree.size() == 0) {
				ship->setTimeless(false);
				mib.forceWinLevel();
				ship->initTimer(0);
				break;
			}
			break;
		case tutorial::BUTTON_LEVEL:
			if (ship->getButtons().size() - buttonFree.size() == 0) {
				ship->setTimeless(false);
				mib.forceWinLevel();
				ship->initTimer(0);
				break;
			}
			break;
		case tutorial::STABILIZER_LEVEL: {
			if (ship->timePassed() < STABILIZER_START) {
				break;
			}
			if (customEventCtr >= mib.getMaxNumPlayers()) {
				customEventCtr = static_cast<int>(mib.getMaxNumPlayers()) - 1;
			}
			// Don't ask inactive donuts to do anything
			// Player 0 will never be inactive since this is player 0
			while (customEventCtr > 0 && !ship->getDonuts().at(customEventCtr)->getIsActive()) {
				customEventCtr--;
			}

			// If stabilizer was successfully completed
			if (ship->stabilizerTutorial) {
				CULog("GLaDOS TUTORIAL: Got stabilizer winner");
				ship->stabilizerTutorial = false;
				customEventCtr--;
				stabilizerStart = -STABILIZER_TIMEOUT;
				// Decrement the player ID, and guarantee that the timeout is exceeded.
				// Then break out; when we come back through next frame, the for loop above
				// will find the next valid player, and then the block below triggers,
				// sending out the stabilizer task.
				break;
			}

			// If timeout expired, which includes if we lose the stabilizer, if the previous packet
			// got lost, or if we won the previous frame and decremented the player ID
			if (ship->canonicalTimeElapsed - stabilizerStart > STABILIZER_TIMEOUT) {
				CULog(
					"GLaDOS TUTORIAL: Generating stabilizer b/c ship time %f, start time %f, "
					"timeout %f, and counter %d",
					ship->canonicalTimeElapsed, stabilizerStart, STABILIZER_TIMEOUT,
					customEventCtr);
				if (customEventCtr < 0) {
					ship->setTimeless(false);
					mib.forceWinLevel();
					ship->initTimer(0);
					break;
				}

				if (customEventCtr != mib.getPlayerID() &&
					ship->getDonuts().at(customEventCtr)->getIsActive()) {
					mib.createAllTask(customEventCtr);
				} else {
					ship->createAllTask();
				}
				stabilizerStart = ship->canonicalTimeElapsed;
			}

			break;
		}
	}
}
