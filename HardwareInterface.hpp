#pragma once
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <bitset>
#define M_PI 3.14159265358979323846


struct point2D {
	int x = 0;
	int y = 0;

	point2D operator+(const point2D& right)const{
		return {x+right.x,y+right.y};
	}
	point2D operator-(const point2D& right)const{
		return {x-right.x,y-right.y};
	}
	point2D operator*(const point2D& right)const{
		return {x*right.x,y*right.y};
	}
	point2D operator/(const point2D& right)const{
		return {x/right.x,y/right.y};
	}
	point2D operator*(const int& right)const{
		return {x*right,y*right};
	}
	point2D operator/(const int& right)const{
		return {x/right,y/right};
	}
};
struct point2Dd {
	double x = 0;
	double y = 0;
};
struct point2Du {
	unsigned int x = 0;
	unsigned int y = 0;
};
struct point2Ds {
	short x = 0;
	short y = 0;
};
struct point2Dus {
	unsigned short x = 0;
	unsigned short y = 0;
};
struct point3Di {
	int x = 0;
	int y = 0;
	int z = 0;

	bool operator==(const point3Di& right) const {
		return x == right.x && y == right.y && z == right.z;
	}
	bool operator!=(const point3Di& right) const {
		return !(*this == right);
	}
};
struct point3Dl {
	long x = 0;
	long y = 0;
	long z = 0;
};
struct point3Dd {
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

namespace HI2 {
	// New and more polished version of HI, breaks compat
	struct Color
	{
		unsigned char r = 0, g = 0, b = 0, a = 0;
		Color() {}
		Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :r(r), g(g), b(b), a(a) {}

		static Color Black;
		static Color White;
		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Yellow;
		static Color Orange;
		static Color Pink;
		static Color LightestGrey;
		static Color LightGrey;
		static Color Grey;
		static Color DarkGrey;
		static Color DarkestGrey;
		static Color Transparent;

	};


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
		Texture(std::vector<std::filesystem::path> paths, double step = 0.2);
		void step(double s);
		void clean();

	private:
		std::vector<void*> _animationTextures;
		double _currentS = 0;
		double _sPerFrame = 0.2;
		unsigned short _currentFrame = 0;

		void* _texture = nullptr;
		std::filesystem::path _path;

		friend void drawTexture(Texture& texture, int posX, int posY,
			double scale, double rotation);
		friend void drawTexturePart(Texture& texture, point2D texturePartStart,
			int sizeX, int sizeY, point2D displayPos);
		friend Texture mergeTextures(Texture& originTexture,
			Texture& destinationTexture,
			point2D position);
		friend void setTextureColorMod(Texture& texture, Color color);
	};

	enum PLATFORM {
		PLATFORM_PSVITA,
		PLATFORM_NINTENDO3DS,
		PLATFORM_PC,
		PLATFORM_SWITCH,
	};
	constexpr int BUTTON_SIZE = 128;
	enum BUTTON {
		BUTTON_A = 0,				///< A
		BUTTON_B = 1,				///< B
		BUTTON_X = 2,				///< X
		BUTTON_Y = 3,				///< Y
		BUTTON_LSTICK = 4,		///< Left Stick Button
		BUTTON_RSTICK = 5,		///< Right Stick Button
		BUTTON_L = 6,				///< L
		BUTTON_R = 7,				///< R
		BUTTON_ZL = 8,			///< ZL
		BUTTON_ZR = 9,			///< ZR
		BUTTON_PLUS = 10,			///< Plus
		BUTTON_MINUS = 11,		///< Minus
		BUTTON_DLEFT = 12,		///< D-Pad Left
		BUTTON_DUP = 13,			///< D-Pad Up
		BUTTON_DRIGHT = 14,		///< D-Pad Right
		BUTTON_DDOWN = 15,		///< D-Pad Down
		BUTTON_LSTICK_LEFT = 16,  ///< Left Stick Left
		BUTTON_LSTICK_UP = 17,	///< Left Stick Up
		BUTTON_LSTICK_RIGHT = 18, ///< Left Stick Right
		BUTTON_LSTICK_DOWN = 19,  ///< Left Stick Down
		BUTTON_RSTICK_LEFT = 20,  ///< Right Stick Left
		BUTTON_RSTICK_UP = 21,	///< Right Stick Up
		BUTTON_RSTICK_RIGHT = 22, ///< Right Stick Right
		BUTTON_RSTICK_DOWN = 23,  ///< Right Stick Down
		BUTTON_SL_LEFT = 24,		///< SL on Left Joy-Con
		BUTTON_SR_LEFT = 25,		///< SR on Left Joy-Con
		BUTTON_SL_RIGHT = 26,		///< SL on Right Joy-Con
		BUTTON_SR_RIGHT = 27,		///< SR on Right Joy-Con

		// Pseudo-key for at least one finger on the touch screen
		TOUCH = 28,

		//PC extra keys
		KEY_Q = 29,
		KEY_W = 30,
		KEY_E = 31,
		KEY_R = 32,
		KEY_T = 33,
		KEY_Y = 34,
		KEY_U = 35,
		KEY_I = 36,
		KEY_O = 37,
		KEY_P = 38,
		KEY_A = 39,
		KEY_S = 40,
		KEY_D = 41,
		KEY_F = 42,
		KEY_G = 43,
		KEY_H = 44,
		KEY_J = 45,
		KEY_K = 46,
		KEY_L = 47,
		KEY_Z = 48,
		KEY_X = 49,
		KEY_C = 50,
		KEY_V = 51,
		KEY_B = 52,
		KEY_N = 53,
		KEY_M = 54,

		KEY_F11 = 55,
		KEY_ESCAPE = 56,
		KEY_BACKSPACE = 57,
		KEY_SPACE = 58,
		KEY_SHIFT = 59,

		// Buttons by orientation (for single Joy-Con), also works with
		// Joy-Con pairs, Pro Controller
		BUTTON_JOYCON_RIGHT = 0,
		BUTTON_JOYCON_DOWN = 1,
		BUTTON_JOYCON_UP = 2,
		BUTTON_JOYCON_LEFT = 3,

		// Generic catch-all directions, also works for single Joy-Con
		BUTTON_UP = BUTTON_DUP, // | BUTTON_LSTICK_UP | BUTTON_RSTICK_UP, ///< D-Pad Up or Sticks Up
		BUTTON_DOWN = BUTTON_DDOWN,//| BUTTON_LSTICK_DOWN | BUTTON_RSTICK_DOWN, ///< D-Pad Down or Sticks Down
		BUTTON_LEFT = BUTTON_DLEFT,//| BUTTON_LSTICK_LEFT | BUTTON_RSTICK_LEFT, ///< D-Pad Left or Sticks Left
		BUTTON_RIGHT = BUTTON_DRIGHT, // | BUTTON_LSTICK_RIGHT | BUTTON_RSTICK_RIGHT,		 ///< D-Pad Right or Sticks Right
		BUTTON_SL = BUTTON_SL_LEFT, // | BUTTON_SL_RIGHT, ///< SL on Left or Right Joy-Con
		BUTTON_SR = BUTTON_SR_LEFT, // | BUTTON_SR_RIGHT, ///< SR on Left or Right Joy-Con

		KEY_ACCEPT = BUTTON_A,
		KEY_CANCEL = BUTTON_B,
	};
	enum JOYSTICK {
		JOY_LEFT,
		JOY_RIGHT,
	};

	// logger
	void logWrite(std::string s);

	// filesystem
	std::filesystem::path getDataPath();
	std::filesystem::path getSavesPath();

	// HardwareInfo
	int getScreenHeight();
	int getScreenWidth();
	PLATFORM getPlatform();

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
	point2D getJoystickPos(JOYSTICK joystick);
	point2D getTouchPos();

	// sound
	void playSound(Audio& audio, float volume = -1);

	// graphics

	void startFrame();
	void toggleFullscreen();
	void setBackgroundColor(Color color);
	void drawText(Font& font, std::string text, point2D pos, int size,
		Color color);
	void setTextureColorMod(Texture& texture, Color color);
	void drawTexture(Texture& texture, int posX, int posY, double scale = 1, double radians = .0f);
	void drawTexturePart(Texture& texture, point2D texturePartStart, int sizeX,
		int sizeY, point2D displayPos);
	Texture mergeTextures(Texture& originTexture, Texture& destinationTexture,
		point2D position);
	void drawRectangle(point2D pos, int width, int height, Color color); // draw a filled rectangle
	void drawEmptyRectangle(point2D pos, int width, int height, Color color);
	void drawPixel(point2D pos, Color color);
	void endFrame();

	void setCursorPos(point2D pos);

} // namespace HI2