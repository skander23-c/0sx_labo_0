#include <Wire.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 2);

// Définition des broches
const int LDR_PIN = A0;
const int LED_PIN = 8;
const int JOY_X_PIN = A1;
const int JOY_Y_PIN = A2;
const int JOY_BUTTON_PIN = 2;

// Constantes
const int LUMINOSITE_MIN = 50;
const int TEMPS_OBSCURITE = 5000;
const int DELAI_BOUTON = 30;
const int INTERVALLE_LCD = 100;
const int INTERVALLE_SERIAL = 100;
const int JOY_X_MIN = 500;
const int JOY_X_MAX = 550;
const int JOY_Y_NEUTRE_MIN = 490;
const int JOY_Y_NEUTRE_MAX = 520;
const int VITESSE_NEG_MIN = -25;
const int VITESSE_POS_MAX = 120;
const int NVLUMMAX=100;
const int NIVEAUPMWMAX=1023;

// Variables globales
bool phareAllume = false;
unsigned long debutObscurite = 0;
unsigned long dernierEnvoiSerial = 0;
unsigned long dernierMajLCD = 0;
unsigned long dernierMajBouton = 0;
bool pageLCD = 0;
bool boutonEtatPrecedent = HIGH;




void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(JOY_BUTTON_PIN, INPUT_PULLUP);
    lcd.begin();
    lcd.backlight();
    
    
    

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Benabed");
    delay(1500);

    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("79");
    delay(1500);

  
    lcd.clear();
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 8; col++) {
            lcd.setCursor(col, row);
            lcd.write(0);  
        }
        for (int col = 8; col < 16; col++) {
            lcd.setCursor(col, row);
            lcd.write(1);  
        }
    }
    delay(1500);
    lcd.clear();
}

void loop() {
     unsigned long tempsActuel = millis();

    int luminosite = analogRead(LDR_PIN);
    int pourcentageLuminosite = map(luminosite, 0, NIVEAUPMWMAX, 0, NVLUMMAX);
    int joyX = analogRead(JOY_X_PIN);
    int joyY = analogRead(JOY_Y_PIN);
    bool boutonPresse = digitalRead(JOY_BUTTON_PIN) == LOW;

    if (pourcentageLuminosite < LUMINOSITE_MIN) {
        if (debutObscurite == 0) debutObscurite = tempsActuel;
        if (tempsActuel - debutObscurite >= TEMPS_OBSCURITE) {
            phareAllume = true;
            digitalWrite(LED_PIN, HIGH);
            delay(5000);
        }
    } else {
        debutObscurite = 0;
        phareAllume = false;
        digitalWrite(LED_PIN, LOW);
    }

    if (tempsActuel - dernierMajBouton > DELAI_BOUTON) {
        if (boutonPresse && boutonEtatPrecedent) {
            pageLCD = !pageLCD;
        }
        boutonEtatPrecedent = boutonPresse;
        dernierMajBouton = tempsActuel;
    }

    if (tempsActuel - dernierMajLCD >= INTERVALLE_LCD) {
        lcd.clear();
        if (pageLCD == 0) {
            lcd.setCursor(0, 0);
            lcd.print("Lum: ");
            lcd.print(pourcentageLuminosite);
            lcd.print("%");
            lcd.setCursor(0, 1);
            lcd.print("Phares: ");
            lcd.print(phareAllume ? "ON" : "OFF");
        } else {
            int vitesse = 0;
            if (joyY > JOY_Y_NEUTRE_MAX) {
                vitesse = map(joyY, JOY_Y_NEUTRE_MAX, NIVEAUPMWMAX, 0, VITESSE_POS_MAX);
            } else if (joyY < JOY_Y_NEUTRE_MIN) {
                vitesse = map(joyY, 0, JOY_Y_NEUTRE_MIN, VITESSE_NEG_MIN, 0);
            }
            
            lcd.setCursor(0, 0);
            lcd.print((vitesse < 0) ? "Recule" : (vitesse > 0) ? "Avance" : "Stop");
            lcd.setCursor(8, 0);
            lcd.print(abs(vitesse));
            lcd.print("km/h");
            lcd.setCursor(0, 1);
            lcd.print("Dir: ");
            lcd.print((joyX < JOY_X_MIN) ? "G" : (joyX > JOY_X_MAX) ? "D" : "N");
        }
        dernierMajLCD = tempsActuel;
    }

    if (tempsActuel - dernierEnvoiSerial >= INTERVALLE_SERIAL) {
        Serial.print("etd:2344779,");
        Serial.print("x:");
        Serial.print(joyX);
        Serial.print(",y:");
        Serial.print(joyY);
        Serial.print(",sys:");
        Serial.println(phareAllume ? 1 : 0);
        dernierEnvoiSerial = tempsActuel;
    }
}
