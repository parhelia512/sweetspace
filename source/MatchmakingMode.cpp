﻿#include "MatchmakingMode.h"

#include <cugl/cugl.h>

#include <iostream>
#include <sstream>

#include "ExternalDonutModel.h"

using namespace cugl;
using namespace std;

#pragma mark -
#pragma mark Constants

/** This is adjusted by screen aspect ratio to get the height */
constexpr unsigned int SCENE_WIDTH = 1024;

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
bool MatchmakingMode::init(const std::shared_ptr<cugl::AssetManager>& assets,
						   std::shared_ptr<MagicInternetBox>& mib) {
	// Initialize the scene to a locked width
	Size dimen = Application::get()->getDisplaySize();
	dimen *= SCENE_WIDTH / dimen.width; // Lock the game to a reasonable resolution
	if (assets == nullptr) {
		return false;
	}
	// Set network controller
	net = mib;
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
	// Neither host nor client
	if (sgRoot.getPlayerId() == -1) {
		int buttonPressed;
		buttonPressed = sgRoot.checkButtons(input.getTapLoc());
		if (buttonPressed == 0) {
			net->initHost();
			sgRoot.setPlayerId(0);
		} else if (buttonPressed == 1) {
			sgRoot.setPlayerId(-2);
		}
	}
	// Client, but needs room and id still
	if (sgRoot.getPlayerId() == -2) {
		// Check if input in TextField is a room and set roomID and set playerID
		// Init client
		string s = sgRoot.getInput(input.getTapLoc());
		if (s != "") {
			sgRoot.setRoomId(s);
			if (net->initClient(s)) {
				sgRoot.setPlayerId(1);
			}
		}
	}
	// Only update network loop if inithost or initclient called
	if (sgRoot.getPlayerId() > -1) {
		net->update();
		sgRoot.setRoomId(net->getRoomID());
		if (net->getPlayerID() > -1) {
			sgRoot.setPlayerId(net->getPlayerID());
		}
		// Check if room is ready for play (Replace with button for play later)
		if (net->getNumPlayers() == 1) {
			gameReady = true;
			net->startGame();
		}
	}
	// Update Scene Graph
	sgRoot.update(timestep);
}

/**
 * Draws the game.
 */
void MatchmakingMode::draw(const std::shared_ptr<cugl::SpriteBatch>& batch) {
	sgRoot.render(batch);
}
