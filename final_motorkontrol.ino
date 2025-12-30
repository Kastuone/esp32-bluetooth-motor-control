#include <Adafruit_NeoPixel.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Forward declarations (fonksiyon önceden tanımlamaları)
void bleBaglandi();
void bleKoptu();
void komutIsle(char komut, String kaynak);

// BLE Server ve karakteristikler
BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// BLE UUID'leri (Nordic UART Service)
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// RGB LED pin tanımlaması (ESP32-S3-Mini-1 için genellikle GPIO48)
#define RGB_LED_PIN 48
#define NUM_PIXELS 1  // Tek LED var

// Motor pin tanımlamaları
#define PIN1 1   // Sol motor ileri
#define PIN2 2   // Sol motor geri  
#define PIN3 3   // Sağ motor ileri
#define PIN4 4   // Sağ motor geri

// Hız değeri (%35 = 90/255)
// Hız değeri (%80 = 204/255)
int hiz80 = 90;  // 3.3V * 0.8 = 2.64V için PWM değeri;

// NeoPixel nesnesi oluştur
Adafruit_NeoPixel strip(NUM_PIXELS, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// BLE Server Callback sınıfı
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("🔵✅ BLE cihazı bağlandı!");
      bleBaglandi();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("🔵❌ BLE cihazı bağlantısı kesildi!");
      bleKoptu();
    }
};

// BLE Characteristic Callback sınıfı  
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String receivedValue = pCharacteristic->getValue().c_str();

      if (receivedValue.length() > 0) {
        char komut = receivedValue[0];
        komutIsle(komut, "BLE");
      }
    }
};

void setup() {
  Serial.begin(115200);
  
  Serial.println("ESP32-S3 PWM WASD Kontrolü + RGB LED + BLE BLUETOOTH");
  Serial.println("=====================================================");
  Serial.println("🔵 BLE Cihaz Adı: ESP32_Robot_Car");
  Serial.println("📱 Bluetooth LE uygulamasından bağlanın!");
  Serial.println("🔧 Nordic UART Service kullanıyor");
  Serial.println("");
  Serial.println("KONTROLLER:");
  Serial.println("W/w = Düz İleri (YEŞIL LED)");
  Serial.println("S/s = Düz Geri (KIRMIZI LED)");
  Serial.println("A/a = Sol Dönüş (MAVİ LED)");
  Serial.println("D/d = Sağ Dönüş (SARI LED)");
  Serial.println("B/b = FREN/DURDUR (LED SÖNER)");
  Serial.println("I/i = BİLGİ (BLE durumu)");
  Serial.println("");
  Serial.println("Sol Motor: Pin1(ileri), Pin2(geri)");
  Serial.println("Sağ Motor: Pin3(ileri), Pin4(geri)");
  Serial.println("");
  
  // RGB LED başlat
  strip.begin();
  strip.clear();
  strip.show();
  strip.setBrightness(50);
  
  // Motor pinlerini çıkış olarak ayarla
  pinMode(PIN1, OUTPUT);
  pinMode(PIN2, OUTPUT);
  pinMode(PIN3, OUTPUT);
  pinMode(PIN4, OUTPUT);
  
  // Başlangıçta tüm pinleri kapat
  tumPinleriKapat();
  
  // RGB LED testi
  Serial.println("RGB LED testi...");
  testRGB();
  
  // BLE başlat
  bleBaslat();
  
  Serial.println("🔵 BLE aktif! Bağlantı bekleniyor...");
  Serial.println("📱 BLE Scanner uygulaması ile 'ESP32_Robot_Car' arayın");
  Serial.println("");
}

void loop() {
  // Seri porttan komut kontrolü
  if (Serial.available()) {
    char komut = Serial.read();
    komutIsle(komut, "Seri Port");
  }
  
  // BLE bağlantı durumu değişikliklerini kontrol et
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // BLE stack'in hazırlanması için bekle
    pServer->startAdvertising(); // Yeniden reklamı başlat
    Serial.println("🔵 BLE reklamı yeniden başlatıldı");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

void bleBaslat() {
  // BLE cihazını başlat
  BLEDevice::init("ESP32_Robot_Car");
  
  // BLE sunucuyu oluştur
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // BLE servisini oluştur
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // TX karakteristiğini oluştur
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  // RX karakteristiğini oluştur
  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                       BLECharacteristic::PROPERTY_WRITE
                     );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Servisi başlat
  pService->start();

  // Reklamı başlat
  pServer->getAdvertising()->start();
  Serial.println("🔵 BLE servisi başlatıldı, reklam yayınlanıyor...");
}

void komutIsle(char komut, String kaynak) {
  if (komut == 'w' || komut == 'W') {
    ileri();
    mesajGonder("✅ DÜZ İLERİ komutu alındı (" + kaynak + ")");
  }
  else if (komut == 's' || komut == 'S') {
    geri();
    mesajGonder("✅ DÜZ GERİ komutu alındı (" + kaynak + ")");
  }
  else if (komut == 'a' || komut == 'A') {
    solDonus();
    mesajGonder("✅ SOL DÖNÜŞ komutu alındı (" + kaynak + ")");
  }
  else if (komut == 'd' || komut == 'D') {
    sagDonus();
    mesajGonder("✅ SAĞ DÖNÜŞ komutu alındı (" + kaynak + ")");
  }
  else if (komut == 'b' || komut == 'B') {
    frenDur();
    mesajGonder("🛑 FREN komutu alındı (" + kaynak + ")");
  }
  else if (komut == 'i' || komut == 'I') {
    bilgiGoster();
  }
  else if (komut == '3') {
    hiz80= 90;
  }
  else if (komut == '5') {
    hiz80= 127;
  }
  else if (komut == '6') {
    hiz80= 155;
  }
}

void ileri() {
  analogWrite(PIN1, hiz80);  
  analogWrite(PIN2, 0);      
  analogWrite(PIN3, hiz80);  
  analogWrite(PIN4, 0);      
  
  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Yeşil
  strip.show();
  
  Serial.println("🟢 DÜZ İLERİ - W tuşuna basıldı (YEŞIL LED)");
  Serial.println("Sol Motor: Pin1=2.64V, Pin2=0V");
  Serial.println("Sağ Motor: Pin3=2.64V, Pin4=0V");
  Serial.println("");
}

void geri() {
  analogWrite(PIN1, 0);      
  analogWrite(PIN2, hiz80);  
  analogWrite(PIN3, 0);      
  analogWrite(PIN4, hiz80);  
  
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Kırmızı
  strip.show();
  
  Serial.println("🔴 DÜZ GERİ - S tuşuna basıldı (KIRMIZI LED)");
  Serial.println("Sol Motor: Pin1=0V, Pin2=2.64V");
  Serial.println("Sağ Motor: Pin3=0V, Pin4=2.64V");
  Serial.println("");
}

void solDonus() {
  analogWrite(PIN1, 0);      
  analogWrite(PIN2, hiz80);  
  analogWrite(PIN3, hiz80);  
  analogWrite(PIN4, 0);      
  
  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Mavi
  strip.show();
  
  Serial.println("🔵 SOL DÖNÜŞ - A tuşuna basıldı (MAVİ LED)");
  Serial.println("Sol Motor Geri: Pin1=0V, Pin2=2.64V");
  Serial.println("Sağ Motor İleri: Pin3=2.64V, Pin4=0V");
  Serial.println("");
}

void sagDonus() {
  analogWrite(PIN1, hiz80);  
  analogWrite(PIN2, 0);      
  analogWrite(PIN3, 0);      
  analogWrite(PIN4, hiz80);  
  
  strip.setPixelColor(0, strip.Color(255, 255, 0)); // Sarı
  strip.show();
  
  Serial.println("🟡 SAĞ DÖNÜŞ - D tuşuna basıldı (SARI LED)");
  Serial.println("Sol Motor İleri: Pin1=2.64V, Pin2=0V");
  Serial.println("Sağ Motor Geri: Pin3=0V, Pin4=2.64V");
  Serial.println("");
}

void frenDur() {
  analogWrite(PIN1, 0);
  analogWrite(PIN2, 0);
  analogWrite(PIN3, 0);
  analogWrite(PIN4, 0);
  
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
  
  strip.clear();
  strip.show();
  
  Serial.println("Tüm pinler kapatıldı - 0V (LED kapalı)");
  Serial.println("");
}

void testRGB() {
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

void bleBaglandi() {
  bleMesajGonder("🎉 ESP32 Robot Arabaya hoş geldiniz!");
  bleMesajGonder("Kontroller: W(ileri) S(geri) A(sol) D(sag) B(dur) I(bilgi)");
  
  // Bağlantı gösterimi için LED animasyonu
  for (int i = 0; i < 3; i++) {
    strip.setPixelColor(0, strip.Color(0, 0, 255)); // Mavi
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
}

void bleKoptu() {
  // Güvenlik için arabayı durdur
  frenDur();
}

void mesajGonder(String mesaj) {
  Serial.println(mesaj);
  if (deviceConnected) {
    bleMesajGonder(mesaj);
  }
}

void bleMesajGonder(String mesaj) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(mesaj.c_str());
    pTxCharacteristic->notify();
    delay(10); // BLE stack için küçük gecikme
  }
}

void bilgiGoster() {
  String durum = deviceConnected ? "🔵✅ BAĞLI" : "🔵❌ BAĞLI DEĞİL";
  
  Serial.println("=================== BİLGİ ===================");
  Serial.println("Cihaz Adı: ESP32_Robot_Car");
  Serial.println("BLE Durumu: " + durum);
  Serial.println("Motor Hızı: %80 (2.64V)");
  Serial.println("RGB LED Pin: 48");
  Serial.println("Motor Pinleri: 1,2,3,4");
  Serial.println("BLE Protokol: Nordic UART Service");
  Serial.println("============================================");
  
  if (deviceConnected) {
    bleMesajGonder("=============== ROBOT BİLGİ ===============");
    bleMesajGonder("🔵 BLE: BAĞLI");
    bleMesajGonder("⚡ Motor Hızı: %80");
    bleMesajGonder("🎮 Kontroller: W/S/A/D/B/I");
    bleMesajGonder("🔴🟢🔵🟡 RGB LED aktif");
    bleMesajGonder("==========================================");
  }
}