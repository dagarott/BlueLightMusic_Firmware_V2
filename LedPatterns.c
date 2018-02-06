/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "LedPatterns.h"
#include "Adafruit_NeoPixel.h"

/*#define first_red (10)
#define first_blue (2)
#define first_green (127)*/

// rgb_led_t led_array[NUM_LEDS];
void showStrip()
{

    Adafruit_NeoPixel_Show();
}
void OffLeds(void)
{
    SetAll(0, 0, 0);
    showStrip();
}

void SetPixel(uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue)
{
    Adafruit_NeoPixel_setPixelColor(pixel, red, green, blue);
}

void SetAll(uint8_t red, uint8_t green, uint8_t blue)
{
    for(int i = 0; i < NUM_LEDS; i++) {

        SetPixel(i, red, green, blue);
    }
    showStrip();
}

/** @brief Function for getting vector of random numbers.
 *
 * @param[out] p_buff                               Pointer to unit8_t buffer for storing the bytes.
 * @param[in]  length                               Number of bytes to take from pool and place in p_buff.
 *
 * @retval     Number of bytes actually placed in p_buff.
 */
uint8_t FadeInOut(void)
{
    // static uint16_t i=0,j=0, k=255;
    static uint16_t index_color = 0;
    static uint16_t up_value = 0;
    static uint16_t down_value = 255;

    if(index_color < 3) {

        if(up_value < 256) {

            switch(index_color) {
            case 0:
                SetAll((uint8_t)up_value, 0, 0);
                break;
            case 1:
                SetAll(0, (uint8_t)up_value, 0);
                break;
            case 2:
                SetAll(0, 0, (uint8_t)up_value);
                break;
            }
            up_value++;
            showStrip();
            return (DELAYFADE);

        } else if(down_value > 0) {
            switch(index_color) {
            case 0:
                SetAll((uint8_t)down_value, 0, 0);
                break;
            case 1:
                SetAll(0, (uint8_t)down_value, 0);
                break;
            case 2:
                SetAll(0, 0, (uint8_t)down_value);
                break;
            }
            down_value--;
            showStrip();
            return (DELAYFADE);
        }
        down_value = 255;
        up_value = 0;
        index_color++;

    } else
        index_color = 0;

    return (0);
}

uint8_t Cyclon(void)
{

    static uint16_t index_led0 = 0, index_led1 = 2;
    const uint8_t red = 127;
    const uint8_t blue = 127;
    const uint8_t green = 127;

    SetAll(0, 0, 0);

    if(index_led0 < 3) {
        SetPixel(index_led0, red, blue, 0);
        showStrip();
        index_led0++;
        return (DELAYCYCLON);
    } else if(index_led1 > 0) {

        SetPixel(index_led1, red, 0, green);
        showStrip();
        index_led1--;
        return (DELAYCYCLON);
    } else {
        index_led0 = 0;
        index_led1 = 2;
    }
    return (0);
}

uint16_t Flash(void)
{

    static uint16_t index_led = 0;
    static uint8_t num_flashes = 0;
    uint8_t first_red = 10;
    uint8_t first_blue = 25;
    uint8_t first_green = 127;
    uint8_t second_red = 0;
    uint8_t second_blue = 255;
    uint8_t second_green = 255;

    // Note:
    // You can't INITIALIZE a static variable with an expression that isn't a
    // constant expression.
    static uint8_t tmp_red;
    static uint8_t tmp_blue;
    static uint8_t tmp_green;
    // But you are free to ASSIGN to a static variable
    // with a variable or constant expression.
    tmp_red = first_red;
    tmp_blue = first_blue;
    tmp_green = first_green;

    SetAll(0, 0, 0);

    if(index_led < 3) {
        SetPixel(index_led, tmp_red, tmp_blue, tmp_green);

        showStrip();

        tmp_red = first_red + second_red - tmp_red;
        tmp_blue = first_blue + second_blue - tmp_blue;
        tmp_green = first_green + second_green - tmp_green;

        if(num_flashes > 3) {
            index_led++;
            num_flashes = 0;
        } else
            num_flashes++;

        return (DELAYFLASHLED);
    } else
        index_led = 0;
    return (0);
}

uint16_t FlashFadeInOut(void)
{

    static uint16_t index_led = 0;
    static uint16_t up_value = 0;
    static uint16_t down_value = 255;
    uint8_t first_red = 255;
    uint8_t first_blue = 0;
    uint8_t first_green = 0;
    const uint8_t second_red = 0;
    const uint8_t second_blue = 0;
    const uint8_t second_green = 255;
    // Note:
    // You can't INITIALIZE a static variable with an expression that isn't a
    // constant expression.
    static uint8_t tmp_red;
    static uint8_t tmp_blue;
    static uint8_t tmp_green;
    // But you are free to ASSIGN to a static variable
    // with a variable or constant expression.
    tmp_red = first_red;
    tmp_blue = first_blue;
    tmp_green = first_green;

    if(index_led < 3) {

        if(up_value < 256) {
            switch(index_led) {
            case 0: {
                SetPixel((index_led % 3), (uint8_t)up_value, 0, 0);
                SetPixel(((index_led + 1) % 3), (uint8_t)up_value, 0, 0);
                SetPixel(((index_led + 2) % 3), tmp_red, tmp_blue, tmp_green);
                // i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
                tmp_red = first_red + second_red - tmp_red;
                tmp_blue = first_blue + second_blue - tmp_blue;
                tmp_green = first_green + second_green - tmp_green;
                break;
            }
            case 1: {
                SetPixel((index_led % 3), 0, (uint8_t)up_value, 0);
                SetPixel(((index_led + 1) % 3), 0, (uint8_t)up_value, 0);
                SetPixel(((index_led + 2) % 3), tmp_red, tmp_blue, tmp_green);
                // i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
                tmp_red = first_red + second_red - tmp_red;
                tmp_blue = first_blue + second_blue - tmp_blue;
                tmp_green = first_green + second_green - tmp_green;
                break;
            }
            case 2: {
                SetPixel((index_led % 3), 0, 0, (uint8_t)up_value);
                SetPixel(((index_led + 1) % 3), 0, 0, (uint8_t)up_value);
                SetPixel(((index_led + 2) % 3), tmp_red, tmp_blue, tmp_green);
                // i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
                tmp_red = first_red + second_red - tmp_red;
                tmp_blue = first_blue + second_blue - tmp_blue;
                tmp_green = first_green + second_green - tmp_green;
                break;
            }
            }
            up_value++;
            showStrip();
            return (DELAYFLASHFADE);

        } else if(down_value > 0) {
            switch(index_led) {
            case 0: {
                SetPixel((index_led % 3), (uint8_t)down_value, 0, 0);
                SetPixel(((index_led + 1) % 3), (uint8_t)down_value, 0, 0);
                SetPixel(((index_led + 2) % 3), tmp_red, tmp_blue, tmp_green);
                // i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
                tmp_red = first_red + second_red - tmp_red;
                tmp_blue = first_blue + second_blue - tmp_blue;
                tmp_green = first_green + second_green - tmp_green;
                break;
            }
            case 1: {

                SetPixel((index_led % 3), 0, (uint8_t)down_value, 0);
                SetPixel(((index_led + 1) % 3), 0, (uint8_t)down_value, 0);
                SetPixel(((index_led + 2) % 3), tmp_red, tmp_blue, tmp_green);
                // i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
                tmp_red = first_red + second_red - tmp_red;
                tmp_blue = first_blue + second_blue - tmp_blue;
                tmp_green = first_green + second_green - tmp_green;
                break;
            }
            case 2: {
                SetPixel((index_led % 3), 0, 0, (uint8_t)down_value);
                SetPixel(((index_led + 1) % 3), 0, 0, (uint8_t)down_value);
                SetPixel(((index_led + 2) % 3), tmp_red, tmp_blue, tmp_green);
                // i2s_ws2812b_drive_xfer(led_array, NUM_LEDS, I2S_STDO_PIN);
                tmp_red = first_red + second_red - tmp_red;
                tmp_blue = first_blue + second_blue - tmp_blue;
                tmp_green = first_green + second_green - tmp_green;
                break;
            }
            }
            down_value--;
            showStrip();
            return (DELAYFLASHFADE);
        }
        down_value = 255;
        up_value = 0;
        index_led++;
    } else
        index_led = 0;
    return (0);
}

uint16_t Wipe(void)
{

    // static uint16_t i=0,j=0, k=255;
    static uint16_t index = 0;
    static uint16_t up_value = 0;
    static uint16_t down_value = 256;
    static uint16_t count_debounce = 0;

    if((up_value < 256) && (down_value > 0)) {

        switch(index) {
        case 0:
            SetPixel(0, up_value, 0, 0);
            SetPixel(1, 0, down_value, 0);
            SetPixel(2, 0, 0, up_value);
            break;
        case 1:
            SetPixel(0, 0, down_value, 0);
            SetPixel(1, 0, 0, up_value);
            SetPixel(2, down_value, 0, 0);
            break;
        case 2:
            SetPixel(0, 0, 0, up_value);
            SetPixel(1, down_value, 0, 0);
            SetPixel(2, 0, down_value, 0);
            break;
        }
        up_value++;
        down_value--;
        showStrip();
        return (DELAYWIPE);

    } else {
        count_debounce++;
        if(count_debounce == DELAYBOUNCE) {
            count_debounce = 0;
            down_value = 256;
            up_value = 0;
            index++;
            if(index == 3)
                index = 0;
        }
    }
    return (0);
}

uint16_t Ring(void)
{
    static uint16_t index0 = 0, index1 = 3, index2 = 0;
    const uint8_t first_red = 255;
    // const uint8_t first_blue = 0;
    // const uint8_t first_green = 0;
    // const uint8_t second_red = 0;
    // const uint8_t second_blue = 0;
    // const uint8_t second_green = 255;

    if(index0 < 3) {
        switch(index0) {
        case 0:
            SetPixel(0, 0, first_red, 0);
            SetPixel(1, 0, 0, 0);
            SetPixel(2, 0, 0, 0);
            break;
        case 1:
            SetPixel(0, 0, first_red, 0);
            SetPixel(1, 0, first_red, 0);
            SetPixel(2, 0, 0, 0);
            break;
        case 2:
            SetPixel(0, 0, first_red, 0);
            SetPixel(1, 0, first_red, 0);
            SetPixel(2, 0, first_red, 0);
            break;
        }
        index0++;
        showStrip();
        return (DELAYRING);
    } else if(index1 > 0) {
        switch(index1) {
        case 1:
            SetPixel(0, 0, 0, first_red);
            SetPixel(1, 0, 0, 0);
            SetPixel(2, 0, 0, 0);
            break;
        case 2:
            SetPixel(0, 0, 0, first_red);
            SetPixel(1, 0, 0, first_red);
            SetPixel(2, 0, 0, 0);
            break;
        case 3:
            SetPixel(0, 0, 0, first_red);
            SetPixel(1, 0, 0, first_red);
            SetPixel(2, 0, 0, first_red);
            break;
        }
        index1--;
        showStrip();
        return (DELAYRING);
    } else if(index2 < 3) {
        switch(index2) {
        case 0:
            SetPixel(0, first_red, 0, 0);
            SetPixel(1, 0, 0, 0);
            SetPixel(2, 0, 0, 0);
            break;
        case 1:
            SetPixel(0, first_red, 0, 0);
            SetPixel(1, first_red, 0, 0);
            SetPixel(2, 0, 0, 0);
            break;
        case 2:
            SetPixel(0, first_red, 0, 0);
            SetPixel(1, first_red, 0, 0);
            SetPixel(2, first_red, 0, 0);
            break;
        }
        index2++;
        showStrip();
        return (DELAYRING);
    } else {
        index0 = 0;
        index1 = 3;
        index2 = 0;
        SetAll(127, 127, 127);
        showStrip();
        return (DELAYRINGBOUNCE);
    }
    return (0);
}
