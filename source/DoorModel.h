﻿#ifndef __door_MODEL_H__
#define __door_MODEL_H__
#include <cugl/cugl.h>

#include <bitset>
/** The max height of the door*/
constexpr int MAX_HEIGHT = 1600;
/** The max height of the door*/
constexpr int HALF_OPEN = 400;
/** The speed of the door raising */
constexpr int SPEED = 20;

class DoorModel {
   private:
	/** The height of the door */
	int height = 0;

   protected:
	/** The angle at which the door exists */
	float angle;
	/** The state of the door */
	unsigned char playersOn;

   public:
#pragma mark Constructors
	/*
	 * Creates a new door at angle 0.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	DoorModel(void) : angle(0), playersOn(0) {}

	/**
	 * Destroys this door, releasing all resources.
	 */
	~DoorModel(void) { dispose(); }

	/**
	 * Disposes all resources and assets of this door
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a door may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes a new door at an unassigned angle (-1).
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init() { return init(-1.0f); }

	/**
	 * Initializes a new door with the given angle
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param a   The angle at which the door exists
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const float a) {
		this->angle = a;
		return true;
	};

	static std::shared_ptr<DoorModel> alloc() {
		std::shared_ptr<DoorModel> result = std::make_shared<DoorModel>();
		return (result->init() ? result : nullptr);
	}

#pragma mark -
#pragma mark Accessors
	/**
	 * Returns the current angle of the door in radians.
	 *
	 * @return the current angle of the door in radians.
	 */
	float getAngle() { return (float)M_PI * angle / 180.0f; }

	/**
	 * Returns the current height of the door.
	 *
	 * @return the current height of the door.
	 */
	int getHeight() { return height; }

	/**
	 * Returns the number of players in range of the door.
	 *
	 * @return the number of players in range of the door.
	 */
	int getPlayersOn() {
		std::bitset<8> ids(playersOn);
		return ids.count();
	}

	/**
	 * Sets the current angle of the door in radians.
	 *
	 * @param value The door angle in radians
	 */
	void setAngle(float value) { angle = 180.0f * value / (float)M_PI; }

	/**
	 * Adds the given player's flag from the door.
	 *
	 */
	void addPlayer(int id) { playersOn = playersOn | (unsigned char)pow(2, id); }

	/**
	 * Removes the given player's flag from the door. Requires that this player is on the door
	 */
	void removePlayer(int id) {
		if (!resolved()) {
			playersOn = playersOn ^ (unsigned char)pow(2, id);
		}
	}

	/**
	 * Raises the door.
	 */
	void raiseDoor() {
		if (height < MAX_HEIGHT) height += SPEED;
	}

	/**
	 * Returns whether this door can be passed under.
	 */
	bool halfOpen() { return height >= HALF_OPEN; }

	/**
	 * Returns whether this door has been resolved and opened.
	 */
	bool resolvedAndRaised() { return resolved() && height >= MAX_HEIGHT; }

	/**
	 * Returns whether this player is on the door.
	 */
	bool isPlayerOn(int id) { return (playersOn & (unsigned char)pow(2, id)) > 0; }

	/**
	 * Returns whether this door is resolved.
	 */
	bool resolved() { return getPlayersOn() >= 2; }

	/**
	 * Resets this door.
	 */
	void clear() {
		playersOn = 0;
		height = 0;
	}
};
#endif /* __door_MODEL_H__ */