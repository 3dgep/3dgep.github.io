#pragma once

#include "surface.hpp"
#include "template.hpp" // ScreenWidth/Height, Rand, vec2, fullscreen and VSync
#include "scene.hpp"
#include "settings.hpp"
#include "sounds.hpp"
#include "text.hpp"

#include <SDL3/SDL_keyboard.h> // SDL_Scancode, SDL_Keycode and SDL_GetScancodeFromKey
#include <SDL3/SDL_mouse.h>    // SDL_BUTTON_LEFT
#include <bitset>
#include <memory>

namespace Tmpl8 {

class Game
{
public:
	explicit Game( Surface& screen ) : screen( screen ) {}
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void MouseUp( int button ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int button ) { clickEvent = clickEvent || button == SDL_BUTTON_LEFT; }
	void MouseMove( int x, int y ) { mouseX = x; mouseY = y; moveEvent = true; }

	// Keyboard: use the SDL_SCANCODE_ names for the keys, e.g. SDL_SCANCODE_A, SDL_SCANCODE_SPACE, SDL_SCANCODE_LEFT.
	bool GetKey( SDL_Scancode key ) const { return held.test( key ); }             // true while the key is held down
	bool GetKeyPressed( SDL_Scancode key ) const { return pressed.test( key ); }   // true in the first frame the key is down
	bool GetKeyReleased( SDL_Scancode key ) const { return released.test( key ); } // true in the frame the key is released
	// KeyFor( SDLK_S ) is the key with an "s" on it, on any keyboard (for keys where the letter matters, like S for save)
	SDL_Scancode KeyFor( SDL_Keycode key ) const { return SDL_GetScancodeFromKey( key, nullptr ); }

	// Called by the template, you don't need to call these yourself.
	void KeyDown( SDL_Scancode key ) { keys.set( key ); downEvents.set( key ); }
	void KeyUp( SDL_Scancode key ) { keys.reset( key ); upEvents.set( key ); }
	void ProcessInput() // once per frame, before Tick
	{
		// A key that is tapped quickly (down and up again before the next frame) still counts as pressed.
		pressed = downEvents;
		released = upEvents;
		held = keys | downEvents;
		downEvents.reset();
		upEvents.reset();
		clicked = clickEvent;
		moved = moveEvent;
		clickEvent = moveEvent = false;
	}

	// For the scenes.
	void ChangeScene( std::unique_ptr<Scene> next ) { nextScene = std::move( next ); } // from the next frame on
	void Quit();
	int MouseX() const { return mouseX; }
	int MouseY() const { return mouseY; }
	bool MouseClicked() const { return clicked; } // true in the frame the left mouse button goes down
	bool MouseMoved() const { return moved; }     // true in a frame in which the mouse moved
	Text& GetText() { return text; }
	Settings& GetSettings() { return settings; }
	Sounds& GetSounds() { return sounds; }

private:
	Surface& screen; // the game window: draw on this

	Text text;
	Settings settings;
	Sounds sounds;
	std::unique_ptr<Scene> scene;     // the scene on the screen
	std::unique_ptr<Scene> nextScene; // the scene that takes over in the next frame, or nullptr

	int mouseX = 0, mouseY = 0;
	bool clickEvent = false, moveEvent = false; // since the previous frame
	bool clicked = false, moved = false;        // in this frame

	std::bitset<SDL_SCANCODE_COUNT> keys;       // keys that are down right now (updated by the template)
	std::bitset<SDL_SCANCODE_COUNT> downEvents; // keys that went down since the previous frame
	std::bitset<SDL_SCANCODE_COUNT> upEvents;   // keys that went up since the previous frame
	std::bitset<SDL_SCANCODE_COUNT> held;       // keys that are down in this frame
	std::bitset<SDL_SCANCODE_COUNT> pressed;    // keys that went down in this frame
	std::bitset<SDL_SCANCODE_COUNT> released;   // keys that went up in this frame
};

} // namespace Tmpl8
