#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ESP32Servo.h>
#include "time.h"

// Replace with your Wi-Fi network credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your own bot token from BotFather
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Authorized Users
unsigned long long authorizedUsers[] = {
  1234567890 // Replace with your own Telegram ID
};
const int userCount = sizeof(authorizedUsers) / sizeof(authorizedUsers[0]);

#define SERVO_PIN 13
Servo feederServo;
int feedCount = 0;

unsigned long lastCheck = 0;
const unsigned long checkInterval = 2000; // Interval to check messages and timers (ms)

bool hasSchedule = false;
int scheduleHour = 0, scheduleMinute = 0;
int schedulePortions = 1;
bool scheduleTriggeredToday = false;

bool hasCountdown = false;
unsigned long countdownTargetMillis = 0;
int countdownPortions = 1;

bool isAuthorized(unsigned long long chat_id) {
  for (int i = 0; i < userCount; i++) {
    if (chat_id == authorizedUsers[i]) return true;
  }
  return false;
}

void feedFish(int times) {
  for (int i = 0; i < times; i++) {
    feederServo.write(90);
    delay(1000);
    feederServo.write(0);
    delay(500);
  }
  feedCount += times;
}

// Handle incoming Telegram messages
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id_str = bot.messages[i].chat_id;
    unsigned long long chat_id = strtoull(chat_id_str.c_str(), NULL, 10);
    String text = bot.messages[i].text;

    if (!isAuthorized(chat_id)) {
      bot.sendMessage(chat_id_str, "🚫 You are not authorized to use this bot.", "");
      continue;
    }

    if (text == "/start" || text == "/help") {
      String msg = "🐟 Fish Feeder Bot Help\n\n";
      msg += "📤 Manual Feed:\n";
      msg += "/feed1 - Feed 1 portion\n";
      msg += "/feed2 - Feed 2 portions\n";
      msg += "/feed3 - Feed 3 portions\n";
      msg += "/feed4 - Feed 4 portions\n";
      msg += "/feed5 - Feed 5 portions\n\n";
      msg += "⏳ Countdown:\n";
      msg += "/countdown N [P] - Feed after N seconds (P portions)\n\n";
      msg += "📅 Schedule:\n";
      msg += "/schedule HH:MM [P] - Feed daily at HH:MM (P portions)\n\n";
      msg += "📊 Info:\n";
      msg += "/status - Show Wi-Fi, counters, timers\n";
      bot.sendMessage(chat_id_str, msg, "");
    }

    else if (text == "/status") {
      String msg = "📡 WiFi: " + WiFi.SSID() + "\n";
      msg += "🍽 Portions fed since startup: " + String(feedCount);
      if (hasSchedule) {
        msg += "\n📅 Schedule: " + String(scheduleHour) + ":" + (scheduleMinute < 10 ? "0" : "") + String(scheduleMinute);
        msg += " (" + String(schedulePortions) + " portion(s))";
      }
      if (hasCountdown) {
        long remaining = (countdownTargetMillis - millis()) / 1000;
        msg += "\n⏳ Countdown: " + String(max(0L, remaining)) + "s remaining (" + String(countdownPortions) + " portion(s))";
      }
      bot.sendMessage(chat_id_str, msg, "");
    }

    else if (text.startsWith("/feed")) {
      if (text.length() == 6) {
        int count = text.charAt(5) - '0';
        if (count >= 1 && count <= 5) {
          feedFish(count);
          bot.sendMessage(chat_id_str, "✅ Fed " + String(count) + " portion(s).", "");
        } else {
          bot.sendMessage(chat_id_str, "⚠️ Invalid portion count. Use /feed1 to /feed5.", "");
        }
      } else {
        bot.sendMessage(chat_id_str, "⚠️ Invalid command format. Use /feed1 to /feed5.", "");
      }
    }

    else if (text.startsWith("/countdown")) {
      String arg = text.substring(10);
      arg.trim();
      int spaceIndex = arg.indexOf(' ');
      int sec = -1, portions = 1;

      if (spaceIndex == -1) {
        sec = arg.toInt();
      } else {
        sec = arg.substring(0, spaceIndex).toInt();
        portions = arg.substring(spaceIndex + 1).toInt();
      }

      if (sec > 0 && sec <= 3600 && portions >= 1 && portions <= 5) {
        countdownTargetMillis = millis() + sec * 1000UL;
        hasCountdown = true;
        countdownPortions = portions;
        bot.sendMessage(chat_id_str, "⏳ Countdown set: " + String(sec) + "s, " + String(portions) + " portion(s).", "");
      } else {
        bot.sendMessage(chat_id_str, "⚠️ Use format: /countdown <seconds> [1–5 portions]\nExample: /countdown 30 2", "");
      }
    }

    else if (text.startsWith("/schedule")) {
      String arg = text.substring(9);
      arg.trim();
      int spaceIndex = arg.indexOf(' ');
      String timeStr;
      int portions = 1;

      if (spaceIndex == -1) {
        timeStr = arg;
      } else {
        timeStr = arg.substring(0, spaceIndex);
        portions = arg.substring(spaceIndex + 1).toInt();
      }

      int colonIndex = timeStr.indexOf(":");
      if (colonIndex > 0) {
        int h = timeStr.substring(0, colonIndex).toInt();
        int m = timeStr.substring(colonIndex + 1).toInt();
        if (h >= 0 && h <= 23 && m >= 0 && m <= 59 && portions >= 1 && portions <= 5) {
          scheduleHour = h;
          scheduleMinute = m;
          schedulePortions = portions;
          hasSchedule = true;
          scheduleTriggeredToday = false;
          bot.sendMessage(chat_id_str, "📅 Daily feed set: " + String(h) + ":" + (m < 10 ? "0" : "") + String(m) + ", " + String(portions) + " portion(s).", "");
        } else {
          bot.sendMessage(chat_id_str, "⚠️ Invalid time or portions. Format: /schedule HH:MM [1–5]", "");
        }
      } else {
        bot.sendMessage(chat_id_str, "⚠️ Format: /schedule HH:MM [P]\nExample: /schedule 08:30 3", "");
      }
    }

    else {
      bot.sendMessage(chat_id_str, "❓ Unknown command. Type /help to see available commands.", "");
    }
  }
}

// Set up NTP time
void setupTime() {
  configTime(25200, 0, "pool.ntp.org", "time.nist.gov");  // UTC+7
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("⌛ Waiting for NTP server...");
    delay(1000);
  }
  Serial.println("✅ Time synced successfully");
}

void setup() {
  Serial.begin(115200);
  feederServo.attach(SERVO_PIN);
  feederServo.write(0);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected to Wi-Fi: " + WiFi.localIP().toString());

  client.setInsecure();
  setupTime();
}

void loop() {
  if (millis() - lastCheck > checkInterval) {
    int newMessages = bot.getUpdates(bot.last_message_received + 1);
    if (newMessages) {
      handleNewMessages(newMessages);
    }

    if (hasCountdown && millis() >= countdownTargetMillis) {
      feedFish(countdownPortions);
      hasCountdown = false;
    }

    if (hasSchedule) {
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        if (timeinfo.tm_hour == scheduleHour && timeinfo.tm_min == scheduleMinute) {
          if (!scheduleTriggeredToday) {
            feedFish(schedulePortions);
            scheduleTriggeredToday = true;
          }
        } else {
          scheduleTriggeredToday = false;
        }
      }
    }

    lastCheck = millis();
  }
}
