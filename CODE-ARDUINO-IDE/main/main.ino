#include <ArduinoJson.h>

void setup() {
  // Cấu hình UART với baud rate 9600
  Serial.begin(115200);
}

void loop() {
  // Tạo đối tượng JSON
  
  // Gửi chuỗi JSON qua UART
  Serial.println("1234");

  // Gửi mỗi 2 giây
  delay(2000);
}
