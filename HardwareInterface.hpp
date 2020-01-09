#pragma once
#include <cmath>
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
	point3Di operator*(const int& i) const{
		return {x*i,y*i,z*i};
	}
	point3Di operator/(const int& i) const{
		return {x/i,y/i,z/i};
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
	point3Dd(double x, double y, double z):x(x),y(y),z(z){}
	point3Dd(const point3Di& p){
		x=p.x;
		y=p.y;
		z=p.z;
	}
	point3Dd operator*(const double& i) const{
		return {x*i,y*i,z*i};
	}
	point3Dd operator/(const double& i) const{
		return {x/i,y/i,z/i};
	}
	operator point3Di() const{
		return {(int)floor(x),(int)floor(y),(int)floor(z)};
	}
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
		Texture(point2D size);
		void clean();

	private:
		void* _texture = nullptr;
		std::filesystem::path _path;

		friend void drawTexture(Texture& texture, int posX, int posY,
			double scale, double rotation);
		friend void drawTextureOverlap(Texture& texture, int posX, int posY,
			double scale, double rotation);
		friend void drawTexture(Texture& texture, int posX, int posY, point2D size, point2D startPos,
			double scale, double rotation);
		friend void drawTextureOverlap(Texture& texture, int posX, int posY, point2D size, point2D startPos,
			double scale, double rotation);
		friend Texture mergeTextures(Texture& originTexture,
			Texture& destinationTexture,
			point2D position);
		friend void setTextureColorMod(Texture& texture, Color color);
		friend void setRenderTarget(Texture* t, bool b);
		friend point2D getTextureSize(Texture& texture);
	};

	enum PLATFORM {
		PLATFORM_PSVITA,
		PLATFORM_NINTENDO3DS,
		PLATFORM_PC,
		PLATFORM_SWITCH,
	};
	constexpr int BUTTON_SIZE = 128;
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

		// Pseudo-key for at least one finger on the touch screen
		TOUCH,

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

		KEY_MOUSEWHEEL_UP,
		KEY_MOUSEWHEEL_DOWN,


		// Buttons by orientation (for single Joy-Con), also works with
		// Joy-Con pairs, Pro Controller
		BUTTON_JOYCON_RIGHT = BUTTON_A,
		BUTTON_JOYCON_DOWN = BUTTON_B,
		BUTTON_JOYCON_UP = BUTTON_X,
		BUTTON_JOYCON_LEFT = BUTTON_Y,

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
	constexpr PLATFORM getPlatform();

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
	void drawTexture(Texture& texture, int posX, int posY, double scale = 1, double radians = 0);
	void drawTextureOverlap(Texture& texture, int posX, int posY, double scale = 1, double radians = 0);
	void drawTexture(Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale = 1, double radians = 0);
	void drawTextureOverlap(Texture& texture, int posX, int posY, point2D size, point2D startPos, double scale = 1, double radians = 0);
	Texture mergeTextures(Texture& originTexture, Texture& destinationTexture,
		point2D position);
	void drawRectangle(point2D pos, int width, int height, Color color); // draw a filled rectangle
	void drawEmptyRectangle(point2D pos, int width, int height, Color color);
	void drawPixel(point2D pos, Color color);
	void setRenderTarget(Texture* t = nullptr, bool clear = false);
	void endFrame();
	point2D getTextureSize(Texture& texture);

	void setCursorPos(point2D pos);

	void createDirectories(std::filesystem::path p);
	void deleteDirectory(std::filesystem::path p);

} // namespace HI2