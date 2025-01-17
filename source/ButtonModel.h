﻿#ifndef BUTTON_MODEL_H
#define BUTTON_MODEL_H
#include <cugl/cugl.h>

#include <bitset>

#include "Globals.h"

class ButtonModel {
   private:
	/** The height of the button, as percentage down (0 = fully up) */
	float height = 0;
	/** The current frame of animation */
	unsigned int frame = 0;
	/** The angle at which the button exists */
	float angle;
	/** Pointer to the pair of this button */
	std::shared_ptr<ButtonModel> pairButton;
	/** ID of the pair of this button */
	uint8_t pairID;
	/** Whether this button is jumped on */
	bool jumped;
	/** Whether this model is active */
	bool isActive;

   public:
#pragma region Constructors
	/*
	 * Creates a new, uninitialized, and unused button.
	 *
	 * Do not call this constructor using new. These models should exclusively be allocated into an
	 * object pool by {@code ShipModel} and accessed from there.
	 */
	ButtonModel() : angle(-1), pairID(-1), jumped(false), isActive(false) {}

	ButtonModel(const ButtonModel&) = delete;

	/**
	 * Initializes a new button with the given angle and pair.
	 *
	 * @param a   The angle at which the button exists
	 * @param pair Pointer to the pair of this button
	 * @param pairID ID of the pair of this button
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	bool init(float a, std::shared_ptr<ButtonModel> pair, uint8_t pairID);

#pragma endregion
#pragma region Accessors
	/** Returns whether this model is active */
	bool getIsActive() const { return isActive; }

	/**
	 * Returns the current angle of the button in degrees.
	 */
	float getAngle() const { return angle; }

	/**
	 * Returns the section of the ship containing this button.
	 */
	int getSection() const;

	/**
	 * Returns the current height of the button, as percentage down, where 0 = fully up and 1 =
	 * fully down
	 */
	float getHeight() const { return height; }

	/**
	 * Returns whether any players are jumping on this button.
	 */
	bool isJumpedOn() const { return jumped; }

	/**
	 * Return a pointer to the pair of this button
	 */
	std::shared_ptr<ButtonModel> getPair() { return pairButton; }

	/**
	 * Return the ID of the pair of this button
	 */
	uint8_t getPairID() const { return pairID; }

#pragma endregion
#pragma region Mutators

	/**
	 * Update the state of this button each frame
	 */
	void update();

	/**
	 * Trigger this button due to a jump.
	 *
	 * @return true iff the trigger was successfully registered (ie was not called during the
	 * i-frames after the last call to trigger)
	 */
	bool trigger();

	/**
	 * Resets this button.
	 */
	void reset();
#pragma endregion
};
#endif /* BUTTON_MODEL_H */
