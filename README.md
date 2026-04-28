Smart Study Focus Tracker
ระบบ IoT อัจฉริยะที่ช่วยยกระดับประสิทธิภาพการเรียนโดยการวิเคราะห์สภาพแวดล้อมและระดับสมาธิแบบ Real-time เพื่อสร้างบรรยากาศที่เหมาะสมที่สุดสำหรับการทำงาน
Smart Study Focus Tracker ใช้หลักการ Environmental Intelligence ผ่านบอร์ด ESP32 และเซนเซอร์หลายมิติเพื่อตรวจวัดปัจจัยที่มีผลต่อสมาธิ ระบบจะคำนวณ Focus Score และใช้ Cloud AI ในการวิเคราะห์เหตุผลพร้อมให้คำแนะนำในการปรับปรุงพื้นที่เรียนโดยอัตโนมัติ

Features
Multi-sensor monitoring — ติดตามค่าความเข้มแสง (LDR), การเคลื่อนไหว (PIR) และระดับเสียงรบกวน (Sound Sensor)   
Edge AI Analytics — คำนวณคะแนนสมาธิ (Focus Score) 0-100 ทันทีบนบอร์ด ESP32 เพื่อการตอบสนองที่รวดเร็ว   
Generative AI Integration — เชื่อมต่อ Llama 3.1 AI (ผ่าน Groq API) เพื่อวิเคราะห์เหตุผลเชิงลึกของสภาพแวดล้อมในรูปแบบภาษาไทยที่เข้าใจง่าย   
Real-time Cloud Sync — ส่งข้อมูลขึ้น Google Sheets ทุก 15 วินาที เพื่อบันทึกประวัติและแสดงผลย้อนหลัง   
Web Analytics Dashboard — หน้าจอแสดงผลกราฟสมาธิย้อนหลัง, KPI สถานะปัจจุบัน และประวัติเหตุผลจาก AI   
3D Digital Twin — แสดงผลจำลองสภาพแวดล้อมแบบ 3 มิติผ่าน Three.js บนหน้าจอ Dashboard   
Human Value (Manual Override) — ระบบควบคุมระยะไกล สามารถสั่งเปิด/ปิด LOCKDOWN ALARM (Buzzer Siren + LED) ได้จากหน้า Dashboard  
Interactive Popup — ป๊อปอัพแจ้งเตือนบนหน้าจอที่จะค้างไว้จนกว่าผู้ใช้จะกด "ตกลง" เพื่อดึงสมาธิกลับมาสู่การเรียน 

อุปกรณ์ที่ใช้ (Hardware Components)
ESP32 DevKit V1 (ESP-WROOM-32)
LDR Photosensitive Sensor Module
PIR Motion Sensor (HC-SR501)
Sound Sensor Module
LED
Buzzer
Breadboard
Jumper Wire
ตัวต้านทาน (220Ω)

Setup
1. Cloud Services (Google Apps Script)
สร้าง Google Sheets ใหม่
เปิดเมนู Extensions > Apps Script แล้วนำโค้ดในไฟล์ Code.gs ไปวาง   
ใส่ GROQ_API_KEY ของคุณในโค้ด   
กด Deploy > New Deployment เลือกประเภท Web App และตั้งค่า Who has access เป็น Anyone   
คัดลอก Web App URL เพื่อนำไปใส่ในโค้ด Arduino   
2. Arduino IDE Settings
Board: ESP32 Dev Module
Upload Speed: 115200
Libraries Required: WiFi, HTTPClient, WiFiClientSecure, WebServer

Edge Intelligence — How it works 
ระบบคำนวณคะแนนสมาธิ (Focus Score) จากปัจจัยพื้นฐาน 100 คะแนน:   
Dark (แสงน้อย): หัก 30 คะแนน   
Motion Detected (มีการเคลื่อนไหว): หัก 30 คะแนน   
Noise (มีเสียงรบกวน): หัก 30 คะแนน   
Logic การแจ้งเตือน:
Score 70-100: สถานะ "Focused" - สภาพแวดล้อมเหมาะสม   
Score < 70: สถานะ "Alarm_on" - LED เริ่มกะพริบแจ้งเตือน และป๊อปอัพบนหน้าจอแสดงผล   
Manual Lockdown: สั่งงานจาก Dashboard เพื่อให้บอร์ดส่งเสียง Siren และไฟกะพริบเร็วทันที   




