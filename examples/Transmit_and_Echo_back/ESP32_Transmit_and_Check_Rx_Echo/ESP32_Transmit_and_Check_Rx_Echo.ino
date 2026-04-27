/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Modified from the examples of the Arduino LoRa library
  More resources: https://RandomNerdTutorials.com/esp32-lora-rfm95-transceiver-arduino-ide/
*********/

#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <WebServer.h>

// define the pins used by the transceiver module
#define ss 5
#define rst 13
#define dio0 14

const char *ssid = "LoRa Terminal";
const char *password = "12345678";

WebServer server(80);

int counter = 0;

unsigned long ackStartTime = 0;
const unsigned long ACK_TIMEOUT = 10000; // 10 giây

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

String htmlPage = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LoRa Control Dashboard</title>
    <style>
        :root {
            --bg-color: #121212;
            --card-bg: #1e1e1e;
            --primary-color: #00adb5;
            --tx-color: #ff9f43;
            --rx-color: #2ecc71;
            --text-color: #eeeeee;
            --border-radius: 12px;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            margin: 0;
            display: flex;
            justify-content: center;
            padding: 20px;
        }

        .container {
            width: 100%;
            max-width: 500px;
            background: var(--card-bg);
            padding: 25px;
            border-radius: var(--border-radius);
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }

        h2 { color: var(--primary-color); margin-bottom: 25px; font-weight: 600; }

        /* Input Group */
        .input-group {
            position: relative;
            margin-bottom: 15px;
        }

        input[type="text"] {
            width: 100%;
            padding: 12px 40px 12px 15px;
            border-radius: var(--border-radius);
            border: 1px solid #333;
            background: #252525;
            color: white;
            box-sizing: border-box;
            outline: none;
            transition: 0.3s;
        }

        input[type="text"]:focus { border-color: var(--primary-color); }

        .clear-icon {
            position: absolute;
            right: 15px;
            top: 50%;
            transform: translateY(-50%);
            cursor: pointer;
            color: #ff4757;
            font-size: 18px;
        }

        /* Buttons */
        .btn-row { display: flex; gap: 10px; margin-bottom: 20px; }
        
        button {
            flex: 1;
            padding: 12px;
            border: none;
            border-radius: var(--border-radius);
            cursor: pointer;
            font-weight: bold;
            transition: 0.3s;
            text-transform: uppercase;
        }

        #sendBtn { background-color: var(--primary-color); color: white; }
        #sendBtn:hover { background-color: #008c94; opacity: 0.9; }
        #sendBtn:disabled { background-color: #444; cursor: not-allowed; }

        .btn-clear { background-color: #333; color: #ccc; }
        .btn-clear:hover { background-color: #444; }

        /* Log Area */
        .log-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 20px;
            border-bottom: 1px solid #333;
            padding-bottom: 5px;
        }

        #log {
            height: 250px;
            background: #151515;
            border-radius: var(--border-radius);
            padding: 15px;
            margin-top: 10px;
            overflow-y: auto;
            text-align: left;
            font-family: 'Courier New', Courier, monospace;
            font-size: 14px;
            border: 1px solid #222;
            line-height: 1.6;
        }

        .tx { color: var(--tx-color); font-weight: bold; }
        .rx { color: var(--rx-color); font-weight: bold; }

        /* Mode Selection */
        .mode-container {
            margin-top: 25px;
            background: #252525;
            padding: 15px;
            border-radius: var(--border-radius);
        }

        .radio-group {
            display: flex;
            justify-content: center;
            gap: 30px;
            margin-bottom: 15px;
        }

        .radio-group label { cursor: pointer; display: flex; align-items: center; gap: 8px; }

        select {
            width: 100%;
            padding: 10px;
            border-radius: 8px;
            background: #333;
            color: white;
            border: none;
            outline: none;
        }

        select:disabled { opacity: 0.3; }

        /* Scrollbar mini */
        #log::-webkit-scrollbar { width: 6px; }
        #log::-webkit-scrollbar-thumb { background: #333; border-radius: 10px; }
    </style>
</head>
<body>

<div class="container">
    <h2>LoRa Dashboard</h2>

    <div class="input-group">
        <input type="text" id="msg" placeholder="Nhập tin nhắn nội dung...">
        <span class="clear-icon" onclick="clearInput()">✖</span>
    </div>

    <div class="btn-row">
        <button id="sendBtn" onclick="sendMsg()">Gửi Lệnh</button>
        <button class="btn-clear" onclick="clearLog()">Xóa Log</button>
    </div>

    <div class="log-header">
        <span style="font-size: 0.9em; color: #888;">LỊCH SỬ HOẠT ĐỘNG</span>
    </div>
    <div id="log"></div>

    <div class="mode-container">
        <div class="radio-group">
            <label>
                <input type="radio" name="mode" value="manual" checked onchange="setMode('manual')"> Thủ công
            </label>
            <label>
                <input type="radio" name="mode" value="auto" onchange="setMode('auto')"> Tự động
            </label>
        </div>

        <select id="intervalSelect" disabled>
            <option value="7000">Gửi mỗi 7 giây</option>
            <option value="10000">Gửi mỗi 10 giây</option>
            <option value="30000">Gửi mỗi 30 giây</option>
        </select>
    </div>
</div>

<script>
    let autoInterval = null;

    // Cập nhật dữ liệu từ Server (Gộp 2 setInterval cũ làm 1)
    setInterval(() => {
        fetch("/data")
            .then(res => res.json())
            .then(data => {
                if (data.log && data.log !== "") {
                    const logDiv = document.getElementById("log");
                    
                    // Xử lý định dạng Tx|Nội dung hoặc Rx|Nội dung
                    let parts = data.log.split("|");
                    let line = "";
                    
                    if (parts.length >= 2) {
                        let type = parts[0].trim();
                        let content = parts.slice(1).join("|");
                        let colorClass = (type === "Tx") ? "tx" : "rx";
                        line = `<span class="${colorClass}">${type}</span>: ${content}`;
                    } else {
                        line = `<span>${data.log}</span>`;
                    }

                    logDiv.innerHTML += `<div>${line}</div>`;
                    logDiv.scrollTop = logDiv.scrollHeight;
                }
            })
            .catch(err => console.log("Lỗi fetch data:", err));
    }, 1000);

    function sendMsg() {
        const msgInput = document.getElementById("msg");
        const msg = msgInput.value;
        if(!msg && document.querySelector('input[name="mode"]:checked').value === "manual") return;
        
        fetch("/send?msg=" + encodeURIComponent(msg || "Test Lora"));
    }

    function clearLog() {
        document.getElementById("log").innerHTML = "";
    }

    function clearInput() {
        document.getElementById("msg").value = "";
    }

    function setMode(mode) {
        const select = document.getElementById("intervalSelect");
        const sendBtn = document.getElementById("sendBtn");

        if (mode === "manual") {
            select.disabled = true;
            sendBtn.disabled = false;
            if (autoInterval) {
                clearInterval(autoInterval);
                autoInterval = null;
            }
        } else {
            select.disabled = false;
            sendBtn.disabled = true;
            startAutoSend();
        }
    }

    function startAutoSend() {
        if (autoInterval) clearInterval(autoInterval);
        const interval = parseInt(document.getElementById("intervalSelect").value);
        
        // Gửi ngay lập tức phát đầu tiên
        sendMsg(); 
        // Sau đó lặp lại
        autoInterval = setInterval(sendMsg, interval);
    }

    document.getElementById("intervalSelect").addEventListener("change", () => {
        const mode = document.querySelector('input[name="mode"]:checked').value;
        if (mode === "auto") startAutoSend();
    });
</script>

</body>
</html>
)rawliteral"
;

void handleRoot()
{
    server.send(200, "text/html", htmlPage);
}

void handleSend()
{
    String msg = server.arg("msg");

    if (msg.length() > 0)
    {
        LoRa.beginPacket();
        LoRa.print(msg);
        LoRa.endPacket();

        // 🔥 log TX
        logLine = "Tx|" + msg;
        newLogAvailable = true;

        Serial.println("Tx: " + msg);
    }

    server.send(200, "text/plain", "OK");
    Serial.println(">>> handleSend CALLED <<<");  // 🔥 thêm dòng này
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

void sendLoRa(String msg)
{
    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();

    Serial.print("Da gui: ");
    Serial.println(msg);
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

void handleLoRaReceive()
{
    int packetSize = LoRa.parsePacket();

    if (packetSize)
    {
        rxMsg = "";

        while (LoRa.available())
        {
            rxMsg += (char)LoRa.read();
        }

        Serial.print("Nhan duoc: ");
        Serial.println(rxMsg);
    }
}

bool handleWaitAck()
{
    int packetSize = LoRa.parsePacket();

    // Nếu nhận được phản hồi
    if (packetSize)
    {
        rxMsg = "";

        while (LoRa.available())
        {
            rxMsg += (char)LoRa.read();
        }

        Serial.print("Nhan ACK: ");
        Serial.println(rxMsg);

        return true; // đã nhận ACK
    }

    // Kiểm tra timeout
    if (millis() - ackStartTime > ACK_TIMEOUT)
    {
        Serial.println("Timeout! Mat goi");
        return false; // timeout
    }

    return false; // vẫn đang chờ
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
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        String msg = "";

        while (LoRa.available())
        {
            msg += (char)LoRa.read();
        }

        int rssi = LoRa.packetRssi();
        float snr = LoRa.packetSnr();

        logLine = "Rx|" + msg +
                  " | RSSI:" + String(rssi) +
                  " | SNR:" + String(snr);

        newLogAvailable = true;

        Serial.println(logLine);
    }
}