#pragma once
#include <fstream>
#include <filesystem>
#include <string>

#define RGBA8(r, g, b, a)                                                      \
	((((a)&0xFF) << 24) | (((b)&0xFF) << 16) | (((g)&0xFF) << 8) |             \
	 (((r)&0xFF) << 0))
#define BIT(n) (1U << (n))

struct point2D {
	int x = 0;
	int y = 0;
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
};
struct point3Df {
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
};

namespace HI2 {
	// New and more polished version of HI, breaks compat
	typedef int Color;
	class Font {
	  public:
		Font();
		~Font();
		Font(std::filesystem::path path, int size=12);
		Font& operator=(Font other);
		void* get();

	  private:
		void *_font=nullptr;
		std::string _name;
		std::filesystem::path _path;
	};
	class Texture {
	  public:
		Texture();
		~Texture();
		Texture(std::filesystem::path path);
		Texture& operator=(Texture other);
		void* get();

	  private:
		void *_texture = nullptr;
		std::filesystem::path _path;
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

		// Buttons by orientation (for single Joy-Con), also works with Joy-Con
		// pairs, Pro Controller
		KEY_JOYCON_RIGHT = BIT(0),
		KEY_JOYCON_DOWN = BIT(1),
		KEY_JOYCON_UP = BIT(2),
		KEY_JOYCON_LEFT = BIT(3),

		// Generic catch-all directions, also works for single Joy-Con
		KEY_UP = KEY_DUP | KEY_LSTICK_UP | KEY_RSTICK_UP, ///< D-Pad Up or Sticks Up
		KEY_DOWN = KEY_DDOWN | KEY_LSTICK_DOWN | KEY_RSTICK_DOWN, ///< D-Pad Down or Sticks Down
		KEY_LEFT = KEY_DLEFT | KEY_LSTICK_LEFT | KEY_RSTICK_LEFT, ///< D-Pad Left or Sticks Left
		KEY_RIGHT = KEY_DRIGHT | KEY_LSTICK_RIGHT | KEY_RSTICK_RIGHT,		 ///< D-Pad Right or Sticks Right
		KEY_SL = KEY_SL_LEFT | KEY_SL_RIGHT, ///< SL on Left or Right Joy-Con
		KEY_SR = KEY_SR_LEFT | KEY_SR_RIGHT, ///< SR on Left or Right Joy-Con
	};
	enum JOYSTICK{
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

	// graphics
	short getR(Color color);
	short getG(Color color);
	short getB(Color color);
	short getA(Color color);

	void startFrame();
	void setBackgroundColor(Color color);
	void drawText(Font& font, std::string text, point2D pos, int size, Color color);
	void drawTexture(Texture &texture, int posX, int posY);

	void drawTexturePart(Texture& texture, point2D texturePartStart, int sizeX, int sizeY, point2D displayPos);
	Texture mergeTextures(Texture& originTexture, Texture& destinationTexture,  point2D position);
	void drawRectangle(point2D pos, int width, int height, Color color);
	void drawPixel(point2D pos, Color color);
	void endFrame();

	
} // namespace HI2
namespace HardwareInterface {

	inline bool exists_test(const std::string &name) {
		std::ifstream f(name.c_str());
		return f.good();
	}

	typedef void *HITexture;
	typedef unsigned char uint8;
	typedef unsigned int uint32;
	typedef uint32 *HIImage;
	typedef void *HIFont;
	typedef int HIColor;
	enum HI_PLATFORM {
		PLATFORM_PSVITA,
		PLATFORM_NINTENDO3DS,
		PLATFORM_PC,
		PLATFORM_SWITCH,
	};
	enum HI_SCREEN { SCREEN_TOP, SCREEN_BOT };
	enum HI_CIRCLEPAD { CIRCLEPAD_LEFT, CIRCLEPAD_RIGHT };
	enum HI_KEYS {
		HI_KEY_A = BIT(0),			   ///< A
		HI_KEY_B = BIT(1),			   ///< B
		HI_KEY_SELECT = BIT(2),		   ///< Select
		HI_KEY_START = BIT(3),		   ///< Start
		HI_KEY_DRIGHT = BIT(4),		   ///< D-Pad Right
		HI_KEY_DLEFT = BIT(5),		   ///< D-Pad Left
		HI_KEY_DUP = BIT(6),		   ///< D-Pad Up
		HI_KEY_DDOWN = BIT(7),		   ///< D-Pad Down
		HI_KEY_R = BIT(8),			   ///< R
		HI_KEY_L = BIT(9),			   ///< L
		HI_KEY_X = BIT(10),			   ///< X
		HI_KEY_Y = BIT(11),			   ///< Y
		HI_KEY_ZL = BIT(14),		   ///< ZL (New 3DS only)
		HI_KEY_ZR = BIT(15),		   ///< ZR (New 3DS only)
		HI_KEY_TOUCH = BIT(20),		   ///< Touch (Not actually provided by HID)
		HI_KEY_CSTICK_RIGHT = BIT(24), ///< C-Stick Right (New 3DS only)
		HI_KEY_CSTICK_LEFT = BIT(25),  ///< C-Stick Left (New 3DS only)
		HI_KEY_CSTICK_UP = BIT(26),	///< C-Stick Up (New 3DS only)
		HI_KEY_CSTICK_DOWN = BIT(27),  ///< C-Stick Down (New 3DS only)
		HI_KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
		HI_KEY_CPAD_LEFT = BIT(29),	///< Circle Pad Left
		HI_KEY_CPAD_UP = BIT(30),	  ///< Circle Pad Up
		HI_KEY_CPAD_DOWN = BIT(31),	///< Circle Pad Down

		// Generic catch-all directions
		HI_KEY_UP = HI_KEY_DUP | HI_KEY_CPAD_UP, ///< D-Pad Up or Circle Pad Up
		HI_KEY_DOWN =
			HI_KEY_DDOWN | HI_KEY_CPAD_DOWN, ///< D-Pad Down or Circle Pad Down
		HI_KEY_LEFT =
			HI_KEY_DLEFT | HI_KEY_CPAD_LEFT, ///< D-Pad Left or Circle Pad Left
		HI_KEY_RIGHT = HI_KEY_DRIGHT |
					   HI_KEY_CPAD_RIGHT ///< D-Pad Right or Circle Pad Right
	};

	// System
	void systemInit();
	void systemFini();
	void consoleInit();
	void consoleFini();
	void sleepThread(unsigned long ns);
	bool aptMainLoop();

	// input
	void updateHID();
	int getKeysDown();
	int getKeysUp();
	int getKeysHeld();
	void updateTouch(point2D &touch);
	void getCirclePadPos(point2D &circle, HI_CIRCLEPAD circlePadID);

	// graphics
	short getRComponent(HIColor color);
	short getGComponent(HIColor color);
	short getBComponent(HIColor color);
	short getAComponent(HIColor color);

	void startFrame(HI_SCREEN screen);
	void setBackgroundColor(HIColor color);
	HIFont loadFont(std::string path);
	void freeFont(HIFont font);
	void drawText(HIFont font, std::string text, int posX, int posY, int size,
				  HIColor color);
	HITexture loadPngFile(std::string path);
	HITexture loadBmpFile(std::string path);
	void drawTexture(HITexture texture, int posX, int posY);
	void drawImage(HIImage image, int sizeX, int sizeY, int posX,
				   int posY); // HIImage image should be a pointer to an array
							  // of RGBA8 pixels
	void drawTexturePart(HITexture texture, int startX, int startY, int posX,
						 int posY, int sizeX, int sizeY);
	void drawTextureRotate(HITexture texture, int posX, int posY, float angle);
	void mergeTextures(HITexture originTexture, HITexture destinationTexture,
					   short posX, short posY);
	void drawRectangle(int posX, int posY, int width, int height,
					   HIColor color);
	void drawPixel(int posX, int posY, HIColor color);
	HITexture createTexture(int sizeX, int sizeY);
	void freeTexture(HITexture texture);
	void endFrame();
	void swapBuffers();

	// filesystem
	std::string getDataPath();
	std::string getSavesPath();
	bool createDir(std::string path);
	bool fsExists(std::string path);
	bool fsIsDirectory(const std::string path);
	bool copyFile(std::string origin, std::string destination);

	// HardwareInfo
	int getScreenHeight();
	int getScreenWidth();
	HI_PLATFORM getPlatform();

	// DEBUGGING
	void debugPrint(std::string s);
	void debugPrint(int n);
	void debugPrint(unsigned n);
	void debugPrint(std::string s, int priority);
	void debugNewLine();

	void waitForVBlank();
	// threads
	void createThread(void *entrypoint,
					  std::reference_wrapper<void(void *)> entrypoint2,
					  void *arg, size_t stack_size, int prio, int affinity,
					  bool detached, size_t arg_size);
	// static vars
} // namespace HardwareInterface
namespace HI = HardwareInterface;

struct textureName {
	HI::HITexture texture = nullptr;
	std::string name = "";
};