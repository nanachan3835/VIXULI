void setup() {
  Serial.begin(115200); // Khởi tạo UART với baud rate 115200
}

void loop() {
  int dataToSend = 1234; // Biến int cần gửi
  Serial.println(dataToSend); // Gửi dữ liệu dạng chuỗi qua UART
  delay(1000); // Gửi mỗi giây một lần
}
