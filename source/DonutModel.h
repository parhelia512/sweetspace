﻿#ifndef DONUT_MODEL_H_
#define DONUT_MODEL_H_
#include <cugl/cugl.h>

#pragma mark -
#pragma mark Donut Model

class DonutModel {
   protected:
	/** This macro disables the copy constructor (not allowed on models) */
	CU_DISALLOW_COPY_AND_ASSIGN(DonutModel);

#pragma region Animation Constants and Functions

	/** The max angular velocity (in degrees) per frame */
	static constexpr float DONUT_MAX_TURN = 1.7f;
	/** The max force to apply to the donut */
	static constexpr float DONUT_MAX_FORCE = 0.5f;
	/** The default amount the angular velocity decays by each frame */
	static constexpr float DEFAULT_DONUT_FRICTION_FACTOR = 0.95f;
	/** The threshold below which the donut has effectively stopped rolling */
	static constexpr float DONUT_STOP_THRESHOLD = 0.01f;

	/** The Default Ship Size */
	static constexpr float DEFAULT_SHIP_SIZE = 360;

   public:
	/** The threshold which the donut will begin to fall back to the ground again */
	static constexpr float JUMP_HEIGHT = 0.35f;
	/** Downward Acceleration for calculating jump offsets */
	static constexpr float GRAVITY = 10.0f;

	enum FaceState {
		/** When donut is still or rolling */
		Idle,
		/** When donut collides with mismatched breach */
		Dizzy,
		/** When donut is fixing own breach */
		Working,
		/** When donut collides with door */
		Colliding
	};

   protected:
	/** Clamp x into the range [y,z] */
	static constexpr float RANGE_CLAMP(float x, float y, float z) {
		return (x < y ? y : (x > z ? z : x));
	}
#pragma endregion

	/** Scene graph position of the donut; used to position the asset in the scene graph. Should not
	 * be modified. */
	cugl::Vec2 sgPos;
	/** Angle of the donut in the world space */
	float angle;
	/** Size of the level */
	float shipSize;
	/** Current turning thrust (stored to facilitate decay) */
	float velocity;
	/** Velocity Adjustment Factor. Not realistic Friction. */
	float friction;
	/** Offset from bottom of ship when Jumping based on proportion of hallway */
	float jumpOffset;
	/** Whether donut is currently jumping */
	bool jumping;
	/** The ellapsed time since the beginning of the jump in seconds */
	float jumpTime;
	/** Initial vertical velocity */
	float jumpVelocity;
	/** Whether or not this player is active */
	bool isActive = true;
	/** Current animation state the player is in */
	FaceState faceState;
	/** New position after stabilizer failure */
	float teleportAngle;

	/**
	 * Performs state and animation updates for a jumping donut.
	 *
	 * Will check if a donut is jumping automatically.
	 */
	void updateJump(float timestep);
	/** Id of donut's color */
	uint8_t colorId;

   public:
#pragma mark Constructors
	/*
	 * Creates a new donut at the origin.
	 *
	 * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a model on
	 * the heap, use one of the static constructors instead.
	 */
	DonutModel()
		: angle(0),
		  shipSize(DEFAULT_SHIP_SIZE),
		  velocity(0),
		  friction(DEFAULT_DONUT_FRICTION_FACTOR),
		  jumpOffset(0),
		  jumping(false),
		  jumpTime(0),
		  jumpVelocity(0),
		  faceState(Idle),
		  teleportAngle(0),
		  colorId(0) {}

	/**
	 * Destroys this donut, releasing all resources.
	 */
	virtual ~DonutModel() { dispose(); }

	/**
	 * Disposes all resources and assets of this donut
	 *
	 * Any assets owned by this object will be immediately released.  Once
	 * disposed, a donut may not be used until it is initialized again.
	 */
	void dispose();

	/**
	 * Initializes a new donut at the origin.
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param shipSize	The max angle the donut is allowed to have
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(float shipSize) { return init(cugl::Vec2::ZERO, shipSize); }

	/**
	 * Initializes a new donut with the given position
	 *
	 * An initializer does the real work that the constructor does not.  It
	 * initializes all assets and makes the object read for use.  By separating
	 * them, we allow ourselfs non-pointer references to complex objects.
	 *
	 * @param pos   Initial position in world coordinates
	 * @param shipSize	The max angle the donut is allowed to have
	 *
	 * @return true if the obstacle is initialized properly, false otherwise.
	 */
	virtual bool init(const cugl::Vec2& pos, float shipSize);

#pragma mark -
#pragma mark Accessors

	/**
	 * Returns the donut position in the scene graph as a reference.
	 *
	 * This allows us to modify the value.
	 *
	 * @return the donut's scene graph position as a reference.
	 */
	cugl::Vec2& getSceneGraphPosition() { return sgPos; }

	/**
	 * Returns the current angle of the donut in degrees.
	 *
	 * @return the current angle of the donut in degrees.
	 */
	float getAngle() const { return angle; }

	/**
	 * Sets the current angle of the donut in degrees.
	 *
	 * @param value The donut angle in degrees
	 */
	virtual void setAngle(float value) { angle = value; }

	/**
	 * Sets the angle after teleportation in degrees.
	 * @param a
	 */
	void setTeleportAngle(float a) { teleportAngle = a; }

	/**
	 * Teleport to new angle.
	 */
	void teleport() { setAngle(teleportAngle); }

	/**
	 * Returns the jump offset.
	 *
	 * @return the jump offset.
	 */
	float getJumpOffset() const { return jumpOffset; }

	/**
	 * Sets the current jump offset of the donut.
	 *
	 * @param value The jump offset
	 */
	void setJumpOffset(float value) { jumpOffset = value; }

	/**
	 * Sets whether the donut is jumping.
	 *
	 * @param b whether or not the donut is jumping
	 */
	void setIsJumping(bool b) { jumping = b; }

	/**
	 * Returns whether the donut is currently jumping.
	 *
	 * @return whether the donut is currently jumping.
	 */
	bool isJumping() const { return jumping; }

	/** Returns whether the donut is currently jumping and is on the descent of the jump arc */
	bool isDescending() const {
		return jumping && (GRAVITY / 2 * jumpTime * jumpTime > jumpVelocity * jumpTime);
	}

	/**
	 * Returns the donut's jump time
	 * @return
	 */
	float getJumpTime() const { return jumpTime; }

	/**
	 * Sets the velocity of the donut directly.
	 * Should really only be called by networking code.
	 *
	 * @param v The new velocity of the donut.
	 */
	void setVelocity(float v) { velocity = v; }

	/**
	 * Returns the current velocity of the donut.
	 *
	 * @return the current velocity of the donut.
	 */
	float getVelocity() const { return velocity; }

	/**
	 * Sets the friction applied to the donut directly.
	 *
	 * @param f The new friction applied to the donut.
	 */
	void setFriction(float f) { friction = f; }

	/**
	 * Returns the current friction applied to the donut.
	 *
	 * @return the current friction applied to the donut.
	 */
	float getFriction() const { return friction; }

	/**
	 * Returns whether this donut is active.
	 *
	 * @return whether this donut is active.
	 */
	bool getIsActive() const { return isActive; }

	/**
	 * Sets whether this donut is active.
	 */
	void setIsActive(bool active) { isActive = active; }

	void setColorId(uint8_t i) { colorId = i; }
	uint8_t getColorId() const { return colorId; }

	/**
	 * Applies a force to the donut.
	 *
	 * @param value The donut turning force
	 */
	void applyForce(float value);

	/**
	 * Starts a fixed height jump for the donut.
	 *
	 * @param value The donut turning force
	 */
	void startJump();

	/**
	 * Transition player animation state
	 * @param newState
	 */
	void transitionFaceState(FaceState newState);

	/**
	 * Returns animation state of donut face
	 * @return
	 */
	FaceState getFaceState() { return faceState; }
#pragma mark -
#pragma mark Animation

	/**
	 * Updates the state of the model
	 *
	 * This method moves the donut forward, dampens the forces (if necessary)
	 * and updates the sprite if it exists.
	 *
	 * @param timestep  Time elapsed (in seconds) since last called.
	 */
	virtual void update(float timestep) = 0;

	/**
	 * Resets the donut back to its original settings
	 */
	void reset();
};

#endif /* __DONUT_MODEL_H__ */
