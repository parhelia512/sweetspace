#ifndef __MAIN_MENU_MODE_H__
#define __MAIN_MENU_MODE_H__
#include <cugl/cugl.h>

#include <vector>

#include "InputController.h"
#include "MagicInternetBox.h"
#include "ShipModel.h"

/**
 * The primary controller for the main menu / matchmaking mode
 */
class MainMenuMode : public cugl::Scene {
   private:
#pragma region Controllers
	/** Controller for abstracting out input across multiple platforms */
	std::shared_ptr<InputController> input;
	/** Networking controller*/
	std::shared_ptr<MagicInternetBox> net;
#pragma endregion

	/** An extra thread used to connect to the server from the host, in case the server is down. */
	std::unique_ptr<std::thread> startHostThread;

	/** Helper object to make the buttons go up and down */
	ButtonManager buttonManager;

	/** The Screen's Height. */
	float screenHeight;

#pragma region State Variables
	/** True if game is ready to start */
	bool gameReady;

	/** The room ID the client is currently entering */
	std::vector<unsigned int> clientEnteredRoom;

	/** Current room ID */
	std::string roomID;

	/** The current frame of a transition; -1 if not transitioning */
	int transitionFrame;

	/** An enum with the current state of the matchmaking mode */
	enum MatchState {
		/** Empty state; used for transitions only; the main state should NEVER be NA */
		NA = -1,
		/** Main menu splash screen */
		StartScreen,
		/** Hosting a game; waiting on ship ID */
		HostScreenWait,
		/** Hosting a game; ship ID received */
		HostScreen,
		/** Host; level select screen */
		HostLevelSelect,
		/** Joining a game; waiting on ship ID */
		ClientScreen,
		/** Joining a game; connected */
		ClientScreenDone,
		/** Matchmaking complete */
		Done
	};

	/** The current state */
	MatchState currState;
	/** The state we are transitioning into, or NA (-1) if not transitioning */
	MatchState transitionState;
#pragma endregion

#pragma region Scene Graph Nodes
	/** Button to create host */
	std::shared_ptr<cugl::Button> hostBtn;
	/** Button to create client */
	std::shared_ptr<cugl::Button> clientBtn;

	/** The node containing all UI for the starting splash screen */
	std::shared_ptr<cugl::Node> mainScreen;
	/** The node containing all UI for the host screen */
	std::shared_ptr<cugl::Node> hostScreen;
	/** The node containing all UI for the client screen */
	std::shared_ptr<cugl::Node> clientScreen;

	/** Connection loading message */
	std::shared_ptr<cugl::Label> connScreen;

	/** Temporary level select */
	std::shared_ptr<cugl::Node> levelSelect;
	/** Temporary level select easy mode */
	std::shared_ptr<cugl::Button> easyBtn;
	/** Temporary level select med mode */
	std::shared_ptr<cugl::Button> medBtn;
	/** Temporary level select hard mode */
	std::shared_ptr<cugl::Button> hardBtn;

	/** Label for room ID (host) */
	std::shared_ptr<cugl::Label> hostLabel;
	/** Button to begin game (host) */
	std::shared_ptr<cugl::Button> hostBeginBtn;
	/** The node containing the player count needle for the host */
	std::shared_ptr<cugl::Node> hostNeedle;

	/** Label for room ID (client) */
	std::shared_ptr<cugl::Label> clientLabel;
	/** Button to confirm room ID (client) */
	std::shared_ptr<cugl::Button> clientJoinBtn;
	/** Vector of 0-9 buttons used to enter room ID (client) */
	std::vector<std::shared_ptr<cugl::Button>> clientRoomBtns;
	/** Clear button from client */
	std::shared_ptr<cugl::Button> clientClearBtn;

#pragma endregion

	/**
	 * Update the client room display using the contents of {@link clientEnteredRoom}
	 */
	void updateClientLabel();

	/**
	 * Query mib and update the room ID for the host accordingly
	 */
	void setRoomID();

	/**
	 * Animate a transition between states.
	 *
	 * PRECONDITION: transitionState != NA
	 */
	void processTransition();

	/**
	 * Process state updates that happene each frame
	 */
	void processUpdate();

	/**
	 * Update button states and handle when buttons are clicked.
	 */
	void processButtons();

   public:
#pragma region Instantiation Logic
	/**
	 * Creates a new game mode with the default values.
	 *
	 * This constructor does not allocate any objects or start the game.
	 * This allows us to use the object without a heap pointer.
	 */
	MainMenuMode()
		: Scene(),
		  net(nullptr),
		  startHostThread(nullptr),
		  screenHeight(0),
		  gameReady(false),
		  transitionFrame(-1),
		  currState(StartScreen),
		  transitionState(NA) {}

	/**
	 * Disposes of all (non-static) resources allocated to this mode.
	 */
	~MainMenuMode() { dispose(); }

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
	bool init(const std::shared_ptr<cugl::AssetManager>& assets);

#pragma endregion

	/**
	 * The method called to update the game mode.
	 *
	 * This method contains any gameplay code that is not an OpenGL call.
	 *
	 * @param timestep  The amount of time (in seconds) since the last frame
	 */
	void update(float timestep) override;

	/**
	 * Draws the game.
	 *
	 * @param batch The sprite batch.
	 */
	void draw(const std::shared_ptr<cugl::SpriteBatch>& batch);

	/**
	 * Checks if game is ready to start
	 *
	 * @return True if game is ready to start, false otherwise
	 */
	bool isGameReady() { return gameReady; }
};

#endif /* __MAIN_MENU_MODE_H__ */
