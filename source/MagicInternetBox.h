﻿#ifndef __NETWORK_CONTROLLER_H__
#define __NETWORK_CONTROLLER_H__

#include <cugl/cugl.h>

#include "ShipModel.h"
#include "libraries/easywsclient.hpp"

class MagicInternetBox {
   public:
	/**
	 * Status of whether the game is ready to start
	 */
	enum MatchmakingStatus {
		Uninitialized = -1,
		/** Connecting to server; room ID not assigned yet */
		HostConnecting = 0,
		/** Connected and room ID assigned; waiting for other players */
		HostWaitingOnOthers,
		/** Unknown error as host */
		HostError,
		/** Connecting to server; player ID not assigned yet */
		ClientConnecting = 100,
		/** Connected and player ID assigned; waiting for other players */
		ClientWaitingOnOthers,
		/** Room ID does not exist */
		ClientRoomInvalid,
		/** Room ID is full already */
		ClientRoomFull,
		/** Unknown error as client */
		ClientError,
		/** Game has started */
		GameStart = 200,
		/** Attempting to reconnect to a room after dropping */
		Reconnecting = 500,
		/** Unknown error when reconnecting */
		ReconnectFailure,
		/** Game has ended */
		GameEnded = 900
	};

   private:
	/**
	 * The actual websocket connection
	 */
	easywsclient::WebSocket::pointer ws;

	/**
	 * The current status
	 */
	MatchmakingStatus status;

	/**
	 * The current frame, modulo the network tick rate.
	 */
	unsigned int currFrame;

	/**
	 * ID of the current player, or -1 if unassigned
	 */
	int playerID;

	/**
	 * The ID of the current room, or "" if unassigned
	 */
	std::string roomID;

	/**
	 * Number of connected players
	 */
	unsigned int numPlayers;

	/**
	 * The type of data being sent during a network packet
	 */
	enum NetworkDataType {
		PositionUpdate,
		Jump,
		BreachCreate,
		BreachShrink,
		DualCreate,
		DualResolve,
		AssignedRoom, // Doubles for both creating and created
		JoinRoom,	  // Doubles for both joining and join response
		PlayerJoined
	};

	/**
	 * Initialize the network connection.
	 * Will establish a connection to the server.
	 *
	 * @returns Whether the connection was successfully established
	 */
	bool initConnection();

	/**
	 * Send data over the network as described in the architecture specification.
	 *
	 * Angle field is for the angle, if applicable.
	 * ID field is for the ID of the object being acted on, if applicable.
	 * Remaining data fields should be filled from first applicable data type back in the same order
	 * that arguments are passed to the calling method in this class.
	 *
	 * Any unused fields should be set to -1.
	 *
	 * For example, create dual task passes angle to angle, task id to id, the two players to data1
	 * and data2 respectively, and sets data3 to -1.
	 */
	void sendData(NetworkDataType type, float angle, int id, int data1, int data2, float data3);

   public:
	/**
	 * Create an empty Network Controller instance. Does no initialization.
	 * Call one of the init methods to connect and stuff.
	 */
	MagicInternetBox() {
		ws = nullptr;
		status = Uninitialized;
		currFrame = 0;
		playerID = -1;
		numPlayers = 0;
	};

	/**
	 * Initialize this controller class as a host.
	 * Will establish a connection to the server and create a room.
	 * Use the {@link getRoomID()} method to get the ID of the formed room.
	 *
	 * @returns Whether a connection was successfully established
	 */
	bool initHost();

	/**
	 * Initialize this controller class as a client.
	 * Will establish a connection to the server and attempt to join the specified room.
	 *
	 * @param id The room ID
	 * @returns Whether a connection was successfully established
	 */
	bool initClient(std::string id);

	/**
	 * Reconnect to a game that you lost connection from.
	 * Will attempt to rejoin the room. Query {@link matchStatus()} over the next few frames to see
	 * the progress. If {@code GameEnded} is returned, then the room does not have a valid game
	 * going at this time.
	 *
	 * @param id The room ID
	 * @returns Whether a connection was successfully established
	 */
	bool reconnect(std::string id);

	/**
	 * Query the current matchmaking status
	 */
	MatchmakingStatus matchStatus();

	/**
	 * Disconnect from the room and cleanup this object.
	 */
	void leaveRoom();

	/**
	 * Returns the room ID this controller is connected to.
	 * Is the empty string if this controller is uninitialized.
	 */
	std::string getRoomID();

	/**
	 * Returns the current player ID, or -1 if uninitialized.
	 * 0 is the host player.
	 */
	int getPlayerID();

	/**
	 * Returns the number of connected players, or -1 if uninitialized.
	 */
	unsigned int getNumPlayers();

	/**
	 * Start the game with the current number of players.
	 * Should only be called when the matchmaking status is waiting on others
	 */
	void startGame();

	/**
	 * Update method called every frame during matchmaking phase.
	 * Should be called as long as {@link matchStatus()} is not returning {@code GameStart} and
	 * should not be called after.
	 */
	void update();

	/**
	 * Update method called every frame during gameplay.
	 * This controller will batch and handle network communication as long as this method is called.
	 */
	void update(std::shared_ptr<ShipModel> state);

	/**
	 * Inform other players that a new breach has been created.
	 *
	 * @param angle The angle of the ship to spawn the breach
	 * @param player The player ID that can resolve this breach
	 * @param id The breach ID used to identify this breach in the future
	 */
	void createBreach(float angle, int player, int id);

	/**
	 * Inform other players that a breach has been reseolved
	 *
	 * @param id The breach ID
	 */
	void resolveBreach(int id);

	/**
	 * Inform other players that a task requiring two players has been created
	 * (eg: locked doors from nondigital)
	 *
	 * @param angle The angle of the ship to spawn the task
	 * @param player1 The player ID of one player that can resolve this task or -1 for anyone
	 * @param player2 The player ID of the second player that can resolve this task or -1 for anyone
	 * @param id The dual-task ID used to identify this task in the future
	 */
	void createDualTask(float angle, int player1, int player2, int id);

	/**
	 * Inform other players that at least one of the two required players has
	 * done their part to resolve a dual-task.
	 * This method does not keep track of whether one or both players are
	 * resolving this task. It merely broadcasts to all other players that
	 * one more person is now on this task; when both players are here, it
	 * is the responsibility of the receivers of this message to resolve the task.
	 *
	 * @param id The dual-task ID
	 * @param player The player ID who is activating the door
	 * @param flag Whether the player is on or off the door (1 or 0)
	 */
	void flagDualTask(int id, int player, int flag);

	/**
	 * Inform other players that a player has initiated a jump.
	 *
	 * @param player The player ID who is jumping
	 */
	void jump(int player);
};

#endif /* __NETWORK_CONTROLLER_H__ */