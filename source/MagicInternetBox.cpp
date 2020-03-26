#include "MagicInternetBox.h"

#include <sstream>

using namespace cugl;

constexpr auto GAME_SERVER = "ws://sweetspace-server.herokuapp.com/";
constexpr float FLOAT_PRECISION = 180.0f;
constexpr unsigned int NETWORK_TICK = 12; // Constant also defined in ExternalDonutModel.cpp
constexpr unsigned int ONE_BYTE = 256;
constexpr unsigned int ROOM_LENGTH = 5;

bool MagicInternetBox::initConnection() {
	switch (status) {
		case Uninitialized:
		case HostError:
		case ClientError:
			break;
		default:
			CULog("ERROR: MIB already initialized");
			return false;
	}
	using easywsclient::WebSocket;

	// I actually don't know what this stuff does but it won't run on Windows without it,
	// so ¯\_(ツ)_/¯
#ifdef _WIN32
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		CULog("WSAStartup Failed");
		return false;
	}
#endif

	ws = WebSocket::from_url(GAME_SERVER);
	if (!ws) {
		CULog("FAILED TO CONNECT");
		return false;
	}

	return true;
}

bool MagicInternetBox::initHost() {
	if (!initConnection()) {
		status = HostError;
		return false;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)NetworkDataType::AssignedRoom);
	ws->sendBinary(data);
	this->playerID = 0;
	this->numPlayers = 1;
	status = HostConnecting;

	return true;
}

bool MagicInternetBox::initClient(std::string id) {
	if (!initConnection()) {
		status = ClientError;
		return false;
	}

	std::vector<uint8_t> data;
	data.push_back((uint8_t)NetworkDataType::JoinRoom);
	for (unsigned int i = 0; i < ROOM_LENGTH; i++) {
		data.push_back((uint8_t)id.at(i));
	}
	ws->sendBinary(data);
	roomID = id;
	status = ClientConnecting;

	return true;
}

/*

DATA FORMAT

[ TYPE (enum) | ANGLE (2 bytes) | ID (2 bytes) | data1 (2 bytes) | data2 (2 bytes) | data3 (3 bytes)

Each 2-byte block is stored smaller first, then larger; ie 2^8 * byte1 + byte0 gives the original.
All data is truncated to fit 16 bytes.
Floats are multiplied by FLOAT_PRECISION and then cast to int before running through same algorithm
Only data3 can handle negative numbers. The first byte is 1 for positive and 0 for negative.

*/

void MagicInternetBox::sendData(NetworkDataType type, float angle, int id, int data1, int data2,
								float data3) {
	std::vector<uint8_t> data;

	data.push_back((uint8_t)type);

	int angleConv = (int)(FLOAT_PRECISION * angle);

	data.push_back((uint8_t)(angleConv % ONE_BYTE));
	data.push_back((uint8_t)(angleConv / ONE_BYTE));

	data.push_back((uint8_t)(id % ONE_BYTE));
	data.push_back((uint8_t)(id / ONE_BYTE));

	data.push_back((uint8_t)(data1 % ONE_BYTE));
	data.push_back((uint8_t)(data1 / ONE_BYTE));

	data.push_back((uint8_t)(data2 % ONE_BYTE));
	data.push_back((uint8_t)(data2 / ONE_BYTE));

	int d3Positive = data3 >= 0 ? 1 : 0;
	int d3 = (int)(FLOAT_PRECISION * abs(data3));

	data.push_back((uint8_t)d3Positive);
	data.push_back((uint8_t)(d3 % ONE_BYTE));
	data.push_back((uint8_t)(d3 / ONE_BYTE));

	ws->sendBinary(data);
}

bool MagicInternetBox::reconnect(std::string id) { return false; }

MagicInternetBox::MatchmakingStatus MagicInternetBox::matchStatus() { return status; }

void MagicInternetBox::leaveRoom() {}

std::string MagicInternetBox::getRoomID() { return roomID; }

int MagicInternetBox::getPlayerID() { return playerID; }

unsigned int MagicInternetBox::getNumPlayers() { return numPlayers; }

void MagicInternetBox::startGame() {
	switch (status) {
		case HostWaitingOnOthers:
		case ClientWaitingOnOthers:
			break;
		default:
			CULog("ERROR: Trying to start game during invalid state %d", status);
			return;
	}

	status = GameStart;
}

void MagicInternetBox::update() {
	switch (status) {
		case Uninitialized:
			CULog("ERROR: Update called on MIB before initialization; aborting");
			return;
		case GameStart:
			CULog("ERROR: Matchmaking update called on MIB after game start; aborting");
			return;
		default:
			break;
	}

	ws->poll();
	ws->dispatchBinary([this](const std::vector<uint8_t>& message) {
		if (message.size() == 0) {
			return;
		}

		NetworkDataType type = static_cast<NetworkDataType>(message[0]);

		switch (type) {
			case AssignedRoom: {
				std::stringstream newRoomId;
				for (unsigned int i = 0; i < ROOM_LENGTH; i++) {
					newRoomId << (char)message[i + 1];
				}
				roomID = newRoomId.str();
				CULog("Got room ID: %s", roomID.c_str());
				status = HostWaitingOnOthers;
				return;
			}
			case JoinRoom: {
				switch (message[1]) {
					case 0: {
						numPlayers = message[2];
						playerID = (int)numPlayers - 1;
						CULog("Join Room Success; player id %d", playerID);
						status = ClientWaitingOnOthers;
						return;
					}
					case 1: {
						CULog("Room Does Not Exist");
						status = ClientRoomInvalid;
						return;
					}
					case 2: {
						CULog("Room Full");
						status = ClientRoomFull;
						return;
					}
				}
			}
			case PlayerJoined: {
				CULog("Player Joined");
				numPlayers++;
				return;
			}
			default:
				CULog("Received invalid gameplay message during connection; %d", message[0]);
				return;
		}
	});
}

void MagicInternetBox::update(std::shared_ptr<ShipModel> state) {
	if (status != GameStart) {
		CULog("ERROR: Gameplay update called on MIB before game start; aborting");
		return;
	}

	// NETWORK TICK
	currFrame = (currFrame + 1) % NETWORK_TICK;
	if (currFrame == 0) {
		std::shared_ptr<DonutModel> player = state->getDonuts()[playerID];
		float angle = player->getAngle();
		float velocity = player->getVelocity();
		sendData(PositionUpdate, angle, playerID, -1, -1, velocity);
	}

	ws->poll();
	ws->dispatchBinary([&state](const std::vector<uint8_t>& message) {
		if (message.size() == 0) {
			return;
		}

		NetworkDataType type = static_cast<NetworkDataType>(message[0]);

		if (type > DualResolve) {
			CULog("Received invalid connection message during gameplay; %d", message[0]);
			return;
		}

		float angle = (float)(message[1] + ONE_BYTE * message[2]) / FLOAT_PRECISION;
		int id = (int)(message[3] + ONE_BYTE * message[4]);
		// Networking code is finnicky and having these magic numbers is the easiest solution
		// NOLINTNEXTLINE Simple counting numbers
		int data1 = (int)(message[5] + ONE_BYTE * message[6]);
		// NOLINTNEXTLINE Ditto
		int data2 = (int)(message[7] + ONE_BYTE * message[8]);
		// NOLINTNEXTLINE Ditto
		float data3 = (message[9] == 1 ? 1 : -1) * (float)(message[10] + ONE_BYTE * message[11]) /
					  FLOAT_PRECISION;

		switch (type) {
			case PositionUpdate: {
				std::shared_ptr<DonutModel> donut = state->getDonuts()[id];
				donut->setAngle(angle);
				donut->setVelocity(data3);
				break;
			}
			case Jump: {
				CULog("Received jump %d", id);
				state->getDonuts()[id]->startJump();
				break;
			}
			case BreachCreate: {
				state->createBreach(angle, data1, id);
				CULog("Creating breach %d at angle %f with user %d", id, angle, data1);
				break;
			}
			case BreachShrink: {
				state->resolveBreach(id);
				CULog("Resolve breach %d", id);
				break;
			}
			case DualCreate: {
				int taskID = id;
				state->createDoor(angle, taskID);
				break;
			}
			case DualResolve: {
				int taskID = id;
				int player = data1;
				int flag = data2;
				CULog("Flag door %d with player %d", id, player);
				state->flagDoor(taskID, player, flag);
				break;
			}
			default:
				break;
		}
	});
}

void MagicInternetBox::createBreach(float angle, int player, int id) {
	sendData(BreachCreate, angle, id, player, -1, -1.0f);
	CULog("Creating breach id %d player %d angle %f", id, player, angle);
}

void MagicInternetBox::resolveBreach(int id) {
	sendData(BreachShrink, -1.0f, id, -1, -1, -1.0f);
	CULog("Sending resolve id %d", id);
}

void MagicInternetBox::createDualTask(float angle, int player1, int player2, int id) {
	sendData(DualCreate, angle, id, player1, player2, -1.0f);
}

void MagicInternetBox::flagDualTask(int id, int player, int flag) {
	sendData(DualResolve, -1.0f, id, player, flag, -1.0f);
}

void MagicInternetBox::jump(int player) { sendData(Jump, -1.0f, player, -1, -1, -1.0f); }
