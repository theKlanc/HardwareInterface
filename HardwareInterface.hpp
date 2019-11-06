#pragma once
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#define M_PI 3.14159265358979323846
#define BIT(n) (1ULL << (n))

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
	enum BUTTON : unsigned long long{
		BUTTON_A = BIT(0ULL),				///< A
		BUTTON_B = BIT(1ULL),				///< B
		BUTTON_X = BIT(2ULL),				///< X
		BUTTON_Y = BIT(3ULL),				///< Y
		BUTTON_LSTICK = BIT(4ULL),		///< Left Stick Button
		BUTTON_RSTICK = BIT(5ULL),		///< Right Stick Button
		BUTTON_L = BIT(6ULL),				///< L
		BUTTON_R = BIT(7ULL),				///< R
		BUTTON_ZL = BIT(8ULL),			///< ZL
		BUTTON_ZR = BIT(9ULL),			///< ZR
		BUTTON_PLUS = BIT(10ULL),			///< Plus
		BUTTON_MINUS = BIT(11ULL),		///< Minus
		BUTTON_DLEFT = BIT(12ULL),		///< D-Pad Left
		BUTTON_DUP = BIT(13ULL),			///< D-Pad Up
		BUTTON_DRIGHT = BIT(14ULL),		///< D-Pad Right
		BUTTON_DDOWN = BIT(15ULL),		///< D-Pad Down
		BUTTON_LSTICK_LEFT = BIT(16ULL),  ///< Left Stick Left
		BUTTON_LSTICK_UP = BIT(17ULL),	///< Left Stick Up
		BUTTON_LSTICK_RIGHT = BIT(18ULL), ///< Left Stick Right
		BUTTON_LSTICK_DOWN = BIT(19ULL),  ///< Left Stick Down
		BUTTON_RSTICK_LEFT = BIT(20ULL),  ///< Right Stick Left
		BUTTON_RSTICK_UP = BIT(21ULL),	///< Right Stick Up
		BUTTON_RSTICK_RIGHT = BIT(22ULL), ///< Right Stick Right
		BUTTON_RSTICK_DOWN = BIT(23ULL),  ///< Right Stick Down
		BUTTON_SL_LEFT = BIT(24ULL),		///< SL on Left Joy-Con
		BUTTON_SR_LEFT = BIT(25ULL),		///< SR on Left Joy-Con
		BUTTON_SL_RIGHT = BIT(26ULL),		///< SL on Right Joy-Con
		BUTTON_SR_RIGHT = BIT(27ULL),		///< SR on Right Joy-Con

		// Pseudo-key for at least one finger on the touch screen
		TOUCH = BIT(28ULL),

		//PC extra keys
		KEY_Q = BIT(29ULL),
		KEY_W = BIT(30ULL),
		KEY_E = BIT(31ULL),
		KEY_R = BIT(32ULL),
		KEY_T = BIT(33ULL),
		KEY_Y = BIT(34ULL),
		KEY_U = BIT(35ULL),
		KEY_I = BIT(36ULL),
		KEY_O = BIT(37ULL),
		KEY_P = BIT(38ULL),
		KEY_A = BIT(39ULL),
		KEY_S = BIT(40ULL),
		KEY_D = BIT(41ULL),
		KEY_F = BIT(42ULL),
		KEY_G = BIT(43ULL),
		KEY_H = BIT(44ULL),
		KEY_J = BIT(45ULL),
		KEY_K = BIT(46ULL),
		KEY_L = BIT(47ULL),
		KEY_Z = BIT(48ULL),
		KEY_X = BIT(49ULL),
		KEY_C = BIT(50ULL),
		KEY_V = BIT(51ULL),
		KEY_B = BIT(52ULL),
		KEY_N = BIT(53ULL),
		KEY_M = BIT(54ULL),

		KEY_F11 = BIT(55ULL),
		KEY_ESCAPE = BIT(56ULL),
		KEY_BACKSPACE = BIT(57ULL),
		KEY_SPACE = BIT(58ULL),
		KEY_SHIFT = BIT(59ULL),

		// Buttons by orientation (for single Joy-Con), also works with
		// Joy-Con pairs, Pro Controller
		BUTTON_JOYCON_RIGHT = BIT(0ULL),
		BUTTON_JOYCON_DOWN = BIT(1ULL),
		BUTTON_JOYCON_UP = BIT(2ULL),
		BUTTON_JOYCON_LEFT = BIT(3ULL),

		// Generic catch-all directions, also works for single Joy-Con
		BUTTON_UP =
		BUTTON_DUP | BUTTON_LSTICK_UP | BUTTON_RSTICK_UP, ///< D-Pad Up or Sticks Up
		BUTTON_DOWN = BUTTON_DDOWN | BUTTON_LSTICK_DOWN |
		BUTTON_RSTICK_DOWN, ///< D-Pad Down or Sticks Down
		BUTTON_LEFT = BUTTON_DLEFT | BUTTON_LSTICK_LEFT |
		BUTTON_RSTICK_LEFT, ///< D-Pad Left or Sticks Left
		BUTTON_RIGHT = BUTTON_DRIGHT | BUTTON_LSTICK_RIGHT |
		BUTTON_RSTICK_RIGHT,		 ///< D-Pad Right or Sticks Right
		BUTTON_SL = BUTTON_SL_LEFT | BUTTON_SL_RIGHT, ///< SL on Left or Right Joy-Con
		BUTTON_SR = BUTTON_SR_LEFT | BUTTON_SR_RIGHT, ///< SR on Left or Right Joy-Con

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
	unsigned long long getKeysDown();
	unsigned long long getKeysUp();
	unsigned long long getKeysHeld();
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