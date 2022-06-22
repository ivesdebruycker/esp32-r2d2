#include "Arduino.h"
#include "Commands.h"

Commands::Commands()
{
}

std::vector<char> Commands::generateCommand(char TID, DeviceId DID, char CID, char SEQ, char DLEN, ...) {
  short i=0;
  char sum=0, data;
  va_list args;
  
  // Calculate checksum
  sum = TID + char(DID) + CID + SEQ;

  int cnter=5;
  char yyy[cnter+DLEN + 2];
  yyy[0]= char(SOP);
  yyy[1]= char(TID);
  yyy[2]= char(DID);
  yyy[3]= char(CID);
  yyy[4]= char(SEQ);

  va_start(args, DLEN);
  for(; i<DLEN; i++){
      data = va_arg(args, int);
      yyy[cnter]=data;
      cnter+=1;
      sum += data;
  }
  va_end(args);

  yyy[cnter]= char(~sum) ;
  yyy[cnter + 1]= char(EOP);
  
  Serial.print("SOP\tTID\tDID\tCID\tSEQ");
  for(i=0; i<DLEN; i++){
      Serial.print("\tD");
      Serial.print(i);
    }
    Serial.println("\tCHK\tEOP");
  
  for(i=0; i<sizeof(yyy); i++){
      Serial.print("0x");
      Serial.print((unsigned char)yyy[i] < 0xa0 ? "0" : "");
    Serial.print((unsigned char)yyy[i], HEX);
      Serial.print("\t");
    }
  Serial.println();
  Serial.println();

  int n = sizeof(yyy) / sizeof(yyy[0]); 
  return std::vector<char> (yyy, yyy + n); 
}


std::vector<char> Commands::wake() {
  return this->generateCommand(0x0A, DeviceId::powerInfo, 0x0D, this->seq++, 0);
}

std::vector<char> Commands::sleep() {
  return this->generateCommand(0x0A, DeviceId::powerInfo, 0x01, this->seq++, 0);
}

std::vector<char> Commands::setVolume(char vol) {
  return this->generateCommand(0x0A, DeviceId::userIO, 0x08, this->seq++, 1, vol);
}

std::vector<char> Commands::generateSetR2D2HoloProjectorIntensity(char vol) {
  return this->generateCommand(0x0A, DeviceId::userIO, 0x0e, this->seq++, 3, 0x00, 0x80, vol);
}

std::vector<char> Commands::generatePlayR2D2Sound(char d0, char d1) {
  return this->generateCommand(0x0A, DeviceId::userIO, 0x07, this->seq++, 3, d0, d1, 0x00);
}

std::vector<char> Commands::generatePlayR2D2Sound(int id) {
  int d0 = id >> 8;
  int d1 = id & 0xff;
  return this->generateCommand(0x0A, DeviceId::userIO, 0x07, this->seq++, 3, d0, d1, 0x00);
}

std::vector<char> Commands::generatePlayAnimation(char id) {
  return this->generateCommand(0x0A, DeviceId::animatronics, 0x05, this->seq++, 2, 0x00, id);
}

std::vector<char> Commands::setR2D2LogicDisplaysIntensity(char val) {
  return this->generateCommand(0x0A, DeviceId::userIO, 0x0E, this->seq++, 3, 0x00, 0x08, val);
}

std::vector<char> Commands::setR2D2LEDColor(char r, char g, char b) {
  return this->generateCommand(0x0A, DeviceId::userIO, 0x0E, this->seq++, 8, 0x00, 0x77, r, g, b, r, g, b);
}