#ifndef JS_EVENT_MODEL_H
#define JS_EVENT_MODEL_H
#include <cugl/cugl.h>
#include <cugl/io/CUJsonReader.h>

#include <vector>

#include "LevelConstants.h"

#pragma mark -
#pragma mark Event Model

/**
 * Class that represents a dynamically loaded event in the game
 *
 */
class EventModel {
   private:
	/**The name of the block to generate*/
	string block;

	/**The time to start this event*/
	int timeStart;

	/**The time to stop this event*/
	int timeStop;

	/**The probability per update frame of occurrence */
	float probability;

   public:
	EventModel(const EventModel&) = delete;
#pragma mark Static Constructors

	/**
	 * Creates a new building block with the given JSON file.
	 *
	 * @return a new building block
	 */
	static std::shared_ptr<EventModel> alloc(const std::shared_ptr<cugl::JsonValue>& json) {
		const std::shared_ptr<EventModel> result = std::make_shared<EventModel>();
		return (result->init(json) ? result : nullptr);
	}

#pragma mark Event Attributes

	/**
	 * Returns the name of the block used
	 *
	 * @return name of block used
	 */
	string getBlock() { return block; }

	/**
	 * Returns the start time
	 *
	 * @return start time
	 */
	int getStart() const { return timeStart; }

	/**
	 * Returns the end time
	 *
	 * @return end time
	 */
	int getEnd() const { return timeStop; }

	/**
	 * Returns the probability this event is generated per update frame
	 *
	 * @return probability per update frame
	 */
	float getProbability() const { return probability; }

	/**
	 * Returns whether this event is active
	 *
	 * @param time the current game time
	 * @return whether this event is active
	 */
	bool isActive(int time) const { return time <= timeStop && time >= timeStart; }

	/**
	 * Returns whether this event is one time
	 *
	 * @return whether this event is one time
	 */
	bool isOneTime() const { return timeStop == timeStart; }

#pragma mark -
#pragma mark Initializers
	/**
	 * Creates a new, empty level.
	 */
	EventModel() : timeStart(0), timeStop(0), probability(0){};

	bool init(const std::shared_ptr<cugl::JsonValue>& json) {
		block = json->get(BLOCK_FIELD)->asString();
		timeStart = json->get(TIME_START_FIELD)->asInt();
		timeStop = json->get(TIME_STOP_FIELD)->asInt();
		probability = json->get(PROBABILITY_FIELD)->asFloat();
		return true;
	}

	/**
	 * Destroys this level, releasing all resources.
	 */
	virtual ~EventModel() = default;
};

#endif /* defined(__JS_LEVEL_MODEL_H__) */
