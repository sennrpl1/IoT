#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
 lcd.init();
 lcd.backlight();
 lcd.setCursor(3, 0);
 lcd.print("halo dunia!");
 lcd.setCursor(1, 1);
 lcd.print("selamat datang");
}

void loop() {

}
