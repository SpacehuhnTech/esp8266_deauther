#ifndef config_h
#define config_h

/* !!! Uncomment the board taht you're using !!! */

// #define NODEMCU
// #define WEMOS_D1_MINI
// #define USB_DEAUTHER
// #define DEAUTHER
// #define DEAUTHER_V1
// #define DEAUTHER_V2
// #define DEAUTHER_V3
// #define D_DUINO_B_V5_LED_RING
// #define DEAUTHER_BOY
// #define DEAUTHER_V3_5
// #define NODEMCU_07
// #define NODEMCU_07_V2
// #define DEAUTHER_OLED_V1_5_S
// #define DEAUTHER_OLED
// #define DEAUTHER_OLED_V1_5
// #define DEAUTHER_OLED_V2
// #define DEAUTHER_OLED_V2_5
// #define DEAUTHER_OLED_V3
// #define DEAUTHER_OLED_V3_5
// #define DEAUTHER_OLED_V4
// #define DEAUTHER_MOSTER
// #define USB_DEAUTHER
// #define USB_DEAUTHER_V2
// #define DEAUTHER_WRISTBAND
// #define DEAUTHER_WRISTBAND_V2
// #define DEAUTHER_MINI

#define DEFAULT_ESP8266

// ========== CONFIGS ========== //
#if defined(DEFAULT_ESP8266) || defined(NODEMCU) || defined(WEMOS_D1_MINI) || defined(USB_DEAUTHER) || defined(NODEMCU_07) || defined(DEAUTHER) || defined(DEAUTHER_V1) || defined(DEAUTHER_V2) || defined(DEAUTHER_V3)
// ===== LED ===== //
  #define LED_DIGITAL
  #define LED_PIN_R 16 // NodeMCU on-board LED
  #define LED_PIN_B 2  // ESP-12 LED

#elif defined(D_DUINO_B_V5_LED_RING)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 12
  #define LED_NEOPIXEL_PIN 15

// ===== DISPLAY ===== //
  #define SH1106_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 12
  #define BUTTON_DOWN 13
  #define BUTTON_A 14

#elif defined(DEAUTHER_BOY)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 1
  #define LED_NEOPIXEL_PIN 15

// ===== DISPLAY ===== //
  #define SH1106_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 10
  #define BUTTON_DOWN 9
  #define BUTTON_A 14
  #define BUTTON_B 12

#elif defined(DEAUTHER_V3_5) || defined(NODEMCU_07_V2)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 1
  #define LED_NEOPIXEL_PIN 15

#elif defined(DEAUTHER_OLED_V1_5_S)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 1
  #define LED_NEOPIXEL_PIN 15

// ===== DISPLAY ===== //
  #define SH1106_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 12
  #define BUTTON_DOWN 13
  #define BUTTON_A 14

#elif defined(DEAUTHER_OLED) || defined(DEAUTHER_OLED_V1_5)

// ===== LED ===== //
  #define LED_DIGITAL

  #define LED_PIN_R 16
  #define LED_PIN_B 2

// ===== DISPLAY ===== //
  #define SSD1306_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 12
  #define BUTTON_DOWN 13
  #define BUTTON_A 14

#elif defined(DEAUTHER_OLED_V2) || defined(DEAUTHER_OLED_V2_5)  || defined(DEAUTHER_OLED_V3)

// ===== LED ===== //
  #define LED_DIGITAL

  #define LED_PIN_R 16
  #define LED_PIN_B 2

// ===== DISPLAY ===== //
  #define SH1106_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 12
  #define BUTTON_DOWN 13
  #define BUTTON_A 14

#elif defined(DEAUTHER_OLED_V3_5) || defined(DEAUTHER_OLED_V4)  || defined(DEAUTHER_MOSTER)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 1
  #define LED_NEOPIXEL_PIN 15

// ===== DISPLAY ===== //
  #define SH1106_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 12
  #define BUTTON_DOWN 13
  #define BUTTON_A 14

#elif defined(USB_DEAUTHER_V2)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 1
  #define LED_NEOPIXEL_PIN 4

#elif defined(DEAUTHER_WRISTBAND) || defined(DEAUTHER_WRISTBAND_V2) || defined(DEAUTHER_MINI)

// ===== LED ===== //
  #define LED_NEOPIXEL_GRB
  #define LED_NEOPIXEL_NUM 1
  #define LED_NEOPIXEL_PIN 15

  #define HIGHLIGHT_LED 16

// ===== DISPLAY ===== //
  #define SH1106_I2C
  #define FLIP_DIPLAY true
  #define DISPLAY_TEXT "Hardware by DSTIKE"

// ===== BUTTONS ===== //
  #define BUTTON_UP 12
  #define BUTTON_DOWN 13
  #define BUTTON_A 14
#endif /* if defined(DEFAULT_ESP8266) || defined(NODEMCU) || defined(WEMOS_D1_MINI) || defined(USB_DEAUTHER) || defined(NODEMCU_07) || defined(DEAUTHER) || defined(DEAUTHER_V1) || defined(DEAUTHER_V2) || defined(DEAUTHER_V3) */
// ============================== //


// ========= FALLBACK ========= //

#ifndef DEFAULT_SSID
  #define DEFAULT_SSID "pwned"
#endif /* ifndef DEFAULT_SSID */

// =============== LED =============== //

#if defined(LED_NEOPIXEL_RGB) || defined(LED_NEOPIXEL_GRB)
  #define NEOPIXEL_LED
#endif /* if defined(LED_NEOPIXEL_RGB) || defined(LED_NEOPIXEL_GRB) */

#if !defined(LED_DIGITAL) && !defined(LED_RGB) && !defined(LED_NEOPIXEL) && !defined(MY92)
  #define LED_DIGITAL
#endif /* if !defined(LED_DIGITAL) && !defined(LED_RGB) && !defined(LED_NEOPIXEL) && !defined(MY92) */

#ifndef LED_PIN_R
  #define LED_PIN_R 255
#endif /* ifndef LED_PIN_R */

#ifndef LED_PIN_G
  #define LED_PIN_G 255
#endif /* ifndef LED_PIN_G */

#ifndef LED_PIN_B
  #define LED_PIN_B 255
#endif /* ifndef LED_PIN_B */

#ifndef LED_ANODE
  #define LED_ANODE false
#endif /* ifndef LED_ANODE */

#ifndef LED_MODE_BRIGHTNESS
  #define LED_MODE_BRIGHTNESS 10
#endif /* ifndef LED_MODE_BRIGHTNESS */

// =============== DISPLAY =============== //

#ifndef DISPLAY_TEXT
  #define DISPLAY_TEXT ""
#endif /* ifndef DISPLAY_TEXT */

#ifndef FLIP_DIPLAY
  #define FLIP_DIPLAY false
#endif /* ifndef USE_DISPLAY */

#if !defined(SSD1306_I2C) && !defined(SSD1306_SPI) && !defined(SH1106_I2C) && !defined(SH1106_SPI)
  #define SSD1306_I2C
  #define USE_DISPLAY false
#else /* if !defined(SSD1306_I2C) && !defined(SSD1306_SPI) && !defined(SH1106_I2C) && !defined(SH1106_SPI) */
  #define USE_DISPLAY true
#endif /* if !defined(SSD1306_I2C) && !defined(SSD1306_SPI) && !defined(SH1106_I2C) && !defined(SH1106_SPI) */

#ifndef I2C_ADDR
  #define I2C_ADDR 0x3C
#endif /* ifndef I2C_ADDR */

#ifndef I2C_SDA
  #define I2C_SDA 5
#endif /* ifndef I2C_SDA */

#ifndef I2C_SCL
  #define I2C_SCL 4
#endif /* ifndef I2C_SCL */

#ifndef SPI_RES
  #define SPI_RES 5
#endif /* ifndef SPI_RES */

#ifndef SPI_DC
  #define SPI_DC 4
#endif /* ifndef SPI_DC */

#ifndef SPI_CS
  #define SPI_CS 15
#endif /* ifndef SPI_CS */

// =============== BUTTONS =============== //
#ifndef BUTTON_UP
  #define BUTTON_UP 255
#endif // ifndef BUTTON_UP

#ifndef BUTTON_DOWN
  #define BUTTON_DOWN 255
#endif // ifndef BUTTON_DOWN

#ifndef BUTTON_A
  #define BUTTON_A 255
#endif // ifndef BUTTON_A

#ifndef BUTTON_B
  #define BUTTON_B 255
#endif // ifndef BUTTON_B

// ======== AVAILABLE SETTINGS ========== //

/*
 #define DEFAULT_SSID "something"

   // ===== LED ===== //
 #define LED_DIGITAL
 #define LED_RGB
 #define LED_NEOPIXEL
 #define LED_MY92

 #define LED_ANODE false

 #define LED_PIN_R 16
 #define LED_PIN_G 255
 #define LED_PIN_B 2

 #define LED_NEOPIXEL_RGB
 #define LED_NEOPIXEL_GRB

 #define LED_NEOPIXEL_NUM 1
 #define LED_NEOPIXEL_PIN 255

 #define LED_MODE_BRIGHTNESS 100

 #define LED_MY92_NUM 1
 #define LED_MY92_DATA 4
 #define LED_MY92_CLOCK 5
 #define LED_MY92_CH_R 0
 #define LED_MY92_CH_G 1
 #define LED_MY92_CH_B 2
 #define LED_MY92_CH_BRIGHTNESS 3
 #define LED_MY92_MODEL MY92XX_MODEL_MY9291
 #define LED_MY92_MODEL MY92XX_MODEL_MY9231

   // ===== DISPLAY ===== //
 #define USE_DISPLAY false
 #define FLIP_DIPLAY false

 #define SSD1306_I2C
 #define SSD1306_SPI
 #define SH1106_I2C
 #define SH1106_SPI

 #define I2C_ADDR 0x3C
 #define I2C_SDA 5
 #define I2C_SCL 4

 #define SPI_RES 5
 #define SPI_DC 4
 #define SPI_CS 15

   // ===== BUTTONS ===== //
 #define BUTTON_UP 255
 #define BUTTON_DOWN 255
 #define BUTTON_A 255
 #define BUTTON_B 255

 */

#endif /* ifndef config_h */