/*
 * main.c - 10-Second Animated Night Story
 * Board   : STM32F446RE Nucleo
 * Display : SSD1306 128x64 I2C OLED
 * I2C1    : PB8 = SCL, PB9 = SDA
 * Address : 0x3C
 *
 * Uses custom STM32F446xx GPIO and I2C drivers.
 * All OLED functions are included in this file.
 */

#include "stm32f446xx.h"
#include "stm32f446xx_gpio_driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <stdint.h>

#define OLED_ADDR   0x3C
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define FRAME_SIZE  1024U

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

/*-------------------- OLED I2C Write --------------------*/
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

/*-------------------- GPIO Initialization --------------------*/
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

/*-------------------- I2C Initialization --------------------*/
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

/*-------------------- Send Framebuffer to OLED --------------------*/
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

/*-------------------- Draw Stars --------------------*/
static void Draw_Stars(int frame)
{
    static const uint8_t star_x[] =
    {
        5, 18, 31, 47, 62, 77,
        93, 109, 121, 26, 85, 115
    };

    static const uint8_t star_y[] =
    {
        6, 13, 4, 19, 9, 15,
        5, 20, 10, 27, 29, 34
    };

    for (int i = 0; i < 12; i++)
    {
        OLED_Pixel(star_x[i], star_y[i], 1);

        if (((frame + i) % 3) == 0)
        {
            OLED_Pixel(star_x[i] - 1, star_y[i], 1);
            OLED_Pixel(star_x[i] + 1, star_y[i], 1);
            OLED_Pixel(star_x[i], star_y[i] - 1, 1);
            OLED_Pixel(star_x[i], star_y[i] + 1, 1);
        }
    }
}

/*-------------------- Draw Crescent Moon --------------------*/
static void Draw_Moon(void)
{
    OLED_Circle(103, 14, 8, 1);
    OLED_Circle(108, 10, 7, 0);
}

/*-------------------- Draw Walking Traveller --------------------*/
static void Draw_Person(int x, int y, int frame)
{
    OLED_Circle(x, y, 3, 1);

    /* Body and arms */
    OLED_Line(x, y + 3, x, y + 12, 1);
    OLED_Line(x, y + 6, x - 5, y + 9, 1);
    OLED_Line(x, y + 6, x + 5, y + 8, 1);

    /* Moving legs */
    if ((frame & 1) == 0)
    {
        OLED_Line(x, y + 12, x - 4, y + 18, 1);
        OLED_Line(x, y + 12, x + 5, y + 17, 1);
    }
    else
    {
        OLED_Line(x, y + 12, x - 5, y + 17, 1);
        OLED_Line(x, y + 12, x + 4, y + 18, 1);
    }
}

/*-------------------- Draw One Story Frame --------------------*/
static void Night_Story_Frame(int frame)
{
    OLED_Clear();

    Draw_Stars(frame);
    Draw_Moon();

    /* Mountain outlines */
    OLED_Line(0, 49, 28, 36, 1);
    OLED_Line(28, 36, 52, 49, 1);
    OLED_Line(52, 49, 78, 33, 1);
    OLED_Line(78, 33, 105, 49, 1);
    OLED_Line(105, 49, 127, 39, 1);

    /* Ground */
    OLED_Line(0, 59, 127, 59, 1);

    /* Traveller moves across the scene */
    int person_x = 10 + ((frame * 2) % 108);

    Draw_Person(person_x, 39, frame);

    /* Small glowing trail */
    if ((frame & 1) == 0)
    {
        OLED_Pixel(person_x - 9, 47, 1);
        OLED_Pixel(person_x - 12, 44, 1);
    }

    OLED_Flush();
}

/*-------------------- Main --------------------*/
int main(void)
{
    OLED_Init();

    /*
     * Display the animated night story for approximately
     * 10 seconds: 50 frames x 200 ms per frame.
     */
    for (int frame = 0; frame < TOTAL_FRAMES; frame++)
    {
        Night_Story_Frame(frame);
        delay_ms(FRAME_DELAY);
    }

    /* Clear OLED after the story finishes */
    OLED_Clear();
    OLED_Flush();

    /* Keep the display blank until reset */
    while (1)
    {
    }
}
