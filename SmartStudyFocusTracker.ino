#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>

// --- ตั้งค่า Network & Cloud ---
const char* ssid = "sunflowaii";
const char* pass = "11111111";
String cloud_url = "https://script.google.com/macros/s/AKfycbyFDJX6ZpiBskzEEQxDMdVtnftjh0If6CX46gm1EsLNcqYaNKNpGN237XW7X3qmct7ILQ/exec";

// --- Hardware Pins ---
#define LDR_PIN    34
#define PIR_PIN    21
#define SOUND_PIN  35
#define LED_PIN    23
#define BUZZER_PIN 19

// --- ตัวแปร Global ---
volatile float g_light       = 0;
volatile int   g_motion      = 0;
volatile int   g_sound       = 0;
volatile bool  cloud_cmd     = false;
volatile bool  manual_led    = false;
volatile int   g_focusScore  = 100;
String         g_ipAddress   = "";
String         g_ledMode     = "AUTO";

WebServer server(80);

// ===================================================
// คำนวณคะแนนสด
// ===================================================
int calculateLocalScore() {
  int score = 100;
  if (analogRead(LDR_PIN)   > 3500) score -= 30;
  if (digitalRead(PIR_PIN) == HIGH) score -= 30;
  if (analogRead(SOUND_PIN)  > 300) score -= 30;
  return max(score, 0);
}

// ===================================================
// Serial Monitor
// ===================================================
unsigned long lastPrintTime = 0;

void printSerialStatus() {
  if (millis() - lastPrintTime < 7000) return;
  lastPrintTime = millis();

  int score   = calculateLocalScore();
  float ldr   = analogRead(LDR_PIN);
  int   pir   = digitalRead(PIR_PIN);
  int   sound = analogRead(SOUND_PIN);

  String lightStat  = (ldr   > 3500) ? "Dark"     : "Bright";
  String motionStat = (pir  == HIGH)  ? "Detected" : "NoMotion";
  String soundStat  = (sound > 300)   ? "Noise"    : "Quiet";
  String scoreBar   = "";
  int bars = score / 10;
  for (int i = 0; i < 10; i++) scoreBar += (i < bars) ? "█" : "░";

  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║       🧠 Study Focus Monitor          ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.printf( "║  📡 IP Address : %-20s║\n", g_ipAddress.c_str());
  Serial.printf( "║  🌐 Web URL    : http://%-16s║\n", g_ipAddress.c_str());
  Serial.println("╠══════════════════════════════════════╣");
  Serial.printf( "║  🎯 Focus Score: %3d/100  [%s]  ║\n", score, scoreBar.c_str());
  Serial.println("╠══════════════════════════════════════╣");
  Serial.printf( "║  💡 แสง (LDR)  : %4.0f  → %-12s║\n", ldr, lightStat.c_str());
  Serial.printf( "║  🚶 Motion(PIR): %4d  → %-12s║\n", pir, motionStat.c_str());
  Serial.printf( "║  🔊 เสียง(SND) : %4d  → %-12s║\n", sound, soundStat.c_str());
  Serial.println("╠══════════════════════════════════════╣");
  Serial.printf( "║  💡 LED Mode   : %-20s║\n", g_ledMode.c_str());
  Serial.printf( "║  ☁️  Cloud Cmd  : %-20s║\n", cloud_cmd ? "ALARM ON 🚨" : "Standby ✅");
  Serial.printf( "║  📊 Status     : %-20s║\n", score >= 70 ? "กำลังโฟกัส ✅" : "ไม่โฟกัส ⚠️");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
}

// ===================================================
// buildHTML()
// ===================================================
String buildHTML() {
  String html = R"rawhtml(
<!DOCTYPE html>
<html lang="th">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Study Focus Tracker</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Kanit:wght@300;400;600;700&display=swap');
    * { margin:0; padding:0; box-sizing:border-box; }
    body { font-family:'Kanit',sans-serif; background:#0f172a; color:#e2e8f0; min-height:100vh; padding:20px; }
    .header { text-align:center; padding:16px 0 8px; }
    .header h1 { font-size:1.5rem; font-weight:700; background:linear-gradient(135deg,#38bdf8,#818cf8); -webkit-background-clip:text; -webkit-text-fill-color:transparent; }
    .header p  { color:#64748b; font-size:0.78rem; margin-top:4px; }
    .ip-badge  { background:#1e293b; border:1px solid #334155; border-radius:8px; padding:8px 16px; text-align:center; margin:10px 0; font-size:0.82rem; color:#38bdf8; }
    .score-card { background:#1e293b; border:1px solid #334155; border-radius:16px; padding:20px; text-align:center; margin:12px 0; }
    .score-label  { font-size:0.72rem; color:#64748b; letter-spacing:2px; text-transform:uppercase; }
    .score-number { font-size:4rem; font-weight:700; margin:6px 0; transition:color 0.3s; }
    .score-bar-wrap { background:#0f172a; border-radius:10px; height:10px; margin:8px 0; overflow:hidden; }
    .score-bar { height:100%; border-radius:10px; transition:width 0.5s, background 0.3s; }
    .score-status { font-size:0.9rem; font-weight:500; }
    .alarm-box { background:linear-gradient(135deg,#dc2626,#991b1b); border-radius:10px; padding:12px; text-align:center; font-weight:600; font-size:0.9rem; animation:pulse 1s infinite; margin:10px 0; display:none; }
    @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.6} }
    .sensors-grid { display:grid; grid-template-columns:1fr 1fr; gap:10px; margin:12px 0; }
    .sensor-card { background:#1e293b; border-radius:12px; padding:12px 14px; border:1px solid #334155; }
    .full { grid-column:1/-1; }
    .s-label { font-size:0.68rem; color:#64748b; text-transform:uppercase; letter-spacing:1px; margin-bottom:4px; }
    .s-val   { font-size:0.95rem; font-weight:600; }
    .s-raw   { font-size:0.65rem; color:#475569; margin-top:2px; font-family:monospace; }
    .btn-row { display:grid; grid-template-columns:1fr 1fr; gap:10px; margin-top:14px; }
    .btn { padding:14px; border-radius:12px; font-family:'Kanit',sans-serif; font-size:0.9rem; font-weight:600; cursor:pointer; border:none; text-align:center; transition:transform 0.15s, opacity 0.15s; }
    .btn:active { transform:scale(0.95); }
    .btn-on  { background:linear-gradient(135deg,#16a34a,#15803d); color:white; }
    .btn-off { background:#0f172a; color:#64748b; border:1px solid #334155; }
    .led-indicator { display:flex; align-items:center; gap:10px; background:#0f172a; border-radius:10px; padding:12px 16px; margin:10px 0; }
    .led-dot { width:16px; height:16px; border-radius:50%; flex-shrink:0; }
    .led-dot.on  { background:#4ade80; box-shadow:0 0 12px #4ade80; animation:glow 1.5s infinite; }
    .led-dot.off { background:#1e293b; border:2px solid #334155; }
    @keyframes glow { 0%,100%{box-shadow:0 0 8px #4ade80} 50%{box-shadow:0 0 20px #4ade80, 0 0 35px rgba(74,222,128,0.3)} }
    .led-text { font-size:0.9rem; font-weight:600; }
    .footer { text-align:center; color:#334155; font-size:0.7rem; margin-top:16px; }
  </style>
</head>
<body>
  <div class="header">
    <h1>🧠 Study Focus Tracker</h1>
    <p>ESP32 Edge Intelligence System</p>
  </div>

  <div class="ip-badge" id="ipBox">📡 กำลังโหลด...</div>
  <div class="alarm-box" id="alarmBox">🚨 สมาธิต่ำ! LED กำลังกะพริบ — ปรับสภาพแวดล้อม!</div>

  <div class="score-card">
    <div class="score-label">Focus Score</div>
    <div class="score-number" id="sc">--</div>
    <div class="score-bar-wrap">
      <div class="score-bar" id="scBar" style="width:100%;background:#4ade80"></div>
    </div>
    <div class="score-status" id="scStatus">รอข้อมูล...</div>
  </div>

  <div class="led-indicator">
    <div class="led-dot off" id="ledDot"></div>
    <div class="led-text"  id="ledText">LED: กำลังโหลด...</div>
  </div>

  <div class="sensors-grid">
    <div class="sensor-card">
      <div class="s-label">💡 แสง (LDR)</div>
      <div class="s-val" id="li">...</div>
      <div class="s-raw" id="liRaw">raw: --</div>
    </div>
    <div class="sensor-card">
      <div class="s-label">🔊 เสียง</div>
      <div class="s-val" id="so">...</div>
      <div class="s-raw" id="soRaw">raw: --</div>
    </div>
    <div class="sensor-card full">
      <div class="s-label">🚶 การเคลื่อนไหว (PIR)</div>
      <div class="s-val" id="mo">...</div>
    </div>
  </div>

  <div class="btn-row">
    <button class="btn btn-on"  onclick="ctrlLED('on')">✅ เปิด LED</button>
    <button class="btn btn-off" onclick="ctrlLED('off')">❌ ปิด LED</button>
  </div>

  <div class="footer" id="lastUpdate">อัปเดตอัตโนมัติทุก 1 วินาที</div>

<script>
  function ctrlLED(cmd) {
    fetch('/led/' + cmd).catch(() => {});
  }

  function update() {
    fetch('/api/data').then(r => r.json()).then(d => {
      const sc = d.score;
      document.getElementById('sc').innerText = sc;
      const c = sc >= 70 ? '#4ade80' : '#f87171';
      document.getElementById('sc').style.color = c;
      document.getElementById('scBar').style.width = sc + '%';
      document.getElementById('scBar').style.background = c;
      document.getElementById('scStatus').innerText = sc >= 70 ? '✅ กำลังโฟกัส' : '⚠️ ไม่โฟกัส';
      document.getElementById('scStatus').style.color = c;
      document.getElementById('alarmBox').style.display = sc < 70 ? 'block' : 'none';
      document.getElementById('li').innerText   = d.light === 'Bright' ? 'สว่าง ☀️' : 'มืด 🌑';
      document.getElementById('li').style.color = d.light === 'Bright' ? '#4ade80' : '#f87171';
      document.getElementById('liRaw').innerText = 'raw: ' + d.ldr_raw;
      document.getElementById('so').innerText   = d.sound === 'Noise' ? 'มีเสียง 🔊' : 'เงียบ 🤫';
      document.getElementById('so').style.color = d.sound === 'Noise' ? '#fbbf24' : '#4ade80';
      document.getElementById('soRaw').innerText = 'raw: ' + d.sound_raw;
      document.getElementById('mo').innerText   = d.motion === 'Detected' ? 'ตรวจพบการเคลื่อนไหว 🚶' : 'นิ่งสงบ 🧘';
      document.getElementById('mo').style.color = d.motion === 'Detected' ? '#fbbf24' : '#4ade80';
      const ledOn = d.led_on;
      const dot = document.getElementById('ledDot');
      dot.className = 'led-dot ' + (ledOn ? 'on' : 'off');
      document.getElementById('ledText').innerText = ledOn
        ? '💡 LED: เปิดอยู่  (Mode: ' + d.led_mode + ')'
        : '⚫ LED: ปิดอยู่  (Mode: ' + d.led_mode + ')';
      document.getElementById('ledText').style.color = ledOn ? '#4ade80' : '#64748b';
      document.getElementById('ipBox').innerText = '📡 http://' + d.ip;
      const now = new Date();
      document.getElementById('lastUpdate').innerText = 'อัปเดตล่าสุด: ' + now.toLocaleTimeString('th-TH');
    }).catch(() => {});
  }

  setInterval(update, 1000);
  update();
</script>
</body>
</html>
)rawhtml";
  return html;
}

// ===================================================
// Core 0: ส่งข้อมูลหา Cloud
// ===================================================
void cloudTask(void * pvParameters) {
  for(;;) {
    if (WiFi.status() == WL_CONNECTED) {
      String lightStat  = (analogRead(LDR_PIN)   > 3500) ? "Dark"     : "Bright";
      String motionStat = (digitalRead(PIR_PIN) ==    1)  ? "Detected" : "NoMotion";
      String soundStat  = (analogRead(SOUND_PIN)  > 300)  ? "Noise"    : "Quiet";

      String json = "{\"device\":\"ESP32_01\","
                    "\"light_status\":\"" + lightStat  + "\","
                    "\"motion\":\""       + motionStat + "\","
                    "\"sound_status\":\"" + soundStat  + "\"}";

      Serial.println("[Cloud] 📤 กำลังส่ง: " + json);

      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;
      http.setTimeout(20000);
      http.begin(client, cloud_url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Accept", "application/json");

      int httpCode = http.POST(json);
      Serial.printf("[Cloud] HTTP Code: %d\n", httpCode);

      if (httpCode == 302 || httpCode == 301) {
        String newUrl = http.getLocation();
        Serial.println("[Cloud] 🔀 Redirect → " + newUrl);
        http.end();
        http.begin(client, newUrl);
        http.addHeader("Accept", "application/json");
        httpCode = http.GET();
        Serial.printf("[Cloud] HTTP Code (after redirect): %d\n", httpCode);
      }

      if (httpCode == 200) {
        String reply = http.getString();
        Serial.println("[Cloud] ✅ ตอบกลับ: " + reply);
        cloud_cmd = (reply.indexOf("\"command\":\"Alarm_on\"") > 0);
        int scoreIdx = reply.indexOf("\"score\":");
        if (scoreIdx > 0) g_focusScore = reply.substring(scoreIdx + 8).toInt();
        Serial.println("[Cloud] 🔔 Command: " + String(cloud_cmd ? "ALARM ON 🚨" : "Standby ✅"));
      } else {
        Serial.printf("[Cloud] ❌ Error: %d\n", httpCode);
      }
      http.end();
    } else {
      Serial.println("[Cloud] ⚠️  WiFi หลุด!");
    }
    vTaskDelay(15000 / portTICK_PERIOD_MS);
  }
}

// ===================================================
// Setup
// ===================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║     🧠 Study Focus Tracker ESP32      ║");
  Serial.println("║          CS423 Bangkok University     ║");
  Serial.println("╚══════════════════════════════════════╝");

  pinMode(PIR_PIN,    INPUT);
  pinMode(LED_PIN,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.print("📶 กำลังเชื่อม WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    if (++dots % 20 == 0) Serial.println();
  }

  g_ipAddress = WiFi.localIP().toString();
  Serial.println();
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║  ✅ WiFi เชื่อมสำเร็จ!               ║");
  Serial.printf( "║  📡 IP Address : %-20s║\n", g_ipAddress.c_str());
  Serial.printf( "║  🌐 เปิดมือถือ : http://%-16s║\n", g_ipAddress.c_str());
  Serial.println("╚══════════════════════════════════════╝");

  server.on("/", []() {
    server.send(200, "text/html; charset=utf-8", buildHTML());
  });

  server.on("/api/data", []() {
    int   score    = calculateLocalScore();
    float ldrRaw   = analogRead(LDR_PIN);
    int   pirVal   = digitalRead(PIR_PIN);
    int   soundRaw = analogRead(SOUND_PIN);
    String lightStat  = (ldrRaw   > 3500) ? "Dark"     : "Bright";
    String motionStat = (pirVal  == HIGH)  ? "Detected" : "NoMotion";
    String soundStat  = (soundRaw > 300)   ? "Noise"    : "Quiet";
    bool ledIsOn = false;
    if      (g_ledMode == "ON")    ledIsOn = true;
    else if (g_ledMode == "OFF")   ledIsOn = false;
    else if (g_ledMode == "ALARM") ledIsOn = true;
    else                           ledIsOn = manual_led;
    String json = "{";
    json += "\"ip\":\""       + g_ipAddress         + "\",";
    json += "\"light\":\""    + lightStat            + "\",";
    json += "\"motion\":\""   + motionStat           + "\",";
    json += "\"sound\":\""    + soundStat            + "\",";
    json += "\"ldr_raw\":"    + String((int)ldrRaw)  + ",";
    json += "\"sound_raw\":"  + String(soundRaw)     + ",";
    json += "\"score\":"      + String(score)        + ",";
    json += "\"led_on\":"     + String(ledIsOn ? "true" : "false") + ",";
    json += "\"led_mode\":\"" + g_ledMode            + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/led/on", []() {
    manual_led = true;
    g_ledMode  = "ON";
    Serial.println("[Web] 💡 Manual LED ON");
    server.sendHeader("Location", "/");
    server.send(302);
  });

  server.on("/led/off", []() {
    manual_led = false;
    g_ledMode  = "OFF";
    Serial.println("[Web] ⚫ Manual LED OFF");
    server.sendHeader("Location", "/");
    server.send(302);
  });

  server.begin();
  Serial.println("🌐 Web Server เริ่มทำงานแล้ว");

  xTaskCreatePinnedToCore(cloudTask, "CloudTask", 10000, NULL, 1, NULL, 0);
}

// ===================================================
// Loop (Core 1)
// ===================================================
void loop() {
  server.handleClient();

  g_light  = analogRead(LDR_PIN);
  g_motion = digitalRead(PIR_PIN);
  g_sound  = analogRead(SOUND_PIN);

  // อัปเดต LED Mode
  if      (manual_led && g_ledMode != "OFF") g_ledMode = "ON";
  else if (!manual_led && g_ledMode == "ON") g_ledMode = "AUTO";
  if (g_ledMode != "ON" && g_ledMode != "OFF") g_ledMode = "AUTO";

  printSerialStatus();

  // ===== คุม LED + Buzzer =====
  if (cloud_cmd) {
    // 🚨 ALARM — เสียง Siren ขึ้นลง + LED กระพริบตาม
    g_ledMode = "ALARM";
    for (int freq = 500; freq <= 1500; freq += 50) {
      tone(BUZZER_PIN, freq);
      digitalWrite(LED_PIN, HIGH);
      delay(20);
    }
    for (int freq = 1500; freq >= 500; freq -= 50) {
      tone(BUZZER_PIN, freq);
      digitalWrite(LED_PIN, LOW);
      delay(20);
    }
    noTone(BUZZER_PIN);
  }
  else if (g_ledMode == "ON" || manual_led) {
    // 💡 Manual ON
    digitalWrite(LED_PIN,    HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
  else {
    // ⚫ ปิด
    digitalWrite(LED_PIN,    LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}
