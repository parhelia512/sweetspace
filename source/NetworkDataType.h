#ifndef NETWORK_DATA_TYPE_H
#define NETWORK_DATA_TYPE_H

/**
 * The type of data being sent during a network packet
 */
enum NetworkDataType {
	// Gameplay messages
	PositionUpdate = 0,
	Jump,
	BreachCreate,
	BreachShrink,
	DualCreate,
	DualResolve,
	ButtonCreate,
	ButtonFlag,
	ButtonResolve,
	AllCreate,
	AllFail,
	AllSucceed,
	ForceWin,
	StateSync,

	// Connection messages that can be received during gameplay
	PlayerJoined = 50, // Doubles for both matchmaking and reconnect
	PlayerDisconnect,  // Doubles for manually disconnecting
	StartGame,
	ChangeGame // Followed by 0 for restart, 1 for next level
};

#endif /* __NETWORK_DATA_TYPE_H__ */
