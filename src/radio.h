#ifndef ________RADIO_LR1121________
#define ________RADIO_LR1121________


#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <stdint.h>
#include <settings.h>
#include <display.h>



int state = RADIOLIB_ERR_NONE; // Переменная, хранящая код состояния передачи/приёма

SPIClass SPI_MODEM;

#ifdef SX1278_MODEM
  SX1278 radio = new Module(NSS_PIN, BUSY_PIN, NRST_PIN, DIO1_PIN); //Инициализируем экземпляр радио
#elif defined(SX1278_MODEM)
  SX1262 radio = new Module(NSS_PIN, BUSY_PIN, NRST_PIN, DIO1_PIN); //Инициализируем экземпляр радио
#endif




//************************************** Строки для формирования вывода информации ***************************************************** */

String RSSI = F("RSSI(");         //Строка для печати RSSI
String dBm = F(")dBm");           //Строка для печати RSSI

String SNR = F("SNR(");           //Строка для печати SNR
String dB = F(")dB");             //Строка для печати SNR

String FR_ERR = F("F_Err(");      //Строка для печати SNR
String HZ = F(")Hz");             //Строка для печати SNR

String DT_RATE = F("RATE(");      //Строка для печати скорости передачи данных
String BS = F(")B/s");            //Строка для печати скорости передачи данных

String RECEIVE = F("RECEIVE: ");  //Строка сообщения для приёма

String TABLE_RIGHT = F("     ***************************");
String TABLE_LEFT  = F("***************************     ");
String SPACE = F(" ");

//************************************** Строки для формирования вывода информации ***************************************************** */





/**
* @brief Структура для зберігання параметрів конфігурації радіо-модуля.
*  Ця структура містить всі налаштування, які можна змінити для налаштування роботи радіо-модуля, 
*  але налаштування будуть застосовані лише після виклику функції setRadioMode(), який бере
*  парамерти з налаштувань, які задані в файлі "settings.h". Тут ніякі параметри змінювати не потрібно
*/
struct LORA_CONFIGURATION
{
  float frequency = 441.0;        //Частота работы передатчика
  float bandwidth = 125.0;        //Полоса пропускания (по-умолчанию 125 килогерц)
  uint8_t spreadingFactor = 9;   //Коэффициент расширения (по-умолчанию 9)
  uint8_t codingRate = 7;         //Скорость кодирования (по-умолчанию 7)
  uint8_t syncWord = 0x18;        //Слово синхронизации (по-умолчанию 0х18). ВНИМАНИЕ! Значение 0x34 зарезервировано для сетей LoRaWAN и нежелательно для использования
  int8_t outputPower = 10;        //Установить выходную мощность (по-умолчанию 10 дБм) (допустимый диапазон -3 - 17 дБм) ПРИМЕЧАНИЕ: значение 20 дБм позволяет работать на большой мощности, но передача рабочий цикл НЕ ДОЛЖЕН ПРЕВЫШАТЬ 1
  uint8_t currentLimit = 100;      //Установить предел защиты по току (по-умолчанию до 80 мА) (допустимый диапазон 45 - 240 мА) ПРИМЕЧАНИЕ: установить значение 0 для отключения защиты от перегрузки по току
  int16_t preambleLength = 8;    //Установить длину преамбулы (по-умолчанию в 8 символов) (допустимый диапазон 6 - 65535)
  uint8_t gain = 0;               //Установить регулировку усилителя (по-умолчанию 1) (допустимый диапазон 1 - 6, где 1 - максимальный рост) ПРИМЕЧАНИЕ: установить значение 0, чтобы включить автоматическую регулировку усиления оставьте в 0, если вы не знаете, что вы делаете


};



// Створюємо екземпляр структури LORA_CONFIGURATION, яка містить параметри конфігурації радіо-модуля
LORA_CONFIGURATION config_radio;


/**
 * @brief Функція, яка задає параметри конфігурації радіо-модуля.
 * 
 */
void setRadioMode()
{
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

//Флаг, який вказує, що операція завершена
volatile bool operationDone = false;


/**
 * @brief // Ця функція встановлює флаг, що вказує на те, що операція завершена
 * Функція викликається в ISR (перериванні) після завершення операції з радіо-модулем
 * Це дозволяє основному циклу програми перевіряти цей флаг і виконувати подальші дії
 * Наприклад, після відправки або отримання пакету даних
 * 
 * @return IRAM_ATTR 
 */
IRAM_ATTR void flag_operationDone(void) {
  // Встановлюємо флаг, що операція завершена
  operationDone = true;
}




/**
 * @brief Функція для очікування, поки радіо не буде вільним
 * 
 * @return true Якщо радіо вільне і готове до роботи
 * @return false Якщо таймаут очікування вичерпано
 */
bool ICACHE_RAM_ATTR WaitOnBusy()
{
    constexpr uint32_t wtimeoutUS = 1000U;
    uint32_t startTime = 0;
    #ifdef DEBUG_PRINT
      Serial.println("");
      Serial.print(F("WaitOnBusy.....................  "));
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
* @brief Функція для виявлення LoRa-пакету на заданому радіо
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
* @brief Функція для виявлення LoRa преамбули на заданому радіо
* 
*/
void detectedPreamble()
{
  int state;
  // Скануємо канал для виявлення LoRa преамбули
  state = radio.scanChannel();
  if (state == RADIOLIB_LORA_DETECTED)
  {
    // LoRa преамбула була виявлена
    #ifdef DEBUG_PRINT
      Serial.println(F("Preamble detected!"));
    #endif

  } else {
    // LoRa преамбула не була виявлена
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
void print_to_terminal_radio_state(String state, String &RADIO_NAME) __attribute__ ((weak));








/**
 * @brief Функція для виведення стану радіо на дисплей
 * 
 * @param state Строка состоянія радіо
 */
void displayPrintState(int16_t x, int16_t y, String state) __attribute__ ((weak));






/**
 * @brief Функція для встановлення налаштувань радіо-модуля.
 * 
 */
#ifdef SX1278_MODEM
  #define RADIO_CLASS_NAME SX1278
  // Функция для встановлення налаштувань радіо-модуля SX1278
  // Ця функція повинна бути реалізована в іншому файлі, якщо вона не переопределена
  // в іншому файлі, то буде використовуватися ця реалізація
  void radio_setSettings(RADIO_CLASS_NAME radio, LORA_CONFIGURATION config_radio) __attribute__ ((weak));
#elif defined(SX1262_MODEM)
  #define RADIO_CLASS_NAME SX1262
  // Функція для встановлення налаштувань радіо-модуля SX1262
  // Ця функція повинна бути реалізована в іншому файлі, якщо вона не переопределена
  // в іншому файлі, то буде використовуватися ця реалізація
void radio_setSettings(RADIO_CLASS_NAME radio, LORA_CONFIGURATION config_radio) __attribute__ ((weak));
#endif








/**
* @brief Функція для друку стану ініціалізації радіо на дисплеї
* 
* @param STATE 
*/
void printRadioBeginResult(int &STATE)
{
  int x,y = 5;
  
  if (STATE == RADIOLIB_ERR_NONE) {
    #ifdef DEBUG_PRINT
      
      print_to_terminal_radio_state(RADIO_NAME, F("INIT_GOOD"));
    #endif
      displayPrintState(x, y, RADIO_NAME, F("INIT_GOOD"));
  } else {

    String str = "ERROR " + (String)STATE;
    #ifdef DEBUG_PRINT
      print_to_terminal_radio_state(RADIO_NAME, F("INIT_GOOD"));
    #endif
      displayPrintState(x, y, RADIO_NAME);
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
 * @brief Налаштування
 * заданными в файле "settings.h".
 */
void radioBeginAll()
{
  // Встановлюємо контакт NSS_PIN в HIGH, щоб не було конфліктів з іншими SPI-пристроями
  pinMode(NSS_PIN, OUTPUT);
  #ifdef DEBUG_PRINT
    Serial.println(" ");
    Serial.println(F(""));
    Serial.println(" ");
    Serial.println(F("RADIO INIT....."));
  #endif
  // Ініціалізуємо радіо
  int state = radio.begin();

  printRadioBeginResult(state);

  WaitOnBusy();
 
  #ifdef DEBUG_PRINT
    delay(1000);
  #endif
}





/**
* @brief Функция установки настроек передатчика
* 
* @param radio - экземпляр класса передатчика
* @param config - экземпляр структуры для настройки модуля
*/
void radio_setSettings(RADIO_CLASS_NAME radio, LORA_CONFIGURATION config_radio)
{
  #ifdef DEBUG_PRINT
    Serial.print(TABLE_LEFT);
    Serial.print(F("SET SETTINGTH OF RADIO "));
    Serial.print(RADIO_NAME);
    Serial.println(TABLE_RIGHT);
  #endif
  
  // Устанавливаем необходимую нам частоту работы трансивера
  if (radio.setFrequency(config_radio.frequency) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Selected frequency is invalid for this module!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set frequency = "));
  Serial.println(config_radio.frequency);
  #endif


  // установить полосу пропускания до 250 кГц
  if (radio.setBandwidth(config_radio.bandwidth) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set bandWidth = "));
  Serial.println(config_radio.bandwidth);
  #endif

  // коэффициент расширения 
  if (radio.setSpreadingFactor(config_radio.spreadingFactor) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set spreadingFactor = "));
  Serial.println(config_radio.spreadingFactor);
  #endif

  // установить скорость кодирования
  if (radio.setCodingRate(config_radio.codingRate) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Selected coding rate is invalid for this module!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set codingRate = "));
  Serial.println(config_radio.codingRate);
  #endif

  // Устанавливаем слово синхронизации
  if (radio.setSyncWord(config_radio.syncWord) != RADIOLIB_ERR_NONE) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Unable to set sync word!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set syncWord = "));
  Serial.println(config_radio.syncWord);
  #endif

  // Устанавливаем выходную мощность трансивера
  if (radio.setOutputPower(config_radio.outputPower) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Selected output power is invalid for this module!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set setOutputPower = "));
  Serial.println(config_radio.outputPower); 
  #endif

  // установить длину преамбулы (допустимый диапазон 6 - 65535)
  if (radio.setPreambleLength(config_radio.preambleLength) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    #ifdef DEBUG_PRINT
    Serial.println(F("Selected preamble length is invalid for this module!"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set preambleLength = "));
  Serial.println(config_radio.preambleLength);

  

  Serial.println(F("All settings successfully changed!"));

  Serial.print(TABLE_LEFT);
  Serial.print(F("END SETTINGTH OF RADIO "));
  Serial.print(RADIO_NAME);
  Serial.println(TABLE_RIGHT);
  Serial.println(SPACE);
  #endif
}







/**
* @brief Функция отправляет данные, выводит на экран информацию об отправке,
* выводит информацию об отправке в сериал-порт
* 
* @param transmit_str - строка для передачи
*/
void transmit_and_print_data(String &transmit_str)
{
  display.clearDisplay();

  //Посылаем пакет
  state = radio.startTransmit(transmit_str);
  //Ждём завершения передачи
  WaitOnBusy();
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResultTX(state, transmit_str);
  
}









void RadioStart()
{
  #ifdef RECEIVER  //Якщо визначена робота модуля як приймача

    //Устанавливаем функцию, которая будет вызываться при получении пакета данных
    radio1.setPacketReceivedAction(setFlag_1);
    #ifdef RADIO_2
    radio2.setPacketReceivedAction(setFlag_2);
    #endif

    #ifdef DEBUG_PRINT
    //Начинаем слушать есть ли пакеты
    Serial.print(TABLE_LEFT);
    Serial.print(F("[SX1278] Starting to listen RX_1 "));
    Serial.println(TABLE_RIGHT);
    #endif

    state_1 = radio1.startReceive();
    if (state_1 == RADIOLIB_ERR_NONE) {
      #ifdef DEBUG_PRINT
      Serial.println(F("success!"));
      Serial.println(SPACE);
      #endif
      digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета
    } else {
      #ifdef DEBUG_PRINT
      Serial.print(F("failed, code: "));
      Serial.println(state_1);
      #endif
      while (true);
    }

    #ifdef RADIO_2
    #ifdef DEBUG_PRINT
    //Начинаем слушать есть ли пакеты
    Serial.print(TABLE_LEFT);
    Serial.print(F("[SX1278] Starting to listen RX_2 "));
    Serial.println(TABLE_RIGHT);
    #endif

    state_2 = radio2.startReceive();
    if (state_2 == RADIOLIB_ERR_NONE) {
      #ifdef DEBUG_PRINT
      Serial.println(F("success!"));
      Serial.println(SPACE);
      #endif
      digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета
    } else {
      #ifdef DEBUG_PRINT
      Serial.print(F("failed, code: "));
      Serial.println(state_2);
      #endif
      while (true);
    }
    #endif

    String str;
    receive_and_print_data(str);

  #endif


  #ifdef TRANSMITTER  //Если определена работа модуля как передатчика

    //Устанавливаем функцию, которая будет вызываться при отправке пакета данных модемом №1
    radio.setPacketSentAction(flag_operationDone);
    
    #ifdef DEBUG_PRINT
      //Начинаем передачу пакетов
      Serial.print(TABLE_LEFT);
      Serial.print(F("SENDING FIRST PACKET"));
      Serial.println(TABLE_RIGHT);
    #endif

    String str = F("RADIO START!");

    transmit_and_print_data(str);
    
    digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета

    #ifdef DEBUG_PRINT
      delay(1000);
    #endif

  #endif
  
  #ifdef DEBUG_PRINT
    Serial.println(F("**************************************"));
  #endif

  digitalWrite(LED_PIN, HIGH);      //Вимикаємо світлодіод, сигналізація про передачу/прийом пакета

  //Если мощность усилителя передатчика больше 200 милливат (вы можете установить своё значение),
  // и вентилятор охлаждения не включен, то включаем вентилятор охлаждения
  if(config_radio.outputPower > 1)
  {
    //и включаем его
    digitalWrite(FAN, HIGH);
  }

}













#endif;