#include <Adafruit_NeoPixel.h>

// RGB LED pin tanımlaması (ESP32-S3-Mini-1 için genellikle GPIO48)
#define RGB_LED_PIN 48
#define NUM_PIXELS 1  // Tek LED var

// Motor pin tanımlamaları
#define PIN1 1   // Sol motor ileri
#define PIN2 2   // Sol motor geri  
#define PIN3 3   // Sağ motor ileri
#define PIN4 4   // Sağ motor geri

// Hız değeri (%80 = 204/255)
const int hiz80 = 204;  // 3.3V * 0.8 = 2.64V için PWM değeri

// NeoPixel nesnesi oluştur
Adafruit_NeoPixel strip(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 PWM WASD Kontrolü + RGB LED");
  Serial.println("====================================");
  Serial.println("W = Düz İleri (YEŞIL LED)");
  Serial.println("S = Düz Geri (KIRMIZI LED)");
  Serial.println("A = Sol Dönüş (MAVİ LED)");
  Serial.println("D = Sağ Dönüş (SARI LED)");
  Serial.println("B = FREN/DURDUR (LED SÖNER)");
  Serial.println("Sol Motor: Pin1(ileri), Pin2(geri)");
  Serial.println("Sağ Motor: Pin3(ileri), Pin4(geri)");
  Serial.println("");
  
  // RGB LED başlat
  strip.begin();
  strip.clear();
  strip.show();
  strip.setBrightness(50); // Parlaklık ayarı (0-255)
  
  // Motor pinlerini çıkış olarak ayarla
  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);
  pinMode(PIN4, OUTPUT);
  
  // Başlangıçta tüm pinleri kapat
  tumPinleriKapat();
  
  // Başlangıç animasyonu
  Serial.println("RGB LED testi...");
  testRGB();
}

void loop() {
  if (Serial.available()) {
    char komut = Serial.read();
    
    if (komut == 'w' || komut == 'W') {
      ileri();
    }
    else if (komut == 's' || komut == 'S') {
      geri();
    }
    else if (komut == 'a' || komut == 'A') {
      solDonus();
    }
    else if (komut == 'd' || komut == 'D') {
      sagDonus();
    }
    else if (komut == 'b' || komut == 'B') {
      frenDur();
    }
  }
}

void ileri() {
  analogWrite(PIN1, hiz80);  // Sol motor ileri = 2.64V
  analogWrite(PIN2, 0);      // Sol motor geri = 0V
  analogWrite(PIN3, hiz80);  // Sağ motor ileri = 2.64V
  analogWrite(PIN4, 0);      // Sağ motor geri = 0V
  
  // Yeşil LED yak
  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Yeşil
  strip.show();
  
  Serial.println("DÜZ İLERİ - W tuşuna basıldı (YEŞIL LED)");
  Serial.println("Sol Motor: Pin1=2.64V, Pin2=0V");
  Serial.println("Sağ Motor: Pin3=2.64V, Pin4=0V");
  Serial.println("");
}

void geri() {
  analogWrite(PIN1, 0);      // Sol motor ileri = 0V
  analogWrite(PIN2, hiz80);  // Sol motor geri = 2.64V
  analogWrite(PIN3, 0);      // Sağ motor ileri = 0V
  analogWrite(PIN4, hiz80);  // Sağ motor geri = 2.64V
  
  // Kırmızı LED yak
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Kırmızı
  strip.show();
  
  Serial.println("DÜZ GERİ - S tuşuna basıldı (KIRMIZI LED)");
  Serial.println("Sol Motor: Pin1=0V, Pin2=2.64V");
  Serial.println("Sağ Motor: Pin3=0V, Pin4=2.64V");
  Serial.println("");
}

void solDonus() {
  // Sol motor geri, Sağ motor ileri
  analogWrite(PIN1, 0);      // Sol motor ileri = 0V
  analogWrite(PIN2, hiz80);  // Sol motor geri = 2.64V
  analogWrite(PIN3, hiz80);  // Sağ motor ileri = 2.64V
  analogWrite(PIN4, 0);      // Sağ motor geri = 0V
  
  // Mavi LED yak
  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Mavi
  strip.show();
  
  Serial.println("SOL DÖNÜŞ - A tuşuna basıldı (MAVİ LED)");
  Serial.println("Sol Motor Geri: Pin1=0V, Pin2=2.64V");
  Serial.println("Sağ Motor İleri: Pin3=2.64V, Pin4=0V");
  Serial.println("");
}

void sagDonus() {
  // Sol motor ileri, Sağ motor geri
  analogWrite(PIN1, hiz80);  // Sol motor ileri = 2.64V
  analogWrite(PIN2, 0);      // Sol motor geri = 0V
  analogWrite(PIN3, 0);      // Sağ motor ileri = 0V
  analogWrite(PIN4, hiz80);  // Sağ motor geri = 2.64V
  
  // Sarı LED yak
  strip.setPixelColor(0, strip.Color(255, 255, 0)); // Sarı
  strip.show();
  
  Serial.println("SAĞ DÖNÜŞ - D tuşuna basıldı (SARI LED)");
  Serial.println("Sol Motor İleri: Pin1=2.64V, Pin2=0V");
  Serial.println("Sağ Motor Geri: Pin3=0V, Pin4=2.64V");
  Serial.println("");
}

void frenDur() {
  // Tüm motorları durdur
  analogWrite(PIN1, 0);
  analogWrite(PIN2, 0);
  analogWrite(PIN3, 0);
  analogWrite(PIN4, 0);
  
  // LED'i söndür
  strip.clear();
  strip.show();
  
  Serial.println("🛑 FREN - B tuşu (ARABA DURDU, LED SÖNDÜ)");
  Serial.println("Tüm motorlar: 0V");
  Serial.println("");
}

void tumPinleriKapat() {
  analogWrite(PIN1, 0);
  analogWrite(PIN2, 0);
  analogWrite(PIN3, 0);
  analogWrite(PIN4, 0);
  
  // LED'i kapat
  strip.clear();
  strip.show();
  
  Serial.println("Tüm pinler kapatıldı - 0V (LED kapalı)");
  Serial.println("");
}

void testRGB() {
  // RGB test animasyonu
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Kırmızı
  strip.show();
  delay(300);
  
  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Yeşil
  strip.show();
  delay(300);
  
  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Mavi
  strip.show();
  delay(300);
  
  strip.setPixelColor(0, strip.Color(255, 255, 0)); // Sarı
  strip.show();
  delay(300);
  
  strip.clear();
  strip.show();
  Serial.println("RGB test tamamlandı!");
  Serial.println("");
}

// Gökkuşağı efekti (bonus özellik)
void rainbowCycle(int wait) {
  for(int j=0; j<256*5; j++) {
    for(int i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
