#ifdef _VITA
#include "core.h"
#include "HardwareInterface.h"
#include <vita2d.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <functional>
#include <psp2/touch.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h> 

#define SCALE 2
#define DEBUG_PRIORITY 0

void HI::systemInit() {
	vita2d_init();
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
}

void HI::systemFini() {
	vita2d_fini();
	sceKernelExitProcess(0);
}

void HI::consoleInit() {
}

void HI::consoleFini() {
};

void HI::startFrame(HI_SCREEN screen) {
	vita2d_start_drawing();
	vita2d_clear_screen();
}

short HI::getRComponent(HIColor color) {
	return((short)color) & 0b0000000011111111;
}

short HI::getGComponent(HIColor color) {
	return((short)color >> 8) & 0b0000000011111111;
}
short HI::getBComponent(HIColor color) {
	return ((short)color >> 16) & 0b0000000011111111;
}
short HI::getAComponent(HIColor color) {
	return((short)color >> 24) & 0b0000000011111111;
}

void HI::setBackgroundColor(HIColor color) {
	vita2d_set_clear_color(color);
}

HardwareInterface::HIFont HardwareInterface::loadFont(std::string path){
	return vita2d_load_font_file(path.c_str());
}

void HardwareInterface::freeFont(HIFont font) {
	vita2d_free_font((vita2d_font*)font);
}

void HardwareInterface::drawText(HIFont font, string text, int posX, int posY, int size, HIColor color){
	vita2d_font_draw_text((vita2d_font*)font, posX*SCALE, posY*SCALE, color, size*SCALE, text.c_str());
}

HI::HITexture HI::loadPngFile(std::string path) {
	return (HI::HITexture)vita2d_load_PNG_file(path.c_str());
}
HI::HITexture HI::loadBmpFile(std::string path) {
	return (HI::HITexture)vita2d_load_BMP_file(path.c_str());
}

void HI::drawTexture(HI::HITexture texture, int posX, int posY) {
	vita2d_draw_texture_scale((vita2d_texture*)texture, posX*SCALE, posY*SCALE, SCALE, SCALE);
}

void HI::drawTextureRotate(HI::HITexture texture, int posX, int posY, float angle) {
	vita2d_draw_texture_scale_rotate((vita2d_texture*)texture, posX*SCALE, posY*SCALE, SCALE, SCALE, angle);
}

void HI::drawTexturePart(HI::HITexture texture, int startX, int startY, int posX, int posY, int sizeX, int sizeY) {
	vita2d_draw_texture_part_scale((vita2d_texture*)texture, posX*SCALE, posY*SCALE, startX, startX, sizeX, sizeY, SCALE, SCALE);
}

void HI::mergeTextures(HITexture origin, HITexture destination, short posX, short posY) {			  //BORKEN
}

void HI::drawRectangle(int posX, int posY, int width, int height, HI::HIColor color) {
	vita2d_draw_rectangle(posX, posY, width, height, color);
}

void drawPixel(int posX, int posY, HIColor color) {
	HardwareInterface::drawRectangle(posX, posY, 1, 1, color);
}

void HI::freeTexture(HITexture texture) {
	vita2d_free_texture((vita2d_texture*)texture);
}
HI::HITexture HI::createTexture(int sizeX, int sizeY) {
	return vita2d_create_empty_texture(sizeX, sizeY);
}

void HI::endFrame() {
	vita2d_end_drawing();
}

void HI::swapBuffers() {
	vita2d_swap_buffers();
}

std::string HI::getDataPath() {
	return "ux0:/data/RogueVITA/data/";
}

std::string HI::getSavesPath() {
	return "ux0:/data/RogueVITA/saves/";
}

bool HardwareInterface::createDir(std::string path) {  		  //BORKEN
	//if (HI::fsExists(path)) {
	//	errno = EEXIST;
	//	return false;
	//}
	//return (mkdir(path.c_str(), 0777) == 0);
	return false;
}
//
//bool HardwareInterface::fsExists(std::string path) {
//	FILE* fd = fopen(path.c_str(), "r");
//	if (fd) {
//		fclose(fd);
//		return true;
//	}
//	return fsIsDirectory(path);
//}
//bool HI::fsIsDirectory(const std::string path) {
//	DIR* dir = opendir(path.c_str());
//	if (dir) {
//		closedir(dir);
//		return true;
//	}
//	return false;
//}


bool HardwareInterface::copyFile(std::string input, std::string output) { 		  //DIRTY BUT NOT BORKEN
	std::ifstream  iFile(input, std::ios::binary);
	std::ofstream  oFile(output, std::ios::binary);
	oFile << iFile.rdbuf();
	oFile.close();
	iFile.close();
	return true;
}

int HI::getScreenHeight() {
	return 544 / SCALE;
}
int HI::getScreenWidth() {
	return 960 / SCALE;
}

HardwareInterface::HI_PLATFORM HI::getPlatform() {
	return PLATFORM_PSVITA;
}
std::string to_string(int i) {
	std::stringstream ss;
	ss << i;
	return ss.str();
}

void HI::createThread(void* entrypoint, std::reference_wrapper<void(void*)>entrypoint2, void* arg, size_t stack_size, int prio, int affinity, bool detached, size_t arg_size) {
	SceUID thread =	sceKernelCreateThread("ChunkLoader", (SceKernelThreadEntry)&(entrypoint2.get()), 0x40, stack_size,0, 0, NULL);
	sceKernelStartThread(thread,arg_size, arg);
}

void HI::updateTouch(point2D &touch) {			  //BORKEN
	SceTouchData touch2;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch2, 1);
	touch.x = ((HI::getScreenWidth() * touch2.report[0].x) / 1920);
	touch.y = ((HI::getScreenHeight() * touch2.report[0].y) / 1080);
}
void HI::updateHID() {
}

int HI::getKeysUp() {  		  //BORKEN
	return HI::getKeysHeld();
}
int HI::getKeysHeld() {
	SceCtrlData pad;
	memset(&pad, 0, sizeof(pad));
	sceCtrlPeekBufferPositive(0, &pad, 1);
	SceTouchData touch2;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch2, 1);
	HI::HI_KEYS keys =(HI_KEYS)0;
	if (pad.buttons & SCE_CTRL_DOWN) keys = (HI_KEYS)((int)keys | HI::HI_KEY_DOWN);
	if (pad.buttons & SCE_CTRL_LEFT)keys = (HI_KEYS)((int)keys | HI::HI_KEY_LEFT);
	if (pad.buttons & SCE_CTRL_RIGHT)keys = (HI_KEYS)((int)keys | HI::HI_KEY_RIGHT);
	if (pad.buttons & SCE_CTRL_UP)keys = (HI_KEYS)((int)keys | HI::HI_KEY_UP);
	if (pad.buttons & SCE_CTRL_START)keys = (HI_KEYS)((int)keys | HI::HI_KEY_START);
	if (pad.buttons & SCE_CTRL_CROSS)keys = (HI_KEYS)((int)keys | HI::HI_KEY_A);
	if (touch2.reportNum == 1)keys = (HI_KEYS)((int)keys | HI::HI_KEY_TOUCH);
	return keys;
}
int HI::getKeysDown() {				//SEMIBORKEN
	return HI::getKeysHeld();
}

void HI::getCirclePadPos(point2D &circle, HI_CIRCLEPAD circlePadID) { 		  //BORKEN
}

void HI::sleepThread(unsigned long ms) {		  //BORKEN
	sceKernelDelayThread(ms * 1000);
}

void HardwareInterface::debugPrint(string s) {
	HI::debugPrint(s + "\n", 1);
	HI::debugNewLine();
}

void HardwareInterface::debugPrint(int n) {
	stringstream ss;
	ss << n;
	string s;
	s = ss.str();
	s += "\n";
	sceClibPrintf(s.c_str());
}
void HardwareInterface::debugPrint(unsigned n) {
	stringstream ss;
	ss << n;
	string s;
	s = ss.str();
	s += "\n";
	sceClibPrintf(s.c_str());
}

void HardwareInterface::debugPrint(string s, int p) {
	string s2 = s + "\n";
	if (p >= DEBUG_PRIORITY)sceClibPrintf(s2.c_str());
}

void HardwareInterface::debugNewLine() {
	sceClibPrintf("\n");
}

void HI::waitForVBlank() {
	sceDisplayWaitVblankStart();
}

bool HI::aptMainLoop() {
	return true;
}
#endif