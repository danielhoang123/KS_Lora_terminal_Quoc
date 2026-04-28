#pragma once

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LoRa Control Dashboard Pro</title>
    <style>
        :root {
            --bg-color: #121212;
            --card-bg: #1e1e1e;
            --primary-color: #00adb5;
            --tx-color: #ffcc00;
            --rx-color: #00cc66;
            --text-color: #eeeeee;
            --border-radius: 16px;
            --accent-purple: #cc66ff;
            --accent-orange: #ff8800;
        }

        body {
            font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
            background-color: var(--bg-color);
            color: var(--text-color);
            margin: 0;
            display: flex;
            justify-content: center;
            padding: 20px;
        }

        .container { width: 100%; max-width: 550px; }
        h2 { color: var(--primary-color); text-align: center; margin-bottom: 25px; text-transform: uppercase; letter-spacing: 2px; }

        /* --- Mode Switcher --- */
        .mode-switcher {
            display: flex;
            gap: 12px;
            margin-bottom: 20px;
            height: 180px;
        }

        .mode-box {
            background: var(--card-bg);
            border-radius: var(--border-radius);
            padding: 15px;
            cursor: pointer;
            transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            border: 2px solid transparent;
            flex: 1;
            position: relative;
            overflow: hidden;
        }

        .mode-box.active {
            flex: 3.5;
            cursor: default;
            border-color: var(--primary-color);
            box-shadow: 0 8px 24px rgba(0, 173, 181, 0.15);
            align-items: flex-start;
            justify-content: flex-start;
        }

        /* --- Header inside Box (Icon + Title) --- */
        .box-header {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 8px;
            transition: 0.3s;
        }

        .active .box-header {
            flex-direction: row;
            margin-bottom: 15px;
        }

        .mode-icon {
            width: 30px;
            height: 30px;
            fill: #555;
            transition: 0.4s;
        }

        .active .mode-icon {
            width: 24px;
            height: 24px;
            fill: var(--primary-color);
        }

        .mode-box h3 {
            margin: 0;
            font-size: 0.7rem;
            color: #666;
            text-transform: uppercase;
            letter-spacing: 1px;
            transition: 0.3s;
        }

        .active h3 {
            font-size: 1.1rem;
            color: var(--primary-color);
        }

        /* --- Content Visibility --- */
        .mode-content {
            opacity: 0;
            display: none;
            width: 100%;
        }

        .active .mode-content {
            display: flex;
            flex-direction: column;
            gap: 10px;
            opacity: 1;
            animation: fadeIn 0.5s forwards;
        }

        @keyframes fadeIn { from { opacity: 0; transform: translateY(5px); } to { opacity: 1; transform: translateY(0); } }

        /* --- Inputs & UI --- */
        .input-wrapper { position: relative; width: 100%; }
        input[type="text"] {
            width: 100%; padding: 12px 35px 12px 12px;
            border-radius: 8px; border: 1px solid #333;
            background: #252525; color: white; box-sizing: border-box; outline: none;
        }
        .clear-icon { position: absolute; right: 12px; top: 50%; transform: translateY(-50%); cursor: pointer; color: #ff4757; font-weight: bold; }
        
        button {
            padding: 12px; border: none; border-radius: 8px;
            cursor: pointer; font-weight: bold; background: var(--primary-color); color: white;
            transition: 0.3s;
        }
        button:hover { filter: brightness(1.1); }
        
        select { padding: 10px; border-radius: 8px; background: #333; color: white; border: none; font-weight: bold; }

        /* --- Log Area --- */
        .log-container { background: var(--card-bg); border-radius: var(--border-radius); padding: 15px; margin-bottom: 15px; }
        #log { height: 200px; background: #0a0a0a; border-radius: 8px; padding: 12px; overflow-y: auto; font-family: 'Consolas', monospace; font-size: 12px; border: 1px solid #222; line-height: 1.6; }

        /* --- Live Status Bar --- */
        .status-bar { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; background: var(--card-bg); padding: 15px; border-radius: var(--border-radius); }
        .status-item { background: #252525; padding: 10px; border-radius: 10px; text-align: center; }
        .status-item label { display: block; font-size: 0.65rem; color: #888; margin-bottom: 3px; font-weight: bold; }
        .status-item span { font-family: 'Consolas', monospace; font-weight: bold; font-size: 1.1rem; }
        
        .val-rtt { color: var(--accent-purple); }
        .val-rssi { color: var(--accent-orange); }
        .val-snr { color: var(--primary-color); }

        .last-msg-box { grid-column: span 3; background: #252525; padding: 10px; border-radius: 10px; margin-top: 5px; border-left: 4px solid var(--rx-color); }

        .tx { color: var(--tx-color); font-weight: bold; }
        .rx { color: var(--rx-color); font-weight: bold; }
        .rssi-tag { color: var(--accent-orange); }
        .snr-tag { color: var(--primary-color); }
        .rtt-tag { color: var(--accent-purple); }
    </style>
</head>
<body>

<div class="container">
    <h2>LoRa Dashboard</h2>

    <div class="mode-switcher">
        <div id="manualBox" class="mode-box active" onclick="setMode('manual')">
            <div class="box-header">
                <svg class="mode-icon" viewBox="0 0 24 24"><path d="M13,1.5c-1.1,0-2,0.9-2,2v9.2l-1.3-1.3c-0.8-0.8-2.1-0.8-2.8,0l0,0c-0.8,0.8-0.8,2.1,0,2.8l4.4,4.4C12,19.3,13,20,14.2,20h4.3 c2.2,0,4-1.8,4-4V7.5c0-1.1-0.9-2-2-2l0,0c-1.1,0-2,0.9-2,2V11H17V4.5c0-1.1-0.9-2-2-2l0,0c-1.1,0-2,0.9-2,2V11h-1.5V1.5z"/></svg>
                <h3>Manual</h3>
            </div>
            <div class="mode-content">
                <div class="input-wrapper">
                    <input type="text" id="msgInput" placeholder="Enter command...">
                    <span class="clear-icon" onclick="clearInput(event)">✕</span>
                </div>
                <button onclick="sendMsg()">SEND PACKET</button>
            </div>
        </div>

        <div id="autoBox" class="mode-box" onclick="setMode('auto')">
            <div class="box-header">
                <svg class="mode-icon" viewBox="0 0 24 24"><path d="M12,2C6.48,2,2,6.48,2,12s4.48,10,10,10s10-4.48,10-10S17.52,2,12,2z M12,20c-4.41,0-8-3.59-8-8s3.59-8,8-8s8,3.59,8,8 S16.41,20,12,20z M12.5,7H11v6l5.2,3.2l0.8-1.3l-4.5-2.7V7z"/></svg>
                <h3>Auto</h3>
            </div>
            <div class="mode-content">
                <label style="font-size: 0.75rem; color: #888;">Send Interval:</label>
                <select id="intervalSelect" onchange="restartAutoIfActive()">
                    <option value="7000">7 Seconds</option>
                    <option value="10000">10 Seconds</option>
                    <option value="30000">30 Seconds</option>
                </select>
                <p style="font-size: 0.7rem; color: var(--rx-color); margin: 0;">● Automatic transmission active</p>
            </div>
        </div>
    </div>

    <div class="log-container">
        <div style="display: flex; justify-content: space-between; margin-bottom: 10px;">
            <span style="font-size: 0.7rem; color: #555; font-weight: bold; letter-spacing: 1px;">ACTIVITY LOG</span>
            <span onclick="clearLog()" style="color: #ff4757; font-size: 0.65rem; cursor: pointer; font-weight: bold;">CLEAR ALL</span>
        </div>
        <div id="log"></div>
    </div>

    <div class="status-bar">
        <div class="status-item">
            <label>LATENCY</label>
            <span id="stat-rtt" class="val-rtt">0</span><small> ms</small>
        </div>
        <div class="status-item">
            <label>RSSI</label>
            <span id="stat-rssi" class="val-rssi">0</span><small> dBm</small>
        </div>
        <div class="status-item">
            <label>SNR</label>
            <span id="stat-snr" class="val-snr">0</span><small> dB</small>
        </div>
        <div class="last-msg-box">
            <label style="font-size: 0.6rem; color: #888; display: block; margin-bottom: 2px;">LATEST MESSAGE RECEIVED:</label>
            <span id="stat-msg" style="color: var(--rx-color); font-weight: bold; font-size: 0.9rem;">---</span>
        </div>
    </div>
</div>

<script>
    let autoInterval = null;
    let currentMode = 'manual';
    let autoCount = 0;

    function setMode(mode) {
        currentMode = mode;
        document.getElementById('manualBox').className = mode === 'manual' ? 'mode-box active' : 'mode-box';
        document.getElementById('autoBox').className = mode === 'auto' ? 'mode-box active' : 'mode-box';
        if (mode === 'manual') stopAuto(); else startAuto();
    }

    function startAuto() {
        stopAuto();
        const interval = parseInt(document.getElementById("intervalSelect").value);
        sendMsg();
        autoInterval = setInterval(sendMsg, interval);
    }

    function stopAuto() { if (autoInterval) { clearInterval(autoInterval); autoInterval = null; } }
    function restartAutoIfActive() { if (currentMode === 'auto') startAuto(); }

    function sendMsg() {
        const input = document.getElementById("msgInput");
        let msg = (currentMode === 'auto') ? "Auto Data " + (++autoCount) : input.value;
        if (!msg) return;
        fetch("/send?msg=" + encodeURIComponent(msg)).catch(e => console.error("Fetch Error:", e));
    }

    // Polling Log Data
    setInterval(() => {
        fetch("/data").then(res => res.json()).then(data => {
            if (data.log) {
                const logDiv = document.getElementById("log");
                let line = data.log;
                let formatted = "";

                if (line.startsWith("[TX]")) {
                    formatted = `<span class="tx">[TX]</span> ${line.substring(4)}`;
                } else if (line.startsWith("[RX]")) {
                    let content = line.substring(4)
                        .replace("[RSSI]", `<span class="rssi-tag">[RSSI]</span>`)
                        .replace("[SNR]", `<span class="snr-tag">[SNR]</span>`)
                        .replace("[RTT]", `<span class="rtt-tag">[RTT]</span>`);
                    formatted = `<span class="rx">[RX]</span> ${content}`;
                } else {
                    formatted = `<span>${line}</span>`;
                }

                logDiv.innerHTML += `<div>${formatted}</div>`;
                logDiv.scrollTop = logDiv.scrollHeight;
            }
        });
    }, 1000);

    // Polling Status Indicators
    function updateStatus() {
        fetch("/status").then(res => res.json()).then(data => {
            document.getElementById("stat-rtt").innerText = data.rtt || 0;
            document.getElementById("stat-rssi").innerText = data.rssi || 0;
            document.getElementById("stat-snr").innerText = data.snr || 0;
            document.getElementById("stat-msg").innerText = data.msg || "---";
        }).catch(e => {});
    }
    setInterval(updateStatus, 500);

    function clearLog() { document.getElementById("log").innerHTML = ""; }
    function clearInput(e) { e.stopPropagation(); document.getElementById("msgInput").value = ""; }
</script>

</body>
</html>
)rawliteral";