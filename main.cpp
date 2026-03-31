#include <Arduino.h>

// 電機驅動引腳 - 嚴格匹配你的硬體接線
// 根據用戶要求，保留 GPIO 0 作為 PWMA
#define PWMA 0 // 保留 GPIO 0
#define AIN1 12
#define AIN2 1
#define PWMB 13
#define BIN1 18
#define BIN2 19

// ESP32 PWM配置
#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1
#define PWM_FREQ 5000
#define PWM_RES 8
#define MOTOR_SPEED 200 // 正常行駛速度
#define BACK_SPEED 90 // 后退行驶速度
#define HIGH_SPEED  200  // 外圈轮速
#define LOW_SPEED  80   // 内圈轮速（大于0是平滑转弯，等于0是单轴转弯，小于0是原地转弯）



// 四路循跡傳感器引腳
#define TRACKs4 3
#define TRACKs3 10
#define TRACKs2 6
#define TRACKs1 7

#define RX1_PIN 8 // 對應原理圖上的 IO8
#define TX1_PIN 9

// --- 動作控制函數 ---

void backCar()
{
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  ledcWrite(PWM_CHANNEL_A, BACK_SPEED); // 停止時將 PWM 佔空比設為 0
  ledcWrite(PWM_CHANNEL_B, BACK_SPEED);
}

void forward()
{
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  ledcWrite(PWM_CHANNEL_A, MOTOR_SPEED);
  ledcWrite(PWM_CHANNEL_B, MOTOR_SPEED);
}

void left()
{
 // 右转：左轮快，右轮慢
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); // 左轮前进
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); // 右轮前进
  
  ledcWrite(PWM_CHANNEL_A, HIGH_SPEED); // 左轮（外侧）快
  ledcWrite(PWM_CHANNEL_B, LOW_SPEED);  // 右轮（内侧）慢
}

void right()
{
  // 左转：右轮快，左轮慢
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); // 左轮前进
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); // 右轮前进
  
  ledcWrite(PWM_CHANNEL_A, LOW_SPEED);  // 左轮（内侧）慢
  ledcWrite(PWM_CHANNEL_B, HIGH_SPEED); // 右轮（外侧）快
}

// --- 主程序 ---

void setup()
{
  Serial.begin(115200);

  Serial.println("Serial1 (IO8, IO9) 初始化完成！");

  // 電機驅動引腳設置
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // 循跡傳感器引腳設置
  pinMode(TRACKs1, INPUT);
  pinMode(TRACKs2, INPUT);
  pinMode(TRACKs3, INPUT);
  pinMode(TRACKs4, INPUT);

  // PWM初始化
  ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RES);

  ledcAttachPin(PWMA, PWM_CHANNEL_A);
  ledcAttachPin(PWMB, PWM_CHANNEL_B);

  backCar(); // 開機默認停止
}

void loop()
{
  int s4 = digitalRead(TRACKs4); // 最左
  int s3 = digitalRead(TRACKs3); // 中左
  int s2 = digitalRead(TRACKs2); // 中右
  int s1 = digitalRead(TRACKs1); // 最右


  

  // 循跡邏輯優化：優先處理轉彎，然後直行，最後處理特殊情況
  // 假設 0 為黑線，1 為白線

  // 1. 小车直行
  if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0)
  {
    forward(); // 十字路口：直衝
    Serial.printf("forward\n");
  }
  else if (s1 == 1 && s2 == 0 && s3 == 0 && s4 == 1)
  {
    forward();
  }
  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 1)
  {
    backCar();
    Serial.printf("backCar"); // 脫軌：停止
  }

  // --- 2. 次優先處理：複雜組合 (三路或兩路黑線) ---
  // 右邊三個黑線 (1000) -> 右轉
  else if (s1 == 1 && s2 == 0 && s3 == 0 && s4 == 0)
  {
    right();
    Serial.printf("right");
  }
  // 左邊三個黑線 (0001) -> 左轉
  else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 1)
  {
    left();
    Serial.printf("left");
  }
  // 右邊兩個黑線 (1100) -> 右轉
  else if (s1 == 1 && s2 == 1 && s3 == 0 && s4 == 0)
  {
    right();
    Serial.printf("right");
  }
  // 左邊兩個黑線 (0011) -> 左轉
  else if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 1)
  {
    left();
    Serial.printf("left");
  }

  // --- 3. 基礎處理：簡單偏離 (單路黑線) ---
  // 只有最右黑線 (1110) -> 右轉
  else if (s1 == 1 && s2 == 1 && s3 == 1 && s4 == 0)
  {
    right();
    Serial.printf("right");
  }

  else if (s1 == 0 && s2 == 1 && s3 == 1 && s4 == 1)
  {
    left();
    Serial.printf("left");
  }

  else if (s3 == 0 && s2 == 1 && s1 == 1 && s4 == 1)
  {
    right();
    Serial.printf("right");
  }

  else if (s2 == 0 && s1 == 1 && s3 == 1 && s4 == 1)
  {
    left();
    Serial.printf("left");
  }
  else
  {
    backCar();
  }

  delay(5);
}