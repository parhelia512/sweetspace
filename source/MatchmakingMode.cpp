﻿#include "MatchmakingMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"
#include "Globals.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Constants

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
bool MatchmakingMode::init(const std::shared_ptr<cugl::AssetManager>& assets) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= globals::SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}
	// Set network controller
	net = MagicInternetBox::getInstance();
	input.init();

	sgRoot.init(assets);

	return true;
}

/**
 * Disposes of all (non-static) resources allocated to this mode.
 */
void MatchmakingMode::dispose() {
	input.dispose();
	sgRoot.dispose();
}

#pragma mark -
#pragma mark Gameplay Handling

/**
 * Resets the status of the game so that we can play again.
 */
void MatchmakingMode::reset() { input.clear(); }

/**
 * The method called to update the game mode.
 *
 * This method contains any gameplay code that is not an OpenGL call.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void MatchmakingMode::update(float timestep) {
	input.update(timestep);
	// Update Scene Graph
	sgRoot.update(timestep);

	switch (sgRoot.checkButtons(input.getTapLoc())) {
		case MatchmakingGraphRoot::StartHost: {
			net->initHost();
			break;
		}
		case MatchmakingGraphRoot::ClientConnect: {
			net->initClient(sgRoot.getRoomID());
		}
		default:
			break;
	}

	if (sgRoot.isConnected()) {
		net->update();
		sgRoot.setRoomID(net->getRoomID());
		// Check if room is ready for play (Replace with button for play later)
		if (net->getNumPlayers() == globals::NUM_PLAYERS) {
			gameReady = true;
			net->startGame();
		}
	}
}

/**
 * Draws the game.
 */
void MatchmakingMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
	sgRoot.render(batch);
}
