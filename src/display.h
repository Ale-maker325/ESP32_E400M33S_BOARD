#ifndef ______DISPLAY__DATA_______
#define ______DISPLAY__DATA_______

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <radio.h>
#include <settings.h>


#define SCREEN_WIDTH 128              // Ширина дисплея в пикселах
#define SCREEN_HEIGHT 64              // Высота дисплея в пикселах
#define OLED_RESET    -1              // Пин сброса # ( -1 если для сброса используется стандартный пин ардуино)
#define SCREEN_ADDRESS 0x3C           // Стандартный адрес I2C для дисплея (в моём случае такой адрес дал I2C-сканнер)

TwoWire display_wire = TwoWire(0);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_wire, OLED_RESET); //Создаём объект дисплея







/**
* @brief Функция инициализации дисплея 
* 
*/
void displayInit()
{

  //Инициализируем дисплей
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {// SSD1306_SWITCHCAPVCC = напряжение дисплея от 3.3V
    #ifdef DEBUG_PRINT
      Serial.println(F("SSD1306 ERROR INIT"));
    #endif
    for(;;); // Don't proceed, loop forever
  }

  // Показываем содержимое буфера дисплея, созданное по-умолчанию
  // библиотека по-умолчанию использует эмблему Adafruit.
  //display.display();
  //delay(1000); // Pause for 2 seconds

  //Очищаем буффер дисплея
  display.clearDisplay();

  display.setTextSize(1);                 // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);    // Draw white text
  display.cp437(true);                    // Use full 256 char 'Code Page 437' font

  display.setCursor(20, 5);
  display.print(F("DISPLAY INIT"));
  display.display();
  delay(2000);
  display.clearDisplay();
  
}













#ifdef DEBUG_PRINT

/**
 * @brief Функция, выводит в сериал монитор сообщения
 * 
 * @param RadioName - имя радио
 * @param state     - текущее состояние радио - это просто информационная сторока
 */
void print_to_terminal_radio_state(String &RadioName, String state)
{
  String str = RadioName + " : " + state;
  Serial.println(str);
}




void (*print_to_terminal)(String &RadioName, String state) = print_to_terminal_radio_state;
// void print_to_terminal_radio_state(String &RadioName, String state)  __attribute__ ((weak));
// int recv_callback()  {return 0;}


#endif








/**
 * @brief Функция для вывода на экран дисплея состояния
 * работы текущего радио
 * 
 * @param x - координата экрана по Х
 * @param y - координата экрана по У
 * @param RadioName - наименование радио
 * @param state - текущее состояние в виде строки STRING
 */
void displayPrintState(int16_t x, int16_t y, String &RadioName, String state)
{
  String str = RadioName + " : " + state;
  display.setCursor(x, y);
  display.print(str);
  display.display();
}






/**
 * @brief Функция, которая обеспечивает вывод текущего состояния 
 * приёмопередатчика в сериал-порт (если он задан) и на дисплей
 * 
 * @param state         - текущее состояние, полученное от передатчика при его работе
 * @param transmit_str  - строка для передачи
 * @param radioNumber   - номер передатчика (так как их может быть два)
 */
void printStateResultTX(int &state, String &transmit_str, Radio_Number radioNumber)
{
  String RADIO_NAME;
  int x, y;
  switch (radioNumber)
  {
  case Radio_1: 
    RADIO_NAME = RADIO_1_NAME;
    x = 5;
    y = 5;
    break;
  case Radio_2: 
    RADIO_NAME = RADIO_2_NAME;
    x = 5;
    y = 20;
    break;
  
  default: 
    RADIO_NAME = "RADIO_NONAME!!!";
    x = 5;
    y = 5;
    break;
  }

  //Если передача успешна, выводим сообщение в сериал-монитор
  if (state == RADIOLIB_ERR_NONE) {
    //Выводим сообщение об успешной передаче
    #ifdef DEBUG_PRINT
    print_to_terminal_radio_state(RADIO_NAME, F("SEND PACKET"));
    print_to_terminal_radio_state(RADIO_NAME, F("TRANSMITT SUCCES!"));
    #endif
    displayPrintState(x, y, RADIO_NAME, transmit_str);

    #ifdef DEBUG_PRINT              
    //Выводим в сериал данные отправленного пакета
    Serial.print(F("Data:\t\t"));
    Serial.println(transmit_str);
    #endif

    digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета

    
          
  } else {
    //Если были проблемы при передаче, сообщаем об этом
    
    String str = (String)state_1;
    #ifdef DEBUG_PRINT
    Serial.print(F("transmission failed, "));
    
    print_to_terminal_radio_state(RADIO_NAME, str);
    #endif

    displayPrintState(x, y, RADIO_NAME, str);
  
  }
  
}










/**
 * @brief Функция, которая обеспечивает вывод текущего состояния 
 * приёмопередатчика в сериал-порт (если он задан) и на дисплей
 * 
 * @param state         - текущее состояние, полученное от передатчика при его работе
 * @param read_str  - строка для передачи
 * @param radioNumber   - номер передатчика (так как их может быть два)
 */
void printStateResult_RX(int &state, String &read_str, Radio_Number radioNumber)
{
  String RADIO_NAME;
  int x, y;
  switch (radioNumber)
  {
  case Radio_1: 
    RADIO_NAME = RADIO_1_NAME;
    x = 5;
    y = 5;
    break;
  case Radio_2: 
    RADIO_NAME = RADIO_2_NAME;
    x = 5;
    y = 20;
    break;
  
  default: 
    RADIO_NAME = "RADIO_NONAME!!!";
    x = 5;
    y = 5;
    break;
  }

  //Если приём успешен, выводим сообщение в сериал-монитор
  if (state == RADIOLIB_ERR_NONE) {
    //Выводим сообщение об успешном приёме
    #ifdef DEBUG_PRINT
    print_to_terminal_radio_state(RADIO_NAME, F("RECEIVE PACKET"));
    #endif
    displayPrintState(x, y, RADIO_NAME, read_str);

    #ifdef DEBUG_PRINT              
    //Выводим в сериал данные отправленного пакета
    Serial.print(F("Data:\t\t"));
    Serial.println(read_str);
    #endif

    digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета

    
          
  } else {
    //Если были проблемы при передаче, сообщаем об этом
    
    String str = (String)state_1;
    #ifdef DEBUG_PRINT
    Serial.print(F("receive failed, "));
    
    print_to_terminal_radio_state(RADIO_NAME, str);
    #endif

    displayPrintState(x, y, RADIO_NAME, str);
  
  }
  
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
  state_1 = radio1.startTransmit(transmit_str);
  //Ждём завершения передачи
  WaitOnBusy(Radio_1);
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResultTX(state_1, transmit_str, Radio_1);
  //Оканчиваем передачу первым передатчиком
  //state_1 = radio1.finishTransmit();
  //state_1 = radio1.startReceive();

  #ifdef RADIO_2
  //Посылаем пакет
  state_2 = radio2.startTransmit(transmit_str);
  //Ждём завершения передачи
  WaitOnBusy(Radio_2);
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResultTX(state_2, transmit_str, Radio_2);
  
  //Оканчиваем передачу вторым передатчиком
  //state_2 = radio2.finishTransmit();
  //state_2 = radio2.startReceive();

  #endif  
}













/**
* @brief Функция отправляет данные, выводит на экран информацию об отправке,
* выводит информацию об отправке в сериал-порт
* 
* @param transmit_str - строка для передачи
*/
void receive_and_print_data(String &receive_str)
{
  display.clearDisplay();
  state_1 = radio1.readData(receive_str);
  //Ждём завершения передачи
  WaitOnBusy(Radio_1);
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResult_RX(state_1, receive_str, Radio_1);
  

  #ifdef RADIO_2
  //Посылаем пакет
  state_2 = radio2.readData(receive_str);
  //Ждём завершения передачи
  WaitOnBusy(Radio_2);
  //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
  printStateResult_RX(state_2, receive_str, Radio_2);
  WaitOnBusy(Radio_2);
  

  #endif  
}



























#ifdef DEBUG_PRINT

/**
 * @brief Функция, выводящая значение прошивки радио модуля
 * 
 */
void printVersions() {
  LR11x0VersionInfo_t version1;
  LR11x0VersionInfo_t version2;

  Serial.println(F("****************** [LR1110] Reading firmware versions ******************** "));

  int16_t state1 = radio1.getVersionInfo(&version1);
  #ifdef RADIO_2
  int16_t state2 = radio2.getVersionInfo(&version2);
  #endif

  if (state1 == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    Serial.print(F("[LR1110] Device1: "));
    Serial.println(version1.device);

    Serial.print(F("[LR1110] Base firmware1: "));
    Serial.print(version1.fwMajor);
    Serial.print('.');
    Serial.println(version1.fwMinor);

    Serial.print(F("[LR1110] WiFi firmware1: "));
    Serial.print(version1.fwMajorWiFi);
    Serial.print('.');
    Serial.println(version1.fwMinorWiFi);

    Serial.print(F("[LR1110] GNSS firmware1: "));
    Serial.print(version1.fwGNSS);
    Serial.print('.');
    Serial.println(version1.almanacGNSS);

  } else {
    Serial.print(F("failed_radio №1:, code "));
    Serial.println(state1);
    while (true) { delay(10); }
  }

  #ifdef RADIO_2
    if (state2 == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    Serial.print(F("[LR1110] Device1: "));
    Serial.println(version2.device);

    Serial.print(F("[LR1110] Base firmware1: "));
    Serial.print(version2.fwMajor);
    Serial.print('.');
    Serial.println(version2.fwMinor);

    Serial.print(F("[LR1110] WiFi firmware1: "));
    Serial.print(version2.fwMajorWiFi);
    Serial.print('.');
    Serial.println(version2.fwMinorWiFi);

    Serial.print(F("[LR1110] GNSS firmware1: "));
    Serial.print(version2.fwGNSS);
    Serial.print('.');
    Serial.println(version2.almanacGNSS);

  } else {
    Serial.print(F("failed_radio №2:, code "));
    Serial.println(state2);
    while (true) { delay(10); }
  }
  #endif

  Serial.println(F("****************** [LR1110] END firmware versions ******************** "));
  
}


#endif












#endif