﻿#ifndef __JS_LEVEL_MODEL_H__
#define __JS_LEVEL_MODEL_H__
#include <cugl/2d/physics/CUObstacleWorld.h>
#include <cugl/assets/CUAsset.h>
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>

#include <vector>

#include "LevelConstants.h"

using namespace cugl;

#pragma mark -
#pragma mark Level Model

const unsigned int DEFAULT_MAX_BREACHES = 3;
const unsigned int DEFAULT_MAX_DOORS = 1;
const unsigned int DEFAULT_SPAWN_RATE = 100;
const float DEFAULT_MIN_ANGLE = 30;
const int DEFAULT_INIT_HEALTH = 11;
const float DEFAULT_TIME = 45;
const float DEFAULT_SHIP_SIZE = 360;

/**
 * Class that represents a dynamically loaded level in the game
 *
 * This class is a subclass of Asset so that we can use it with a GenericLoader.
 */
class LevelModel : public Asset {
   private:
	/** The maximum number of events on ship at any one time. This will probably need to scale with
	 * the number of players*/
	unsigned int maxBreaches;
	/** The maximum number of events on ship at any one time. This will probably need to scale with
	 * the number of players*/
	unsigned int maxDoors;
	/** Spawn rate of breaches = 1/SPAWN_RATE for EVERY UPDATE FRAME. 100 is a very fast rate
	 * already. */
	unsigned int spawnRate;
	/** Min angle difference between a donut and an obstacle*/
	float minAngleDiff;
	/** Size of the ship in degrees*/
	float shipSize;
	/** Starting time for the timer*/
	float time;
	/** Starting health for the ship*/
	int initHealth;

   public:
#pragma mark Static Constructors
	/**
	 * Creates a new game level with no source file.
	 *
	 * The source file can be set at any time via the setFile() method. This method
	 * does NOT load the asset.  You must call the load() method to do that.
	 *
	 * @return  an autoreleased level file
	 */
	static std::shared_ptr<LevelModel> alloc() {
		std::shared_ptr<LevelModel> result = std::make_shared<LevelModel>();
		return (result->init("") ? result : nullptr);
	}

	/**
	 * Creates a new game level with the given source file.
	 *
	 * This method does NOT load the level. You must call the load() method to do that.
	 * This method returns false if file does not exist.
	 *
	 * @return  an autoreleased level file
	 */
	static std::shared_ptr<LevelModel> alloc(std::string file) {
		std::shared_ptr<LevelModel> result = std::make_shared<LevelModel>();
		return (result->init(file) ? result : nullptr);
	}

#pragma mark Level Attributes
	/**
	 * Returns the max breaches
	 *
	 * @return the max breaches
	 */
	const unsigned int getMaxBreaches() const { return maxBreaches; }

	/**
	 * Returns the max doors
	 *
	 * @return the max doors
	 */
	const unsigned int getMaxDoors() const { return maxDoors; }

	/**
	 * Returns the spawn rate
	 *
	 * @return the spawn rate
	 */
	const unsigned int getSpawnRate() const { return spawnRate; }

	/**
	 * Returns the min angle diff in which events can be generated near donuts
	 *
	 * @return the min angle diff
	 */
	const float getMinAngleDiff() const { return minAngleDiff; }

	/**
	 * Returns the ship size
	 *
	 * @return the ship size
	 */
	const float getShipSize() const { return shipSize; }

	/**
	 * Returns the starting time
	 *
	 * @return the starting time
	 */
	const float getTime() const { return time; }

	/**
	 * Returns the init health
	 *
	 * @return the init health
	 */
	const int getInitHealth() const { return initHealth; }
#pragma mark -
#pragma mark Asset Loading
	/**
	 * Loads this game level from the source file
	 *
	 * This load method should NEVER access the AssetManager.  Assets are loaded in
	 * parallel, not in sequence.  If an asset (like a game level) has references to
	 * other assets, then these should be connected later, during scene initialization.
	 *
	 * @return true if successfully loaded the asset from a file
	 */
	bool preload(const std::string& file) override {
		std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(file);
		return preload(reader->readJson());
	}

	/**
	 * Loads this game level from the source file
	 *
	 * This load method should NEVER access the AssetManager.  Assets are loaded in
	 * parallel, not in sequence.  If an asset (like a game level) has references to
	 * other assets, then these should be connected later, during scene initialization.
	 *
	 * @return true if successfully loaded the asset from a file
	 */
	bool preload(const std::shared_ptr<cugl::JsonValue>& json) override {
		if (json == nullptr) {
			CUAssertLog(false, "Failed to load level file");
			return false;
		}
		maxBreaches = json->get(MAX_BREACH_FIELD)->asInt();
		maxDoors = json->get(MAX_DOOR_FIELD)->asInt();
		spawnRate = json->get(SPAWN_RATE_FIELD)->asInt();
		minAngleDiff = json->get(MIN_ANGLE_DIFF_FIELD)->asFloat();
		shipSize = json->get(SHIP_SIZE_FIELD)->asFloat();
		time = json->get(TIME_FIELD)->asFloat();
		initHealth = json->get(INIT_HEALTH_FIELD)->asInt();
		return true;
	}

	/**
	 * Unloads this game level, releasing all sources
	 *
	 * This load method should NEVER access the AssetManager.  Assets are loaded and
	 * unloaded in parallel, not in sequence.  If an asset (like a game level) has
	 * references to other assets, then these should be disconnected earlier.
	 */
	void unload() {}

	//#pragma mark -
	//#pragma mark Initializers
	/**
	 * Creates a new, empty level.
	 */
	LevelModel(void) {
		maxBreaches = DEFAULT_MAX_BREACHES;
		maxDoors = DEFAULT_MAX_DOORS;
		spawnRate = DEFAULT_SPAWN_RATE;
		minAngleDiff = DEFAULT_MIN_ANGLE;
		shipSize = DEFAULT_SHIP_SIZE;
		time = DEFAULT_TIME;
		initHealth = DEFAULT_INIT_HEALTH;
	};

	/**
	 * Destroys this level, releasing all resources.
	 */
	virtual ~LevelModel(void){};
};

#endif /* defined(__JS_LEVEL_MODEL_H__) */