#ifndef Commands_h
#define Commands_h

#include <string>

enum class DeviceId {
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
    std::vector<char> wake();
    std::vector<char> sleep();
    std::vector<char> setVolume(char vol);
    std::vector<char> generateSetR2D2HoloProjectorIntensity(char vol);
    std::vector<char> generatePlayR2D2Sound(int id);
    std::vector<char> generatePlayR2D2Sound(char d0, char d1);
    std::vector<char> generatePlayAnimation(char id);
    std::vector<char> setR2D2LogicDisplaysIntensity(char val);
    std::vector<char> setR2D2LEDColor(char r, char g, char b);

  private:
    char SOP = 0x8D;
    char EOP = 0xD8;
    int seq = 0;
};

#endif
