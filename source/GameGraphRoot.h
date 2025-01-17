﻿#ifndef GAME_GRAPH_ROOT_H
#define GAME_GRAPH_ROOT_H
#include <cugl/cugl.h>

#include <vector>

#include "BreachModel.h"
#include "BreachNode.h"
#include "ButtonManager.h"
#include "ButtonNode.h"
#include "DonutModel.h"
#include "DoorNode.h"
#include "ExternalDonutNode.h"
#include "Globals.h"
#include "HealthNode.h"
#include "InputController.h"
#include "MagicInternetBox.h"
#include "PauseMenu.h"
#include "PlayerDonutNode.h"
#include "ReconnectScreen.h"
#include "ShipModel.h"
#include "ShipSegmentWrap.h"
#include "StabilizerNode.h"
#include "TutorialNode.h"
#include "UnopenableNode.h"
#include "WinScreen.h"

class GameGraphRoot : public cugl::Scene {
   public:
	/** Enum for determining drawing state */
	enum DrawStatus {
		/** Reconnecting */
		Reconnecting = -1,
		/** Normal Gameplay */
		Normal,
		/** Win Screen */
		Win,
		/** Loss Screen */
		Loss,
		/** Game Ended Unexpectedly */
		Ended
	};

	/** Buttons that can be pressed  */
	enum GameButton { None, Restart, NextLevel };

   protected:
	/** The asset manager for this game mode. */
	std::shared_ptr<cugl::AssetManager> assets;
	/** The Screen's Height. */
	float screenHeight;

	/** Helper object to make the buttons go up and down */
	ButtonManager buttonManager;

	/** Seconds in a minute for timer display */
	static constexpr int SEC_IN_MIN = 60;
	static constexpr int TEN_SECONDS = 10;

	// VIEW COMPONENTS
	/** Filmstrip representing the player's animated donut */
	std::shared_ptr<PlayerDonutNode> donutNode;
	/** Label for on-screen coordinate HUD */
	std::shared_ptr<cugl::Label> coordHUD;
	/** Node to hold all of our graphics. Necesary for resolution indepedence. */
	std::shared_ptr<cugl::Node> allSpace;
	/** Background in animation parallax. Stores the field of stars */
	std::shared_ptr<cugl::Node> farSpace;
	/** Foreground in animation parallax. Stores the planets. */
	std::shared_ptr<cugl::Node> nearSpace;
	/** Parent node of all breaches, is child of nearSpace */
	std::shared_ptr<cugl::Node> breachesNode;
	/** Parent node of all breach sparkle nodes, is child of nearSpace */
	std::shared_ptr<cugl::Node> breachSparklesNode;
	/** Parent node of all ship segments, is child of nearSpace */
	std::shared_ptr<ShipSegmentWrap> shipSegsNode;
	/** Parent node of all doors, is child of nearSpace */
	std::shared_ptr<cugl::Node> doorsNode;
	/** Parent node of all unops, is child of nearSpace */
	std::shared_ptr<cugl::Node> unopsNode;
	/** Parent node of all external donuts, is child of nearSpace */
	std::shared_ptr<cugl::Node> externalDonutsNode;
	/** Stabilizer */
	std::shared_ptr<StabilizerNode> stabilizerNode;
	/** Health bar */
	std::shared_ptr<cugl::PolygonNode> healthNode;
	std::shared_ptr<cugl::PolygonNode> healthNodeOverlay;
	std::shared_ptr<cugl::PolygonNode> healthNodeNumbers;
	/** Tutorial */
	std::shared_ptr<cugl::PolygonNode> moveTutorial;
	std::shared_ptr<cugl::PolygonNode> healthTutorial;
	std::shared_ptr<cugl::PolygonNode> rollTutorial;
	std::shared_ptr<cugl::PolygonNode> communicateTutorial;
	std::shared_ptr<cugl::PolygonNode> timerTutorial;
	std::shared_ptr<cugl::PolygonNode> buttLabelTutorial;
	std::shared_ptr<cugl::Node> tutorialNode;
	std::shared_ptr<cugl::PolygonNode> timerBorder;

	// Reconnection Textures
	/** Node to hold all of the Reconnect Overlay.*/
	std::shared_ptr<ReconnectScreen> reconnectScreen;

	// Pause Textures
	/** Pause menu node */
	std::shared_ptr<PauseMenu> pauseMenu;

	// Loss Screen Textures
	/** Node to hold all of the Loss Screen.*/
	std::shared_ptr<cugl::Node> lossScreen;
	/** Button to restart game */
	std::shared_ptr<cugl::Button> restartBtn;
	/** Text to wait for game restart */
	std::shared_ptr<cugl::Label> lostWaitText;

	// Win Screen Textures
	std::shared_ptr<WinScreen> winScreen;

	// DRAWING STATE VARIABLES
	/** The donut's base position. */
	cugl::Vec2 donutPos;

	/** Parent node of all buttons, is child of nearSpace */
	std::shared_ptr<cugl::Node> buttonsNode;
	/** Parent node of all button sparkle nodes, is child of nearSpace */
	std::shared_ptr<cugl::Node> buttonSparklesNode;

	// MODEL INFORMATION
	/** Id of the current client */
	unsigned int playerID;
	/** The ship */
	std::shared_ptr<ShipModel> ship;
	/** Angle of the player donut model from the last frame */
	float prevPlayerAngle;

	/** Current animation frame for ship flashing red */
	int currentHealthWarningFrame;

	// TELEPORTATION ANIMATION
	/** Reference to black image that covers all */
	std::shared_ptr<cugl::PolygonNode> blackoutOverlay;
	/** Current animation frame for stabilizer fail teleportation */
	int currentTeleportationFrame;
	/** Whether stabilizer is failed in last frame */
	bool prevIsStabilizerFail;

	/** Animation constants */
	static constexpr int TELEPORT_FRAMECUTOFF_FIRST = 40;
	static constexpr int TELEPORT_FRAMECUTOFF_SECOND = 120;
	static constexpr int TELEPORT_FRAMECUTOFF_THIRD = 200;

	/**
	 * Returns an informative string for the timer
	 *
	 * This function is for writing the current donut position to the HUD.
	 *
	 * @param coords The current donut coordinates
	 *
	 * @return an informative string for the position
	 */
	std::string timerText();

	/**
	 * The current Drawing status
	 */
	DrawStatus status;

	/** Process Buttons in Special Screens */
	void processButtons();

	/** Whether to go back to main menu */
	bool isBackToMainMenu;

	/** The last pressed button */
	GameButton lastButtonPressed;

   public:
#pragma mark -
#pragma mark Public Consts
	/** Possible colors for player representations */
	const std::vector<string> PLAYER_COLOR{"yellow", "red", "green", "orange", "cyan", "purple"};
	/** Possible colors for breach representations */
	static const std::vector<cugl::Color4> BREACH_COLOR;
	/** Number of possible player colors */
	static constexpr int NUM_COLORS = 6;

#pragma mark -
#pragma mark Constructors
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	GameGraphRoot()
		: screenHeight(0),
		  playerID(0),
		  prevPlayerAngle(0),
		  currentHealthWarningFrame(0),
		  currentTeleportationFrame(0),
		  prevIsStabilizerFail(false),
		  status(Normal),
		  isBackToMainMenu(false),
		  lastButtonPressed(None) {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 *
	 * This method is different from dispose() in that it ALSO shuts off any
	 * static resources, like the input controller.
	 */
	virtual ~GameGraphRoot() { dispose(); }

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	void dispose() override;

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
	bool init(const std::shared_ptr<cugl::AssetManager>& assets,
			  const std::shared_ptr<ShipModel>& ship, unsigned int playerID);

#pragma mark -
#pragma mark Gameplay Handling
	/**
	 * The method called to update the game mode.
	 *
	 * This method contains any gameplay code that is not an OpenGL call.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/**
	 * Resets the status of the game so that we can play again.
	 */
	void reset() override;

	/**
	 * Healper function for setting alpha value of ship health warning
	 */
	void setSegHealthWarning(int alpha);

	/**
	 * Do teleportation animation
	 */
	void doTeleportAnimation();
#pragma mark -
#pragma mark Accessors
	/**
	 * Set Drawing status
	 *
	 * @param status the drawing status of the ship
	 */
	void setStatus(DrawStatus status) { this->status = status; }

	/**
	 * Get Drawing Status
	 *
	 * @return Drawing Status
	 */
	DrawStatus getStatus() { return status; }

	/**
	 * Get whether to go back to the main menu
	 *
	 * @return whether to go back to the main menu
	 */
	bool getIsBackToMainMenu() const { return isBackToMainMenu; }

	/**
	 * Returns the last button pressed, if any, and resets the field so future calls to this method
	 * will return None until another button is pressed.
	 */
	GameButton getAndResetLastButtonPressed() {
		const GameButton ret = lastButtonPressed;
		lastButtonPressed = None;
		return ret;
	}
};
#endif /* GAME_GRAPH_ROOT_H */
