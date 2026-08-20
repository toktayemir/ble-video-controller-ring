#include <Adafruit_TinyUSB.h>
#include <bluefruit.h>

BLEDis bledis;
BLEHidAdafruit blehid;

// Physical directions after correcting the joystick's rotated mounting.
const uint8_t buttonPins[] = {
  D1, // Up
  D0, // Down
  D3, // Left
  D2, // Right
  D4  // Center press
};

bool previousState[5] = {
  HIGH, HIGH, HIGH, HIGH, HIGH
};

unsigned long lastPressTime[5] = {
  0, 0, 0, 0, 0
};

void startAdvertising() {
  Bluefruit.Advertising.addFlags(
    BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE
  );

  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void sendCharacter(char key) {
  if (!Bluefruit.connected()) {
    Serial.println("Bluetooth is not connected.");
    return;
  }

  blehid.keyPress(key);
  delay(40);
  blehid.keyRelease();
}

void sendArrowKey(uint8_t keyCode) {
  if (!Bluefruit.connected()) {
    Serial.println("Bluetooth is not connected.");
    return;
  }

  uint8_t keys[6] = {keyCode, 0, 0, 0, 0, 0};

  blehid.keyboardReport(0, keys);
  delay(40);
  blehid.keyRelease();
}

void sendPlayPause() {
  if (!Bluefruit.connected()) {
    Serial.println("Bluetooth is not connected.");
    return;
  }

  blehid.consumerKeyPress(HID_USAGE_CONSUMER_PLAY_PAUSE);
  delay(40);
  blehid.consumerKeyRelease();
}

void runCommand(int buttonNumber) {
  switch (buttonNumber) {
    case 0:
      Serial.println("Increase playback speed");
      sendCharacter('>');
      break;

    case 1:
      Serial.println("Decrease playback speed");
      sendCharacter('<');
      break;

    case 2:
      Serial.println("Seek backward");
      sendArrowKey(HID_KEY_ARROW_LEFT);
      break;

    case 3:
      Serial.println("Seek forward");
      sendArrowKey(HID_KEY_ARROW_RIGHT);
      break;

    case 4:
      Serial.println("Play / Pause");
      sendPlayPause();
      break;
  }
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  Bluefruit.begin();
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Video Kumanda");

  bledis.setManufacturer("Emir Toktay");
  bledis.setModel("Video Controller V1");
  bledis.begin();

  blehid.begin();
  startAdvertising();

  Serial.println("BLE Video Controller is ready.");
}

void loop() {
  for (int i = 0; i < 5; i++) {
    bool currentState = digitalRead(buttonPins[i]);

    if (
      currentState == LOW &&
      previousState[i] == HIGH &&
      millis() - lastPressTime[i] > 250
    ) {
      lastPressTime[i] = millis();
      runCommand(i);
    }

    previousState[i] = currentState;
  }

  delay(10);
}
