#pragma once
#include <string>

#define RGBA8(r, g, b, a) ((((a)&0xFF)<<24) | (((b)&0xFF)<<16) | (((g)&0xFF)<<8) | (((r)&0xFF)<<0))
#define BIT(n) (1U<<(n))

struct point2D {
	int x = -1;
	int y = -1;
};

namespace HardwareInterface {
	typedef void* HITexture;
	typedef unsigned char uint8;
	typedef unsigned int uint32;
	typedef uint32* HIImage;
	typedef void* HIFont;
	typedef int HIColor;
	enum HI_PLATFORM {
		PLATFORM_PSVITA,
		PLATFORM_NINTENDO3DS,
		PLATFORM_PC,
	};
	enum HI_SCREEN {
		SCREEN_TOP,
		SCREEN_BOT
	};
	enum HI_CIRCLEPAD {
		CIRCLEPAD_LEFT,
		CIRCLEPAD_RIGHT
	};
	enum HI_KEYS {
		HI_KEY_A = BIT(0),       ///< A
		HI_KEY_B = BIT(1),       ///< B
		HI_KEY_SELECT = BIT(2),       ///< Select
		HI_KEY_START = BIT(3),       ///< Start
		HI_KEY_DRIGHT = BIT(4),       ///< D-Pad Right
		HI_KEY_DLEFT = BIT(5),       ///< D-Pad Left
		HI_KEY_DUP = BIT(6),       ///< D-Pad Up
		HI_KEY_DDOWN = BIT(7),       ///< D-Pad Down
		HI_KEY_R = BIT(8),       ///< R
		HI_KEY_L = BIT(9),       ///< L
		HI_KEY_X = BIT(10),      ///< X
		HI_KEY_Y = BIT(11),      ///< Y
		HI_KEY_ZL = BIT(14),      ///< ZL (New 3DS only)
		HI_KEY_ZR = BIT(15),      ///< ZR (New 3DS only)
		HI_KEY_TOUCH = BIT(20),      ///< Touch (Not actually provided by HID)
		HI_KEY_CSTICK_RIGHT = BIT(24), ///< C-Stick Right (New 3DS only)
		HI_KEY_CSTICK_LEFT = BIT(25), ///< C-Stick Left (New 3DS only)
		HI_KEY_CSTICK_UP = BIT(26), ///< C-Stick Up (New 3DS only)
		HI_KEY_CSTICK_DOWN = BIT(27), ///< C-Stick Down (New 3DS only)
		HI_KEY_CPAD_RIGHT = BIT(28),   ///< Circle Pad Right
		HI_KEY_CPAD_LEFT = BIT(29),   ///< Circle Pad Left
		HI_KEY_CPAD_UP = BIT(30),   ///< Circle Pad Up
		HI_KEY_CPAD_DOWN = BIT(31),   ///< Circle Pad Down

								   // Generic catch-all directions
								   HI_KEY_UP = HI_KEY_DUP | HI_KEY_CPAD_UP,    ///< D-Pad Up or Circle Pad Up
								   HI_KEY_DOWN = HI_KEY_DDOWN | HI_KEY_CPAD_DOWN,  ///< D-Pad Down or Circle Pad Down
								   HI_KEY_LEFT = HI_KEY_DLEFT | HI_KEY_CPAD_LEFT,  ///< D-Pad Left or Circle Pad Left
								   HI_KEY_RIGHT = HI_KEY_DRIGHT | HI_KEY_CPAD_RIGHT ///< D-Pad Right or Circle Pad Right
	};

	//System
	void systemInit();
	void systemFini();
	void consoleInit();
	void consoleFini();
	void sleepThread(unsigned long ns);
	bool aptMainLoop();

	//input
	void updateHID();
	int getKeysDown();
	int getKeysUp();
	int getKeysHeld();
	void updateTouch(point2D &touch);
	void getCirclePadPos(point2D &circle, HI_CIRCLEPAD circlePadID);

	//graphics
	short getRComponent(HIColor color);
	short getGComponent(HIColor color);
	short getBComponent(HIColor color);
	short getAComponent(HIColor color);

	void startFrame(HI_SCREEN screen);
	void setBackgroundColor(HIColor color);
	HIFont loadFont(std::string path);
	void freeFont(HIFont font);
	void drawText(HIFont font, std::string text, int posX, int posY, int size, HIColor color);
	HITexture loadPngFile(std::string path);
	HITexture loadBmpFile(std::string path);
	void drawTexture(HITexture texture, int posX, int posY);
	void drawImage(HIImage image, int sizeX, int sizeY, int posX, int posY); //HIImage image should be a pointer to an array of RGBA8 pixels
	void drawTexturePart(HITexture texture, int startX, int startY, int posX, int posY, int sizeX, int sizeY);
	void drawTextureRotate(HITexture texture, int posX, int posY, float angle);
	void mergeTextures(HITexture originTexture, HITexture destinationTexture, short posX, short posY);
	void drawRectangle(int posX, int posY, int width, int height, HIColor color);
	void drawPixel(int posX, int posY, HIColor color);
	HITexture createTexture(int sizeX, int sizeY);
	void freeTexture(HITexture texture);
	void endFrame();
	void swapBuffers();

	//filesystem
	std::string getDataPath();
	std::string getSavesPath();
	bool createDir(std::string path);
	bool fsExists(std::string path);
	bool fsIsDirectory(const std::string path);
	bool copyFile(std::string origin, std::string destination);

	//HardwareInfo
	int getScreenHeight();
	int getScreenWidth();
	HI_PLATFORM getPlatform();

	//DEBUGGING
	void debugPrint(std::string s);
	void debugPrint(int n);
	void debugPrint(unsigned n);
	void debugPrint(std::string s, int priority);
	void debugNewLine();

	void waitForVBlank();
	//threads
	void createThread(void* entrypoint, std::reference_wrapper<void(void*)> entrypoint2, void* arg, size_t stack_size, int prio, int affinity, bool detached, size_t arg_size);
	//static vars
}
namespace HI = HardwareInterface;

struct textureName {
	HI::HITexture texture = NULL;
	std::string name = "";
};
