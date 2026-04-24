/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Modified from the examples of the Arduino LoRa library
  More resources: https://RandomNerdTutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
*********/

#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 13
#define dio0 14

int counter = 0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);

  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //868E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(915E6)) {
    Serial.println(".");
    delay(500);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF

  /*Cấu hình ổn định*/

  LoRa.setSyncWord(0xF3);          // Đồng bộ thông số với module nhận để đảm bảo nhận đúng package
                                   // và không bị decode sai từ đó rớt package
  LoRa.setSpreadingFactor(12);     // max range, ít rớt
  LoRa.setSignalBandwidth(125E3);  // ổn định
  LoRa.setCodingRate4(8);          // chống lỗi tốt
  LoRa.setPreambleLength(12);      // sync tốt hơn
  LoRa.enableCrc();                // phát hiện lỗi

  /*Cấu hình test truyền xa*/
  
  // LoRa.setSpreadingFactor(12);     // Hệ số khuếch tán tín hiệu set thành max để độ phủ tín hiệu rộng nhất
  // LoRa.setSignalBandwidth(125E3);  // Giảm ban
  // LoRa.setCodingRate4(8);          // max chống lỗi
  // LoRa.setTxPower(20);             // max công suất (SX1276 support ~20dBm)
  // LoRa.setPreambleLength(12);      // có thể tăng
  // LoRa.enableCrc();
  // LoRa.setSyncWord(0xF3);


  Serial.println("LoRa Initializing OK!");
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(10000);
}