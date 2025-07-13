#ifndef ______DISPLAY__DATA_______
#define ______DISPLAY__DATA_______

#include <Arduino.h>
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
* @brief Функція ініциалізації дисплея
* 
*/
void displayInit()
{

  //Ініціалізуємо I2C шину для дисплея
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {// SSD1306_SWITCHCAPVCC = напряжение дисплея от 3.3V
    #ifdef DEBUG_PRINT
      Serial.println(F("SSD1306 ERROR INIT"));
    #endif
    for(;;); // Якщо дисплей не ініціалізувався, то зацикливаемся
  }

  #ifdef DEBUG_PRINT
    Serial.println(F("DISPLAY INIT SUCCESS!"));
    Serial.println("");
  #endif

  //Очищуємо буфер дисплея
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
  void print_to_terminal_radio_state(String RadioName, String state)
  {
    String str = RadioName + "RECEIVED DATA " + state;
    Serial.println(str);
  }

#endif





static bool buttons_flag = false; //Флаг для відстеження натискання кнопок
static int16_t old_button_string_length = 0; // Переменная для хранения длины предыдущего состояния вывода по нажатию кнопки
static int16_t old_radio_string_length = 0; // Переменная для хранения длины предыдущего состояния вывода радио

/**
 * @brief Функция для вывода на экран дисплея состояния
 * работы текущего радио
 * 
 * @param x - координата экрана по Х
 * @param y - координата экрана по У
 * @param RadioName - наименование радио
 * @param state - текущее состояние в виде строки STRING
 */
void displayPrintState(int16_t x, int16_t y, String RadioName, String state)
{
  String str = RadioName + state; //формируем строку для вывода на экран
  //Если строка длиннее 20 символов, то обрезаем её
  int16_t str_length = str.length();
  if (str_length > 20)  str = str.substring(0, 20);

  if(!buttons_flag) //Если флаг кнопок не установлен, то это значит, что мы выводим состояние радио
  {
    //Устанавливаем курсор на нужные координаты
    display.setTextSize(1);                 // Normal 1:1 pixel scale
    display.fillRect(x, y, 128, 10, SSD1306_BLACK); // Очищаем область для текста с начала координат и до конца дисплея
    //display.display();
    display.setCursor(x, y);
    display.print(str);
    display.display();
  }

  //Если эта функция была вызвана для отрисовки вывода кнопок
  #ifdef BUTTONS
    if(buttons_flag)
    {
      display.fillRect(x, y, 128, 10, SSD1306_BLACK); // Очищаем область для текста
      display.display();
      display.setCursor(x, y);
      display.print(str);
      display.display();
      buttons_flag = false; //Сбрасываем флаг кнопок
    }
  #endif
  
  
}






/**
 * @brief Функция, которая обеспечивает вывод текущего состояния 
 * приёмопередатчика в сериал-порт (если он задан) и на дисплей
 * 
 * @param state         - текущее состояние, полученное от передатчика при его работе
 * @param transmit_str  - строка для передачи
 */
void printStateResultTX(int &state, String &transmit_str)
{
  int x, y;
  x = 5;
  y = 5;
  
  //Если передача успешна, выводим сообщение в сериал-монитор
  if (state == RADIOLIB_ERR_NONE) {
    //Выводим сообщение об успешной передаче
    #ifdef DEBUG_PRINT
      print_to_terminal_radio_state(RADIO_NAME, "SEND PACKET");
      print_to_terminal_radio_state(RADIO_NAME, "TRANSMITT SUCCES!");
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
    
    String str = (String)state;
    #ifdef DEBUG_PRINT
      Serial.print(F("transmission failed, "));
      print_to_terminal_radio_state(RADIO_NAME, str);
    #endif

    displayPrintState(x, y, RADIO_NAME, str);
  
  }
  
}










/**
 * @brief Функція, яка забезпечує вивід поточного стану
 * прийомопередавача у серіал порт (якщо він заданий) та на дисплей
 * 
 * @param state     - текущий стан прийомопередавача, отриманий від приймача під час його роботи
 * @param read_str  - строка для прийому даних
 */
void printStateResult_RX(int &state, String &read_str)
{
  String error_str;

  //Якщо прийом успішний, виводимо повідомлення в серіал-монітор
  if (state == RADIOLIB_ERR_NONE) {
    #ifdef DEBUG_PRINT
      print_to_terminal_radio_state(RADIO_NAME, read_str);
      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F(" RSSI:\t\t"));
      Serial.print(radio.getRSSI());
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F(" SNR:\t\t"));
      Serial.print(radio.getSNR());
      Serial.println(F(" dB"));

      // print frequency error
      Serial.print(F(" FREQ ERR:\t"));
      Serial.print(radio.getFrequencyError());
      Serial.println(F(" Hz"));

    #endif
    //Выводим сообщение об успішному прийомі
    displayPrintState(5, 5, RADIO_NAME, read_str);
    displayPrintState(5, 20, F("RSSI: "), String(radio.getRSSI()) + " dBm");
    displayPrintState(5, 30, F("SNR: "), String(radio.getSNR()) + " dB");
    displayPrintState(5, 40, F("FREQ ERR: "), String(radio.getFrequencyError()) + " Hz");

    // #ifdef DEBUG_PRINT              
    //   //Выводим в сериал данные отправленного пакета
    //   Serial.print(F("RECEIVE DATA:\t\t"));
    //   Serial.println(read_str);
    // #endif

    digitalWrite(LED_PIN, LOW);     //Включаем светодиод, сигнализация об передаче/приёма пакета
  
  } else if(state == RADIOLIB_ERR_LORA_HEADER_DAMAGED) {
    
    // Якщо прийом не вдався, то це означає, що прийнятий пакет пошкоджений, або ми не отримали даних
    #ifdef DEBUG_PRINT
      Serial.println(F("RECEIVE BAD, NO DATA (-24)"));
    #endif
    displayPrintState(5, 5, RADIO_NAME, F("NO DATA (-24)"));
    //return;
    
  }else{
    String error_str = "ERROR:(" + (String)state + ")";
    #ifdef DEBUG_PRINT
      // packet was received, but is malformed
      Serial.println(error_str);
    #endif
    displayPrintState(5, 5, RADIO_NAME, error_str);
    //return;
  }
}






// /**
// * @brief Функция отправляет данные, выводит на экран информацию об отправке,
// * выводит информацию об отправке в сериал-порт
// * 
// * @param transmit_str - строка для передачи
// */
// void transmit_and_print_data(String &transmit_str)
// {
//   display.clearDisplay();

//   //Посылаем пакет
//   state = radio.startTransmit(transmit_str);
//   //Ждём завершения передачи
//   WaitOnBusy();
//   //Печатаем данные куда надо (в сериал, если он активирован, и на дисплей)
//   printStateResultTX(state, transmit_str);
  
// }





















#endif