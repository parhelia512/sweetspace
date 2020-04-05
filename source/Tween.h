#ifndef __TWEEN_H__
#define __TWEEN_H__

#include <cugl/cugl.h>

/**
 * A class containing static methods useful for tweening animations.
 */
class Tween {
   private:
	/**
	 * Linearly interpolate between start and end at a percentage.
	 *
	 * @param start The starting position
	 * @param end The ending position
	 * @param percentage The percent through the animation, in range [0,1]
	 *
	 * @return The interpolated position
	 */
	static float linInterp(float start, float end, float percentage);

   public:
	/**
	 * Linearly interpolate between start and end.
	 *
	 * @param start The starting position
	 * @param end The ending position
	 * @param currFrame The current frame of the animation, >=0 and <= maxFrame
	 * @param maxFrame The last frame of the animation, >=0
	 *
	 * @return The interpolated position
	 */
	static float linear(float start, float end, int currFrame, int maxFrame);

	/**
	 * Quartic ease in interpolation between start and end.
	 *
	 * @param start The starting position
	 * @param end The ending position
	 * @param currFrame The current frame of the animation, >=0 and <= maxFrame
	 * @param maxFrame The last frame of the animation, >=0
	 *
	 * @return The interpolated position
	 */
	static float easeIn(float start, float end, int currFrame, int maxFrame);

	/**
	 * Quartic ease out interpolation between start and end.
	 *
	 * @param start The starting position
	 * @param end The ending position
	 * @param currFrame The current frame of the animation, >=0 and <= maxFrame
	 * @param maxFrame The last frame of the animation, >=0
	 *
	 * @return The interpolated position
	 */
	static float easeOut(float start, float end, int currFrame, int maxFrame);

	/**
	 * Generate a color that can be used for fading.
	 */
	static cugl::Color4 fade(float a);
};
#endif /* __TWEEN_H__ */