#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
// Định nghĩa các chân điều khiển động cơ và nút bấm
const int motorEnable = 9;   // Chân PWM điều khiển tốc độ động cơ
const int motorIn1 = 8;      // Chân điều khiển chiều động cơ
const int motorIn2 = 10;      // Chân điều khiển chiều động cơ
const int button1 = 7;       // Nút bật/tắt động cơ
const int button2 = 6;
const int button3 = 5;      // Nút đổi chiều động cơ
const int potentiometer = A0; // Chân đọc giá trị từ chiết áp

// Biến trạng thái
bool motorOn = false;         // Trạng thái bật/tắt động cơ
bool motorDirection = true;   // Trạng thái chiều động cơ
int motorSpeed = 0;           // Tốc độ động cơ

// Biến thời gian
unsigned long startTime = 0;  // Lưu thời điểm bắt đầu chạy động cơ
unsigned long runTime = 0;    // Thời gian chạy của động cơ

//khai báo biến của cảm biến speed
const byte PulsesPerRevolution = 2;
const unsigned long ZeroTimeout = 100000;
const byte numReadings = 2;

volatile unsigned long LastTimeWeMeasured;
volatile unsigned long PeriodBetweenPulses = ZeroTimeout + 1000;
volatile unsigned long PeriodAverage = ZeroTimeout + 1000;
unsigned long FrequencyRaw;
unsigned long FrequencyReal;
unsigned long RPM;
unsigned int PulseCounter = 1;
unsigned long PeriodSum;

unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;
unsigned long CurrentMicros = micros();
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;
unsigned long readings[numReadings];
unsigned long readIndex;  
unsigned long total; 
unsigned long average;










void setup() {
  // Thiết lập chế độ cho các chân
  pinMode(motorEnable, OUTPUT);
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(potentiometer,INPUT);
  lcd.init();
  lcd.backlight();
  // Khởi tạo LCD
  lcd.begin(16, 2); // LCD 16x2
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
  //khai bao chan cho cam bien la chan d2
  attachInterrupt(digitalPinToInterrupt(2), Pulse_Event, RISING);
  // Bắt đầu giao tiếp Serial
  Serial.begin(115200);
}



//hàm  xử lí tín hiệu cảm biến
void Pulse_Event() {
  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros();
  if (PulseCounter >= AmountOfReadings)  {
    PeriodAverage = PeriodSum / AmountOfReadings;
    PulseCounter = 1;
    PeriodSum = PeriodBetweenPulses;

    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;
  } else {
    PulseCounter++;
    PeriodSum = PeriodSum + PeriodBetweenPulses;
  }
}



// Hàm dừng động cơ
void motorStop() {
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
  analogWrite(motorEnable, 0);
}

// Hàm tạm dừng động cơ với độ trễ
void motorStopWithDelay(int delayMs) {
  motorStop();
  delay(delayMs);
}









void loop() {
  // dọc các chan digital 
  
  bool button1State = digitalRead(button1);
  
  bool button2State = digitalRead(button2);

  bool button3State = digitalRead(button3);
  runTime = (millis() - startTime) / 1000; // Chuyển đổi thành giây
   //init(dlc
   lcd.setCursor(0,1);
   lcd.print("Runtime: ");
   lcd.print(runTime);
   
   // xu li cam bien
   LastTimeCycleMeasure = LastTimeWeMeasured;
  CurrentMicros = micros();
  if (CurrentMicros < LastTimeCycleMeasure) {
    LastTimeCycleMeasure = CurrentMicros;
  }
  FrequencyRaw = 10000000000 / PeriodAverage;
  if (PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra) {
    FrequencyRaw = 0;  // Set frequency as 0.
    ZeroDebouncingExtra = 2000;
  } else {
    ZeroDebouncingExtra = 0;
  }
  FrequencyReal = FrequencyRaw / 10000;

  RPM = FrequencyRaw / PulsesPerRevolution * 60;
  RPM = RPM / 10000;
  total = total - readings[readIndex];
  readings[readIndex] = RPM;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;
  /////////////////////////////////////
  



  
   //logic 3
   if (button3State == LOW) 
   {
    motorDirection = true;
   }
   else
   {
    motorDirection = false;
   }
   
   // logic1 va 2
   if (button1State == HIGH) {
      //Serial.println("NUT 1 OFF");
     
      //nut 2 tat
      if(button2State == HIGH)
      {
      //hienthilcd
      lcd.setCursor(0,0);
      lcd.print("Speed:");
      lcd.print(motorSpeed);
      //nut 3
      if (motorDirection) 
      {
      //Serial.println("NUT 3 OFF");
      digitalWrite(motorIn1, HIGH);
      digitalWrite(motorIn2, LOW);
      } 
      else 
      {
       //Serial.println("NUT 3 ON"); 
      digitalWrite(motorIn1, LOW);
      digitalWrite(motorIn2, HIGH);
      }
    analogWrite(motorEnable, motorSpeed);
    }
     //nut  2 bat
     else
     {
          //Serial.println("NUT 1 OFF 2 ON");
          motorStop();
          //lcd.clear();
          //lcd.print("NUT: 1 2 ON") ;
          StaticJsonDocument<200> jsonDoc;
          jsonDoc["speed"] = motorSpeed;
          jsonDoc["runtime"] = runTime;
          jsonDoc["Frequency"] = FrequencyReal;
          jsonDoc["RPM"] = RPM;
          char jsonBuffer[512];
          serializeJson(jsonDoc, jsonBuffer);
          Serial.println(jsonBuffer);
          /////// //////////////////////
          lcd.clear();
          lcd.print("SEND DATA");
          delay(2000);
          lcd.clear();
     }  
 }
    else 
    {
      //Serial.println("NUT 1 ON");
      runTime = 0;
      motorStop();
      if (button2State == LOW) 
      {
        //Serial.println("NUT 2 ON");
        motorSpeed = analogRead(potentiometer);
        motorSpeed = map(motorSpeed, 0, 1023, 0, 255);
        lcd.clear();
        delay(1000);
        lcd.print(" SPEED SAVE");
        lcd.setCursor(0,1);
        lcd.print(motorSpeed);
        delay(1000);
        lcd.clear();
      }
      else
      {
        //Serial.println("NUT 2 OFF");
        lcd.clear();
        lcd.print(" SPEED SETTING");
        delay(2000); 
        lcd.clear();
      }
    }
  }
 
  
