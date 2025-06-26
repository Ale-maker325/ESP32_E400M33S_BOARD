#ifndef ________RADIO_LR1121________
#define ________RADIO_LR1121________


#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <stdint.h>
#include <settings.h>
#include <display.h>



int state_1 = RADIOLIB_ERR_NONE; // Переменная, хранящая код состояния передачи/приёма

SPIClass SPI_MODEM;
SX1278 radio = new Module(NSS_PIN, BUSY_PIN, NRST_PIN, DIO1_PIN); //Инициализируем экземпляр радио




/**
* @brief Структура для настройки параметров радиотрансивера
* 
*/
struct LORA_CONFIGURATION
{
  float frequency = 441.0;        //Частота работы передатчика (по-умолчанию 434 MHz)
  float bandwidth = 125.0;        //Полоса пропускания (по-умолчанию 125 килогерц)
  uint8_t spreadingFactor = 9;   //Коэффициент расширения (по-умолчанию 9)
  uint8_t codingRate = 7;         //Скорость кодирования (по-умолчанию 7)
  uint8_t syncWord = 0x18;        //Слово синхронизации (по-умолчанию 0х18). ВНИМАНИЕ! Значение 0x34 зарезервировано для сетей LoRaWAN и нежелательно для использования
  int8_t outputPower = 10;        //Установить выходную мощность (по-умолчанию 10 дБм) (допустимый диапазон -3 - 17 дБм) ПРИМЕЧАНИЕ: значение 20 дБм позволяет работать на большой мощности, но передача рабочий цикл НЕ ДОЛЖЕН ПРЕВЫШАТЬ 1
  uint8_t currentLimit = 100;      //Установить предел защиты по току (по-умолчанию до 80 мА) (допустимый диапазон 45 - 240 мА) ПРИМЕЧАНИЕ: установить значение 0 для отключения защиты от перегрузки по току
  int16_t preambleLength = 8;    //Установить длину преамбулы (по-умолчанию в 8 символов) (допустимый диапазон 6 - 65535)
  uint8_t gain = 0;               //Установить регулировку усилителя (по-умолчанию 1) (допустимый диапазон 1 - 6, где 1 - максимальный рост) ПРИМЕЧАНИЕ: установить значение 0, чтобы включить автоматическую регулировку усиления оставьте в 0, если вы не знаете, что вы делаете


};



//Экземпляр структуры для настройки параметров радиотрансивера
LORA_CONFIGURATION config_radio;


/**
 * @brief Настройка радио передатчика в соответствии с директивами,
 * которые заданы в файле "settings.h"
 * 
 */
void setRadioMode()
{
  //Задаём параметры конфигурации радиотрансивера 1
  config_radio.frequency = RADIO_FREQ;
  config_radio.bandwidth = RADIO_BANDWIDTH;
  config_radio.spreadingFactor = RADIO_SPREAD_FACTOR;
  config_radio.codingRate = RADIO_CODING_RATE;
  config_radio.syncWord = RADIO_SYNC_WORD;
  config_radio.outputPower = RADIO_OUTPUT_POWER;
  config_radio.currentLimit = RADIO_CURRENT_LIMIT;
  config_radio.preambleLength = RADIO_PREAMBLE_LENGTH;
  config_radio.gain = RADIO_GAIN;
}

//Флаг окончания операции отправки/получения модема №1 чтобы указать, что пакет был отправлен или получен
volatile bool operationDone = false;

// Эта функция вызывается, когда модем №1 передает или получает полный пакет
// ВАЖНО: эта функция ДОЛЖНА БЫТЬ 'пуста' типа и НЕ должна иметь никаких аргументов!
IRAM_ATTR void flag_operationDone(void) {
// мы отправили или получили пакет, установите флаг
  operationDone = true;
}




/**
 * @brief Функция ожидания, пока радио не будет занято
 * 
 * @param radioNumber Номер радио, для которого нужно ждать
 * @return true Если радио не занято
 * @return false Если время ожидания истекло
 */
bool ICACHE_RAM_ATTR WaitOnBusy()
{
    constexpr uint32_t wtimeoutUS = 1000U;
    uint32_t startTime = 0;
    #ifdef DEBUG_PRINT
    Serial.println("");
    Serial.print("WaitOnBusy.....................  ");
    Serial.println("");
    #endif

    while (true)
    {
      if (digitalRead(BUSY_PIN) == LOW) return true;
      
      // Use this time to call micros().
      uint32_t now = micros();
      if (startTime == 0) startTime = now;
      if ((now - startTime) > wtimeoutUS) return false;
    }
}


/**
* @brief Функция для обнаружения LoRa-пакета на заданном радио
*/
void detected_CAD()
{
  int state;
  state = radio.getChannelScanResult();
  if (state == RADIOLIB_LORA_DETECTED) {
    // LoRa пакет було виявлено
    #ifdef DEBUG_PRINT
      Serial.println(F("RADIO RX packet detected!"));
    #endif
  } else {
    //
    #ifdef DEBUG_PRINT
      Serial.print(F("RADIO RX packet detected Failed, code "));
      Serial.println(state);
    #endif
  }
}


/**
* @brief Функция для обнаружения LoRa-преамбулы на заданном радио
* 
*/
void detectedPreamble()
{
  int state;
  // start scanning current channel
  state = radio.scanChannel();
  if (state == RADIOLIB_LORA_DETECTED)
  {
    // LoRa preamble was detected
    #ifdef DEBUG_PRINT
      Serial.println(F("Preamble detected!"));
    #endif

  } else {
    // some other error occurred
    #ifdef DEBUG_PRINT
      Serial.print(F("Preamble detected failed, code "));
      Serial.println(state);
    #endif
    
  }
}

/**
 * @brief Функция для печати состояния радио в терминал
 * 
 * @param RadioName Имя радио
 * @param state Состояние радио
 */
void print_to_terminal_radio_state(String &RadioName, String state) __attribute__ ((weak));
void displayPrintState(int16_t x, int16_t y, String &RadioName, String state) __attribute__ ((weak));



/**
* @brief Функція для друку стану ініціалізації радіо на дисплеї
* 
* @param STATE 
*/
void printRadioBeginResult(int &STATE)
{
  String radio_name;
  int x,y = 5;
  
  if (STATE == RADIOLIB_ERR_NONE) {
    #ifdef DEBUG_PRINT
      print_to_terminal_radio_state(radio_name, F("INIT_GOOD"));
    #endif
      displayPrintState(x, y, radio_name, F("INIT_GOOD"));
  } else {

    String str = "ERROR " + (String)STATE;
    #ifdef DEBUG_PRINT
      print_to_terminal_radio_state(radio_name, str);
    #endif
      displayPrintState(x, y, radio_name, str);
    while (true);
  }
}


/**
* @brief Функция для выбора радио. У випадку підключення іншого устройства SPI на 
* шину SPI радіо, ця функція дозволяє обирати радіо, з яким буде проводитися
* 
*/
void ICACHE_RAM_ATTR selectRadio()
{
    digitalWrite(NSS_PIN, LOW);
}




/**
 * @brief Настройка радио передатчика в соответствии с директивами,
 * которые заданы в файле "settings.h"
 */
void radioBeginAll()
{
  pinMode(NSS_PIN, OUTPUT);
  //Инициализируем радиотрансивер 1 со значениями по-умолчанию, заданными в
  //структуре LORA_CONFIGURATION
  #ifdef DEBUG_PRINT
    Serial.println(" ");
    Serial.println(F(""));
    Serial.println(" ");
    Serial.println(F("RADIO INIT....."));
  #endif
  
  int state = radio.begin();
  printRadioBeginResult(state);

  WaitOnBusy();
 
  #ifdef DEBUG_PRINT
    delay(1000);
  #endif
}

#endif;