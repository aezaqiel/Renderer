#pragma once

#include <ostream>

#include "Types.hpp"

namespace Renderer {

	typedef enum class KeyCode : u16
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	enum class KeyState
	{
		None = -1,
		Pressed,
		Held,
		Released
	};

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	typedef enum class MouseButton : u16
	{
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Left = Button0,
		Right = Button1,
		Middle = Button2
	} Button;

}

inline std::ostream& operator<<(std::ostream& os, Renderer::KeyCode keyCode)
{
    os << static_cast<Renderer::i32>(keyCode);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Renderer::MouseButton button)
{
    os << static_cast<Renderer::i32>(button);
    return os;
}

// From glfw3.h
#define KEY_SPACE           ::Renderer::Key::Space
#define KEY_APOSTROPHE      ::Renderer::Key::Apostrophe    /* ' */
#define KEY_COMMA           ::Renderer::Key::Comma         /* , */
#define KEY_MINUS           ::Renderer::Key::Minus         /* - */
#define KEY_PERIOD          ::Renderer::Key::Period        /* . */
#define KEY_SLASH           ::Renderer::Key::Slash         /* / */
#define KEY_0               ::Renderer::Key::D0
#define KEY_1               ::Renderer::Key::D1
#define KEY_2               ::Renderer::Key::D2
#define KEY_3               ::Renderer::Key::D3
#define KEY_4               ::Renderer::Key::D4
#define KEY_5               ::Renderer::Key::D5
#define KEY_6               ::Renderer::Key::D6
#define KEY_7               ::Renderer::Key::D7
#define KEY_8               ::Renderer::Key::D8
#define KEY_9               ::Renderer::Key::D9
#define KEY_SEMICOLON       ::Renderer::Key::Semicolon     /* ; */
#define KEY_EQUAL           ::Renderer::Key::Equal         /* = */
#define KEY_A               ::Renderer::Key::A
#define KEY_B               ::Renderer::Key::B
#define KEY_C               ::Renderer::Key::C
#define KEY_D               ::Renderer::Key::D
#define KEY_E               ::Renderer::Key::E
#define KEY_F               ::Renderer::Key::F
#define KEY_G               ::Renderer::Key::G
#define KEY_H               ::Renderer::Key::H
#define KEY_I               ::Renderer::Key::I
#define KEY_J               ::Renderer::Key::J
#define KEY_K               ::Renderer::Key::K
#define KEY_L               ::Renderer::Key::L
#define KEY_M               ::Renderer::Key::M
#define KEY_N               ::Renderer::Key::N
#define KEY_O               ::Renderer::Key::O
#define KEY_P               ::Renderer::Key::P
#define KEY_Q               ::Renderer::Key::Q
#define KEY_R               ::Renderer::Key::R
#define KEY_S               ::Renderer::Key::S
#define KEY_T               ::Renderer::Key::T
#define KEY_U               ::Renderer::Key::U
#define KEY_V               ::Renderer::Key::V
#define KEY_W               ::Renderer::Key::W
#define KEY_X               ::Renderer::Key::X
#define KEY_Y               ::Renderer::Key::Y
#define KEY_Z               ::Renderer::Key::Z
#define KEY_LEFT_BRACKET    ::Renderer::Key::LeftBracket   /* [ */
#define KEY_BACKSLASH       ::Renderer::Key::Backslash     /* \ */
#define KEY_RIGHT_BRACKET   ::Renderer::Key::RightBracket  /* ] */
#define KEY_GRAVE_ACCENT    ::Renderer::Key::GraveAccent   /* ` */
#define KEY_WORLD_1         ::Renderer::Key::World1        /* non-US #1 */
#define KEY_WORLD_2         ::Renderer::Key::World2        /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE          ::Renderer::Key::Escape
#define KEY_ENTER           ::Renderer::Key::Enter
#define KEY_TAB             ::Renderer::Key::Tab
#define KEY_BACKSPACE       ::Renderer::Key::Backspace
#define KEY_INSERT          ::Renderer::Key::Insert
#define KEY_DELETE          ::Renderer::Key::Delete
#define KEY_RIGHT           ::Renderer::Key::Right
#define KEY_LEFT            ::Renderer::Key::Left
#define KEY_DOWN            ::Renderer::Key::Down
#define KEY_UP              ::Renderer::Key::Up
#define KEY_PAGE_UP         ::Renderer::Key::PageUp
#define KEY_PAGE_DOWN       ::Renderer::Key::PageDown
#define KEY_HOME            ::Renderer::Key::Home
#define KEY_END             ::Renderer::Key::End
#define KEY_CAPS_LOCK       ::Renderer::Key::CapsLock
#define KEY_SCROLL_LOCK     ::Renderer::Key::ScrollLock
#define KEY_NUM_LOCK        ::Renderer::Key::NumLock
#define KEY_PRINT_SCREEN    ::Renderer::Key::PrintScreen
#define KEY_PAUSE           ::Renderer::Key::Pause
#define KEY_F1              ::Renderer::Key::F1
#define KEY_F2              ::Renderer::Key::F2
#define KEY_F3              ::Renderer::Key::F3
#define KEY_F4              ::Renderer::Key::F4
#define KEY_F5              ::Renderer::Key::F5
#define KEY_F6              ::Renderer::Key::F6
#define KEY_F7              ::Renderer::Key::F7
#define KEY_F8              ::Renderer::Key::F8
#define KEY_F9              ::Renderer::Key::F9
#define KEY_F10             ::Renderer::Key::F10
#define KEY_F11             ::Renderer::Key::F11
#define KEY_F12             ::Renderer::Key::F12
#define KEY_F13             ::Renderer::Key::F13
#define KEY_F14             ::Renderer::Key::F14
#define KEY_F15             ::Renderer::Key::F15
#define KEY_F16             ::Renderer::Key::F16
#define KEY_F17             ::Renderer::Key::F17
#define KEY_F18             ::Renderer::Key::F18
#define KEY_F19             ::Renderer::Key::F19
#define KEY_F20             ::Renderer::Key::F20
#define KEY_F21             ::Renderer::Key::F21
#define KEY_F22             ::Renderer::Key::F22
#define KEY_F23             ::Renderer::Key::F23
#define KEY_F24             ::Renderer::Key::F24
#define KEY_F25             ::Renderer::Key::F25

/* Keypad */
#define KEY_KP_0            ::Renderer::Key::KP0
#define KEY_KP_1            ::Renderer::Key::KP1
#define KEY_KP_2            ::Renderer::Key::KP2
#define KEY_KP_3            ::Renderer::Key::KP3
#define KEY_KP_4            ::Renderer::Key::KP4
#define KEY_KP_5            ::Renderer::Key::KP5
#define KEY_KP_6            ::Renderer::Key::KP6
#define KEY_KP_7            ::Renderer::Key::KP7
#define KEY_KP_8            ::Renderer::Key::KP8
#define KEY_KP_9            ::Renderer::Key::KP9
#define KEY_KP_DECIMAL      ::Renderer::Key::KPDecimal
#define KEY_KP_DIVIDE       ::Renderer::Key::KPDivide
#define KEY_KP_MULTIPLY     ::Renderer::Key::KPMultiply
#define KEY_KP_SUBTRACT     ::Renderer::Key::KPSubtract
#define KEY_KP_ADD          ::Renderer::Key::KPAdd
#define KEY_KP_ENTER        ::Renderer::Key::KPEnter
#define KEY_KP_EQUAL        ::Renderer::Key::KPEqual

#define KEY_LEFT_SHIFT      ::Renderer::Key::LeftShift
#define KEY_LEFT_CONTROL    ::Renderer::Key::LeftControl
#define KEY_LEFT_ALT        ::Renderer::Key::LeftAlt
#define KEY_LEFT_SUPER      ::Renderer::Key::LeftSuper
#define KEY_RIGHT_SHIFT     ::Renderer::Key::RightShift
#define KEY_RIGHT_CONTROL   ::Renderer::Key::RightControl
#define KEY_RIGHT_ALT       ::Renderer::Key::RightAlt
#define KEY_RIGHT_SUPER     ::Renderer::Key::RightSuper
#define KEY_MENU            ::Renderer::Key::Menu

// Mouse
#define MOUSE_BUTTON_LEFT       ::Renderer::Button::Left
#define MOUSE_BUTTON_RIGHT      ::Renderer::Button::Right
#define MOUSE_BUTTON_MIDDLE     ::Renderer::Button::Middle
