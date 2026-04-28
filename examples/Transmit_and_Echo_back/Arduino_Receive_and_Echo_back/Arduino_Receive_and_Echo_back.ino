/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Modified from the examples of the Arduino LoRa library
  More resources: https://RandomNerdTutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
*********/

#include <SPI.h>
#include <LoRa.h>

// define the pins used by the transceiver module
#define ss 10
#define rst 9
#define dio0 8

int counter = 0;

String receivedMsg = "";
int rssi = 0;
float snr = 0;

void basicReceivePackage()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        // received a packet
        Serial.print("Received packet '");

        // read packet
        while (LoRa.available())
        {
            Serial.print((char)LoRa.read());
        }

        // print RSSI of packet
        Serial.print("' with RSSI ");
        Serial.print(LoRa.packetRssi());
        Serial.print("' and SNR is ");
        Serial.println(LoRa.packetSnr());
    }
}

// void handleLoRaReceiveAndReply()
// {
//     int packetSize = LoRa.parsePacket();

//     if (packetSize)
//     {
//         receivedMsg = "";

//         // Đọc dữ liệu
//         while (LoRa.available())
//         {
//             receivedMsg += (char)LoRa.read();
//         }

//         // Lấy RSSI và SNR
//         rssi = LoRa.packetRssi();
//         snr = LoRa.packetSnr();

//         // In debug
//         Serial.print("Nhan duoc: ");
//         Serial.println(receivedMsg);

//         Serial.print("RSSI: ");
//         Serial.print(rssi);
//         Serial.print(" dBm | SNR: ");
//         Serial.print(snr);
//         Serial.println(" dB");

//         // ===== TÁCH CHUỖI =====
//         int idx = receivedMsg.indexOf('|');
//         String dataOnly = "";

//         if (idx != -1)
//         {
//             dataOnly = receivedMsg.substring(idx + 1);
//         }
//         else
//         {
//             // Nếu không có dấu '|', fallback luôn
//             dataOnly = receivedMsg;
//         }

//         // ===== ECHO LẠI CHỈ DATA =====
//         LoRa.beginPacket();
//         LoRa.print(dataOnly);
//         LoRa.endPacket();

//         Serial.print("Echo lai: ");
//         Serial.println(dataOnly);
//     }
// }

void handleLoRaReceiveAndReply()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        String receivedMsg = "";

        while (LoRa.available())
        {
            receivedMsg += (char)LoRa.read();
        }

        int rssi = LoRa.packetRssi();
        float snr = LoRa.packetSnr();

        Serial.print("Nhan duoc: ");
        Serial.println(receivedMsg);

        Serial.print("RSSI: ");
        Serial.print(rssi);
        Serial.print(" dBm | SNR: ");
        Serial.print(snr);
        Serial.println(" dB");

        // 🔁 Echo NGUYÊN GÓI
        LoRa.beginPacket();
        LoRa.print(receivedMsg);
        LoRa.endPacket();

        Serial.println("Da echo lai");
    }
}

void setup()
{
    // initialize Serial Monitor
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.println("LoRa Receiver");

    // setup LoRa transceiver module
    LoRa.setPins(ss, rst, dio0);

    // replace the LoRa.begin(---E-) argument with your location's frequency
    // 433E6 for Asia
    // 868E6 for Europe
    // 915E6 for North America
    while (!LoRa.begin(915E6))
    {
        Serial.println(".");
        delay(500);
    }
    LoRa.setSyncWord(0xF3);         // Đồng bộ với module truyền để nhận đúng để đảm bảo nhận được package tốt nhất
    LoRa.setSpreadingFactor(12);    // max range, ít rớt
    LoRa.setSignalBandwidth(125E3); // ổn định
    LoRa.setCodingRate4(8);         // chống lỗi tốt
    LoRa.setPreambleLength(12);     // sync tốt hơn
    LoRa.enableCrc();               // phát hiện lỗi
                                    // Change sync word (0xF3) to match the receiver
    // The sync word assures you don't get LoRa messages from other LoRa transceivers
    // ranges from 0-0xFF
    // LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
}

void loop()
{
    // try to parse packet
    handleLoRaReceiveAndReply();
}