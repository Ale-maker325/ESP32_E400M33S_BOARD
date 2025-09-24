#ifndef ________RADIO_LR1121________
#define ________RADIO_LR1121________


#include <Arduino.h>
#include <SPI.h>
#include <display.h>
#include <RadioLib.h>
#include <stdint.h>
#include <settings.h>




static int state = RADIOLIB_ERR_NONE; // Переменная, хранящая код состояния передачи/приёма

SPIClass SPI_MODEM;

#ifdef SX1278_MODEM
  SX1278 radio = new Module(NSS_PIN, BUSY_PIN, NRST_PIN, DIO1_PIN); //Инициализируем экземпляр радио
  #define RADIO_CLASS_NAME SX1278
#endif
#ifdef SX1268_MODEM
  SX1268 radio = new Module(NSS_PIN, DIO1_PIN, NRST_PIN, BUSY_PIN); //Инициализируем экземпляр радио
  #define RADIO_CLASS_NAME SX1268
#endif

#ifdef RECEIVER
  static String receive_str = " ";  // Строка для зберігання отриманих даних по радіо
#endif

// Add this declaration if 'display' is defined elsewhere (e.g., in display.h or another source file)
extern Adafruit_SSD1306 display; // Replace 'DisplayClass' with the actual class type of 'display'

void print_to_terminal_radio_state(String state, String RADIO_NAME);
void displayPrintState(int16_t x, int16_t y, String RadioName, String state);
void printStateResultTX(int &state, String &transmit_str);
void printStateResult_RX(int &state, String &read_str);


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
*  парамерти з налаштувань, які задані в файлі "settings.h". Тут ніякі параметри змінювати не потрібно,
*  всі налаштування будуть задані в файлі "settings.h", та ініційовані в функції setRadioMode().
*/
struct LORA_CONFIGURATION
{
  float frequency = 0.0;        //Частота работы передатчика
  float bandwidth = 0.0;        //Полоса пропускания (по-умолчанию 125 килогерц)
  uint8_t spreadingFactor = 0;   //Коэффициент расширения (по-умолчанию 9)
  uint8_t codingRate = 0;         //Скорость кодирования (по-умолчанию 7)
  uint8_t syncWord = 0x00;        //Слово синхронизации (по-умолчанию 0х18). ВНИМАНИЕ! Значение 0x34 зарезервировано для сетей LoRaWAN и нежелательно для использования
  int8_t outputPower = 0;        //Установить выходную мощность (по-умолчанию 15 дБм) (допустимый диапазон -3 - 17 дБм) ПРИМЕЧАНИЕ: значение 20 дБм позволяет работать на большой мощности, но передача рабочий цикл НЕ ДОЛЖЕН ПРЕВЫШАТЬ 1
  uint8_t currentLimit = 0;      //Установить предел защиты по току (по-умолчанию до 80 мА) (допустимый диапазон 45 - 240 мА) ПРИМЕЧАНИЕ: установить значение 0 для отключения защиты от перегрузки по току
  int16_t preambleLength = 0;    //Установить длину преамбулы (по-умолчанию в 8 символов) (допустимый диапазон 6 - 65535)
  #ifdef SX1278_MODEM
    uint8_t gain = 0;               //Установить регулировку усилителя (по-умолчанию 1) (допустимый диапазон 1 - 6, где 1 - максимальный рост) ПРИМЕЧАНИЕ: установить значение 0, чтобы включить автоматическую регулировку усиления оставьте в 0, если вы не знаете, что вы делаете
  #endif
  #ifdef SX1268_MODEM
    bool CRC = false;             //Установить контрольную сумму CRC (по-умолчанию отключено)
    float TCXO_V = 0;             //Установить напряжение TCXO. Примечание: установите значение 0, чтобы отключить TCXO
    bool DIO2_RF_SW = false;      //Установить DIO2 как RF Switch (по-умолчанию отключено)
  #endif

};



// Створюємо екземпляр структури LORA_CONFIGURATION, яка містить параметри конфігурації радіо-модуля
LORA_CONFIGURATION config_radio;


/**
 * @brief Функція, яка задає параметри конфігурації налаштувань радіо-модуля.
 * 
 */
void setRadioMode()
{
  #ifdef SX1278_MODEM
    config_radio.frequency = RADIO_FREQ;
    config_radio.bandwidth = RADIO_BANDWIDTH;
    config_radio.spreadingFactor = RADIO_SPREAD_FACTOR;
    config_radio.codingRate = RADIO_CODING_RATE;
    config_radio.syncWord = RADIO_SYNC_WORD;
    config_radio.outputPower = RADIO_OUTPUT_POWER;
    config_radio.currentLimit = RADIO_CURRENT_LIMIT;
    config_radio.preambleLength = RADIO_PREAMBLE_LENGTH;
    config_radio.gain = RADIO_GAIN;
  #endif
  #ifdef SX1268_MODEM
    config_radio.frequency = RADIO_FREQ;
    config_radio.bandwidth = RADIO_BANDWIDTH;
    config_radio.spreadingFactor = RADIO_SPREAD_FACTOR;
    config_radio.codingRate = RADIO_CODING_RATE;
    config_radio.syncWord = RADIO_SYNC_WORD;
    config_radio.outputPower = RADIO_OUTPUT_POWER;
    config_radio.currentLimit = RADIO_CURRENT_LIMIT;
    config_radio.preambleLength = RADIO_PREAMBLE_LENGTH;
    config_radio.CRC = RADIO_CRC;
    config_radio.TCXO_V = RADIO_TCXO_V;
    config_radio.DIO2_RF_SW = RADIO_DIO2_AS_RF_SWITCH;
  #endif
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
ICACHE_RAM_ATTR void flag_operationDone(void) {
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
      Serial.println(F("WaitOnBusy(): Ожидаем освобождения радио BUSY_PIN"));
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
* @brief Функція для друку стану ініціалізації радіо на дисплеї
* 
* @param STATE 
*/
void printRadioBeginResult(int &STATE)
{
  int x = 5;
  int y = 5;
  String good = F("INIT_GOOD");
  
  if (STATE == RADIOLIB_ERR_NONE) {
    #ifdef DEBUG_PRINT
      // Якщо ініціалізація пройшла успішно, виводимо повідомлення на термінал
      print_to_terminal_radio_state(RADIO_NAME, good);
      #ifdef DEBUG_PRINT
        Serial.print(TABLE_LEFT);
        Serial.print(F("RADIO INIT SUCCESS"));
        Serial.println(TABLE_RIGHT);
        Serial.println(SPACE);
        Serial.println(SPACE);
        Serial.println(SPACE);
      #endif
    #endif
      // Якщо ініціалізація пройшла успішно, виводимо повідомлення на дисплей
      displayPrintState(x, y, RADIO_NAME, good);
      
  } else {

    String str = "ERROR " + (String)STATE;
    #ifdef DEBUG_PRINT
      print_to_terminal_radio_state(RADIO_NAME, str);
    #endif
      displayPrintState(x, y, RADIO_NAME, str);
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
  pinMode(NSS_PIN, OUTPUT);
  digitalWrite(NSS_PIN, LOW);
}



/**
 * @brief Налаштування
 * заданными в файле "settings.h".
 */
void radioBeginAll()
{
  selectRadio();
  
  #ifdef DEBUG_PRINT
    Serial.println("");
    Serial.println(F("START RADIO INIT....."));
  #endif
  // Ініціалізуємо радіо
  int state = radio.begin();

  // Якщо ініціалізація пройшла успішно, то виводимо повідомлення на термінал та на дисплей
  printRadioBeginResult(state);

  //#ifdef SX1278_MODEM
    //Зачекаємо, поки радіо не буде готове до роботи
    //WaitOnBusy();
  //#endif
 
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR FREQ"));
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR BADWIDTH"));
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR SF"));
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR CR"));
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR SYNC"));
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR POWER"));
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
      display.clearDisplay();
      displayPrintState(5, 5, RADIO_NAME, F("ERROR PREAMBLE"));
    #endif
    while (true);
  }
  #ifdef DEBUG_PRINT
  Serial.print(F("Set preambleLength = "));
  Serial.println(config_radio.preambleLength);

  #ifdef SX1268_MODEM
    // встановити контрольну суму CRC
    if (radio.setCRC(config_radio.CRC) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
      #ifdef DEBUG_PRINT
        Serial.println(F("Selected CRC is invalid for this module!"));
        display.clearDisplay();
        displayPrintState(5, 5, RADIO_NAME, F("ERROR CRC"));
      #endif
      while (true);
    }
    #ifdef DEBUG_PRINT
      Serial.print(F("Set CRC = "));
      Serial.println(config_radio.CRC);
    #endif

    // встановити напругу TCXO
    if (radio.setTCXO(config_radio.TCXO_V) == RADIOLIB_ERR_INVALID_TCXO_VOLTAGE) {
      #ifdef DEBUG_PRINT
        Serial.println(F("Selected TCXO voltage is invalid for this module!"));
        display.clearDisplay();
        displayPrintState(5, 5, RADIO_NAME, F("ERROR TCXO"));
      #endif
      while (true);
    }
    #ifdef DEBUG_PRINT
      Serial.print(F("Set TCXO_V = "));
      Serial.println(config_radio.TCXO_V);    
    #endif
    
    // встановити DIO2 як RF Switch
    if (radio.setDio2AsRfSwitch(config_radio.DIO2_RF_SW) != RADIOLIB_ERR_NONE) {
      #ifdef DEBUG_PRINT
        Serial.println(F("Unable to set DIO2 as RF Switch!"));
        display.clearDisplay();
        displayPrintState(5, 5, RADIO_NAME, F("ERROR DIO2"));
      #endif
      while (true);
    }
    #ifdef DEBUG_PRINT
      Serial.print(F("Set DIO2_RF_SW = "));
      Serial.println(config_radio.DIO2_RF_SW);
    #endif

  #endif
  

  Serial.println(F("All settings successfully set!"));

  Serial.print(TABLE_LEFT);
  Serial.print(F("END SETTINGTH OF RADIO "));
  Serial.print(RADIO_NAME);
  Serial.println(TABLE_RIGHT);
  Serial.println(SPACE);
  #endif
}







/**
 * @brief Функция отправляет данные по радио и выводит результат передачи на экран и в сериал-порт
 * 
 * @param transmit_str - строка с данными для передачи
 */
void transmit_and_print_data(String &transmit_str)
{
  
  #ifdef DEBUG_PRINT
    uint16_t irqFlags = radio.getIrqFlags();
    Serial.print(F("IrqFlags: 0x"));
    Serial.println(irqFlags, HEX);
    Serial.print("BUSY перед началом передачи: ");
    Serial.println(digitalRead(BUSY_PIN));
    Serial.print("transmit_and_print_data: operationDone = ");
    if(operationDone == true) Serial.println("true");
    else  Serial.println("false");  
  #endif


  //Посылаем пакет
  state = radio.startTransmit(transmit_str);

  #ifdef DEBUG_PRINT
    irqFlags = radio.getIrqFlags();
    Serial.print(F("IrqFlags: 0x"));
    Serial.println(irqFlags, HEX);
    Serial.print("BUSY после начала передачи: ");
    Serial.println(digitalRead(BUSY_PIN));
    Serial.print("transmit_and_print_data: operationDone = ");
    if(operationDone == true) Serial.println("true");
    else  Serial.println("false");
  #endif


  //Ждём завершения передачи
  // WaitOnBusy();
  
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResultTX(state, transmit_str);
  radio.finishTransmit();

  
}





/**
* @brief Функция берёт данные, которые были получены по вызову метода startReceive(), 
* выводит на экран информацию об отправке, выводит информацию об отправке в сериал-порт
* 
* @param transmit_str
*/
void receive_and_print_data(String &receive_str)
{
  //Читаем данные, которые были получены по вызову метода startReceive() 
  int state_read = radio.readData(receive_str);
  // you can also read received data as byte array
  /*
  byte byteArr[8];
  int numBytes = radio.getPacketLength();
  int state = radio.readData(byteArr, numBytes);
  */
  
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResult_RX(state_read, receive_str);
}



/**
 * @brief Ініціалізація радіо-модуля та запуск прийому даних або передачі даних.
 * Ця функція виконує ініціалізацію радіо-модуля, встановлює його налаштування та
 * запускає прийом або передачу даних в залежності від того, чи визначено 
 * макрос RECEIVER або TRANSMITTER.
 * 
 */
void RadioStart()
{
  #ifdef RECEIVER  //Якщо визначена робота модуля як приймача

    //Встановлюємо функцію, яка буде викликатись при отриманні пакета даних модемом
    radio.setPacketReceivedAction(flag_operationDone);
    
    #ifdef DEBUG_PRINT
      //Починаємо прийом даних
      Serial.print(TABLE_LEFT);
      Serial.print(F("STARTING RECEIVE PACKET"));
      Serial.println(TABLE_RIGHT);
      Serial.println(SPACE);
    #endif

    state = radio.startReceive();
    // if needed, 'listen' mode can be disabled by calling
    // any of the following methods:
    //
    // radio.standby()
    // radio.sleep()
    // radio.transmit();
    // radio.receive();
    // radio.scanChannel();

    if (state == RADIOLIB_ERR_NONE) {
      String state_str = F("RECEIVE STARTED!");
      #ifdef DEBUG_PRINT
        Serial.println(state_str);
        Serial.println(SPACE);
      #endif
      printStateResult_RX(state, state_str);
      #ifdef DEBUG_PRINT
        delay(500);
      #endif
      
    } else {
      String state_str = F("FAILED RECEIVE, CODE: ");
      #ifdef DEBUG_PRINT
        Serial.print(state_str);
        Serial.println(state);
      #endif
      printStateResult_RX(state, state_str);
      while (true);
    }

    receive_and_print_data(receive_str);

    digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета

    #ifdef DEBUG_PRINT
      delay(500);
    #endif

  #endif


  #ifdef TRANSMITTER  //Если определена работа модуля как передатчика

    #ifdef DEBUG_PRINT
      //Начинаем передачу первого проверочного пакета
      Serial.println(" ");
      Serial.println("===============================================================================================");
      Serial.println(F("RadioStart(): начинаем передачу первого стартового пакета"));
    #endif

    String str = F("RADIO START!");

    transmit_and_print_data(str);

    #ifdef DEBUG_PRINT
      //Закончили передачу первого проверочного пакета
      Serial.println(F("RadioStart(): передача стартового пакета завершена"));
      Serial.println("===============================================================================================");
      Serial.println(" ");
    #endif
    
    digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета

    #ifdef DEBUG_PRINT
      delay(1000);
    #endif

  #endif
  
  digitalWrite(LED_PIN, HIGH);      //Вимикаємо світлодіод, сигналізація про передачу/прийом пакета

  #ifdef TRANSMITTER
    #ifdef FAN_IS_ACTIVE
    //Якщо вентилятор активний, то перевіряємо, чи потрібно його вмикати
    //Вентилятор буде вмикатись, якщо потужність передавача більша або дорівнює FAN_ON_POWER
      if(config_radio.outputPower >= FAN_ON_POWER)
      {
        //Якщо потужність передавача більша або дорівнює FAN_ON_POWER, то вмикаємо вентилятор
        digitalWrite(FAN, HIGH);
      } else {
        //Інакше вимикаємо вентилятор
        digitalWrite(FAN, LOW);
      }
    #endif
  #endif

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






#endif