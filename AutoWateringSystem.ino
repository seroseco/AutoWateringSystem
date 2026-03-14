const int SENSOR_PIN = A0;     // 토양 수분 센서 아날로그 핀
const int RELAY_PIN  = 7;      // 릴레이 제어 핀

// ====== 여기 값을 나중에 조정하세요 ======
int dryThreshold = 600;        // 이 값보다 크면 '건조'로 판단
                               // 센서마다 다르므로 반드시 보정 필요

const unsigned long pumpOnTime = 3000;        // 펌프 작동 시간 (3초)
const unsigned long soakTime = 10000;         // 물 준 뒤 흙에 스며들 시간 (10초)
const unsigned long readInterval = 2000;      // 평소 측정 주기 (2초)
const unsigned long minWateringGap = 60000;   // 최소 급수 간격 (60초)
// =====================================

const int RELAY_ON  = LOW;
const int RELAY_OFF = HIGH;

unsigned long lastReadTime = 0;
unsigned long lastWaterTime = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  Serial.begin(9600);
  Serial.println("자동 급수 시스템 시작");
}

void loop() {
  unsigned long now = millis();

  if (now - lastReadTime >= readInterval) {
    lastReadTime = now;

    // 💡 수정된 부분 1: 그냥 analogRead() 대신 새로 만든 평균값 함수를 호출합니다.
    int moistureValue = getAverageMoisture();

    Serial.print("Soil Value (Average): ");
    Serial.println(moistureValue);

    if (moistureValue > dryThreshold) {
      Serial.println("상태: 건조함");

      if (now - lastWaterTime >= minWateringGap) {
        waterPlant();
        lastWaterTime = millis();
      } else {
        Serial.println("최근에 급수했으므로 잠시 대기");
      }
    } else {
      Serial.println("상태: 충분히 촉촉함");
    }

    Serial.println("------------------------");
  }
}

// 💡 새로 추가된 부분: 센서 값을 10번 읽어서 평균을 내는 함수입니다.
int getAverageMoisture() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(10); // 아주 짧은 간격(0.01초)을 두고 읽어서 정확도를 높임
  }
  return sum / 10;
}

void waterPlant() {
  Serial.println("펌프 ON - 물 공급 시작");
  digitalWrite(RELAY_PIN, RELAY_ON);
  delay(pumpOnTime);

  digitalWrite(RELAY_PIN, RELAY_OFF);
  Serial.println("펌프 OFF - 물 공급 종료");

  Serial.println("물이 흙에 스며들도록 대기 중...");
  delay(soakTime);

  // 💡 수정된 부분 2: 재측정할 때도 평균값 함수를 사용합니다.
  int checkValue = getAverageMoisture();
  Serial.print("재측정 Soil Value (Average): ");
  Serial.println(checkValue);

  if (checkValue > dryThreshold) {
    Serial.println("아직 건조함. 다음 주기에서 다시 판단");
  } else {
    Serial.println("급수 후 충분히 촉촉해짐");
  }

  Serial.println("========================");
}