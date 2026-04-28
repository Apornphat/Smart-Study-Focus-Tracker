// ============================================================

// CS423 - Study Focus Tracker

// Google Apps Script - Code.gs

// ============================================================



const GROQ_API_KEY = ""; //ใส่ API 

const GROQ_URL = "https://api.groq.com/openai/v1/chat/completions";



// ============================================================

// doPost - รับข้อมูลจาก ESP32

// ============================================================

function doPost(e) {

  var ss = SpreadsheetApp.getActiveSpreadsheet();

  var sheet = ss.getSheets()[0];



  try {

    if (!e.postData || !e.postData.contents) return ContentService.createTextOutput("No Data");

    var data = JSON.parse(e.postData.contents);

    var timestamp = new Date();



    // 1. คำนวณคะแนน

    var finalScore = 100;

    if (data.light_status === "Dark")     finalScore -= 30;

    if (data.motion === "Detected")       finalScore -= 30;

    if (data.sound_status === "Noise")    finalScore -= 30;



    var finalStatus = (finalScore >= 70) ? "Focused" : "Alarm_on";



    // 2. สร้าง Prompt ให้ AI

    var prompt =

      "ข้อมูลเซนเซอร์: แสง=" + data.light_status +

      ", การเคลื่อนไหว=" + data.motion +

      ", เสียง=" + data.sound_status + ".\n\n" +

      "ภารกิจ: เขียนเหตุผล (Reason) 1 ประโยคสั้นๆ ขึ้นต้นด้วย 'สภาพแวดล้อม...'\n\n" +

      "คลังคำที่บังคับใช้:\n" +

      "- Bright → แสงเพียงพอ / สว่างพอดี / แสงไฟนวลตา\n" +

      "- Dark   → แสงน้อย / ค่อนข้างมืด / แสงสว่างไม่เพียงพอ\n" +

      "- Detected  → มีการเคลื่อนไหวรอบข้าง / พื้นที่เริ่มไม่นิ่งสงบ\n" +

      "- NoMotion  → ไร้ความเคลื่อนไหว / พื้นที่สงบนิ่ง\n" +

      "- Quiet  → เงียบสงัด / ไร้เสียงรบกวน\n" +

      "- Noise  → มีเสียงดังรบกวน / บรรยากาศเริ่มวุ่นวาย\n\n" +

      "กฎ: ใช้คำตรงกับค่าเซนเซอร์เท่านั้น, ภาษาไทยล้วน, ห้ามซ้ำ\n\n" +

      "ตอบกลับเป็น JSON เท่านั้น: {\"reason\":\"string\"}";



    var payload = {

      "model": "llama-3.1-8b-instant",

      "messages": [

        {

          "role": "system",

          "content": "คุณคือ AI วิเคราะห์สภาพแวดล้อมตามข้อมูลจริง 100% ใช้ภาษาไทยสั้นกระชับ"

        },

        { "role": "user", "content": prompt }

      ],

      "response_format": { "type": "json_object" },

      "temperature": 0.8

    };



    var options = {

      "method": "post",

      "headers": {

        "Authorization": "Bearer " + GROQ_API_KEY,

        "Content-Type": "application/json"

      },

      "payload": JSON.stringify(payload),

      "muteHttpExceptions": true

    };



    var response = UrlFetchApp.fetch(GROQ_URL, options);

    var ai = JSON.parse(JSON.parse(response.getContentText()).choices[0].message.content);



    // 3. บันทึกลง Sheet

    sheet.appendRow([

      timestamp,

      data.device,

      data.light_status,

      data.motion,

      data.sound_status,

      finalScore,

      finalStatus,

      ai.reason

    ]);



    // 4. เช็ค Manual Command จาก Cell I1

    var manualCommand = sheet.getRange("I1").getValue();

    var finalCommand = (manualCommand != "" && manualCommand != null)

                       ? manualCommand

                       : finalStatus;

    

    return ContentService

      .createTextOutput(JSON.stringify({

        "status":  "ok",

        "command": finalCommand,

        "score":   finalScore

      }))

      .setMimeType(ContentService.MimeType.JSON);



  } catch (err) {

    return ContentService.createTextOutput("Error: " + err.message);

  }

}



// ============================================================

// doGet - Dashboard + รับคำสั่ง + ส่งข้อมูล

// ============================================================

function doGet(e) {

  var ss = SpreadsheetApp.getActiveSpreadsheet();

  var sheet = ss.getSheets()[0];



  // --- รับคำสั่งจาก Dashboard (?action=Alarm_on หรือ ?action=) ---

  if (e.parameter.action !== undefined) {

    sheet.getRange("I1").setValue(e.parameter.action);

    return ContentService

      .createTextOutput("Command '" + e.parameter.action + "' saved")

      .setMimeType(ContentService.MimeType.TEXT);

  }



  // --- ส่งข้อมูลล่าสุด (?fetch=1) ---

  if (e.parameter.fetch) {

    var lastRow = sheet.getLastRow();

    if (lastRow < 2) {

      return ContentService

        .createTextOutput(JSON.stringify({ error: "no data" }))

        .setMimeType(ContentService.MimeType.JSON);

    }

    var row = sheet.getRange(lastRow, 1, 1, 8).getValues()[0];

    var result = {

      "timestamp":    Utilities.formatDate(row[0], "GMT+7", "HH:mm:ss"),

      "device":       row[1],

      "light_status": row[2],

      "motion":       row[3],

      "sound_status": row[4],

      "focus_score":  row[5],

      "focus_status": row[6],

      "reason":       row[7]

    };

    return ContentService

      .createTextOutput(JSON.stringify(result))

      .setMimeType(ContentService.MimeType.JSON);

  }



  // --- ส่งประวัติย้อนหลัง 20 แถว (?history=1) ---

  if (e.parameter.history) {

    var lastRow = sheet.getLastRow();

    if (lastRow < 2) {

      return ContentService

        .createTextOutput(JSON.stringify([]))

        .setMimeType(ContentService.MimeType.JSON);

    }

    var startRow = Math.max(2, lastRow - 19);

    var numRows  = lastRow - startRow + 1;

    var dataRange = sheet.getRange(startRow, 1, numRows, 8).getValues();



    var history = dataRange.map(function(row) {

      return {

        "timestamp": Utilities.formatDate(row[0], "GMT+7", "HH:mm:ss"),

        "device":    row[1],

        "light":     row[2],

        "motion":    row[3],

        "sound":     row[4],

        "score":     row[5],

        "status":    row[6],

        "reason":    row[7]

      };

    });



    return ContentService

      .createTextOutput(JSON.stringify(history))

      .setMimeType(ContentService.MimeType.JSON);

  }



  // --- แสดงหน้า Dashboard HTML ---

  // ★ inject URL จริงของ GAS ผ่าน template เพื่อให้ fetch() ใช้ได้ถูกต้อง

  var template = HtmlService.createTemplateFromFile('index');

  template.gasUrl = ScriptApp.getService().getUrl();

  return template.evaluate()

    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);

}




