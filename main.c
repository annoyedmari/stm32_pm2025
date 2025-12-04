#include <stdint.h>
#include "stm32f10x.h"
#define LED_PIN (1 << 13)
#define BUTTON_A_PIN (1 << 0)
#define BUTTON_C_PIN (1 << 1)

void GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN; // тактирование A, C
    GPIOC->CRH = (GPIOC->CRH & ~(GPIO_CRH_CNF13 | GPIO_CRH_MODE13)) | (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0); // 50MHz
    GPIOA->CRL = (GPIOA->CRL & ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0 | GPIO_CRL_CNF1 | GPIO_CRL_MODE1)) | 
                 (GPIO_CRL_CNF0_1 | GPIO_CRL_CNF1_1); // PA0, PA1 pull-up/pull-down
    GPIOA->ODR |= (BUTTON_A_PIN | BUTTON_C_PIN);
}

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        GPIOC->ODR ^= LED_PIN;
        TIM2->SR &= ~TIM_SR_UIF;
    }
}

void process_buttons(void) {
    static uint8_t last_button_a = 1;
    static uint8_t last_button_c = 1;
    uint8_t current_button_a, current_button_c;
    
    current_button_a = (GPIOA->IDR & BUTTON_A_PIN) ? 1 : 0; // 1 | 0 - нажата | нет
    current_button_c = (GPIOA->IDR & BUTTON_C_PIN) ? 1 : 0;
    
    if (!current_button_a && last_button_a) { // нажата кнопка А - увеличение частоты мигания
        for(volatile uint32_t i = 0; i < 5000; i++); // задержка
        
        if (!(GPIOA->IDR & BUTTON_A_PIN)) { // проверка нажатия кнопки
            if (TIM2->PSC > 1) {
                TIM2->PSC = TIM2->PSC >> 1;
            }
            while (!(GPIOA->IDR & BUTTON_A_PIN)) { // ожидание отпускания кнопки
                __NOP();
            }
        }
    }

    if (!current_button_c && last_button_c) { // нажата кнопка С - уменьшение частоты мигания
        for(volatile uint32_t i = 0; i < 5000; i++); 
        if (!(GPIOA->IDR & BUTTON_C_PIN)) { // проверка нажатия кнопки
            TIM2->PSC = TIM2->PSC << 1;
            while (!(GPIOA->IDR & BUTTON_C_PIN)) {
                __NOP();
            }
        }
    }

    last_button_a = current_button_a; // сохранение текущего состояния
    last_button_c = current_button_c;
}

int main(void) {
    GPIO_Init();
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // тактирование TIM2
    RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST; // сброс TIM2

    TIM2->PSC = 1023;     // Предделитель
    TIM2->ARR = 4095;     // Автоперезагрузка
    
    TIM2->DIER |= TIM_DIER_UIE; //  прерывание по обновлению
    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN; // запуск таймера

    while (1) {
        process_buttons();
        for(volatile uint32_t i = 0; i < 1000; i++);
    }
}