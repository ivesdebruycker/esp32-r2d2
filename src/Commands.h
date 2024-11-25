#ifndef Commands_h
#define Commands_h

#include <string>

enum class DeviceId
{
  apiProcessor = 0x10,
  systemInfo = 0x11,
  powerInfo = 0x13,
  driving = 0x16,
  animatronics = 0x17,
  sensor = 0x18,
  userIO = 0x1a,
  somethingAPI = 0x1f
};

class Commands
{
public:
  Commands();
  std::vector<char> generateCommand(char TID, DeviceId DID, char CID, char SEQ, char DLEN, ...);
  std::vector<uint8_t> on();
  std::vector<uint8_t> off();
  std::vector<uint8_t> setPixel(int x, int y, int r, int g, int b);
  std::vector<uint8_t> clock(int style, bool visibleDate, bool hour24, int r, int g, int b);
  std::vector<uint8_t> setBrightness(int brightnessPct);
  std::vector<uint8_t> setDrawMode(int drawMode);

private:
  char SOP = 0x8D;
  char EOP = 0xD8;
  int seq = 0;
};

#endif
