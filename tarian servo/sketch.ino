#include <ESP32Servo.h>

const int JUMLAH_SERVO = 14;
Servo servos[JUMLAH_SERVO];
int sudutSekarang[JUMLAH_SERVO];

// Pin PWM, indeks 0..13 = servo1..servo14 (sesuai diagram.json)
const int pinServo[JUMLAH_SERVO] = {
  13, 12, 14, 27, 26,   // servo1..servo5
  25, 33, 32, 15, 2,    // servo6..servo10
  4, 16, 17, 5          // servo11..servo14
};

// Urutan fisik lingkaran searah jarum jam (indeks servo, 0-based)
const int urutanFisik[JUMLAH_SERVO] = {
  12, 13, 4, 0, 1, 2, 3, 11, 10, 9, 8, 7, 6, 5
};

const int SUDUT_TENGAH = 90;
const int JUMLAH_POLA = 8;

// ==========================================
// FUNGSI BANTU
// ==========================================

bool validasiUrutan() {
  bool terpakai[JUMLAH_SERVO] = { false };
  for (int i = 0; i < JUMLAH_SERVO; i++) {
    int idx = urutanFisik[i];
    if (idx < 0 || idx >= JUMLAH_SERVO || terpakai[idx]) {
      Serial.printf("urutanFisik[%d] = %d tidak valid atau duplikat!\n", i, idx);
      return false;
    }
    terpakai[idx] = true;
  }
  return true;
}

void tulisServo(int idx, int sudut) {
  sudut = constrain(sudut, 0, 180);
  sudutSekarang[idx] = sudut;
  servos[idx].write(sudut);
}

// Kembali ke posisi tengah secara halus (bukan loncat)
void kembaliKeTengah(int langkah = 15, int jeda = 25) {
  int awal[JUMLAH_SERVO];
  for (int idx = 0; idx < JUMLAH_SERVO; idx++) awal[idx] = sudutSekarang[idx];

  for (int s = 1; s <= langkah; s++) {
    for (int idx = 0; idx < JUMLAH_SERVO; idx++) {
      tulisServo(idx, awal[idx] + (SUDUT_TENGAH - awal[idx]) * s / langkah);
    }
    delay(jeda);
    yield();
  }
}

// ==========================================
// POLA 1 & 3: Gelombang Sutra (arah = +1 searah, -1 berlawanan)
// ==========================================
void polaGelombang(int jumlahPutaran, int arah, int amplitudo) {
  for (int putaran = 0; putaran < jumlahPutaran; putaran++) {
    for (int langkah = 0; langkah < 360; langkah += 6) {
      for (int i = 0; i < JUMLAH_SERVO; i++) {
        float fase = arah * langkah + i * (360.0 / JUMLAH_SERVO);
        tulisServo(urutanFisik[i], SUDUT_TENGAH + (int)(sin(radians(fase)) * amplitudo));
      }
      delay(30);
      yield();
    }
  }
}

// ==========================================
// POLA 2: Teratai Mekar
// ==========================================
void polaTeratai() {
  const int SUDUT_MEKAR = 130;
  const int separuh = JUMLAH_SERVO / 2;

  for (int i = 0; i < separuh; i++) {
    tulisServo(urutanFisik[i], SUDUT_MEKAR);
    tulisServo(urutanFisik[JUMLAH_SERVO - 1 - i], SUDUT_MEKAR);
    delay(80);
    yield();
  }
  delay(400);

  for (int i = separuh - 1; i >= 0; i--) {
    tulisServo(urutanFisik[i], SUDUT_TENGAH);
    tulisServo(urutanFisik[JUMLAH_SERVO - 1 - i], SUDUT_TENGAH);
    delay(80);
    yield();
  }
  delay(300);
}

// ==========================================
// POLA 4: Riak Simetris (menyebar dari puncak lingkaran)
// Puncak berada di antara urutanFisik[0] dan urutanFisik[1]
// ==========================================
void polaRiakSimetris(int jumlahPutaran) {
  for (int putaran = 0; putaran < jumlahPutaran; putaran++) {
    for (int langkah = 0; langkah < 360; langkah += 6) {
      for (int i = 0; i < JUMLAH_SERVO; i++) {
        float d = fabs(i - 0.5);                      // jarak ke puncak
        if (d > JUMLAH_SERVO / 2.0) d = JUMLAH_SERVO - d;
        float fase = langkah - d * 45.0;
        tulisServo(urutanFisik[i], SUDUT_TENGAH + (int)(sin(radians(fase)) * 50));
      }
      delay(30);
      yield();
    }
  }
}

// ==========================================
// POLA 5: Komet Pengejar (satu kepala dengan ekor memudar)
// ==========================================
void polaKomet(int jumlahPutaran) {
  const float PANJANG_EKOR = 5.0;
  float total = jumlahPutaran * (float)JUMLAH_SERVO;

  for (float kepala = 0; kepala < total; kepala += 0.25) {
    for (int i = 0; i < JUMLAH_SERVO; i++) {
      float s = fmod(kepala - i, (float)JUMLAH_SERVO);
      if (s < 0) s += JUMLAH_SERVO;
      float tinggi = (s < PANJANG_EKOR) ? (1.0 - s / PANJANG_EKOR) : 0.0;
      tulisServo(urutanFisik[i], SUDUT_TENGAH + (int)(tinggi * 50));
    }
    delay(25);
    yield();
  }
}

// ==========================================
// POLA 6: Napas (naik turun pelan, ada sedikit selisih fase)
// ==========================================
void polaNapas(int jumlahNapas) {
  for (int n = 0; n < jumlahNapas; n++) {
    for (int langkah = 0; langkah < 360; langkah += 4) {
      for (int i = 0; i < JUMLAH_SERVO; i++) {
        float fase = langkah + i * 8.0;
        float amp = 0.5 - 0.5 * cos(radians(fase));   // 0..1
        tulisServo(urutanFisik[i], SUDUT_TENGAH + (int)(amp * 50));
      }
      delay(25);
      yield();
    }
  }
}

// ==========================================
// POLA 7: Zig-Zag (genap vs ganjil berlawanan)
// ==========================================
void polaZigZag(int jumlahSiklus) {
  for (int langkah = 0; langkah < 360 * jumlahSiklus; langkah += 8) {
    float s = sin(radians(langkah));
    for (int i = 0; i < JUMLAH_SERVO; i++) {
      int arah = (i % 2 == 0) ? 1 : -1;
      tulisServo(urutanFisik[i], SUDUT_TENGAH + (int)(arah * s * 45));
    }
    delay(20);
    yield();
  }
}

// ==========================================
// POLA 8: Percikan Acak (bergerak halus ke target acak)
// ==========================================
void polaPercikan(int jumlahFrame) {
  int target[JUMLAH_SERVO];
  for (int idx = 0; idx < JUMLAH_SERVO; idx++) target[idx] = SUDUT_TENGAH;

  for (int frame = 0; frame < jumlahFrame; frame++) {
    if (frame % 4 == 0) {
      int pilih = random(JUMLAH_SERVO);
      target[pilih] = random(40, 141);
    }
    for (int idx = 0; idx < JUMLAH_SERVO; idx++) {
      int selisih = target[idx] - sudutSekarang[idx];
      tulisServo(idx, sudutSekarang[idx] + constrain(selisih, -6, 6));
    }
    delay(25);
    yield();
  }
}

// ==========================================
// SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(500);

  if (!validasiUrutan()) {
    Serial.println("Sistem dihentikan. Perbaiki array urutanFisik.");
    while (true) delay(1000);
  }

  randomSeed(esp_random());

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  for (int i = 0; i < JUMLAH_SERVO; i++) {
    int idx = urutanFisik[i];
    servos[idx].setPeriodHertz(50);
    servos[idx].attach(pinServo[idx], 500, 2400);
    tulisServo(idx, SUDUT_TENGAH);
    delay(20);
  }
  delay(300);
  Serial.println("Sistem air mancur siap!");
}

void loop() {
  static int pola = 0;

  switch (pola) {
    case 0:
      Serial.println("Pola 1: Gelombang Sutra");
      polaGelombang(2, +1, 50);
      break;
    case 1:
      Serial.println("Pola 2: Teratai Mekar");
      polaTeratai();
      break;
    case 2:
      Serial.println("Pola 3: Gelombang Balik");
      polaGelombang(2, -1, 40);
      break;
    case 3:
      Serial.println("Pola 4: Riak Simetris");
      polaRiakSimetris(2);
      break;
    case 4:
      Serial.println("Pola 5: Komet Pengejar");
      polaKomet(2);
      break;
    case 5:
      Serial.println("Pola 6: Napas");
      polaNapas(2);
      break;
    case 6:
      Serial.println("Pola 7: Zig-Zag");
      polaZigZag(2);
      break;
    case 7:
      Serial.println("Pola 8: Percikan Acak");
      polaPercikan(150);
      break;
  }

  pola = (pola + 1) % JUMLAH_POLA;
  kembaliKeTengah();
  delay(300);
}
