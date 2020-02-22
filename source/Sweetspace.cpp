//
//  SDApp.cpp
//  Ship Demo
//
//  This is the root class for your game.  The file main.cpp accesses this class
//  to run the application.  While you could put most of your game logic in
//  this class, we prefer to break the game up into player modes and have a
//  class for each mode.
//
//  Author: Walker White
//  Version: 1/10/17
//
#include "Sweetspace.h"

using namespace cugl;

#pragma mark -
#pragma mark Gameplay Control

/**
 * The method called after OpenGL is initialized, but before running the application.
 *
 * This is the method in which all user-defined program intialization should
 * take place.  You should not create a new init() method.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to FOREGROUND,
 * causing the application to run.
 */
void Sweetspace::onStartup() {
	assets = AssetManager::alloc();
	batch = SpriteBatch::alloc();

	// Start-up basic input
#ifdef CU_TOUCH_SCREEN
	Input::activate<Touchscreen>();
#else
	Input::activate<Mouse>();
#endif

	assets->attach<Font>(FontLoader::alloc()->getHook());
	assets->attach<Texture>(TextureLoader::alloc()->getHook());
	assets->attach<Node>(SceneLoader::alloc()->getHook());

	// Create a "loading" screen
	loaded = false;
	loading.init(assets);

	// Queue up the other assets
	assets->loadDirectoryAsync("json/assets.json", nullptr);

	Application::onStartup();  // YOU MUST END with call to parent
}

/**
 * The method called when the application is ready to quit.
 *
 * This is the method to dispose of all resources allocated by this
 * application.  As a rule of thumb, everything created in onStartup()
 * should be deleted here.
 *
 * When overriding this method, you should call the parent method as the
 * very last line.  This ensures that the state will transition to NONE,
 * causing the application to be deleted.
 */
void Sweetspace::onShutdown() {
	loading.dispose();
	gameplay.dispose();
	assets = nullptr;
	batch = nullptr;

	// Shutdown input
#ifdef CU_TOUCH_SCREEN
	Input::deactivate<Touchscreen>();
#else
	Input::deactivate<Mouse>();
#endif

	Application::onShutdown();	// YOU MUST END with call to parent
}

/**
 * The method called to update the application data.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should contain any code that is not an OpenGL call.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 *
 * @param timestep  The amount of time (in seconds) since the last frame
 */
void Sweetspace::update(float timestep) {
	if (!loaded && loading.isActive()) {
		loading.update(0.01f);
	} else if (!loaded) {
		loading.dispose();	// Disables the input listeners in this mode
		gameplay.init(assets);
		loaded = true;
	} else {
		gameplay.update(timestep);
	}
}

/**
 * The method called to draw the application to the screen.
 *
 * This is your core loop and should be replaced with your custom implementation.
 * This method should OpenGL and related drawing calls.
 *
 * When overriding this method, you do not need to call the parent method
 * at all. The default implmentation does nothing.
 */
void Sweetspace::draw() {
	if (!loaded) {
		loading.render(batch);
	} else {
		gameplay.render(batch);
	}
}
