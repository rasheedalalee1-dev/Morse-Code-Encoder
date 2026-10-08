
#define BLYNK_TEMPLATE_ID "TMPL6_mi7a8xi"
#define BLYNK_TEMPLATE_NAME "final project"
#define BLYNK_AUTH_TOKEN "82vK3R2eSz4kiuBqGNQx27UxM2pyRU4z"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// =====================================================
// Wi-Fi
// =====================================================

char ssid[] = "Wokwi-GUEST";
char pass[] = "";


// =====================================================
// ESP32 PINS
// =====================================================

#define LED_PIN       2
#define BUZZER_PIN    4

#define DOT_BUTTON    18
#define DASH_BUTTON   19


// =====================================================
// BLYNK VIRTUAL PINS
// =====================================================

// غيّرها لاحقاً حسب تصميم واجهة Blynk

#define VPIN_MODE        V0
#define VPIN_INPUT_TEXT  V1
#define VPIN_MORSE       V2
#define VPIN_OUTPUT_TEXT V3
#define VPIN_STATUS      V4
#define VPIN_CLEAR       V5


// =====================================================
// MORSE TIMING
// =====================================================

#define DOT_TIME       200
#define DASH_TIME      600
#define SYMBOL_SPACE   200
#define LETTER_SPACE   600
#define WORD_SPACE     1400


// =====================================================
// MORSE TABLE
// =====================================================

struct MorseMap {
  char letter;
  const char* code;
};

MorseMap morseTable[] = {

  {' ', "..--"},

  {'A', ".-"},
  {'B', "-..."},
  {'C', "-.-."},
  {'D', "-.."},
  {'E', "."},
  {'F', "..-."},
  {'G', "--."},
  {'H', "...."},
  {'I', ".."},
  {'J', ".---"},
  {'K', "-.-"},
  {'L', ".-.."},
  {'M', "--"},
  {'N', "-."},
  {'O', "---"},
  {'P', ".--."},
  {'Q', "--.-"},
  {'R', ".-."},
  {'S', "..."},
  {'T', "-"},
  {'U', "..-"},
  {'V', "...-"},
  {'W', ".--"},
  {'X', "-..-"},
  {'Y', "-.--"},
  {'Z', "--.."},

  {'0', "-----"},
  {'1', ".----"},
  {'2', "..---"},
  {'3', "...--"},
  {'4', "....-"},
  {'5', "....."},
  {'6', "-...."},
  {'7', "--..."},
  {'8', "---.."},
  {'9', "----."}
};

const int MORSE_COUNT =
  sizeof(morseTable) / sizeof(morseTable[0]);


// =====================================================
// SYSTEM VARIABLES
// =====================================================

bool encoderMode = false;

String currentMorse = "";
String decodedMessage = "";


// =====================================================
// GET MORSE CODE
// =====================================================

const char* getMorse(char c) {

  c = toupper(c);

  for (int i = 0; i < MORSE_COUNT; i++) {

    if (morseTable[i].letter == c) {
      return morseTable[i].code;
    }

  }

  return "";
}


// =====================================================
// DECODE MORSE
// =====================================================

char decodeMorse(String code) {

  for (int i = 0; i < MORSE_COUNT; i++) {

    if (code.equals(morseTable[i].code)) {
      return morseTable[i].letter;
    }

  }

  return '?';
}


// =====================================================
// HARDWARE SIGNAL ON
// =====================================================

void signalOn() {

  digitalWrite(LED_PIN, HIGH);

  tone(BUZZER_PIN, 1000);
  tone(BUZZER_PIN, 2000);
  tone(BUZZER_PIN, 3000);
  tone(BUZZER_PIN, 4000);
  tone(BUZZER_PIN, 5000);
  tone(BUZZER_PIN, 6000);
}


// =====================================================
// HARDWARE SIGNAL OFF
// =====================================================

void signalOff() {

  digitalWrite(LED_PIN, LOW);

  noTone(BUZZER_PIN);
}


// =====================================================
// SEND ONE MORSE SYMBOL
// =====================================================

void sendSymbol(char symbol) {

  signalOn();

  if (symbol == '.') {

    delay(DOT_TIME);

  } else {

    delay(DASH_TIME);

  }

  signalOff();

  delay(SYMBOL_SPACE);
}


// =====================================================
// ENCODE TEXT
// =====================================================

void encodeText(String text) {

  text.toUpperCase();

  String completeMorse = "";

  for (int i = 0; i < text.length(); i++) {

    char c = text.charAt(i);

    // Space between words
    if (c == ' ') {

      completeMorse += "/ ";

      continue;
    }

    const char* code = getMorse(c);

    if (code[0] == '\0') {

      completeMorse += "? ";

      continue;
    }

    completeMorse += code;
    completeMorse += " ";

  }


  // ---------------------------------------------------
  // Send Morse to Blynk
  // ---------------------------------------------------

  Blynk.virtualWrite(VPIN_MORSE, completeMorse);


  // ---------------------------------------------------
  // Transmit using LED + Buzzer
  // ---------------------------------------------------

  for (int i = 0; i < text.length(); i++) {

    char c = text.charAt(i);

    // Word space
    if (c == ' ') {

      delay(WORD_SPACE);

      continue;
    }

    const char* code = getMorse(c);

    if (code[0] == '\0') {
      continue;
    }


    for (int j = 0; code[j] != '\0'; j++) {

      sendSymbol(code[j]);

    }

    delay(LETTER_SPACE);
  }


  signalOff();


  // ---------------------------------------------------
  // Status
  // ---------------------------------------------------

  Blynk.virtualWrite(
    VPIN_STATUS,
    "Encoding completed"
  );
}


// =====================================================
// ADD DOT
// =====================================================

void addDot() {

  currentMorse += ".";

  Serial.print(".");

  Blynk.virtualWrite(
    VPIN_MORSE,
    currentMorse
  );

  signalOn();

  delay(DOT_TIME);

  signalOff();
}


// =====================================================
// ADD DASH
// =====================================================

void addDash() {

  currentMorse += "-";

  Serial.print("-");

  Blynk.virtualWrite(
    VPIN_MORSE,
    currentMorse
  );

  signalOn();

  delay(DASH_TIME);

  signalOff();
}


// =====================================================
// FINISH LETTER
// =====================================================

void finishLetter() {

  if (currentMorse.length() == 0) {
    return;
  }

  char decoded = decodeMorse(currentMorse);


  if (decoded != '?') {

    decodedMessage += decoded;

    Serial.print(" = ");
    Serial.println(decoded);

    Blynk.virtualWrite(
      VPIN_OUTPUT_TEXT,
      decodedMessage
    );

  }


  currentMorse = "";

  Blynk.virtualWrite(
    VPIN_MORSE,
    ""
  );
}


// =====================================================
// BLYNK - MODE
// =====================================================

BLYNK_WRITE(VPIN_MODE) {

  int value = param.asInt();


  if (value == 1) {

    encoderMode = true;

    currentMorse = "";
    decodedMessage = "";

    Blynk.virtualWrite(
      VPIN_STATUS,
      "ENCODER MODE"
    );

    Serial.println("ENCODER MODE");

  }

  else {

    encoderMode = false;

    currentMorse = "";
    decodedMessage = "";

    Blynk.virtualWrite(
      VPIN_STATUS,
      "DECODER MODE"
    );

    Serial.println("DECODER MODE");

  }
}


// =====================================================
// BLYNK - TEXT INPUT
// =====================================================

BLYNK_WRITE(VPIN_INPUT_TEXT) {

  if (!encoderMode) {
    return;
  }

  String text = param.asString();

  if (text.length() == 0) {
    return;
  }


  Serial.print("Input: ");
  Serial.println(text);


  Blynk.virtualWrite(
    VPIN_OUTPUT_TEXT,
    text
  );


  encodeText(text);
}


// =====================================================
// BLYNK - CLEAR
// =====================================================

BLYNK_WRITE(VPIN_CLEAR) {

  if (param.asInt() == 1) {

    currentMorse = "";
    decodedMessage = "";

    Blynk.virtualWrite(VPIN_INPUT_TEXT, "");
    Blynk.virtualWrite(VPIN_MORSE, "");
    Blynk.virtualWrite(VPIN_OUTPUT_TEXT, "");
    Blynk.virtualWrite(VPIN_STATUS, "Cleared");

    Serial.println("System cleared");

  }
}


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);


  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(DOT_BUTTON, INPUT_PULLUP);
  pinMode(DASH_BUTTON, INPUT_PULLUP);


  signalOff();


  Serial.println();
  Serial.println("==============================");
  Serial.println("      ESP32 MORSE SYSTEM");
  Serial.println("==============================");


  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );


  Blynk.virtualWrite(
    VPIN_STATUS,
    "System Ready"
  );
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  Blynk.run();


  // ===================================================
  // DECODER MODE
  // ===================================================

  if (!encoderMode) {

    // DOT BUTTON

    if (digitalRead(DOT_BUTTON) == LOW) {

      addDot();

      while (digitalRead(DOT_BUTTON) == LOW) {
        delay(10);
      }

      delay(150);
    }


    // DASH BUTTON

    if (digitalRead(DASH_BUTTON) == LOW) {

      addDash();

      while (digitalRead(DASH_BUTTON) == LOW) {
        delay(10);
      }

      delay(150);
    }


    // -------------------------------------------------
    // Automatic letter detection
    //
    // إذا لم يتم الضغط على أي زر لمدة 1 ثانية
    // يعتبر الحرف مكتملًا
    // -------------------------------------------------

    static unsigned long lastInputTime = 0;

    if (currentMorse.length() > 0) {

      if (lastInputTime == 0) {
        lastInputTime = millis();
      }

      if (digitalRead(DOT_BUTTON) == LOW ||
          digitalRead(DASH_BUTTON) == LOW) {

        lastInputTime = millis();

      }

      if (millis() - lastInputTime > 1000) {

        finishLetter();

        lastInputTime = 0;
      }
    }
  }
}