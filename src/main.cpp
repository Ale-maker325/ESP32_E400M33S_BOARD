/**
 * @file main.cpp
 * @author Ale-maker325 (https://github.com/Ale-maker325/ESP32_E400M33S_BOARD)
 * 
 * @brief Приклад роботи з SPI модулем E32/E22-M30S/M33S для ESP32. Приклад заснован на інших прикладах з бібліотек Adafruit SSD1306 и RadioLib,
 * та розрахован на використання з дісплеєм OLED SSD1306.
 * 
 * УВАГА!!! Усі налаштування перед компіляцією програми необхідно зробити у файлі "settings.h", який знаходиться в тій же теці, що і цей файл.
 * 
 * Перш за все потрібно обрати у якості чого буде працювати плата: передавач чи приймач. Для цього необхідно
 * розкмментувати один з дефайнів: #define RECEIVER чи #define TRANSMITTER (інший дефайн треба закоментувати).
 * Далі потрібно обрати піни, які потрібні для роботи з платою
 * 
 *  
 */

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <display.h>


uint64_t count = 0;           //Лічильник для відстеження кількості прийнятих пакетів

// #include "esp_clk.h"
// void displaySlowClockCalibration() { uint32_t slow_clk_cal = esp_clk_slowclk_cal_get(); Serial.print("Slow Clock Calibration Value: "); Serial.print(slow_clk_cal); Serial.println(" microseconds"); }
// void displayCpuFrequency() { int cpu_freq = esp_clk_cpu_freq(); Serial.print("CPU Frequency: "); Serial.print(cpu_freq); Serial.println(" Hz"); }
// void displayApbFrequency() { int apb_freq = esp_clk_apb_freq(); Serial.print("APB Frequency: "); Serial.print(apb_freq); Serial.println(" Hz"); }
// void displayRtcTime() { uint64_t rtc_time = esp_clk_rtc_time(); Serial.print("RTC Time: "); Serial.print(rtc_time); Serial.println(" microseconds"); }


void setup() {
  //Ініціалізуємо серійний порт для виводу інформації
  #ifdef DEBUG_PRINT
    Serial.begin(115200);
  #endif

  //Ініціалізуємо шину I2C для роботи з дисплеєм
  display_wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
  //Ініціалізуємо SPI для роботи з радіо-модулем
  SPI_MODEM.begin(SCK_RADIO, MISO_RADIO, MOSI_RADIO);
  
  #ifdef DEBUG_PRINT
    Serial.printf("Chip Model %s, ChipRevision %d, Cpu Freq %d, SDK Version %s\n", ESP.getChipModel(), ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
  #endif
    
  //Ініціалізуємо дисплей
  displayInit();

  #ifdef DEBUG_PRINT
    Serial.print(TABLE_LEFT);
    Serial.print(F("DISPLAY INIT"));
    Serial.println(TABLE_RIGHT);
    Serial.println(SPACE);
  #endif
  
  pinMode(LED_PIN, OUTPUT);      //Контакт управління світлодіодом налаштовуємо як вихідний
  pinMode(FAN, OUTPUT);          //Контакт управління вентилятором налаштовуємо як вихідний
  
  //Встановлюємо параметри конфігурації радіо-модуля.
  setRadioMode();

  radioBeginAll();
    

  #ifdef DEBUG_PRINT
    Serial.println(SPACE);
    Serial.println(SPACE);
  #endif
    
  //Устанавливаем наши значения, определённые ранее в структуре config_radio1
  radio_setSettings(radio, config_radio);
  
  RadioStart(); //Запускаем радио-модуль

  #ifdef DEBUG_PRINT
    Serial.println(" ");
  #endif
}

 























void loop() {
  
  delay(300);
  
  digitalWrite(LED_PIN, HIGH); //Выключаем светодиод, сигнализация об окончании передачи/приёма пакета
  


  #ifdef RECEIVER   //Если определен модуль как приёмник
    //проверяем, была ли предыдущая передача успешной
    #ifdef DEBUG_PRINT
    Serial.println("..................................................");
    #endif
    if(operationDone_2) {
      
      //Сбрасываем сработавший флаг прерывания
      operationDone_2 = false;

      //готовим строку для отправки
      String str = "#" + String(count++);

      receive_and_print_data(str);
      
      
    }

    // check CAD result
    detected_CAD(Radio_1);
    detectedPreamble(Radio_1);
    

    #ifdef RADIO_2
    if(operationDone_2) {
      
      //Сбрасываем сработавший флаг прерывания
      operationDone_2 = false;

      //готовим строку для отправки
      String str = "#" + String(count++);

      receive_and_print_data(str);

      // check CAD result
      detected_CAD(Radio_2);
      detectedPreamble(Radio_2);

      
      
    }
    #endif
  #endif


  #ifdef TRANSMITTER   //Если определен как передатчик
    //проверяем, была ли предыдущая передача успешной
    #ifdef DEBUG_PRINT
    Serial.println("..................................................");
    #endif
    if(operationDone) {
      
      //Сбрасываем сработавший флаг прерывания
      operationDone = false;

      //готовим строку для отправки
      String str = "#" + String(count++);

      transmit_and_print_data(str);
      
      
    }






    // // check CAD result
    // int state = radio1.getChannelScanResult();

    // if (state == RADIOLIB_LORA_DETECTED) {
    //   // LoRa packet was detected
    //   #ifdef DEBUG_PRINT
    //   Serial.println(F("[LR1110] Packet detected!"));
    //   #endif

    // } else if (state == RADIOLIB_CHANNEL_FREE) {
    //   // channel is free
    //   #ifdef DEBUG_PRINT
    //   Serial.println(F("[LR1110] Channel is free!"));
    //   #endif

    // } else {
    //   // some other error occurred
    //   #ifdef DEBUG_PRINT
    //   Serial.print(F("[LR1110] Failed, code "));
    //   Serial.println(state);
    //   #endif

    // }





    // #ifdef DEBUG_PRINT
    // Serial.println(F("[LR1110] Scanning channel for LoRa transmission ... "));
    // #endif

    // // start scanning current channel
    // state = radio1.scanChannel();

    // if (state == RADIOLIB_LORA_DETECTED) {
    //   // LoRa preamble was detected
    //   #ifdef DEBUG_PRINT
    //   Serial.println(F("detected!"));
    //   #endif

    // } else if (state == RADIOLIB_CHANNEL_FREE) {
    //   // no preamble was detected, channel is free
    //   #ifdef DEBUG_PRINT
    //   Serial.println(F("channel is free!"));
    //   #endif

    // } else {
    //   // some other error occurred
    //   #ifdef DEBUG_PRINT
    //   Serial.print(F("failed, code "));
    //   Serial.println(state);
    //   #endif

    // }



    


    #ifdef RADIO_2
    if(operationDone_2) {
      
      //Сбрасываем сработавший флаг прерывания
      operationDone_2 = false;

      //готовим строку для отправки
      String str = "#" + String(count++);

      transmit_and_print_data(str);
      
      
    }
    #endif
  #endif

  
}



