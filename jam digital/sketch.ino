#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// Inisialisasi LCD 1602 I2C (Alamat umum 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Inisialisasi objek RTC
RTC_DS3231 rtc;

// Array nama hari dalam bahasa Indonesia (disingkat 3 huruf agar muat di LCD 16 kolom)
char daysOfTheWeek[7][4] = {"Min", "Sen", "Sel", "Rab", "Kam", "Jum", "Sab"};

void setup() {
  Serial.begin(115200);

  // Inisialisasi LCD
  lcd.init();                      
  lcd.backlight();
  
  // Inisialisasi I2C & RTC
  Wire.begin();
  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC Not Found!");
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Jika RTC kehilangan daya, atur ke waktu kompilasi
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Tampilan awal / SplashScreen
  lcd.setCursor(0, 0);
  lcd.print(" ESP32 RTC CLOCK ");
  lcd.setCursor(0, 1);
  lcd.print("   Initializing ");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Ambil waktu saat ini dari modul RTC
  DateTime now = rtc.now();

  // --- TAMPILKAN HARI & TANGGAL DI BARIS PERTAMA (Format: Sen, 03/06/2026) ---
  lcd.setCursor(0, 0);
  
  // Menampilkan nama hari berdasarkan index (0-6)
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(", ");
  
  // Menampilkan tanggal
  if (now.day() < 10) lcd.print("0");
  lcd.print(now.day());
  lcd.print("/");
  
  if (now.month() < 10) lcd.print("0");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());

  // --- TAMPILKAN JAM DI BARIS KEDUA (Format: HH:MM:SS) ---
  lcd.setCursor(4, 1);
  
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());

  delay(1000); // Perbarui setiap 1 detik
}
