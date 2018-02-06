/*-------------------------------------------------------------------------
 Arduino library to control a wide variety of WS2811- and WS2812-based RGB
 LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
 Currently handles 400 and 800 KHz bitstreams on 8, 12 and 16 MHz ATmega
 MCUs, with LEDs wired for various color orders.  Handles most output pins
 (possible exception with upper PORT registers on the Arduino Mega).

 Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
 contributions by PJRC, Michael Miller and other members of the open
 source community.

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing products
 from Adafruit!

 -------------------------------------------------------------------------
 This file is part of the Adafruit NeoPixel library.

 NeoPixel is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as
 published by the Free Software Foundation, either version 3 of
 the License, or (at your option) any later version.

 NeoPixel is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General PublicP
 License along with NeoPixel.  If not, see
 <http://www.gnu.org/licenses/>.
 -------------------------------------------------------------------------*/


#include "Adafruit_NeoPixel.h"

/**
 * @brief      { function_description }
 *
 * @param[in]  n     { parameter_description }
 */
void Adafruit_NeoPixel_updateLength(uint16_t n)
{
    if (pixels)
        free(pixels); // Free existing data (if any)

    // Allocate new data -- note: ALL PIXELS ARE CLEARED
    numBytes = n * ((wOffset == rOffset) ? 3 : 4);
    if ((pixels = (uint8_t*)malloc(numBytes))) {
        memset(pixels, 0, numBytes);
        numLEDs = n;
    } else

    {
        numLEDs = numBytes = 0;
    }
}

/**
 * @brief      { function_description }
 *
 * @param[in]  t     { parameter_description }
 */
void Adafruit_NeoPixel_updateType(neoPixelType t)
{
    bool oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

    wOffset = (t >> 6) & 0b11; // See notes in header file
    rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
    gOffset = (t >> 2) & 0b11;
    bOffset = t & 0b11;
#ifdef NEO_KHZ400
    is800KHz = (t < 256); // 400 KHz flag is 1<<8
#endif

    // If bytes-per-pixel has changed (and pixel data was previously
    // allocated), re-allocate to new size.  Will clear any data.
    if (pixels) {
        bool newThreeBytesPerPixel = (wOffset == rOffset);
        if ((newThreeBytesPerPixel) != (oldThreeBytesPerPixel))
            Adafruit_NeoPixel_updateLength(numLEDs);
    }
}

// Set the output pin number
/**
 * @brief
 * @param p
 */
void Adafruit_NeoPixel_setPin(uint8_t p)
{
    // if (begun && (pin >= 0))
    // pinMode(pin, INPUT);
    // nrf_gpio_cfg_input(pin);
    // nrf_gpio_pin_write(pin, 0);
    pin = p;
    if (begun) {
        // pinMode(p, OUTPUT);
        // digitalWrite(p, LOW);
        nrf_gpio_cfg_output(pin);
        nrf_gpio_pin_write(pin, 0);
    }
}

/**
 * @brief      { function_description }
 *
 * @param[in]  n     { parameter_description }
 * @param[in]  p     { parameter_description }
 * @param[in]  t     { parameter_description }
 */
void Adafruit_NeoPixel_Init(uint16_t n, uint8_t p, neoPixelType t)
{
    begun = false;
    brightness = 0;
    pixels = NULL;
    Adafruit_NeoPixel_updateType(t);
    Adafruit_NeoPixel_updateLength(n);
    Adafruit_NeoPixel_setPin(p);
}

/**
 * @brief      { function_description }
 */
void Adafruit_NeoPixel_Begin(void)
{
    if (pin >= 0) {
        // pinMode(pin, OUTPUT);
        // digitalWrite(pin, LOW);
        nrf_gpio_cfg_output(pin);
        nrf_gpio_pin_write(pin, 0);
    }
    begun = true;
}
/**
 * @brief
 */

void Adafruit_NeoPixel_Show(void)
{

    if (!pixels)
        return;

    // Data latch = 300+ microsecond pause in the output stream.  Rather than
    // put a delay at the end of the function, the ending time is noted and
    // the function will simply hold off (if needed) on issuing the
    // subsequent round of data until the latch time has elapsed.  This
    // allows the mainline code to start generating the next frame of data
    // rather than stalling for the latch.
    // while (!canShow());
    nrf_delay_us(300);
// Begin of support for NRF52832 based boards  -------------------------
// [[[Begin of the Neopixel NRF52 EasyDMA implementation
//                                    by the Hackerspace San Salvador]]]
// This technique uses the PWM peripheral on the NRF52. The PWM uses the
// EasyDMA feature included on the chip. This technique loads the duty
// cycle configuration for each cycle when the PWM is enabled. For this
// to work we need to store a 16 bit configuration for each bit of the
// RGB(W) values in the pixel buffer.
// Comparator values for the PWM were hand picked and are guaranteed to
// be 100% organic to preserve freshness and high accuracy. Current
// parameters are:
//   * PWM Clock: 16Mhz
//   * Minimum step time: 62.5ns
//   * Time for zero in high (T0H): 0.31ms
//   * Time for one in high (T1H): 0.75ms
//   * Cycle time:  1.25us
//   * Frequency: 800Khz
// For 400Khz we just double the calculated times.
// ---------- BEGIN Constants for the EasyDMA implementation -----------
// The PWM starts the duty cycle in LOW. To start with HIGH we
// need to set the 15th bit on each register.

// WS2812 (rev A) timing is 0.35 and 0.7us
//#define MAGIC_T0H               5UL | (0x8000) // 0.3125us
//#define MAGIC_T1H              12UL | (0x8000) // 0.75us

// WS2812B (rev B) timing is 0.4 and 0.8 us
#define MAGIC_T0H 6UL | (0x8000)  // 0.375us
#define MAGIC_T1H 13UL | (0x8000) // 0.8125us

// WS2811 (400 khz) timing is 0.5 and 1.2
#define MAGIC_T0H_400KHz 8UL | (0x8000)  // 0.5us
#define MAGIC_T1H_400KHz 19UL | (0x8000) // 1.1875us

// For 400Khz, we double value of CTOPVAL
#define CTOPVAL 20UL        // 1.25us
#define CTOPVAL_400KHz 40UL // 2.5us

// ---------- END Constants for the EasyDMA implementation -------------
//
// If there is no device available an alternative cycle-counter
// implementation is tried.
// The nRF52832 runs with a fixed clock of 64Mhz. The alternative
// implementation is the same as the one used for the Teensy 3.0/1/2 but
// with the Nordic SDK HAL & registers syntax.
// The number of cycles was hand picked and is guaranteed to be 100%
// organic to preserve freshness and high accuracy.
// ---------- BEGIN Constants for cycle counter implementation ---------
#define CYCLES_800_T0H 18 // ~0.36 uS
#define CYCLES_800_T1H 41 // ~0.76 uS
#define CYCLES_800 71     // ~1.25 uS

#define CYCLES_400_T0H 26 // ~0.50 uS
#define CYCLES_400_T1H 70 // ~1.26 uS
#define CYCLES_400 156    // ~2.50 uS
    // ---------- END of Constants for cycle counter implementation --------

    // To support both the SoftDevice + Neopixels we use the EasyDMA
    // feature from the NRF25. However this technique implies to
    // generate a pattern and store it on the memory. The actual
    // memory used in bytes corresponds to the following formula:
    //              totalMem = numBytes*8*2+(2*2)
    // The two additional bytes at the end are needed to reset the
    // sequence.
    //
    // If there is not enough memory, we will fall back to cycle counter
    // using DWT
    uint32_t pattern_size = numBytes * 8 * sizeof(uint16_t) + 2 * sizeof(uint16_t);
    uint16_t* pixels_pattern = NULL;

    NRF_PWM_Type* pwm = NULL;

    // Try to find a free PWM device, which is not enabled
    // and has no connected pins
    NRF_PWM_Type* PWM[3] = { NRF_PWM0, NRF_PWM1, NRF_PWM2 };

    for (int device = 0; device < 3; device++) {
        if ((PWM[device]->ENABLE == 0) && (PWM[device]->PSEL.OUT[0] & PWM_PSEL_OUT_CONNECT_Msk) &&
            (PWM[device]->PSEL.OUT[1] & PWM_PSEL_OUT_CONNECT_Msk) &&
            (PWM[device]->PSEL.OUT[2] & PWM_PSEL_OUT_CONNECT_Msk) &&
            (PWM[device]->PSEL.OUT[3] & PWM_PSEL_OUT_CONNECT_Msk)) {
            pwm = PWM[device];
            break;
        }
    }

    // only malloc if there is PWM device available
    if (pwm != NULL) {

        pixels_pattern = (uint16_t*)malloc(pattern_size);
    }
    // Use the identified device to choose the implementation
    // If a PWM device is available use DMA
    if ((pixels_pattern != NULL) && (pwm != NULL)) {
        uint16_t pos = 0; // bit position

        for (uint16_t n = 0; n < numBytes; n++) {
            uint8_t pix = pixels[n];

            for (uint8_t mask = 0x80, i = 0; mask > 0; mask >>= 1, i++) {
#ifdef NEO_KHZ400
                if (!is800KHz) {
                    pixels_pattern[pos] = (pix & mask) ? MAGIC_T1H_400KHz : MAGIC_T0H_400KHz;
                } else
#endif
                {
                    pixels_pattern[pos] = (pix & mask) ? MAGIC_T1H : MAGIC_T0H;
                }

                pos++;
            }
        }

        // Zero padding to indicate the end of que sequence
        pixels_pattern[++pos] = 0 | (0x8000); // Seq end
        pixels_pattern[++pos] = 0 | (0x8000); // Seq end

        // Set the wave mode to count UP
        pwm->MODE = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);

        // Set the PWM to use the 16MHz clock
        pwm->PRESCALER = (PWM_PRESCALER_PRESCALER_DIV_1 << PWM_PRESCALER_PRESCALER_Pos);

// Setting of the maximum count
// but keeping it on 16Mhz allows for more granularity just
// in case someone wants to do more fine-tuning of the timing.
#ifdef NEO_KHZ400
        if (!is800KHz) {
            pwm->COUNTERTOP = (CTOPVAL_400KHz << PWM_COUNTERTOP_COUNTERTOP_Pos);
        } else
#endif
        {
            pwm->COUNTERTOP = (CTOPVAL << PWM_COUNTERTOP_COUNTERTOP_Pos);
        }

        // Disable loops, we want the sequence to repeat only once
        pwm->LOOP = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);

        // On the "Common" setting the PWM uses the same pattern for the
        // for supported sequences. The pattern is stored on half-word
        // of 16bits
        pwm->DECODER =
            (PWM_DECODER_LOAD_Common << PWM_DECODER_LOAD_Pos) | (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);

        // Pointer to the memory storing the patter
        pwm->SEQ[0].PTR = (uint32_t)(pixels_pattern) << PWM_SEQ_PTR_PTR_Pos;

        // Calculation of the number of steps loaded from memory.
        pwm->SEQ[0].CNT = (pattern_size / sizeof(uint16_t)) << PWM_SEQ_CNT_CNT_Pos;

        // The following settings are ignored with the current config.
        pwm->SEQ[0].REFRESH = 0;
        pwm->SEQ[0].ENDDELAY = 0;

        // The Neopixel implementation is a blocking algorithm. DMA
        // allows for non-blocking operation. To "simulate" a blocking
        // operation we enable the interruption for the end of sequence
        // and block the execution thread until the event flag is set by
        // the peripheral.
        //    pwm->INTEN |= (PWM_INTEN_SEQEND0_Enabled<<PWM_INTEN_SEQEND0_Pos);

        // PSEL must be configured before enabling PWM
        pwm->PSEL.OUT[0] = pin;

        // Enable the PWM
        pwm->ENABLE = 1;

        // After all of this and many hours of reading the documentation
        // we are ready to start the sequence...
        pwm->EVENTS_SEQEND[0] = 0;
        pwm->TASKS_SEQSTART[0] = 1;

        // But we have to wait for the flag to be set.
        while (!pwm->EVENTS_SEQEND[0]) {
        }

        // Before leave we clear the flag for the event.
        pwm->EVENTS_SEQEND[0] = 0;

        // We need to disable the device and disconnect
        // all the outputs before leave or the device will not
        // be selected on the next call.
        // TODO: Check if disabling the device causes performance issues.
        pwm->ENABLE = 0;

        pwm->PSEL.OUT[0] = 0xFFFFFFFFUL;

        free(pixels_pattern);

    } // End of DMA implementation
    // ---------------------------------------------------------------------
    else {
        // Fall back to DWT
        //#ifdef ARDUINO_FEATHER52
        //// Bluefruit Feather 52 uses freeRTOS
        //// Critical Section is used since it does not block SoftDevice execution
        // taskENTER_CRITICAL();
        //#elif defined(NRF52_DISABLE_INT)
        //// If you are using the Bluetooth SoftDevice we advise you to not disable
        //// the interrupts. Disabling the interrupts even for short periods of time
        //// causes the SoftDevice to stop working.
        //// Disable the interrupts only in cases where you need high performance for
        //// the LEDs and if you are not using the EasyDMA feature.
        //__disable_irq();
        //#endif

        uint32_t pinMask = 1UL << pin;

        uint32_t CYCLES_X00 = CYCLES_800;
        uint32_t CYCLES_X00_T1H = CYCLES_800_T1H;
        uint32_t CYCLES_X00_T0H = CYCLES_800_T0H;

#ifdef NEO_KHZ400
        if (!is800KHz) {
            CYCLES_X00 = CYCLES_400;
            CYCLES_X00_T1H = CYCLES_400_T1H;
            CYCLES_X00_T0H = CYCLES_400_T0H;
        }
#endif

        // Enable DWT in debug core
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

        // Tries to re-send the frame if is interrupted by the SoftDevice.
        while (1) {
            uint8_t* p = pixels;

            uint32_t cycStart = DWT->CYCCNT;
            uint32_t cyc = 0;

            for (uint16_t n = 0; n < numBytes; n++) {
                uint8_t pix = *p++;

                for (uint8_t mask = 0x80; mask; mask >>= 1) {
                    while (DWT->CYCCNT - cyc < CYCLES_X00)
                        ;
                    cyc = DWT->CYCCNT;

                    NRF_GPIO->OUTSET |= pinMask;

                    if (pix & mask) {
                        while (DWT->CYCCNT - cyc < CYCLES_X00_T1H)
                            ;
                    } else {
                        while (DWT->CYCCNT - cyc < CYCLES_X00_T0H)
                            ;
                    }

                    NRF_GPIO->OUTCLR |= pinMask;
                }
            }
            while (DWT->CYCCNT - cyc < CYCLES_X00)
                ;

            // If total time longer than 25%, resend the whole data.
            // Since we are likely to be interrupted by SoftDevice
            if ((DWT->CYCCNT - cycStart) < (8 * numBytes * ((CYCLES_X00 * 5) / 4))) {
                break;
            }
            // re-send need 300us delay
            nrf_delay_us(300);
        }
    }
    // END of NRF52 implementation
    // END ARCHITECTURE SELECT ------------------------------------------------

    // endTime = micros();     // Save EOD time for latch on next call
}

// Set pixel color from separate R,G,B components:
// void Adafruit_NeoPixel_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
//{

// if (n < numLEDs)
//{
// if (brightness)     // See notes in setBrightness()
//{
// r = (r * brightness) >> 8;
// g = (g * brightness) >> 8;
// b = (b * brightness) >> 8;
//}
// uint8_t *p;
// if (wOffset == rOffset)     // Is an RGB-type strip
//{
// p = &pixels[n * 3]; // 3 bytes per pixel
//}
// else            // Is a WRGB-type strip
//{
// p = &pixels[n * 4]; // 4 bytes per pixel
// p[wOffset] = 0; // But only R,G,B passed -- set W to 0
//}
// p[rOffset] = r;     // R,G,B always stored
// p[gOffset] = g;
// p[bOffset] = b;
//}
//}
void Adafruit_NeoPixel_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
// void Adafruit_NeoPixel_setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{

    if (n < numLEDs) {
        if (brightness) { // See notes in setBrightness()
            r = (r * brightness) >> 8;
            g = (g * brightness) >> 8;
            b = (b * brightness) >> 8;
            // w = (w * brightness) >> 8;
        }
        uint8_t* p;
        if (wOffset == rOffset) { // Is an RGB-type strip
            p = &pixels[n * 3]; // 3 bytes per pixel (ignore W)
        }
        // else            // Is a WRGB-type strip
        //{
        //  p = &pixels[n * 4]; // 4 bytes per pixel
        // p[wOffset] = w; // Store W
        //}
        p[rOffset] = r; // Store R,G,B
        p[gOffset] = g;
        p[bOffset] = b;
    }
}

// Set pixel color from 'packed' 32-bit RGB color:
// void Adafruit_NeoPixel_setPixelColor(uint16_t n, uint32_t c)
//{
// if (n < numLEDs)
//{
// uint8_t *p, r = (uint8_t) (c >> 16), g = (uint8_t) (c >> 8), b = (uint8_t) c;
// if (brightness)     // See notes in setBrightness()
//{
// r = (r * brightness) >> 8;
// g = (g * brightness) >> 8;
// b = (b * brightness) >> 8;
//}
// if (wOffset == rOffset)
//{
// p = &pixels[n * 3];
//}
// else
//{
// p = &pixels[n * 4];
// uint8_t w = (uint8_t) (c >> 24);
// p[wOffset] = brightness ? ((w * brightness) >> 8) : w;
//}

// p[rOffset] = r;
// p[gOffset] = g;
// p[bOffset] = b;
//}
//}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t Adafruit_NeoPixel_Color(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Convert separate R,G,B,W into packed 32-bit WRGB color.
// Packed format is always WRGB, regardless of LED strand color order.
// uint32_t Adafruit_NeoPixel_Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
//{
// return ((uint32_t) w << 24) | ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
//}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t Adafruit_NeoPixel_getPixelColor(uint16_t n)
{
    if (n >= numLEDs)
        return 0; // Out of bounds, return no color.

    uint8_t* p;

    if (wOffset == rOffset) {
        // Is RGB-type device
        p = &pixels[n * 3];
        if (brightness) {
            // Stored color was decimated by setBrightness().  Returned value
            // attempts to scale back to an approximation of the original 24-bit
            // value used when setting the pixel color, but there will always be
            // some error -- those bits are simply gone.  Issue is most
            // pronounced at low brightness levels.
            return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
                   (((uint32_t)(p[gOffset] << 8) / brightness) << 8) | ((uint32_t)(p[bOffset] << 8) / brightness);
        } else {
            // No brightness adjustment has been made -- return 'raw' color
            return ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
        }
    } else { // Is RGBW-type device
        p = &pixels[n * 4];
        if (brightness) { // Return scaled color
            return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) |
                   (((uint32_t)(p[rOffset] << 8) / brightness) << 16) | (((uint32_t)(p[gOffset] << 8) / brightness) << 8) |
                   ((uint32_t)(p[bOffset] << 8) / brightness);
        } else { // Return raw color
            return ((uint32_t)p[wOffset] << 24) | ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) |
                   (uint32_t)p[bOffset];
        }
    }
}

// Returns pointer to pixels[] array.  Pixel data is stored in device-
// native format and is not translated here.  Application will need to be
// aware of specific pixel data format and handle colors appropriately.
uint8_t* Adafruit_NeoPixel_getPixels(void)
{
    return pixels;
}
uint16_t Adafruit_NeoPixel_numPixels(void)
{
    return numLEDs;
}
// Adjust output brightness; 0=darkest (off), 255=brightest.  This does// NOT immediately affect what's currently
// displayed on the LEDs.  The// next call to show() will refresh the LEDs at this level.  However,// this process is
// potentially "lossy," especially when increasing// brightness.  The tight timing in the WS2811/WS2812 code means
// there// aren't enough free cycles to perform this scaling on the fly as data// is issued.  So we make a pass through
// the existing color data in RAM// and scale it (subsequent graphics commands also work at this// brightness level).
// If there's a significant step up in brightness,// the limited number of steps (quantization) in the old data will
// be// quite visible in the re-scaled version.  For a non-destructive// change, you'll need to re-render the full strip
// data.  C'est la vie.
void Adafruit_NeoPixel_setBrightness(uint8_t b)
{
    // Stored brightness value is different than what's passed.
    // This simplifies the actual scaling math later, allowing a fast
    // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
    // adding 1 here may (intentionally) roll over...so 0 = max brightness
    // (color values are interpreted literally; no scaling), 1 = min
    // brightness (off), 255 = just below max brightness.
    uint8_t newBrightness = b + 1;
    if (newBrightness != brightness) { // Compare against prior value
        // Brightness has changed -- re-scale existing data in RAM
        uint8_t c, *ptr = pixels, oldBrightness = brightness - 1; // De-wrap old brightness value
        uint16_t scale;
        if (oldBrightness == 0)
            scale = 0; // Avoid /0
        else if (b == 255)
            scale = 65535 / oldBrightness;
        else
            scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
        for (uint16_t i = 0; i < numBytes; i++) {
            c = *ptr;
            *ptr++ = (c * scale) >> 8;
        }
        brightness = newBrightness;
    }
}

// Return the brightness value
uint8_t Adafruit_NeoPixel_getBrightness(void)
{
    return brightness - 1;
}
void Adafruit_NeoPixel_clear(void)
{
    memset(pixels, 0, numBytes);
}
