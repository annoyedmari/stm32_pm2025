#include <stdint.h>
#include "stm32f10x.h"
#define LED_PIN (1 << 13)
#define BUTTON_A_PIN (1 << 0)
#define BUTTON_C_PIN (1 << 1)

volatile uint32_t blink_delay = 500; // Начальная задержка 500 мс (1 Гц)
volatile uint32_t min_delay = 15;    // ~15 мс (64 Гц)
volatile uint32_t max_delay = 64000; // 64000 мс (1/64 Гц)


void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;
    GPIOC->CRH = (GPIOC->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | GPIO_CRH_MODE13_0;
    GPIOA->CRL = (GPIOA->CRL & ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0 | GPIO_CRL_CNF1 | GPIO_CRL_MODE1)) 
                 | (GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1);
    
    GPIOA->ODR &= ~(BUTTON_A_PIN | BUTTON_C_PIN);
}


void delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms * 1000; i++) {
        __NOP();
    }
}


void process_buttons(void) {
    static uint8_t last_button_a = 1;
    static uint8_t last_button_c = 1;
    uint32_t debounce_delay = 5000;
    uint8_t current_button_a = (GPIOA->IDR & BUTTON_A_PIN) ? 1 : 0;
    uint8_t current_button_c = (GPIOA->IDR & BUTTON_C_PIN) ? 1 : 0;
    
    if (!current_button_a && last_button_a) { // button a
        delay_ms(10);
        if (!(GPIOA->IDR & BUTTON_A_PIN)) {
            if (blink_delay > min_delay) {
                blink_delay /= 2;
            }
            while (!(GPIOA->IDR & BUTTON_A_PIN)) {
                __NOP();
            }
            delay_ms(10);
        }
    }
    
    if (!current_button_c && last_button_c) { // button c
        delay_ms(10);
        if (!(GPIOA->IDR & BUTTON_C_PIN)) {
            if (blink_delay < max_delay) {
                blink_delay *= 2;
            }
            while (!(GPIOA->IDR & BUTTON_C_PIN)) {
                __NOP();
            }
            delay_ms(10);
        }
    }
    
    last_button_a = current_button_a;
    last_button_c = current_button_c;
}

int main(void) {
    GPIO_Init();
    while (1) {
        process_buttons();
        GPIOC->ODR ^= LED_PIN;
        delay_ms(blink_delay);
    }
}