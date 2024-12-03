#ifndef BUTTON_H
#define BUTTON_H



//nut bam
#define BUTTON_PIN_1 GPIO_NUM_0 // Nút bấm
//#define LED_PIN GPIO_NUM_2    // LED
#define BUTTON_PIN_2 GPIO_NUM_4 // Nút bấm
#define BUTTON_PIN_3 GPIO_NUM_5 // Nút bấm



//cau hinh num xoay
#define ROTARY_CLK_GPIO    GPIO_NUM_18     //18 // GPIO cho tín hiệu CLK từ rotary encoder
#define ROTARY_DT_GPIO     GPIO_NUM_19   //19 // GPIO cho tín hiệu DT từ rotary encoder    
//#define POT_PIN GPIO_NUM_34 
extern volatile int speed_control_enabled; // Cờ cho phép điều khiển tốc độ

/////hàm khởi tạo các chân linh kiện cho esp32
/////////////////hàm khoi tao điều khiển motor


// Biến lưu trạng thái tốc độ động cơ
extern volatile int motor_speed; // Tốc độ (-255 đến 255)

// Cờ trạng thái quay chiều
extern volatile int last_clk_state;

void configure_button();
int button1_logic();
int button2_logic();
int button3_logic();
static void IRAM_ATTR rotary_isr_handler(void* arg);
void rotary_encoder_init();

#endif  