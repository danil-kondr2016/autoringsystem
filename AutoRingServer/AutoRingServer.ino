/*

AutoRingServer.ino

Copyright (C) 2019 Danila Kondratenko <dan.kondratenko2013@ya.ru>

    This file is part of Autoring System.

    Autoring System is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл - часть Системы автоподачи звонков.

   Система автоподачи звонков - свободная программа: вы можете перераспространять ее и/или изменять
   ее на условиях Стандартной общественной лицензии GNU в том виде, в каком
   она была опубликована Фондом свободного программного обеспечения; либо
   версии 3 лицензии, либо (по вашему выбору) любой более поздней версии.

   Эта программа распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

/*
 * Система автоматической подачи звонков
 * Серверная часть
 * 
 * Автор: Кондратенко Данила
 * Платформа: ESP8266 (ESP-12E), SPIFFS = 1 МБ
 * Периферия:
 *   4 МБ flash-память (для хранения программы);
 *   реле 5В=, нормально разомкнутое.
 * 
 */ 

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#include <DNSServer.h>
#include <stdio.h>
#include <string.h>

#include <FS.h>

uint16_t lessons[] = {
  510, 555,
  565, 610,
  630, 675,
  695, 740,
  755, 800,
  810, 855,
  865, 910,
  930, 975,
  985, 1030,
  1040, 1085,
  1440, 1485,
  1440, 1485,
  1440, 1485,
  1440, 1485,
  1440, 1485
};

#define NULLPWD "NULL"
#define PWDHASH "/PWDHASH"

const int ring = 4;

unsigned long Hr = 0;
unsigned long Mn = 0;
unsigned long Sc = 0;
unsigned long Ct = 0, St = 0;

unsigned long S = 0, R;

uint8_t rings[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

String methods[] = {"set", "doring", "schedule"};
const int nMethods = 3;

int   lesson_num  = 10;
bool  is_lesson   = false;

const char *ssid = "Autoring";

IPAddress apIP(192, 168, 4, 1);
ESP8266WebServer server(80);
DNSServer dns;

void update_time() {
  Ct = (St + (millis() - S) / 1000) % 86400;
  Hr = Ct / 3600;
  Mn = (Ct / 60) % 60;
  Sc = Ct % 60;
}

void set_time(int hr, int mn, int sec) {
  St = (hr * 3600 + mn * 60 + sec) % 86400;
  S = millis();
  Ct = (St + (millis() - S) / 1000) % 86400;
  Hr = Ct / 3600;
  Mn = (Ct / 60) % 60;
  Sc = Ct % 60;
}

void handleRoot() {
  String answer;
  
  char buf[256];
  answer += "<!DOCTYPE HTML PUBLIC \"-//DTD//HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n";
  answer += "<html>\r\n";
  answer += "\t<head>\r\n";
  answer += "\t\t<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n";
  answer += "\t\t<title>Система автоматической подачи звонков</title>\r\n";
  answer += "\t</head>\r\n";
  answer += "\t<body>\r\n";
  answer += "\t\t<h1 align=\"center\">Система автоматической подачи звонков</h1>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p align=\"center\">[<a href=\"/currstate\">Текущее состояние системы</a>]&nbsp;";
  answer += "\t\t[<a href=\"/timeset\">Изменить текущее время</a>]</p>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p align=\"center\">Создано Кондратенко Данилой в июле 2019 года.</p>\r\n";
  answer += "\t</body>\r\n";
  answer += "<html>\r\n";

  server.send(200, "text/html", answer);
}

void handleCurrState() {
  String answer;
  
  char buf[256];
  answer += "<!DOCTYPE HTML PUBLIC \"-//DTD//HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n";
  answer += "<html>\r\n";
  answer += "\t<head>\r\n";
  answer += "\t\t<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n";
  answer += "\t\t<title>Текущее состояние - Система автоматической подачи звонков</title>\r\n";
  answer += "\t</head>\r\n";
  answer += "\t<body>\r\n";
  answer += "\t\t<h1 align=\"center\">Система автоматической подачи звонков</h1>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p align=\"center\">[<a href=\"/\">На главную</a>]&nbsp;";
  answer += "\t\t[<a href=\"/timeset\">Изменить текущее время</a>]</p>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p><b>Текущее время системы: </b>";
  sprintf(buf, "%02d:%02d:%02d (%ld): ", Hr, Mn, Sc, Ct);
  answer += buf;
  if (is_lesson) answer += "идёт урок";
  else answer += "идёт перемена";
  answer += "</p>\r\n";
  answer += "\t\t<p><b>Текущее расписание: </b></p>\r\n";
  answer += "\t\t<table align=\"center\" border=\"1\">\r\n";
  answer += "\t\t\t<thead align=\"center\">\r\n";
  answer += "\t\t\t\t<tr><th>№</th><th>Начало урока</th><th>Конец урока</th><th>Кол-во звонков</th>\r\n";
  answer += "\t\t\t</thead>\r\n";
  answer += "\t\t\t<tbody align=\"center\">\r\n";
  for (int i = 0; i < lesson_num; i++) {
    answer += "\t\t\t\t<tr>\r\n";
    answer += "\t\t\t\t\t<td>" + String(i+1) + "</td>\r\n";
    sprintf(buf, "\t\t\t\t\t<td>%02d:%02d</td>\r\n", lessons[i*2] / 60, lessons[i*2] % 60);
    answer += buf;
    sprintf(buf, "\t\t\t\t\t<td>%02d:%02d</td>\r\n", lessons[i*2+1] / 60, lessons[i*2+1] % 60);
    answer += buf;
    answer += "\t\t\t\t\t<td>" + String(rings[i]) + "</td>\r\n";
    answer += "\t\t\t\t</tr>\r\n";
  }
  answer += "\t\t\t</tbody>\r\n";
  answer += "\t\t</table>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p align=\"center\">Создано Кондратенко Данилой в июле 2019 года.</p>\r\n";
  answer += "\t</body>\r\n";
  answer += "<html>\r\n";
  
  server.send(200, "text/html", answer);
}

void handleTimeSetPage() {
  String answer;

  answer += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n";
  answer += "<html>\r\n";
  answer += "\t<head>\r\n";
  answer += "\t\t<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n";
  answer += "\t\t<title>Установка времени - Система автоматической подачи звонков</title>\r\n";
  answer += "\t\t<script>\r\n";
  answer += "function set_time_values() {\r\n";
  answer += "\tvar now = new Date();\r\n";
  answer += "\tdocument.getElementById(\"client_time\").innerHTML = \"<b>Клиентское время: </b>\" + now.toLocaleTimeString()\r\n";
  answer += "\tdocument.getElementById(\"time\").value = now.toLocaleTimeString();\r\n";
  answer += "}\r\n";
  answer += "\r\n";
  answer += "window.onload = function() {\r\n";
  answer += "\tvar now = new Date();\r\n";
  answer += "\tdocument.getElementById(\"client_time\").innerHTML = \"<b>Клиентское время: </b>\" + now.toLocaleTimeString();\r\n";
  answer += "\tdocument.getElementById(\"time\").value = now.toLocaleTimeString();\r\n";
  answer += "\r\n";
  answer += "}\r\n";
  answer += "setInterval(set_time_values, 1000);\r\n";
  answer += "\t\t</script>\r\n";
  answer += "\t</head>\r\n";
  answer += "\t<body>\r\n";
  answer += "\t\t<h1 align=\"center\">Система автоматической подачи звонков</h1>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p align=\"center\">[<a href=\"/\">На главную</a>]&nbsp;\r\n";
  answer += "\t\t[<a href=\"/currstate\">Текущее состояние системы</a>]</p>\r\n";
  answer += "\t\t<hr>\r\n";
  answer += "\t\t<p id=\"client_time\"><b>Клиентское время:</b></p>\r\n";
  answer += "\t\t<form id=\"timeset\" method=\"post\" action=\"/autoring\">\r\n";
  answer += "\t\t\t<input type=\"hidden\" name=\"method\" value=\"set\">\r\n";
  answer += "\t\t\t<input type=\"hidden\" name=\"time\" id=\"time\" value=\"\">\r\n";
  answer += "\t\t\t<input type=\"submit\" value=\"Синхронизировать время\">\r\n";
  answer += "\t\t</form>\r\n";
  answer += "\t</body>\r\n";
  answer += "</html>\r\n";

  server.send(200, "text/html", answer);
}

void handleAutoring() {
  String method, answer = "", pwdhash = NULLPWD;

  /* Method validation */
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "method") method = server.arg(i);
  }

  bool isCorrect = false;
  for (int i = 0; i < nMethods; i++) {
    if (method == methods[i]) {
      isCorrect = true;
      break;
    }
  }

  if (!isCorrect) {
    answer = "state=2";
    server.send(200, "text/plain", answer);
    return;
  }

  /* Password validation */
  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "pwdhash") pwdhash = server.arg(i);
  }

  File hashfile = SPIFFS.open(PWDHASH, "r");
  String correctHash;
  char inc;
  while (hashfile.available()) {
    inc = hashfile.read();
    if (inc != '\r' && inc != '\n')
      correctHash += inc;
    else
      break;
  }

  hashfile.close();
  if ((pwdhash != correctHash) && (correctHash.length() > 0)) {
    answer = "state=1";
    server.send(200, "text/plain", answer);
    return;
  }

  int ls = 0, le = 0, rn = 0, ln = 0;
  int hour = 0, minute = 0, second = 0;
  int rt = 0, rp = 0;

  String schedule, newHash;
  for (int i = 0; (i < server.args()) && isCorrect; i++) {
    if (method == "set") {
      if (server.argName(i) == "schedule") {
        schedule = server.arg(i);
      } else if (server.argName(i) == "lessnum") {
        lesson_num = server.arg(i).toInt();
      } else if (server.argName(i) == "time") {
        sscanf(server.arg(i).c_str(), "%d:%d:%d", &hour, &minute, &second);
        set_time(hour, minute, second);
        is_lesson = false;
        for (int j = 0; j < lesson_num; j++) {
          if ((Hr * 60 + Mn) >= lessons[j * 2] && (Hr * 60 + Mn) <= lessons[j * 2 + 1]) {
            is_lesson = true;
            break;
          }
        }
      } else if (server.argName(i) == "passwd") {
        newHash = server.arg(i);
        hashfile = SPIFFS.open(PWDHASH, "w");
        for (int i = 0; i < newHash.length(); i++) {
          hashfile.write(newHash[i]);
        }
        hashfile.close();

        hashfile = SPIFFS.open(PWDHASH, "r");
        Serial.println("New PWDHASH:");
        while (hashfile.available())
          Serial.write(hashfile.read());
        Serial.println();
        
      } else {
        isCorrect = false;
      }
      answer = "state=0";
    } else if (method == "schedule") {
      answer = "";
      answer += "lessnum=" + String(lesson_num) + "&schedule=";
      for (int i = 0; i < lesson_num; i++) {
        answer += String(i + 1) + "-" + String(lessons[i * 2]) + "." + String(lessons[i * 2 + 1]) + "." + String(rings[i]);
        answer += "_";
      }
    } else if (method == "doring") {
      if (server.argName(i) == "number") 
        rn = server.arg(i).toInt();
      else if (server.argName(i) == "time")
        rt = server.arg(i).toInt();
      else if (server.argName(i) == "pause")
        rp = server.arg(i).toInt();
      else
        isCorrect = false;
    }
  }

  if (method == "set" && schedule.length() && isCorrect) {
    String record = "";
    for (int i = 0; i < schedule.length(); i++) {
      if (schedule[i] != '_') record += schedule[i];
      else {
        sscanf(record.c_str(), "%d-%d.%d.%d", &ln, &ls, &le, &rn);
        lessons[(ln - 1) * 2] = ls;
        lessons[(ln - 1) * 2 + 1] = le;
        rings[ln - 1] = rn;
        record = "";
      }
    }
<<<<<<< HEAD
  } else if (method == "doring" && isCorrect) {
=======
	  update_time();
	  is_lesson = false;
	  for (int j = 0; j < lesson_num; j++) {
      if ((Hr * 60 + Mn) >= lessons[j * 2] && (Hr * 60 + Mn) <= lessons[j * 2 + 1]) {
        is_lesson = true;
        break;
      }
    }
  } else if (method == "doring") {
>>>>>>> 81126f9f963d3c4d67b9a3a462bdcfddfc0aff01
    for (int j = 0; j < rn; j++) {
      digitalWrite(ring, HIGH);
      delay(rt * 1000);
      update_time();
      digitalWrite(ring, LOW);
      if (((j + 1) < rn) && (rp != 0)) {
        delay(rp * 1000);
        update_time();
      }
    }
    answer = "state=0";
  }

  if (!isCorrect) {
    answer = "state=2";
  }

  server.send(200, "text/plain", answer);
}

void setup() {
  SPIFFS.begin();
  pinMode(ring, OUTPUT);

  Serial.begin(115200);

  Serial.println();
  Serial.println("Contents of PWDHASH:");
  File hashfile = SPIFFS.open(PWDHASH, "r");
  if (!hashfile)
    Serial.println("ERROR: file not found");
  while (hashfile.available())
    Serial.write(hashfile.read());

  Serial.println("Configuring access point...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  
  Serial.println("Initalizing DNS server...");
  dns.setTTL(300);
  dns.start(53, "autoring.local", apIP);

  server.on("/", handleRoot);
  server.on("/autoring", handleAutoring);
  server.on("/currstate", handleCurrState);
  server.on("/timeset", handleTimeSetPage);
  server.begin();
  set_time(0, 0, 0);
}

void loop() {
  update_time();
  dns.processNextRequest();
  server.handleClient();
  
  for (int i = 0; i < lesson_num; i++) {
    if (((Hr * 60 + Mn) == lessons[i * 2]) && (lessons[i * 2] != 1440) && !(is_lesson)) {
      digitalWrite(ring, HIGH);
      delay(3000);
      digitalWrite(ring, LOW);
      delay(5000);
      digitalWrite(ring, HIGH);
      delay(3000);
      digitalWrite(ring, LOW);
      is_lesson = true;
      update_time();
    } else if (((Hr * 60 + Mn) == lessons[i * 2 + 1]) && (lessons[i * 2 + 1] != 1440) && (is_lesson)) {
      digitalWrite(ring, HIGH);
      delay(3000);
      digitalWrite(ring, LOW);
      if (rings[i] != 0) {
        for (int j = 0; j < rings[i]; j++) {
          digitalWrite(ring, HIGH);
          delay(1000);
          update_time();
          digitalWrite(ring, LOW);
          if ((j+1) < rings[i]) delay(1000);
        }
      }
      update_time();
      is_lesson = false;
    }
  }
}
