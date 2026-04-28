📘 Smart Study Focus Tracker                                                                                               
🚀 ระบบ IoT อัจฉริยะสำหรับวิเคราะห์สมาธิและสภาพแวดล้อมการเรียนแบบ Real-time                                                        
________________________________________
🧠 Overview                                                                                                                
Smart Study Focus Tracker คือระบบ IoT + Edge AI + Cloud AI ที่ถูกออกแบบมาเพื่อยกระดับประสิทธิภาพการเรียนรู้ โดยการตรวจจับและวิเคราะห์สภาพแวดล้อมรอบตัวผู้ใช้งานแบบเรียลไทม์
ระบบใช้แนวคิด Environmental Intelligence ในการประเมินปัจจัยที่มีผลต่อสมาธิ เช่น แสง การเคลื่อนไหว และเสียงรบกวน ผ่านเซนเซอร์หลายชนิดบนบอร์ด ESP32 จากนั้นจะทำการคำนวณ Focus Score (0–100) พร้อมส่งข้อมูลไปยัง Cloud เพื่อวิเคราะห์เชิงลึกด้วย AI และแสดงผลผ่าน Web Dashboard
________________________________________
✨ Features                                                                                                                
🔍 Intelligent Monitoring                                                                                                  
•	ตรวจวัดสภาพแวดล้อมแบบ Real-time ด้วย Multi-Sensor                                                                          
o	🌞 แสง (LDR)                                                                                                             
o	🚶 การเคลื่อนไหว (PIR)                                                                                                     
o	🔊 เสียงรบกวน (Sound Sensor)                                                                                              
________________________________________
⚡ Edge AI (On-device Intelligence)                                                                                        
•	คำนวณ Focus Score (0–100) บน ESP32 ทันที                                                                                  
•	ไม่ต้องรอ Cloud → ตอบสนองรวดเร็ว                                                                                             
•	เหมาะกับงาน Real-time IoT                                                                                                  
________________________________________
🤖 Cloud AI (Generative AI)                                                                                                
•	ใช้ Llama 3.1 (ผ่าน GROQ API)                                                                                              
•	วิเคราะห์ “เหตุผล” ของสภาพแวดล้อมเป็นภาษาไทย                                                                                  
•	ให้ Insight แบบเข้าใจง่าย                                                                                                    
________________________________________
☁️ Real-time Cloud Sync
•	ส่งข้อมูลขึ้น Google Sheets ทุก 15 วินาที 
•	เก็บเป็นฐานข้อมูลสำหรับวิเคราะห์ย้อนหลัง 
________________________________________
📊 Web Analytics Dashboard
•	แสดงผลแบบ Interactive: 
o	Focus Score (KPI) 
o	กราฟย้อนหลัง 
o	สถานะ Sensor 
o	AI Insight (Reason) 
•	อัปเดตอัตโนมัติแบบ Real-time 
________________________________________
🧊 3D Digital Twin
•	จำลองสภาพแวดล้อมแบบ 3 มิติด้วย Three.js 
•	เปลี่ยนพฤติกรรมตาม Sensor: 
o	สั่น = มีการเคลื่อนไหว 
o	คลื่น = มีเสียง 
o	สี = ระดับสมาธิ 
________________________________________
🎛️ Remote Control (Human-in-the-loop)
•	ควบคุมอุปกรณ์จาก Dashboard: 
o	🔴 LOCKDOWN → Siren + LED Alert 
o	⚪ STANDBY → ปิดการแจ้งเตือน 
________________________________________
🚨 Interactive Alert System
•	Popup แจ้งเตือนเมื่อ “เสียสมาธิ” 
•	บังคับให้ผู้ใช้กด “ตกลง” → ดึงโฟกัสกลับ 
________________________________________
🛠️ Tech Stack
🔌 Embedded / IoT
•	ESP32 DevKit V1 
•	Firmware: SmartStudyFocusTracker.ino 
•	ทำหน้าที่: 
o	Sensor Reading 
o	Edge AI 
o	Local Web Server 
________________________________________
☁️ Cloud Backend
•	Google Apps Script (Code.gs) 
•	ทำหน้าที่: 
o	รับข้อมูลจาก ESP32 
o	คำนวณ Score ซ้ำ (Validation) 
o	เรียกใช้ AI (Llama 3.1) 
o	บันทึกลง Google Sheets 
________________________________________
🌐 Frontend (Web Dashboard)
•	HTML + JavaScript 
•	ใช้: 
o	Chart.js → กราฟ 
o	Three.js → 3D 
•	ไฟล์: index.html 
________________________________________
🧩 System Architecture
[ Sensors (LDR / PIR / Sound) ]
                ↓
          [ ESP32 ]
   (Edge AI + Local Processing)
                ↓
        HTTP POST (JSON)
                ↓
[ Google Apps Script (Cloud) ]
     + AI (Llama 3.1 Analysis)
                ↓
        Google Sheets (DB)
                ↓
      Web Dashboard (Real-time)
________________________________________
⚙️ Edge Intelligence (Focus Score Logic)
ระบบเริ่มต้นที่ 100 คะแนน และหักตามเงื่อนไข:
•	🌑 แสงน้อย (Dark) → -30 
•	🚶 มีการเคลื่อนไหว → -30 
•	🔊 มีเสียงรบกวน → -30 
________________________________________
📊 การแปลผล
Score	Status	การทำงาน
70–100	Focused	ปกติ
< 70	Alarm_on	LED กระพริบ + แจ้งเตือน
________________________________________
🚨 Alert Logic
•	Score < 70
→ แสดง Popup + Dashboard Alert 
•	Manual Lockdown
→ Siren + LED (ควบคุมจากผู้ใช้) 
________________________________________
🔩 Hardware Components
•	ESP32 DevKit V1 
•	LDR Sensor 
•	PIR Motion Sensor (HC-SR501) 
•	Sound Sensor 
•	LED 
•	Buzzer 
•	Breadboard + Jumper Wires 
•	Resistor 220Ω 
________________________________________
⚙️ Setup
☁️ Cloud (Google Apps Script)
1.	สร้าง Google Sheets 
2.	เปิด Apps Script 
3.	วางโค้ด Code.gs 
4.	ใส่ GROQ_API_KEY 
5.	Deploy เป็น Web App 
6.	ตั้งค่าเป็น Anyone 
7.	นำ URL ไปใส่ใน ESP32 
________________________________________
🔧 Arduino IDE
•	Board: ESP32 Dev Module 
•	Upload Speed: 115200 
Libraries ที่ใช้:
•	WiFi 
•	HTTPClient 
•	WiFiClientSecure 
•	WebServer 
________________________________________
🚀 การใช้งาน
•	เปิด Dashboard 
•	เริ่มระบบ ESP32 
•	ดู Focus Score แบบ Real-time 
•	ตรวจสอบ AI Insight 
•	ปรับสภาพแวดล้อมตามคำแนะนำ 
________________________________________
📂 Project Structure
Smart-Study-Focus-Tracker/
│
├── SmartStudyFocusTracker.ino   # ESP32 Firmware
├── Code.gs                      # Cloud Backend + AI
├── index.html                   # Web Dashboard
└── README.md
________________________________________
💡 จุดเด่นของโปรเจกต์นี้
•	✅ IoT + AI + Web ครบวงจร 
•	✅ มี Edge AI จริงบนอุปกรณ์ 
•	✅ ใช้ Generative AI วิเคราะห์ 
•	✅ มี Dashboard + 3D Visualization 
•	✅ มี Human Interaction (Control System) 
________________________________________
📈 Future Improvements
•	📱 Mobile App 
•	🔔 Notification System 
•	🤖 AI Recommendation เชิงลึก 
•	☁️ User Authentication 
________________________________________
👨‍💻 Author
•	Apornphat 
•	ทีม CS423 – Internet of Things and Applications 
________________________________________
🏫 Academic Context
โปรเจกต์นี้เป็นส่วนหนึ่งของรายวิชา
CS423: Internet of Things and Applications 

