﻿#ifndef GAME_MODE_H
#define GAME_MODE_H
#include <cugl/cugl.h>

#include <tl/optional.hpp>
#include <vector>

#include "GLaDOS.h"
#include "GameGraphRoot.h"
#include "InputController.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"
#include "SoundEffectController.h"

/**
 * The primary gameplay controller.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.
 */
class GameMode {
   private:
	// CONTROLLERS
	/** Controller for abstracting out input across multiple platforms */
	std::shared_ptr<InputController> input;
	/** Controller for abstracting out sound effects */
	std::shared_ptr<SoundEffectController> soundEffects;
	/** Controller for GM */
	tl::optional<GLaDOS> gm;
	/** Networking controller*/
	MagicInternetBox& net;

	// VIEW
	/** Scenegraph root node */
	GameGraphRoot sgRoot;

	// MODEL
	/** A reference to the player donut */
	std::shared_ptr<DonutModel> donutModel;
	/** The Ship model */
	std::shared_ptr<ShipModel> ship;

	/** Whether to go back to main menu */
	bool isBackToMainMenu;

#pragma region Update Helpers
	/** Process inputs for the player donut */
	void applyInputsToPlayerDonut();

	/**
	 * Handle changes in connection status.
	 *
	 * @param timestep The amount of time since the last frame
	 *
	 * @return false iff should skip this update frame
	 */
	bool connectionUpdate(float timestep);

	/** Handle loss. Returns true if loss. */
	bool lossCheck();
	/** Handle win. Returns true if won. */
	bool winCheck();
#pragma endregion

   public:
#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	GameMode()
		: input(nullptr),
		  soundEffects(nullptr),
		  net(MagicInternetBox::getInstance()),
		  isBackToMainMenu(false) {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	virtual ~GameMode() { dispose(); }

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
	 *
	 * @return true if the controller is initialized properly, false otherwise.
	 */
	bool init(const std::shared_ptr<cugl::AssetManager>& assets);

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

#pragma mark -
#pragma mark Accessors

	/**
	 * Get whether to go back to the main menu
	 *
	 * @return whether to go back to the main menu
	 */
	bool getIsBackToMainMenu() const { return isBackToMainMenu; }
};

#endif /* GAME_MODE_H */
