#include <Wire.h>  
#include <Adafruit_GFX.h>  
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int electromagnetPin1 = 8;
const int electromagnetPin2 = 9;
const int ledPin1 = 2;  
const int ledPin2 = 3;  

const String targetBarcode1 = "PE0275";
const String targetBarcode2 = "PE0513";
const String degradedModeBarcode = "14810";

unsigned long lastDegradedTime = 0;
const unsigned long degradedModeTimeout = 30000;
bool degradedModeActive = false;

bool electromagnetActive1 = true;
bool electromagnetActive2 = true;
unsigned long unlockTime1 = 0;
unsigned long unlockTime2 = 0;
const unsigned long unlockDuration = 5000;  // Temps avant que l'électroaimant se referme (5 secondes)

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  
  pinMode(electromagnetPin1, OUTPUT);
  pinMode(electromagnetPin2, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
    Serial.println(F("Échec de l'initialisation de l'écran OLED"));
    while (true);
  }
  display.display();
  delay(2000);  

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  Serial.println("Prêt à recevoir les données du lecteur de code-barres.");
  updateLED();
}

void loop() {
  if (Serial1.available() > 0) {
    String barcodeData = Serial1.readString();
    barcodeData.trim();
    
    Serial.print("Code-barres scanné: ");
    Serial.println(barcodeData);

    if (barcodeData == targetBarcode1) {
      unlockElectromagnet1();
      updateDisplay("Acces autorise.");
    } else if (barcodeData == targetBarcode2) {
      unlockElectromagnet2();
      updateDisplay("Acces autorise.");
    } else if (barcodeData == degradedModeBarcode) {
      toggleDegradedMode();
      updateDisplay("Mode degrade active.");
    } else {
      updateDisplay("Code incorrect.\nAcces refuse.");
    }
  }

  // Réactivation automatique des électroaimants après un certain temps
  if (!electromagnetActive1 && millis() - unlockTime1 >= unlockDuration) {
    electromagnetActive1 = true;
    Serial.println("Électroaimant 1 refermé.");
  }
  if (!electromagnetActive2 && millis() - unlockTime2 >= unlockDuration) {
    electromagnetActive2 = true;
    Serial.println("Électroaimant 2 refermé.");
  }

  if (!degradedModeActive) {
    updateDisplay("ENSTO");
  }

  // Gestion du mode dégradé
  if (degradedModeActive) {
    electromagnetActive1 = false;
    electromagnetActive2 = false;
    analogWrite(electromagnetPin1, 0);
    analogWrite(electromagnetPin2, 0);
  } else {
    analogWrite(electromagnetPin1, electromagnetActive1 ? 255 : 0);
    analogWrite(electromagnetPin2, electromagnetActive2 ? 255 : 0);
  }

  if (degradedModeActive && (millis() - lastDegradedTime >= degradedModeTimeout)) {
    degradedModeActive = false;
    electromagnetActive1 = true;
    electromagnetActive2 = true;
    Serial.println("Mode dégradé expiré. Les électroaimants sont réactivés.");
  }

  updateLED();
}

void unlockElectromagnet1() {
  electromagnetActive1 = false;
  unlockTime1 = millis();
  analogWrite(electromagnetPin1, 0);
}

void unlockElectromagnet2() {
  electromagnetActive2 = false;
  unlockTime2 = millis();
  analogWrite(electromagnetPin2, 0);
}

void toggleDegradedMode() {
  if (degradedModeActive) {
    degradedModeActive = false;
    electromagnetActive1 = true;
    electromagnetActive2 = true;
    Serial.println("Mode degrade desactive.");
    updateDisplay("Mode degrade desactive.");
  } else {
    degradedModeActive = true;
    electromagnetActive1 = false;
    electromagnetActive2 = false;
    lastDegradedTime = millis();
    Serial.println("Mode degrade active.");
    updateDisplay("Mode degrade active.");
  }
  updateLED();
}

void updateLED() {
  static unsigned long lastBlinkTime = 0;
  static bool ledState1 = false;
  static bool ledState2 = false;
  
  if (degradedModeActive) {
    unsigned long timeLeft = degradedModeTimeout - (millis() - lastDegradedTime);
    unsigned long blinkInterval = (timeLeft <= 5000) ? 200 : 1000;

    if (millis() - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = millis();
      ledState1 = !ledState1;
      ledState2 = !ledState2;

      digitalWrite(ledPin1, ledState1 ? HIGH : LOW);
      digitalWrite(ledPin2, ledState2 ? HIGH : LOW);
    }
  } else {
    digitalWrite(ledPin1, electromagnetActive1 ? LOW : HIGH);
    digitalWrite(ledPin2, electromagnetActive2 ? LOW : HIGH);
  }
}

void updateDisplay(String text) {
  display.clearDisplay();
  delay(2000);  // Petit délai pour éviter les bugs d'affichage

  display.setCursor(0, 0);
  display.print(text);
  display.display();
}
