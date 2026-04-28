/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Modified from the examples of the Arduino LoRa library
  More resources: https://RandomNerdTutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
*********/

#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>
#include "index_html.h"

// define the pins used by the transceiver module
#define ss 5
#define rst 13
#define dio0 14

const char *ssid = "LoRa Terminal";
const char *password = "12345678";

WebServer server(80);

int counter = 0;
unsigned long lastSendTime = 0;
unsigned long ackStartTime = 0;
const unsigned long ACK_TIMEOUT = 10000; // 10 giây
unsigned long lastRTT = 0;
int lastRSSI = 0;
float lastSNR = 0;
String lastMsg = "";

String txMsg = "";
String rxMsg = "";
String latestMsg = "";
int latestRSSI = 0;
float latestSNR = 0;
bool newDataAvailable = false;
String logLine = "";
bool newLogAvailable = false;

enum State
{
    RX_MODE,
    TX_MODE,
    WAIT_ACK
};

State currentState = RX_MODE;

String getTimeString() {

  unsigned long ms = millis();

  unsigned long totalSeconds = ms / 1000;
  unsigned int hours = totalSeconds / 3600;
  unsigned int minutes = (totalSeconds % 3600) / 60;
  unsigned int seconds = totalSeconds % 60;

  char buffer[9]; // "hh:mm:ss" = 8 ký tự + null
  sprintf(buffer, "%02u:%02u:%02u", hours, minutes, seconds);

  return String(buffer);
}

void handleRoot()
{
    server.send(200, "text/html", index_html);
}

// void handleSend()
// {
//     String msg = server.arg("msg");

//     if (msg.length() > 0)
//     {
//         String timeStr = getTimeString();

//         // Gộp timestamp + message
//         String fullMsg = timeStr + "|" + msg;

//         LoRa.beginPacket();
//         LoRa.print(fullMsg);
//         LoRa.endPacket();

//         // 🔥 log TX
//         logLine = "Tx|" + fullMsg;
//         newLogAvailable = true;

//         Serial.println("Tx: " + fullMsg);
//     }

//     server.send(200, "text/plain", "OK");
// }

void handleSend()
{
    String msg = server.arg("msg");

    if (msg.length() > 0)
    {
        String timeStr = getTimeString();
        unsigned long now = millis();

        // 📦 Packet gửi đi (giữ nguyên để tính RTT)
        String fullMsg = timeStr + "|" + String(now) + "|" + msg;

        LoRa.beginPacket();
        LoRa.print(fullMsg);
        LoRa.endPacket();

        Serial.println("Tx: " + fullMsg);

        // ✅ Log hiển thị GUI (đã đúng format yêu cầu)
        logLine = "[TX] " + timeStr + " | " + msg;
        newLogAvailable = true;
    }

    server.send(200, "text/plain", "OK");
}

void handleData()
{
    if (newLogAvailable)
    {
        String json = "{";
        json += "\"log\":\"" + logLine + "\"";
        json += "}";

        // reset
        logLine = "";
        newLogAvailable = false;

        server.send(200, "application/json", json);
    }
    else
    {
        server.send(200, "application/json", "{\"log\":\"\"}");
    }
}

String readSerialLine()
{
    static String input = "";

    while (Serial.available())
    {
        char c = Serial.read();

        // Nếu là newline (Enter)
        if (c == '\n' || c == '\r')
        {
            if (input.length() > 0)
            {
                String result = input;
                input = ""; // reset buffer
                return result;
            }
        }
        else
        {
            // Giới hạn 100 ký tự
            if (input.length() < 120)
            {
                input += c;
            }
        }
    }

    return ""; // chưa có dữ liệu hoàn chỉnh
}

// void handleLoRaReceive()
// {
//     int packetSize = LoRa.parsePacket();

//     if (packetSize)
//     {
//         String rxMsg = "";

//         while (LoRa.available())
//         {
//             rxMsg += (char)LoRa.read();
//         }

//         unsigned long t2 = millis();

//         Serial.println("Nhan duoc: " + rxMsg);

//         // 🔥 Tách timestamp và data
//         int sepIndex = rxMsg.indexOf('|');

//         if (sepIndex > 0)
//         {
//             String t1_str = rxMsg.substring(0, sepIndex);
//             String data = rxMsg.substring(sepIndex + 1);

//             unsigned long t1 = t1_str.toInt();

//             unsigned long rtt = t2 - t1;

//             Serial.print("RTT: ");
//             Serial.print(rtt);
//             Serial.println(" ms");

//             // Log về web
//             logLine = "Rx|" + data + "|RTT:" + String(rtt) + "ms";
//             newLogAvailable = true;
//         }
//         else
//         {
//             Serial.println("Packet sai format!");
//         }
//     }
// }

void handleLoRaReceive()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        String received = "";

        while (LoRa.available())
        {
            received += (char)LoRa.read();
        }


        
        int first = received.indexOf('|');
        int second = received.indexOf('|', first + 1);

        if (first == -1 || second == -1) return;

        String timeStr = received.substring(0, first);

        unsigned long sentMillis = received.substring(first + 1, second).toInt();
        if (sentMillis == 0) return;
        String msg = received.substring(second + 1);

        unsigned long now = millis();
        unsigned long rtt = now - sentMillis;

        // 🔥 LƯU LẠI
        lastRTT = rtt;
        lastRSSI = LoRa.packetRssi();
        lastSNR = LoRa.packetSnr();
        lastMsg = msg;

        // ✅ Log Serial (debug)
        Serial.printf("RX: %s | RTT: %lu ms | RSSI: %d | SNR: %.2f\n",
                      msg.c_str(), rtt, lastRSSI, lastSNR);

        // ✅ Log Web GUI (đúng format yêu cầu)
        logLine = "[RX] " + msg +
                  " [RSSI] " + String(lastRSSI) + "dBm" +
                  " [SNR] " + String(lastSNR, 2) + "dB" +
                  " [RTT] " + String(rtt) + "ms";

        newLogAvailable = true;
    }
}

void handleStatus()
{
    String json = "{";
    json += "\"rtt\":" + String(lastRTT) + ",";
    json += "\"rssi\":" + String(lastRSSI) + ",";
    json += "\"snr\":" + String(lastSNR, 2) + ",";
    json += "\"msg\":\"" + lastMsg + "\"";
    json += "}";

    server.send(200, "application/json", json);
}

bool handleWaitAck()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        String rxMsg = "";

        while (LoRa.available())
        {
            rxMsg += (char)LoRa.read();
        }

        Serial.println("Nhan ACK: " + rxMsg);

        // 🔥 Bạn có thể parse ở đây để lấy:
        // time_rx, RSSI, SNR

        return true;
    }

    if (millis() - ackStartTime > ACK_TIMEOUT)
    {
        Serial.println("Timeout!");
        return false;
    }

    return false;
}

void setup()
{
    // initialize Serial Monitor
    Serial.begin(115200);

    while (!Serial)
        ;

    Serial.println("LoRa Sender");

    WiFi.softAP(ssid, password);
    Serial.println("AP started");
    Serial.println(WiFi.softAPIP());

    server.on("/", handleRoot);
    server.on("/send", handleSend);
    server.on("/data", handleData);
    server.on("/status", handleStatus);
    server.begin();

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
    // Change sync word (0xF3) to match the receiver
    // The sync word assures you don't get LoRa messages from other LoRa transceivers
    // ranges from 0-0xFF

    /*Cấu hình ổn định*/

    LoRa.setSyncWord(0xF3);         // Đồng bộ thông số với module nhận để đảm bảo nhận đúng package
                                    // và không bị decode sai từ đó rớt package
    LoRa.setSpreadingFactor(12);    // max range, ít rớt
    LoRa.setSignalBandwidth(125E3); // ổn định
    LoRa.setCodingRate4(8);         // chống lỗi tốt
    LoRa.setPreambleLength(12);     // sync tốt hơn
    LoRa.enableCrc();               // phát hiện lỗi

    /*Cấu hình test truyền xa*/

    // LoRa.setSpreadingFactor(12);     // Hệ số khuếch tán tín hiệu set thành max để độ phủ tín hiệu rộng nhất
    // LoRa.setSignalBandwidth(125E3);  // Giảm băng thông
    // LoRa.setCodingRate4(8);          // max chống lỗi
    // LoRa.setTxPower(20);             // max công suất (SX1276 support ~20dBm)
    // LoRa.setPreambleLength(12);      // có thể tăng
    // LoRa.enableCrc();
    // LoRa.setSyncWord(0xF3);

    Serial.println("LoRa Initializing OK!");
}

void loop()
{
    //   Serial.print("Sending packet: ");
    //   Serial.println(counter);

    //   //Send LoRa packet to receiver
    //   LoRa.beginPacket();
    //   LoRa.print("hello ");
    //   LoRa.print(counter);
    //   LoRa.endPacket();

    //   counter++;

    //   delay(10000);
    // String msg = readSerialLine();
    // if (msg.length() > 0)
    // {
    //     Serial.print("Ban vua nhap: ");
    //     Serial.println(msg);

    //     // Gửi LoRa ở đây
    //     LoRa.beginPacket();
    //     LoRa.print(msg);
    //     LoRa.endPacket();
    // }

    // switch (currentState)
    // {

    // case RX_MODE:
    //     // luôn nghe
    //     handleLoRaReceive();

    //     // kiểm tra có dữ liệu từ Serial không
    //     txMsg = readSerialLine();
    //     if (txMsg.length() > 0)
    //     {
    //         currentState = TX_MODE;
    //     }
    //     break;

    // case TX_MODE:
    //     sendLoRa(txMsg);

    //     // gửi xong quay lại RX
    //     currentState = RX_MODE;
    //     break;
    // }

    // switch (currentState)
    // {

    // case RX_MODE:
    //     handleLoRaReceive();

    //     txMsg = readSerialLine();
    //     if (txMsg.length() > 0)
    //     {
    //         currentState = TX_MODE;
    //     }
    //     break;

    // case TX_MODE:
    //     sendLoRa(txMsg);

    //     // Bắt đầu đếm thời gian chờ ACK
    //     ackStartTime = millis();

    //     currentState = WAIT_ACK;
    //     break;

    // case WAIT_ACK:
    //     if (handleWaitAck())
    //     {
    //         // nhận được ACK → quay về RX
    //         currentState = RX_MODE;
    //     }
    //     else if (millis() - ackStartTime > ACK_TIMEOUT)
    //     {
    //         // timeout → cũng quay về RX
    //         currentState = RX_MODE;
    //         Serial.println("Chuyen ve RX sau khi timeout");
    //     }
    //     break;
    // }
    server.handleClient();
    // int packetSize = LoRa.parsePacket();
    // if (packetSize)
    // {
    //     String msg = "";

    //     while (LoRa.available())
    //     {
    //         msg += (char)LoRa.read();
    //     }

    //     int rssi = LoRa.packetRssi();
    //     float snr = LoRa.packetSnr();

    //     logLine = "Rx|" + msg +
    //               " | RSSI:" + String(rssi) +
    //               " | SNR:" + String(snr);

    //     newLogAvailable = true;

    //     Serial.println(logLine);
    // }
    handleLoRaReceive();
}