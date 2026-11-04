/*
 * main.c - Space Adventure OLED Animation
 * Board: STM32 Nucleo-F446RE
 * OLED: SSD1306 128x64, I2C address 0x3C
 * I2C1: PB8 = SCL, PB9 = SDA
 *
 * Animation: approximately 10 seconds
 * Uses custom STM32F446RE GPIO and I2C drivers.
 */

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <stdint.h>

#define OLED_ADDR       0x3C
#define OLED_WIDTH      128U
#define OLED_HEIGHT     64U
#define OLED_BUFFER     1024U
#define TOTAL_FRAMES    50U
#define FRAME_DELAY     200U

static I2C_Handle_t hi2c;
static uint8_t framebuffer[OLED_BUFFER];

/* Approximate delay; depends on the system clock. */
static void delay_ms(volatile uint32_t ms)
{
    while (ms--)
    {
        for (volatile uint32_t i = 0; i < 8000U; i++)
        {
            __asm volatile ("nop");
        }
    }
}

static void I2C1_GPIO_Init(void)
{
    GPIO_Handle_t gpio = {0};

    GPIO_PeriClockControl(GPIOB, ENABLE);

    gpio.pGPIOx = GPIOB;
    gpio.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    gpio.GPIO_PinConfig.GPIO_PinAltFunMode = 4;
    gpio.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    gpio.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    gpio.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

    /* PB8: I2C1 SCL */
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
    GPIO_Init(&gpio);

    /* PB9: I2C1 SDA */
    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
    GPIO_Init(&gpio);
}

static void I2C1_Init(void)
{
    hi2c.pI2Cx = I2C1;

    hi2c.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    hi2c.I2C_Config.I2C_DeviceAddress = 0x00;
    hi2c.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
    hi2c.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;

    I2C_PeriClockControl(I2C1, ENABLE);
    I2C_Init(&hi2c);
    I2C_PeripheralControl(I2C1, ENABLE);
}

static void OLED_Write(uint8_t control,
                       const uint8_t *data,
                       uint16_t length)
{
    uint8_t packet[17];

    while (length > 0U)
    {
        uint16_t count = (length > 16U) ? 16U : length;

        packet[0] = control;

        for (uint16_t i = 0; i < count; i++)
        {
            packet[i + 1U] = *data++;
        }

        I2C_MasterSendData(&hi2c, packet, count + 1U,
                           OLED_ADDR, I2C_DISABLE_SR);

        length -= count;
    }
}

static void OLED_Command(uint8_t command)
{
    OLED_Write(0x00, &command, 1U);
}

static void OLED_Init(void)
{
    static const uint8_t commands[] =
    {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F,
        0xD3, 0x00, 0x40, 0x8D, 0x14,
        0x20, 0x00, 0xA1, 0xC8, 0xDA,
        0x12, 0x81, 0x7F, 0xD9, 0xF1,
        0xDB, 0x40, 0xA4, 0xA6, 0xAF
    };

    delay_ms(100U);

    for (uint32_t i = 0;
         i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        OLED_Command(commands[i]);
    }
}

static void OLED_Clear(void)
{
    for (uint16_t i = 0; i < OLED_BUFFER; i++)
    {
        framebuffer[i] = 0x00;
    }
}

static void OLED_Pixel(int16_t x, int16_t y, uint8_t color)
{
    if (x < 0 || x >= (int16_t)OLED_WIDTH ||
        y < 0 || y >= (int16_t)OLED_HEIGHT)
    {
        return;
    }

    uint16_t index = (uint16_t)x +
                     ((uint16_t)y / 8U) * OLED_WIDTH;
    uint8_t mask = (uint8_t)(1U << ((uint16_t)y % 8U));

    if (color)
        framebuffer[index] |= mask;
    else
        framebuffer[index] &= (uint8_t)~mask;
}

static void OLED_Line(int16_t x0, int16_t y0,
                      int16_t x1, int16_t y1)
{
    int16_t dx = (x1 > x0) ? x1 - x0 : x0 - x1;
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = (y1 > y0) ? y0 - y1 : y1 - y0;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t error = dx + dy;

    while (1)
    {
        OLED_Pixel(x0, y0, 1U);

        if (x0 == x1 && y0 == y1)
            break;

        int16_t e2 = 2 * error;

        if (e2 >= dy)
        {
            error += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}

static void OLED_Rect(int16_t x, int16_t y,
                      int16_t w, int16_t h)
{
    OLED_Line(x, y, x + w, y);
    OLED_Line(x, y, x, y + h);
    OLED_Line(x + w, y, x + w, y + h);
    OLED_Line(x, y + h, x + w, y + h);
}

static void OLED_Flush(void)
{
    OLED_Command(0x21);
    OLED_Command(0x00);
    OLED_Command(0x7F);

    OLED_Command(0x22);
    OLED_Command(0x00);
    OLED_Command(0x07);

    OLED_Write(0x40, framebuffer, OLED_BUFFER);
}

/* Small twinkling stars */
static void Draw_Stars(uint8_t frame)
{
    static const uint8_t stars[][2] =
    {
        {8, 8}, {25, 20}, {42, 5}, {60, 15},
        {82, 7}, {110, 17}, {120, 5}, {18, 40},
        {48, 31}, {99, 35}, {115, 48}, {70, 48}
    };

    for (uint8_t i = 0; i < 12U; i++)
    {
        OLED_Pixel(stars[i][0], stars[i][1], 1U);

        if (((frame + i) % 3U) == 0U)
        {
            OLED_Pixel(stars[i][0] - 1, stars[i][1], 1U);
            OLED_Pixel(stars[i][0] + 1, stars[i][1], 1U);
        }
    }
}

/* Simple rocket outline */
static void Draw_Rocket(int16_t x, int16_t y)
{
    /* Nose */
    OLED_Line(x + 5, y, x + 10, y + 7);
    OLED_Line(x + 10, y + 7, x, y + 7);
    OLED_Line(x, y + 7, x + 5, y);

    /* Body */
    OLED_Rect(x, y + 7, 10, 17);

    /* Window */
    OLED_Pixel(x + 4, y + 12, 1U);
    OLED_Pixel(x + 5, y + 12, 1U);
    OLED_Pixel(x + 4, y + 13, 1U);
    OLED_Pixel(x + 5, y + 13, 1U);

    /* Fins */
    OLED_Line(x, y + 18, x - 4, y + 24);
    OLED_Line(x - 4, y + 24, x, y + 24);
    OLED_Line(x + 10, y + 18, x + 14, y + 24);
    OLED_Line(x + 14, y + 24, x + 10, y + 24);

    /* Flame flickers */
    OLED_Line(x + 3, y + 24, x + 2, y + 27);
    OLED_Line(x + 7, y + 24, x + 8, y + 27);
}

static void Draw_Planet(int16_t x, int16_t y)
{
    /* Diamond-like planet */
    OLED_Line(x, y - 7, x + 8, y);
    OLED_Line(x + 8, y, x, y + 7);
    OLED_Line(x, y + 7, x - 8, y);
    OLED_Line(x - 8, y, x, y - 7);

    /* Ring */
    OLED_Line(x - 12, y + 3, x + 12, y - 3);
}

int main(void)
{
    I2C1_GPIO_Init();
    I2C1_Init();
    OLED_Init();

    for (uint8_t frame = 0; frame < TOTAL_FRAMES; frame++)
    {
        OLED_Clear();
        Draw_Stars(frame);

        if (frame < 17U)
        {
            /* Scene 1: rocket travels into space */
            int16_t rocket_x = 5 + frame * 3;
            int16_t rocket_y = 35 - frame;

            Draw_Rocket(rocket_x, rocket_y);
        }
        else if (frame < 34U)
        {
            /* Scene 2: rocket approaches a planet */
            Draw_Rocket(55, 12);
            Draw_Planet(92, 34);

            OLED_Line(20, 58, 45, 58);
            OLED_Line(30, 54, 48, 54);
        }
        else
        {
            /* Scene 3: rocket explores the planet */
            Draw_Planet(88, 34);
            Draw_Rocket(45, 25);

            OLED_Line(10, 58, 35, 58);
            OLED_Line(80, 55, 115, 55);
        }

        OLED_Flush();
        delay_ms(FRAME_DELAY);
    }

    /* Clear display after the animation */
    OLED_Clear();
    OLED_Flush();

    while (1)
    {
        /* Animation has finished */
    }
}
