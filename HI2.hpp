#pragma once
#include <cmath>
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <bitset>
#include <cstdlib>
#include <algorithm>
#include "platform/types.hpp"

struct HI2PCTextureAccess;

namespace HI2 {
	// New and more polished version of HI, breaks compat

	enum class FLIP
	{
		NONE = 0,
		H = 1,
		V = 2,
	};
	
	// The colour type moved to platform/types.hpp; this alias keeps HI2 itself compiling
	// until the rest of it goes (plans/UI_PLATFORM_PLAN.md).
	using Color = ::Color;


	class Audio {
	public:
		Audio();
		Audio(std::filesystem::path path, bool loop = false, float volume = 1);
		void clean();

	private:
		void* _audio = nullptr;
		bool _loop = false;
		float _volume = 1;
		std::filesystem::path _path;

		friend void playSound(Audio& audio, float volume);
	};

	class Font {
	public:
		Font();
		Font(std::filesystem::path path);
		void clean();

	private:
		void* _font = nullptr;
		std::string _name;
		std::filesystem::path _path;

		friend void drawText(Font& font, std::string text, point2D pos,
			int size, Color color);
	};

	class Texture {
	public:
		Texture();
		Texture(std::filesystem::path path);
		Texture(point2D size);
		void clean(){}

	private:
		class _internalWeakTextureRAIIWrapper{
			public:
			_internalWeakTextureRAIIWrapper(void* pointer);
			virtual ~_internalWeakTextureRAIIWrapper();
			void* get() const;
			protected:
			void* _texture = nullptr;
		};
		class _internalTextureRAIIWrapper : public _internalWeakTextureRAIIWrapper{
		public:
			_internalTextureRAIIWrapper(void* pointer): _internalWeakTextureRAIIWrapper(pointer){};
			virtual ~_internalTextureRAIIWrapper() override;
		};

		std::shared_ptr<_internalWeakTextureRAIIWrapper> _texture;
		std::filesystem::path _path;

		friend void drawTexture(const Texture& texture, int posX, int posY,
			double scale, double rotation, FLIP flip);
		friend void drawTextureOverlap(const Texture& texture, int posX, int posY,
			double scale, double rotation, FLIP flip);
		friend void drawTextureF(const Texture& texture, float posX, float posY,
			double scale, double rotation, FLIP flip);
		friend void drawTexture(const Texture& texture, int posX, int posY, point2D size, point2D startPos,
			double scale, double rotation, FLIP flip);
		friend void drawTextureOverlap(const Texture& texture, int posX, int posY, point2D size, point2D startPos,
			double scale, double rotation, FLIP flip);
		friend void drawTextureF(const Texture& texture, float posX, float posY, point2D size, point2D startPos,
			double scale, double rotation, FLIP flip);
		friend Texture mergeTextures(Texture& originTexture,
			Texture& destinationTexture,
			point2D position);
		friend void setTextureColorMod(Texture& texture, Color color);
		friend void setRenderTarget(Texture* t, bool b);
		friend Texture getRenderTarget();
		friend point2D getTextureSize(Texture& texture);
		friend struct ::HI2PCTextureAccess;
	};

	enum BUTTON {
		BUTTON_A,				///< A
		BUTTON_B,				///< B
		BUTTON_X,				///< X
		BUTTON_Y,				///< Y
		BUTTON_LSTICK,		///< Left Stick Button
		BUTTON_RSTICK,		///< Right Stick Button
		BUTTON_L,				///< L
		BUTTON_R,				///< R
		BUTTON_ZL,			///< ZL
		BUTTON_ZR,			///< ZR
		BUTTON_PLUS,			///< Plus
		BUTTON_MINUS,		///< Minus
		BUTTON_DLEFT,		///< D-Pad Left
		BUTTON_DUP,			///< D-Pad Up
		BUTTON_DRIGHT,		///< D-Pad Right
		BUTTON_DDOWN,		///< D-Pad Down
		BUTTON_LSTICK_LEFT,  ///< Left Stick Left
		BUTTON_LSTICK_UP,	///< Left Stick Up
		BUTTON_LSTICK_RIGHT, ///< Left Stick Right
		BUTTON_LSTICK_DOWN,  ///< Left Stick Down
		BUTTON_RSTICK_LEFT,  ///< Right Stick Left
		BUTTON_RSTICK_UP,	///< Right Stick Up
		BUTTON_RSTICK_RIGHT, ///< Right Stick Right
		BUTTON_RSTICK_DOWN,  ///< Right Stick Down
		BUTTON_SL_LEFT,		///< SL on Left Joy-Con
		BUTTON_SR_LEFT,		///< SR on Left Joy-Con
		BUTTON_SL_RIGHT,		///< SL on Right Joy-Con
		BUTTON_SR_RIGHT,		///< SR on Right Joy-Con



		//PC extra keys
		KEY_Q,
		KEY_W,
		KEY_E,
		KEY_R,
		KEY_T,
		KEY_Y,
		KEY_U,
		KEY_I,
		KEY_O,
		KEY_P,
		KEY_A,
		KEY_S,
		KEY_D,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_Z,
		KEY_X,
		KEY_C,
		KEY_V,
		KEY_B,
		KEY_N,
		KEY_M,

		KEY_F11,
		KEY_ESCAPE,
		KEY_BACKSPACE,
		KEY_SPACE,
		KEY_SHIFT,
		KEY_CONTROL,
		KEY_ENTER,
		KEY_CONSOLE,
		KEY_TAB,
		KEY_HOME,
		KEY_END,

		KEY_PEIXMARTI_RIGHT,
		KEY_PEIXMARTI_LEFT,

		KEY_UP,
		KEY_DOWN,
		KEY_LEFT,
		KEY_RIGHT,

		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,

		KEY_DASH,
		KEY_PLUS,

		KEY_MOUSEWHEEL_UP,
		KEY_MOUSEWHEEL_DOWN,
		KEY_LEFTCLICK,
		KEY_RIGHTCLICK,

		TOUCHSCREEN,
		// Pseudo-key for at least one finger on the touch screen
		TOUCH,

		// Generic catch-all directions, also works for single Joy-Con
		UP,// = BUTTON_DUP, // | BUTTON_LSTICK_UP | BUTTON_RSTICK_UP, ///< D-Pad Up or Sticks Up
		DOWN,// = BUTTON_DDOWN,//| BUTTON_LSTICK_DOWN | BUTTON_RSTICK_DOWN, ///< D-Pad Down or Sticks Down
		LEFT,// = BUTTON_DLEFT,//| BUTTON_LSTICK_LEFT | BUTTON_RSTICK_LEFT, ///< D-Pad Left or Sticks Left
		RIGHT,// = BUTTON_DRIGHT, // | BUTTON_LSTICK_RIGHT | BUTTON_RSTICK_RIGHT,		 ///< D-Pad Right or Sticks Right
		BUTTON_SL,// = BUTTON_SL_LEFT, // | BUTTON_SL_RIGHT, ///< SL on Left or Right Joy-Con
		BUTTON_SR,// = BUTTON_SR_LEFT, // | BUTTON_SR_RIGHT, ///< SR on Left or Right Joy-Con

		ACCEPT,// = BUTTON_A,
		CANCEL,// = BUTTON_B,



		BUTTON_SIZE,
	};

	enum class JOYSTICK {
		LEFT,
		RIGHT,
	};

	// logger
	void logWrite(std::string s);

	// HardwareInfo
	int getScreenHeight();
	int getScreenWidth();

	// System
	void systemInit();
	void systemFini();
	void consoleInit();
	void consoleInit(std::filesystem::path path);
	void consoleFini();
	void consoleClear();
	void sleepThread(unsigned long ns);
	bool aptMainLoop();

	// input
	const std::bitset<BUTTON_SIZE>& getKeysDown();
	const std::bitset<BUTTON_SIZE>& getKeysUp();
	const std::bitset<BUTTON_SIZE>& getKeysHeld();
	// UTF-8 text typed during the last polled frame (from the OS/IME, so it honours
	// keyboard layout, shift and dead keys). Empty when nothing was typed. Control
	// keys (enter/backspace/arrows) are NOT reported here — read those from the
	// button bitsets. Used for free text entry such as the in-game computer editor.
	const std::string& getTextInput();
	point2D getJoystickPos(JOYSTICK joystick);
	point2D getTouchPos();
	void setMouseRelative(bool rel);
	point2D getRelativeMouseMovement();

	// sound
	void playSound(Audio& audio, float volume = -1);

	// graphics

	void startFrame();
	void toggleFullscreen();
	void setBackgroundColor(Color color);
	void drawText(Font& font, std::string text, point2D pos, int size,
		Color color);
	void setTextureColorMod(Texture& texture, Color color);
	void drawTexture(const Texture& texture, int posX, int posY, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureOverlap(const Texture& texture, int posX, int posY, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureF(const Texture& texture, float posX, float posY, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTexture(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureOverlap(const Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	void drawTextureF(const Texture& texture, float posX, float posY, point2D size, point2D startPos, double scale = 1, double radians = 0, HI2::FLIP flip = HI2::FLIP::NONE);
	Texture mergeTextures(Texture& originTexture, Texture& destinationTexture,
		point2D position);
	void drawRectangle(point2D pos, int width, int height, Color color); // draw a filled rectangle
	void drawEmptyRectangle(point2D pos, int width, int height, Color color);
	void drawEmptyRectangle(point2D pos, int width, int height, int strokewidth, Color color);
	void drawLine(point2D start, point2D end, Color color);
	void drawLines(const std::vector<point2D>& points, Color color);

	void drawPixel(point2D pos, Color color);
	void setRenderTarget(Texture* t = nullptr, bool clear = false);
	Texture getRenderTarget();

	void endFrame();
	point2D getTextureSize(Texture& texture);

	void setCursorPos(point2D pos);

	void createDirectories(std::filesystem::path p);
	void deleteDirectory(std::filesystem::path p);

	void setClipboard(std::string mucho_texto);
	std::string getClipboard();

	inline void calculateAggregators(std::bitset<HI2::BUTTON_SIZE>& buttons){
		buttons[HI2::BUTTON::DOWN] = buttons[HI2::BUTTON::BUTTON_DDOWN] || buttons[HI2::BUTTON::KEY_S] || buttons[HI2::BUTTON::BUTTON_LSTICK_DOWN];
		buttons[HI2::BUTTON::UP] = buttons[HI2::BUTTON::BUTTON_DUP] || buttons[HI2::BUTTON::KEY_W] || buttons[HI2::BUTTON::BUTTON_LSTICK_UP];
		buttons[HI2::BUTTON::LEFT] = buttons[HI2::BUTTON::BUTTON_DLEFT] || buttons[HI2::BUTTON::KEY_A] || buttons[HI2::BUTTON::BUTTON_LSTICK_LEFT];
		buttons[HI2::BUTTON::RIGHT] = buttons[HI2::BUTTON::BUTTON_DRIGHT] || buttons[HI2::BUTTON::KEY_D] || buttons[HI2::BUTTON::BUTTON_LSTICK_RIGHT];

		buttons[HI2::BUTTON::ACCEPT] = buttons[HI2::BUTTON::BUTTON_A] || buttons[HI2::BUTTON::KEY_ENTER];
		buttons[HI2::BUTTON::CANCEL] = buttons[HI2::BUTTON::BUTTON_PLUS] || buttons[HI2::BUTTON::KEY_ESCAPE];
		buttons[HI2::BUTTON::TOUCH] = buttons[HI2::BUTTON::TOUCHSCREEN] || buttons[HI2::BUTTON::KEY_LEFTCLICK];
	}

} // namespace HI2
