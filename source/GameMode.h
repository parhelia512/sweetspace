﻿#ifndef __GAME_MODE_H__
#define __GAME_MODE_H__
#include <cugl/cugl.h>

#include <vector>

#include "GLaDOS.h"
#include "GameGraphRoot.h"
#include "InputController.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

/**
 * The primary gameplay controller.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.
 */
class GameMode {
   protected:
	// CONTROLLERS
	/** Controller for abstracting out input across multiple platforms */
	std::shared_ptr<InputController> input;
	/** Controller for GM */
	GLaDOS gm;
	/** Networking controller*/
	std::shared_ptr<MagicInternetBox> net;

	// VIEW
	/** Scenegraph root node */
	GameGraphRoot sgRoot;

	// MODEL
	/** A reference to the player donut */
	std::shared_ptr<DonutModel> donutModel;
	/** The Ship model */
	std::shared_ptr<ShipModel> ship;

	/** Local record of Player ID */
	int playerID;
	/** Local record of Room ID */
	std::string roomId;

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	GameMode() : input(nullptr), net(nullptr), playerID(-1), roomId("") {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	~GameMode() { dispose(); }

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	void dispose();

	/**
	 * Initializes the controller contents, and starts the game
	 *
	 * The constructor does not allocate any objects or memory.  This allows
	 * us to have a non-pointer reference to this controller, reducing our
	 * memory allocation.  Instead, allocation happens in this method.
	 *
	 * @param assets    The (loaded) assets for this game mode
	 * @param levelName The name of the level to load
	 *
	 * @return true if the controller is initialized properly, false otherwise.
	 */
	bool init(const std::shared_ptr<cugl::AssetManager>& assets, const char* levelName);

#pragma mark -
#pragma mark Gameplay Handling
	/**
	 * The method called to update the game mode.
	 *
	 * This method contains any gameplay code that is not an OpenGL call.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep);

	/**
	 * Draws the game.
	 */
	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);
};

#endif /* __GAME_MODE_H__ */
