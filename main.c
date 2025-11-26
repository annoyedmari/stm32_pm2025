#include <stm32f10x.h>
#define DC_PIN   (1 << 1)   // PA1
#define RST_PIN  (1 << 2)   // PA2  
#define CS_PIN   (1 << 3)   // PA3


void delay(uint32_t ticks) { 
    while(ticks--) __NOP(); 
}

void SPI1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_IOPAEN;
    GPIOA->CRL = (GPIOA->CRL & ~(0xFFF << 20)) | (0xB8B << 20);
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE;
}

void SPI1_Write(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;
    while (SPI1->SR & SPI_SR_BSY);
}

void SSD1306_Write(uint8_t is_data, uint8_t byte) {
    GPIOA->BRR = CS_PIN;                    
    if(is_data)
        GPIOA->BSRR = DC_PIN;
    else 
        GPIOA->BRR = DC_PIN;
    
    SPI1_Write(byte);
    GPIOA->BSRR = CS_PIN;
}

void SSD1306_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(GPIO_CRL_CNF1 | GPIO_CRL_MODE1 |
                    GPIO_CRL_CNF2 | GPIO_CRL_MODE2 |
                    GPIO_CRL_CNF3 | GPIO_CRL_MODE3);
    GPIOA->CRL |= (GPIO_CRL_MODE1_0 | GPIO_CRL_MODE2_0 | GPIO_CRL_MODE3_0);
    // Начальные состояния
    GPIOA->BSRR = CS_PIN | RST_PIN;
    // Сброс
    GPIOA->BRR = RST_PIN; delay(10000);
    GPIOA->BSRR = RST_PIN; delay(10000);
    
    // Дисплей SSD1306
    const uint8_t init_cmds[] = {
        0xAE,       // Display OFF
        0x20, 0x00, // Horizontal addressing mode
        0x21, 0x00, 0x7F, // Set column address (0-127)
        0x22, 0x00, 0x07, // Set page address (0-7)
        0x40,       // Set start line
        0xA1,       // Segment remap
        0xC8,       // COM output scan direction
        0xDA, 0x12, // COM pins hardware configuration
        0x81, 0x7F, // Set contrast
        0xA4,       // Entire display ON
        0xA6,       // Normal display
        0xD5, 0x80, // Set oscillator frequency
        0x8D, 0x14, // Enable charge pump
        0xAF        // Display ON
    };
    
    for(int i = 0; i < sizeof(init_cmds); i++) {
        SSD1306_Write(0, init_cmds[i]);
    }
    delay(10000);
}

void SSD1306_Clear(void) {
    for(uint8_t page = 0; page < 8; page++) {
        SSD1306_Write(0, 0xB0 + page);
        SSD1306_Write(0, 0x00);
        SSD1306_Write(0, 0x10);
        
        for(uint8_t col = 0; col < 128; col++) {
            SSD1306_Write(1, 0x00);
        }
    }
}


void Chessboard(void) {
    for(uint8_t page = 0; page < 8; page++) {
        SSD1306_Write(0, 0xB0 + page);
        SSD1306_Write(0, 0x00);
        SSD1306_Write(0, 0x10);
        
        for(uint8_t col = 0; col < 128; col++) {
            uint8_t cell_x = col / 16; // 16 x 16
            uint8_t cell_y = page / 2;
            uint8_t pattern;
            if((cell_x + cell_y) % 2 == 0) {
                pattern = 0xFF;
            } else {
                pattern = 0x00;
            }
            SSD1306_Write(1, pattern);
        }
    }
}

int main(void) {
    SPI1_Init();
    SSD1306_Init();
    SSD1306_Clear();

    delay(100000);

    Chessboard();

    while(1) {
        delay(1000000);
    }
}