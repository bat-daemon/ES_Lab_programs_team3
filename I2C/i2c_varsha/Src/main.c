/*
 * main.c - 10-Second Rainy-Day Story
 * Board   : STM32F446RE Nucleo
 * Display : SSD1306 128x64 OLED
 * I2C1    : PB8 = SCL, PB9 = SDA
 * Address : 0x3C
 *
 * Uses custom STM32F446xx GPIO and I2C drivers.
 * All graphics and animation functions are in this file.
 */

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <stdint.h>

#define OLED_ADDR    0x3C
#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define FRAME_SIZE   1024U

#define TOTAL_FRAMES 50U
#define FRAME_DELAY  200U

static I2C_Handle_t hi2c;
static uint8_t framebuffer[FRAME_SIZE];

/*-------------------- Delay --------------------*/
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

/*-------------------- OLED Write --------------------*/
static void OLED_Write(uint8_t control,
                       const uint8_t *data,
                       uint32_t length)
{
    uint8_t packet[17];

    while (length > 0U)
    {
        uint32_t count = (length > 16U) ? 16U : length;

        packet[0] = control;

        for (uint32_t i = 0; i < count; i++)
        {
            packet[i + 1U] = data[i];
        }

        I2C_MasterSendData(&hi2c, packet, count + 1U,
                           OLED_ADDR, I2C_DISABLE_SR);

        data += count;
        length -= count;
    }
}

static void OLED_Command(uint8_t command)
{
    OLED_Write(0x00, &command, 1U);
}

/*-------------------- I2C GPIO Initialization --------------------*/
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

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_8;
    GPIO_Init(&gpio);

    gpio.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_9;
    GPIO_Init(&gpio);
}

/*-------------------- I2C1 Initialization --------------------*/
static void I2C1_Init(void)
{
    hi2c.pI2Cx = I2C1;

    hi2c.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    hi2c.I2C_Config.I2C_DeviceAddress = 0x00;
    hi2c.I2C_Config.I2C_ACKControl = I2C_ACK_ENABLE;
    hi2c.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;

    I2C_Init(&hi2c);
    I2C_PeripheralControl(I2C1, ENABLE);
}

/*-------------------- OLED Initialization --------------------*/
static void OLED_Init(void)
{
    static const uint8_t commands[] =
    {
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0x7F,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0xAF
    };

    I2C1_GPIO_Init();
    I2C1_Init();
    delay_ms(100);

    for (uint32_t i = 0; i < sizeof(commands); i++)
    {
        OLED_Command(commands[i]);
    }
}

/*-------------------- Clear Framebuffer --------------------*/
static void OLED_Clear(void)
{
    for (uint32_t i = 0; i < FRAME_SIZE; i++)
    {
        framebuffer[i] = 0x00;
    }
}

/*-------------------- Draw Pixel --------------------*/
static void OLED_Pixel(int x, int y, uint8_t color)
{
    if (x < 0 || x >= OLED_WIDTH ||
        y < 0 || y >= OLED_HEIGHT)
    {
        return;
    }

    uint32_t index = (uint32_t)x +
                     ((uint32_t)y / 8U) * OLED_WIDTH;

    uint8_t mask = (uint8_t)(1U << (y & 7));

    if (color)
        framebuffer[index] |= mask;
    else
        framebuffer[index] &= (uint8_t)~mask;
}

/*-------------------- Draw Line --------------------*/
static void OLED_Line(int x0, int y0, int x1, int y1,
                      uint8_t color)
{
    int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
    int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1)
    {
        OLED_Pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

/*-------------------- Draw Circle --------------------*/
static void OLED_Circle(int cx, int cy, int radius,
                        uint8_t color)
{
    int x = radius;
    int y = 0;
    int error = 1 - radius;

    while (x >= y)
    {
        OLED_Pixel(cx + x, cy + y, color);
        OLED_Pixel(cx + y, cy + x, color);
        OLED_Pixel(cx - y, cy + x, color);
        OLED_Pixel(cx - x, cy + y, color);
        OLED_Pixel(cx - x, cy - y, color);
        OLED_Pixel(cx - y, cy - x, color);
        OLED_Pixel(cx + y, cy - x, color);
        OLED_Pixel(cx + x, cy - y, color);

        y++;

        if (error < 0)
            error += 2 * y + 1;
        else
        {
            x--;
            error += 2 * (y - x) + 1;
        }
    }
}

/*-------------------- Flush Framebuffer --------------------*/
static void OLED_Flush(void)
{
    OLED_Command(0x21);
    OLED_Command(0x00);
    OLED_Command(0x7F);

    OLED_Command(0x22);
    OLED_Command(0x00);
    OLED_Command(0x07);

    OLED_Write(0x40, framebuffer, FRAME_SIZE);
}

/*-------------------- Draw Cloud --------------------*/
static void Draw_Cloud(int x, int y)
{
    OLED_Circle(x, y, 5, 1);
    OLED_Circle(x + 7, y - 3, 7, 1);
    OLED_Circle(x + 15, y, 5, 1);

    OLED_Line(x - 4, y + 3, x + 19, y + 3, 1);
}

/*-------------------- Draw Falling Rain --------------------*/
static void Draw_Rain(int frame)
{
    static const uint8_t rain_x[] =
    {
        5, 16, 27, 38, 49, 60,
        71, 82, 93, 104, 115, 124
    };

    for (int i = 0; i < 12; i++)
    {
        int x = rain_x[i];
        int y = 15 + ((frame * 3 + i * 7) % 37);

        OLED_Line(x, y, x - 2, y + 4, 1);
    }
}

/*-------------------- Draw Umbrella --------------------*/
static void Draw_Umbrella(int x, int y)
{
    OLED_Line(x - 12, y, x - 6, y - 6, 1);
    OLED_Line(x - 6, y - 6, x, y - 9, 1);
    OLED_Line(x, y - 9, x + 6, y - 6, 1);
    OLED_Line(x + 6, y - 6, x + 12, y, 1);
    OLED_Line(x - 12, y, x + 12, y, 1);

    OLED_Line(x - 12, y, x - 7, y - 2, 1);
    OLED_Line(x - 7, y - 2, x - 3, y, 1);
    OLED_Line(x - 3, y, x + 2, y - 2, 1);
    OLED_Line(x + 2, y - 2, x + 7, y, 1);
    OLED_Line(x + 7, y, x + 12, y, 1);

    /* Umbrella handle */
    OLED_Line(x, y, x, y + 10, 1);
    OLED_Line(x, y + 10, x + 3, y + 12, 1);
}

/*-------------------- Draw Walking Person --------------------*/
static void Draw_Person(int x, int y, int frame)
{
    OLED_Circle(x, y, 3, 1);

    OLED_Line(x, y + 3, x, y + 12, 1);
    OLED_Line(x, y + 5, x - 4, y + 8, 1);
    OLED_Line(x, y + 5, x + 4, y + 7, 1);

    if ((frame & 1) == 0)
    {
        OLED_Line(x, y + 12, x - 4, y + 18, 1);
        OLED_Line(x, y + 12, x + 4, y + 17, 1);
    }
    else
    {
        OLED_Line(x, y + 12, x - 5, y + 17, 1);
        OLED_Line(x, y + 12, x + 3, y + 18, 1);
    }
}

/*-------------------- Draw Rainy-Day Frame --------------------*/
static void Rainy_Story_Frame(int frame)
{
    OLED_Clear();

    /* Moving cloud */
    int cloud_x = 12 + ((frame / 3) % 18);
    Draw_Cloud(cloud_x, 10);

    /* Rain keeps falling */
    Draw_Rain(frame);

    /* Three story phases */
    int scene = (frame / 17) % 3;
    int person_x;

    if (scene == 0)
    {
        /* Person walks into the rain */
        person_x = 14 + ((frame % 17) * 3);
    }
    else if (scene == 1)
    {
        /* Person stops under the umbrella */
        person_x = 76;
    }
    else
    {
        /* Person continues walking */
        person_x = 76 + ((frame % 17) * 2);
    }

    if (person_x > 116)
        person_x = 116;

    Draw_Umbrella(person_x, 35);
    Draw_Person(person_x, 38, frame);

    /* Ground and puddles */
    OLED_Line(0, 59, 127, 59, 1);

    OLED_Line(8, 62, 18, 62, 1);
    OLED_Line(40, 62, 53, 62, 1);
    OLED_Line(91, 62, 104, 62, 1);

    /* Sun appears near the end */
    if (frame >= 42)
    {
        OLED_Circle(116, 12, 5, 1);
        OLED_Line(116, 4, 116, 2, 1);
        OLED_Line(108, 12, 106, 12, 1);
        OLED_Line(124, 12, 126, 12, 1);
    }

    OLED_Flush();
}

/*-------------------- Main --------------------*/
int main(void)
{
    OLED_Init();

    /* 50 frames at a nominal 200 ms per frame */
    for (int frame = 0; frame < TOTAL_FRAMES; frame++)
    {
        Rainy_Story_Frame(frame);
        delay_ms(FRAME_DELAY);
    }

    /* Clear display after the story */
    OLED_Clear();
    OLED_Flush();

    /* Remain blank until reset */
    while (1)
    {
    }
}
