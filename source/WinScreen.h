#ifndef WIN_SCREEN_H
#define WIN_SCREEN_H
#include <cugl/cugl.h>

#include "ButtonManager.h"

/**
 * Scene graph node representing the screen to show upon winning a level.
 * Will cover the whole screen automatically.
 * Starts disabled; query {@link isVisible()} to check if the screen is active.
 */
class WinScreen : public cugl::Node {
   private:
	/** Current frame of the animation */
	size_t currFrame;

	/** Starting position of the ship (x-coord) */
	float startPos;
	/** Ending position of the ship (x-coord) */
	float endPos;
	/** Whether to shift the whole thing over one afterwards */
	bool mustShift;
	/** Completed level */
	uint8_t completedLevel;

	/** Whether this player is the host */
	bool isHost;

	/** Scene graph node representing the current location of the ship */
	std::shared_ptr<cugl::TexturedNode> ship;
	/** Scene graph node representing the circle behind the ship*/
	std::shared_ptr<cugl::PathNode> circle;
	/** Scene graph node for the next level button */
	std::shared_ptr<cugl::Button> btn;
	/** Scene graph node for the waiting for host text */
	std::shared_ptr<cugl::Node> waitText;
	/** Star markers of each individual level */
	std::vector<std::shared_ptr<cugl::TexturedNode>> levelMarkers;

	/** Helper class managing the level checkpoint icons */
	class IconManager;
	/** Helper class managing the level checkpoint icons */
	std::unique_ptr<IconManager> icons;

	/** Button manager for the next level button */
	ButtonManager btns;

	/**
	 * Layout and make invisible all level markers for a completed level.
	 * Returns the number of levels and the left level.
	 */
	std::pair<uint8_t, uint8_t> layoutLevelMarkers(uint8_t completedLevel);

   public:
	/**
	 * Construct this win screen with assets from the pointed asset manager.
	 * The screen will remain invisible until {@link activate()} is called
	 *
	 * @param assets Asset manager to load win screen assets from
	 */
	explicit WinScreen(const std::shared_ptr<cugl::AssetManager> &assets);
	/** Cleanup and delete this win screen */
	virtual ~WinScreen();

	/**
	 * Initialize this win screen with assets from the pointed asset manager.
	 * The screen will remain invisible until {@link activate()} is called
	 *
	 * @param assets Asset manager to load win screen assets from
	 */
	bool init(const std::shared_ptr<cugl::AssetManager> &assets);

	/**
	 * Cleanup and dispose of all assets pointed to by this node
	 */
	void dispose() override;

	/**
	 * Activate the win screen.
	 *
	 * @param completedLevel The level just completed
	 */
	void activate(uint8_t completedLevel);

	/**
	 * Whether the given tap data tapped the next level button.
	 *
	 * @param tapData Tap data from the Input Controller
	 *
	 * @return True iff the next level button was tapped
	 */
	bool tappedNext(const std::tuple<cugl::Vec2, cugl::Vec2> &tapData) const;

	/**
	 * Update the animation for this node. Should be called once every frame.
	 */
	void update();
};

#endif // WIN_SCREEN_H
