#include <stdint.h>

// RCC Registers
#define RCC_APB2ENR    *((volatile uint32_t *)0x40021018)

// GPIOA Registers
#define GPIOA_CRL      *((volatile uint32_t *)0x40010800)
#define GPIOA_CRH      *((volatile uint32_t *)0x40010804)

// USART1 Registers
#define USART1_SR      *((volatile uint32_t *)0x40013800)
#define USART1_DR      *((volatile uint32_t *)0x40013804)
#define USART1_BRR     *((volatile uint32_t *)0x40013808)
#define USART1_CR1     *((volatile uint32_t *)0x4001380C)

// Timer 1 Registers
#define TIM1_CR1       *((volatile uint32_t *)0x40012C00)
#define TIM1_PSC       *((volatile uint32_t *)0x40012C28)
#define TIM1_ARR       *((volatile uint32_t *)0x40012C2C)
#define TIM1_CCR1      *((volatile uint32_t *)0x40012C34)
#define TIM1_CCMR1     *((volatile uint32_t *)0x40012C18)
#define TIM1_CCER      *((volatile uint32_t *)0x40012C20)
#define TIM1_BDTR      *((volatile uint32_t *)0x40012C44)

// ─── UART Functions ───────────────────────────────────────

void uart_init(void) {
    // Enable clocks for GPIOA and USART1
    RCC_APB2ENR |= (1 << 2);   // GPIOA
    RCC_APB2ENR |= (1 << 14);  // USART1

    // Configure PA9 as TX (Alternate function push-pull)
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xB << 4);  // AF push-pull, 50MHz

    // Set baud rate 9600 @ 72MHz
    // BRR = 72000000 / 9600 = 7500 = 0x1D4C
    USART1_BRR = 0x1D4C;

    // Enable USART, TX
    USART1_CR1 |= (1 << 13);  // USART enable
    USART1_CR1 |= (1 << 3);   // TX enable
}

void uart_send_char(char c) {
    // Wait until TX buffer is empty
    while(!(USART1_SR & (1 << 7)));
    USART1_DR = c;
}

void uart_send_string(const char *str) {
    while(*str) {
        uart_send_char(*str++);
    }
}

void uart_send_number(uint32_t num) {
    // Convert number to string and send
    char buf[10];
    int i = 0;

    if(num == 0) {
        uart_send_char('0');
        return;
    }

    while(num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    // Send in reverse
    while(i > 0) {
        uart_send_char(buf[--i]);
    }
}

// ─── PWM Functions ────────────────────────────────────────

void pwm_init(void) {
    // Enable TIM1 clock
    RCC_APB2ENR |= (1 << 11);

    // Configure PA8 as alternate function push-pull
    GPIOA_CRH &= ~(0xF << 0);
    GPIOA_CRH |=  (0xB << 0);

    // Timer config
    TIM1_PSC   = 72 - 1;     // 1MHz timer clock
    TIM1_ARR   = 1000 - 1;   // 1kHz PWM

    // PWM mode 1 on channel 1
    TIM1_CCMR1 |= (6 << 4);
    TIM1_CCMR1 |= (1 << 3);

    // Enable channel output
    TIM1_CCER |= (1 << 0);

    // Enable main output
    TIM1_BDTR  |= (1 << 15);

    // Start timer
    TIM1_CR1   |= (1 << 0);
}

void pwm_set_duty(uint32_t duty) {
    // duty: 0-1000 (0% to 100%)
    TIM1_CCR1 = duty;
}

// ─── PI Controller ────────────────────────────────────────

// Simulated target speed (RPM)
#define TARGET_SPEED   200

// PI gains
#define KP   2
#define KI   1

int32_t integral    = 0;
int32_t prev_error  = 0;

uint32_t pi_controller(int32_t measured_speed) {
    int32_t error = TARGET_SPEED - measured_speed;

    // Accumulate integral
    integral += error;

    // Anti-windup: clamp integral
    if(integral >  500) integral =  500;
    if(integral < -500) integral = -500;

    // PI output
    int32_t output = (KP * error) + (KI * integral);

    // Clamp output to valid PWM range
    if(output >  1000) output = 1000;
    if(output <  0)    output = 0;

    return (uint32_t)output;
}

// ─── Main ─────────────────────────────────────────────────

int main(void) {

    uart_init();
    pwm_init();

    uart_send_string("PWM PI Controller Started\r\n");

    // Simulate a motor that responds slowly to PWM
    int32_t simulated_speed = 0;
    uint32_t duty           = 0;

    while(1) {
        // Simulate motor speed responding to PWM
        // Speed increases toward (duty / 2) slowly
        simulated_speed += (((int32_t)duty / 2) - simulated_speed) / 10;

        // Run PI controller
        duty = pi_controller(simulated_speed);

        // Apply to PWM
        pwm_set_duty(duty);

        // Log over UART
        uart_send_string("Speed: ");
        uart_send_number((uint32_t)simulated_speed);
        uart_send_string(" | Target: ");
        uart_send_number(TARGET_SPEED);
        uart_send_string(" | Duty: ");
        uart_send_number(duty);
        uart_send_string("\r\n");

        // Small delay
        volatile int i;
        for(i = 0; i < 200000; i++);
    }

    return 0;
}
