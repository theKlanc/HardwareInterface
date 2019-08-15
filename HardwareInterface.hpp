#pragma once
#include <filesystem>
#include <fstream>
#include <string>

#define BIT(n) (1U << (n))

struct point2D {
	int x = 0;
	int y = 0;
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

	bool operator==(const point3Di &right) const {
		return x == right.x && y == right.y && z == right.z;
	}
	bool operator!=(const point3Di &right) const {
		return !(*this==right);
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
		unsigned short r=0, g=0, b=0, a=0;
		Color(){}
		Color(unsigned short r,unsigned short g,unsigned short b,unsigned short a):r(r),g(g),b(b),a(a){}
	};

	class Audio {
	  public:
		Audio();
		Audio(std::filesystem::path path, bool loop = false, float volume = 1);
		void clean();

	  private:
		void *_audio = nullptr;
		bool _loop = false;
		float _volume = 1;
		std::filesystem::path _path;

		friend void playSound(Audio &audio, float volume);
	};

	class Font {
	  public:
		Font();
		Font(std::filesystem::path path, int size = 12);
		void clean();

	  private:
		void *_font = nullptr;
		std::string _name;
		std::filesystem::path _path;

		friend void drawText(Font &font, std::string text, point2D pos,
							 int size, Color color);
	};

	class Texture {
	  public:
		Texture();
		Texture(std::filesystem::path path);
		void clean();

	  private:
		void *_texture = nullptr;
		std::filesystem::path _path;

		friend void drawTexture(Texture &texture, int posX, int posY,
								double scale, double rotation);
		friend void drawTexturePart(Texture &texture, point2D texturePartStart,
									int sizeX, int sizeY, point2D displayPos);
		friend Texture mergeTextures(Texture &originTexture,
									 Texture &destinationTexture,
									 point2D position);
	};

	enum PLATFORM {
		PLATFORM_PSVITA,
		PLATFORM_NINTENDO3DS,
		PLATFORM_PC,
		PLATFORM_SWITCH,
	};
	enum BUTTON {
		KEY_A = BIT(0),				///< A
		KEY_B = BIT(1),				///< B
		KEY_X = BIT(2),				///< X
		KEY_Y = BIT(3),				///< Y
		KEY_LSTICK = BIT(4),		///< Left Stick Button
		KEY_RSTICK = BIT(5),		///< Right Stick Button
		KEY_L = BIT(6),				///< L
		KEY_R = BIT(7),				///< R
		KEY_ZL = BIT(8),			///< ZL
		KEY_ZR = BIT(9),			///< ZR
		KEY_PLUS = BIT(10),			///< Plus
		KEY_MINUS = BIT(11),		///< Minus
		KEY_DLEFT = BIT(12),		///< D-Pad Left
		KEY_DUP = BIT(13),			///< D-Pad Up
		KEY_DRIGHT = BIT(14),		///< D-Pad Right
		KEY_DDOWN = BIT(15),		///< D-Pad Down
		KEY_LSTICK_LEFT = BIT(16),  ///< Left Stick Left
		KEY_LSTICK_UP = BIT(17),	///< Left Stick Up
		KEY_LSTICK_RIGHT = BIT(18), ///< Left Stick Right
		KEY_LSTICK_DOWN = BIT(19),  ///< Left Stick Down
		KEY_RSTICK_LEFT = BIT(20),  ///< Right Stick Left
		KEY_RSTICK_UP = BIT(21),	///< Right Stick Up
		KEY_RSTICK_RIGHT = BIT(22), ///< Right Stick Right
		KEY_RSTICK_DOWN = BIT(23),  ///< Right Stick Down
		KEY_SL_LEFT = BIT(24),		///< SL on Left Joy-Con
		KEY_SR_LEFT = BIT(25),		///< SR on Left Joy-Con
		KEY_SL_RIGHT = BIT(26),		///< SL on Right Joy-Con
		KEY_SR_RIGHT = BIT(27),		///< SR on Right Joy-Con

		// Pseudo-key for at least one finger on the touch screen
		KEY_TOUCH = BIT(28),

		// Buttons by orientation (for single Joy-Con), also works with
		// Joy-Con pairs, Pro Controller
		KEY_JOYCON_RIGHT = BIT(0),
		KEY_JOYCON_DOWN = BIT(1),
		KEY_JOYCON_UP = BIT(2),
		KEY_JOYCON_LEFT = BIT(3),

		// Generic catch-all directions, also works for single Joy-Con
		KEY_UP =
			KEY_DUP | KEY_LSTICK_UP | KEY_RSTICK_UP, ///< D-Pad Up or Sticks Up
		KEY_DOWN = KEY_DDOWN | KEY_LSTICK_DOWN |
				   KEY_RSTICK_DOWN, ///< D-Pad Down or Sticks Down
		KEY_LEFT = KEY_DLEFT | KEY_LSTICK_LEFT |
				   KEY_RSTICK_LEFT, ///< D-Pad Left or Sticks Left
		KEY_RIGHT = KEY_DRIGHT | KEY_LSTICK_RIGHT |
					KEY_RSTICK_RIGHT,		 ///< D-Pad Right or Sticks Right
		KEY_SL = KEY_SL_LEFT | KEY_SL_RIGHT, ///< SL on Left or Right Joy-Con
		KEY_SR = KEY_SR_LEFT | KEY_SR_RIGHT, ///< SR on Left or Right Joy-Con
	};
	enum JOYSTICK {
		JOY_LEFT,
		JOY_RIGHT,
	};

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
	unsigned long getKeysDown();
	unsigned long getKeysUp();
	unsigned long getKeysHeld();
	point2D getJoystickPos(JOYSTICK joystick);
	point2D getTouchPos();

	// sound
	void playSound(Audio &audio, float volume = -1);

	// graphics

	void startFrame();
	void setBackgroundColor(Color color);
	void drawText(Font &font, std::string text, point2D pos, int size,
				  Color color);
	void drawTexture(Texture &texture, int posX, int posY, double scale = 1, double radians = .0f);
	void drawTexturePart(Texture &texture, point2D texturePartStart, int sizeX,
						 int sizeY, point2D displayPos);
	Texture mergeTextures(Texture &originTexture, Texture &destinationTexture,
						  point2D position);
	void drawRectangle(point2D pos, int width, int height, Color color);
	void drawPixel(point2D pos, Color color);
	void endFrame();

} // namespace HI2