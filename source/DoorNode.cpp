﻿#include "DoorNode.h"

#include <cugl/2d/CUAnimationNode.h>

#include <utility>

#include "Globals.h"

using namespace cugl;

/** Number of animation frames of doors */
constexpr int DOOR_FRAMES = 32;

/** Number of animation rows of doors */
constexpr int DOOR_ROWS = 1;

/** Number of animation cols of doors */
constexpr int DOOR_COLS = 32;

/** The radius used for placement of the doors. */
constexpr float DOOR_RADIUS = 660;

/** The scale of the doors. */
constexpr float DOOR_SCALE = 0.3f;

/** The frame of the animation strip to freeze on when one player is on the door */
constexpr int ONE_PLAYER_FRAME = 16;
/** The frame of the animation strip to freeze on when two players are on the door */
constexpr int TWO_PLAYER_FRAME = 31;

bool DoorNode::init(const std::shared_ptr<DoorModel>& door, std::shared_ptr<DonutModel> player,
					float shipSize, const std::shared_ptr<cugl::AssetManager>& assets) {
	CustomNode::init(std::move(player), shipSize, door->getAngle(), DOOR_RADIUS);

	doorModel = door;
	animationNode =
		cugl::AnimationNode::alloc(assets->get<Texture>("door"), DOOR_ROWS, DOOR_COLS, DOOR_FRAMES);
	animationNode->setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
	animationNode->setPosition(0, 0);
	animationNode->setFrame(0);
	addChild(animationNode);
	setAnchor(Vec2::ANCHOR_BOTTOM_CENTER);
	setScale(DOOR_SCALE);

	isDirty = true;

	return true;
}

void DoorNode::prePosition() {
	if (angle != doorModel->getAngle()) {
		isDirty = true;
		angle = doorModel->getAngle();
	}
}

void DoorNode::postPosition() {
	frameCap = doorModel->getPlayersOn() < 2 ? doorModel->getPlayersOn() * ONE_PLAYER_FRAME
											 : TWO_PLAYER_FRAME;
	if (animationNode->getFrame() < frameCap) {
		animationNode->setFrame(static_cast<int>(animationNode->getFrame()) + 1);
	} else if (animationNode->getFrame() > frameCap) {
		animationNode->setFrame(static_cast<int>(animationNode->getFrame()) - 1);
	}

	const float diff = static_cast<float>(height) - static_cast<float>(doorModel->getHeight());
	height = doorModel->getHeight();
	if (diff != 0) {
		animationNode->shiftPolygon(0, diff);
	}
}
