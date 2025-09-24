/**
 * @file main.cpp
 * @author Ale-maker325 (https://github.com/Ale-maker325/ESP32_E400M33S_BOARD)
 * 
 * @brief Приклад роботи з SPI модулем E32/E22-M30S/M33S для ESP32. Приклад заснован на інших прикладах з бібліотек Adafruit SSD1306 и RadioLib,
 * та розрахован на використання з дісплеєм OLED SSD1306
 * 
 * 1) Усі налаштування перед компіляцією програми необхідно зробити у файлі "settings.h", який знаходиться в тій же теці, що і цей файл.
 * 2) Перш за все потрібно обрати у якості чого буде працювати плата: передавач чи приймач. Для цього необхідно
 * розкмментувати один з дефайнів: #define RECEIVER чи #define TRANSMITTER (інший дефайн треба закоментувати).
 * 3) Також якщо потрібна відладочна інформація, то потрібно розкоментувати дефайн #define DEBUG_PRINT. Проте треба
 * мати на увазі, що при цьому буде використовуватись більше пам'яті, а також буде суттєва затримка при передачі даних
 * через серійний порт, бо це підключає затримку delay;
 * 4) Далі необхідно обрати тип модуля, який буде використовуватись. Для цього потрібно розкоментувати один з дефайнів:
 * #define SX1278_MODEM для модуля SX1278, або #define SX1268_MODEM для модуля SX1268.
 * 5) Також можна обрати своє ім'я модуля, яке буде виводитись на дисплей та в серійний порт. Для цього потрібно завдати строку RADIO_NAME
 * Слід мати на увазі, що дисплей виводить тільки строку довжиною до 20 символів, тому якщо ви вкажете більше, то вона буде обрізана.
 * 6) Передача даних здійснюється з певними промідками часу, які завдаються таймером. Період таймера можна налаштувати, змінивши значення дефайну SPEED_TRANSMIT.
 * Це значення вказує на період таймера в мікросекундах. Наприклад, якщо ви хочете, щоб передача даних відбувалась кожну секунду, то потрібно встановити
 * значення 1000000 (1 000 000 мкс = 1 секунда).
 * 7) Далі необхідно завдати параметри конфігурації модуля, які знаходяться у файлі "settings.h".
 * 8) Для прикладу роботи я використав бібліотеку кнопки Button2, яка дозволяє легко працювати з кнопками.
 * Для цього потрібно розкоментувати дефайн #define BUTTONS, а також вказати пін кнопки, яка буде використовуватись для управління. Треба мати на увазі,
 * що якщо ви не використовуєте кнопки (перемички у вас не запаяні), цей дефайн треба закоментувати, бо можливі хибні спрацювання кнопок.
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
#include <Button2.h>            //бібліотека для роботи з кнопками
#include <esp_task_wdt.h>       //бібліотека для роботи з watchdog таймером
#include <Streaming.h>          //бібліотека для зручного виводу даних в серійний порт

#ifdef BUTTONS
  //Визначаємо пін кнопки для управління. Якщо ви використовуєте інші піни, то змініть їх тут
  #define BUTTON_PIN_0  0   //Пін кнопки для управління
  #define BUTTON_PIN_32 32  //Пін кнопки для управління
  #define BUTTON_PIN_33 33  //Пін кнопки для управління
  #define BUTTON_PIN_34 34  //Пін кнопки для управління
  #define BUTTON_PIN_35 35  //Пін кнопки для управління

  Button2 button0(BUTTON_PIN_0);   //Кнопка для управління
  Button2 button32(BUTTON_PIN_32); //Кнопка для управління
  Button2 button33(BUTTON_PIN_33); //Кнопка для управління
  Button2 button34(BUTTON_PIN_34); //Кнопка для управління
  Button2 button35(BUTTON_PIN_35); //Кнопка для управління
#endif



uint64_t count = 0;           //Лічильник для відстеження кількості прийнятих пакетів
String str_read_data = "";    //Создаём пустую строку для приёма данных



//Створюємо вказівник на структуру таймера 0
hw_timer_t *myTIMER_0 = NULL;
//Створюємо вказівник на структуру таймера 1
hw_timer_t *myTIMER_1 = NULL;

//Логічний флаг срабатывания таймера 0
volatile boolean flagTimer_0;
//Логічний флаг срабатывания таймера 1
volatile boolean flagTimer_1;

//значення періоду счета таймера 0 (1000000 (миллисекунд) = 1 секунда)
uint64_t periodTimera_0 = SPEED_TRANSMIT;
//значення періоду счета таймера 1 (1000000 (миллисекунд) = 1 секунда)
uint64_t periodTimera_1 = SPEED_TRANSMIT;

//оголошуємо змінну для синхронізації між основним циклом і ISR
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
//оголошуємо змінну для синхронізації між основним циклом і ISR
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;




/**
 * @brief Функція обробник преривань для таймера "TIMER_0"
 * 
 * Функция ISR должна быть функцией, которая возвращает значение void и не получает аргументов. Процедура обработки прерывания
 * должна иметь атрибут IRAM_ATTR , чтобы компилятор мог поместить код в IRAM. Кроме того, процедуры обработки прерываний должны 
 * вызывать только функции, также размещенные в IRAM.
 * 
 */
void IRAM_ATTR timer_0()
{
  //Увійшли в критичний розділ, щоб захистити змінну flagTimer_0 від конкурентного доступу  
  portENTER_CRITICAL_ISR(&timerMux0);
  flagTimer_0 = true;
  portEXIT_CRITICAL_ISR(&timerMux0);
}




/**
 * @brief Функція обробник преривань для таймера "TIMER_1"
 * 
 * Функция ISR должна быть функцией, которая возвращает значение void и не получает аргументов. Процедура обработки прерывания
 * должна иметь атрибут IRAM_ATTR , чтобы компилятор мог поместить код в IRAM. Кроме того, процедуры обработки прерываний должны 
 * вызывать только функции, также размещенные в IRAM.
 * 
 */
void IRAM_ATTR timer_1()
{
  portENTER_CRITICAL_ISR(&timerMux1);
  flagTimer_1 = true;
  portEXIT_CRITICAL_ISR(&timerMux1);
}







#ifdef BUTTONS

  int button_x = 5; //Початкове значення координати X для виводу інформації на дисплей
  int button_y = 55; //Початкове значення координати Y для виводу інформації на дисплей

  void pressed_button_0(Button2& btn) {
    #ifdef DEBUG_PRINT
      buttons_flag = true; //Встановлюємо флаг натискання кнопки
      Serial.println("pressed_button_0");
    #endif
    displayPrintState(button_x, button_y, RADIO_NAME,"Button 0");
  }

  void pressed_button_32(Button2& btn) {
    #ifdef DEBUG_PRINT
      buttons_flag = true; //Встановлюємо флаг натискання кнопки
      Serial.println("pressed_button_32");
    #endif
    displayPrintState(button_x, button_y, RADIO_NAME,"Button 32");
  }

  void pressed_button_33(Button2& btn) {
    #ifdef DEBUG_PRINT
      buttons_flag = true; //Встановлюємо флаг натискання кнопки
      Serial.println("pressed_button_33");
    #endif
    displayPrintState(button_x, button_y, RADIO_NAME,"Button 33");
  }

  void pressed_button_34(Button2& btn) {
    #ifdef DEBUG_PRINT
      buttons_flag = true; //Встановлюємо флаг натискання кнопки
      Serial.println("pressed_button_34");
    #endif
    displayPrintState(button_x, button_y, RADIO_NAME,"Button 34");
  }

  void pressed_button_35(Button2& btn) {
    #ifdef DEBUG_PRINT
      buttons_flag = true; //Встановлюємо флаг натискання кнопки
      Serial.println("pressed_button_35");
    #endif
    displayPrintState(button_x, button_y, RADIO_NAME,"Button 35");
  }
#endif












/**
 * @brief  Функція для виводу інформації про чіп ESP32
 * 
 */
void printChipInfo() {
  #ifdef DEBUG_PRINT
    Serial.println("");
    Serial.print(TABLE_LEFT);
    Serial.print(F("CHIP INFO"));
    Serial.println(TABLE_RIGHT);
    Serial << F("Чіп = ") << ESP.getChipModel() << endl;
    Serial << F("Частота CPU = ") << getCpuFrequencyMhz() << F(" MHz") << endl;
    Serial << F("Частота работы кварца = ") << getXtalFrequencyMhz() << F(" MHz") << endl;
    Serial << F("Частота работы APB = ") << getApbFrequency()/1000000 << F(" MHz") << endl;
    Serial << F("Ревізія чіпа = ") << ESP.getChipRevision() << endl;
    Serial << F("Версія SDK = ") << ESP.getSdkVersion() << endl;
    Serial.println("***********************************************************************");
    Serial.println("");
  #endif
}




void radio_TX_loop()
{
  #ifdef TRANSMITTER   //Если определен как передатчик
    //проверяем, была ли предыдущая передача успешной
    #ifdef DEBUG_PRINT
      Serial.println("================================================================================================");
      Serial.println(F("radio_TX_loop(): Ожидаем окончания передачи предыдущего пакета ..."));
      Serial.print("radio_TX_loop(): operationDone = ");
      if(operationDone == true) Serial.println("true");
      else  Serial.println("false");  
    #endif

    if(operationDone) {
      #ifdef DEBUG_PRINT
        Serial.println("================================================================================================");
        Serial.println(F("radio_TX_loop(): передаём новый пакет ..."));
      #endif
      
      //Сбрасываем сработавший флаг прерывания
      operationDone = false;

      //готовим строку для отправки
      String str = "#" + String(count++);

      transmit_and_print_data(str);

      #ifdef DEBUG_PRINT
        Serial.println(F("radio_TX_loop(): Передача пакета завершена"));
        Serial.print("radio_TX_loop(): operationDone = ");
        if(operationDone == true) Serial.println("true");
        else  Serial.println("false"); 
        Serial.println("================================================================================================");
      #endif
       
    }
    else {
      #ifdef DEBUG_PRINT
        Serial.println(F("radio_TX_loop(): Передача до сих пор не окончена ..."));
        Serial.print("radio_TX_loop(): operationDone = ");
        if(operationDone == true) Serial.println("true");
        else  Serial.println("false"); 
        Serial.println("================================================================================================");
      #endif
    }

       
  #endif
  
}



// void button_print_state()
// {
//   Serial.print("button 34 state: ");
//   Serial.println(digitalRead(BUTTON_PIN_34));
//   Serial.print("button 35 state: ");
//   Serial.println(digitalRead(BUTTON_PIN_35));
// }




void setup() {
  //Ініціалізуємо таймер watchdog для захисту від зависання
  //Таймер буде перезапускатись кожні 10 секунд, якщо програма не зависне
  //Це дозволить уникнути зависання програми, якщо вона не буде відповідати
  esp_task_wdt_init(10, true); // таймаут 10 сек
  esp_task_wdt_add(NULL);

  //Ініціалізуємо серійний порт для виводу інформації
  #ifdef DEBUG_PRINT
    Serial.begin(115200);
  #endif

    
  //Ініціалізуємо шину I2C для роботи з дисплеєм
  display_wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
  //Ініціалізуємо SPI для роботи з радіо-модулем
  SPI_MODEM.begin(SCK_RADIO, MISO_RADIO, MOSI_RADIO);
  
  #ifdef DEBUG_PRINT
    printChipInfo();
  #endif
    
  //Ініціалізуємо дисплей
  displayInit();

  pinMode(LED_PIN, OUTPUT);      //Контакт управління світлодіодом налаштовуємо як вихідний
  pinMode(FAN, OUTPUT);          //Контакт управління вентилятором налаштовуємо як вихідний
  
  //Визначаємо параметри конфігурації радіо-модуля (просто викликаємо функцію, яка задасть всі параметри, які задані в файлі "settings.h")
  setRadioMode();

  //Ініціалізуємо радіо-модуль
  radioBeginAll();
  
  //Назначаем контакты для управлением вкл./выкл. усилителем передатчика
  radio.setRfSwitchPins(RX_EN_PIN, TX_EN_PIN);
  radio.setDio2AsRfSwitch(true); //Використовуємо DIO2 для управління перемиканням антени
  radio.setTCXO(2.4); //Вказуємо напругу живлення TCXO, якщо він використовується



    
  //Устанавливаем наши значения, определённые ранее для структуры config_radio1
  radio_setSettings(radio, config_radio);

  //Устанавливаем функцию, которая будет вызываться при отправке пакета данных модемом
  radio.setPacketSentAction(flag_operationDone);
  //radio.setDio1Action(flag_operationDone);
  
  RadioStart(); //Запускаем радио-модуль

  #ifdef DEBUG_PRINT
    Serial.println(" ");
  #endif

  #ifdef BUTTONS
    
    // pinMode(BUTTON_PIN_34, INPUT); //Підтягуємо кнопку 34 до HIGH
    // pinMode(BUTTON_PIN_35, INPUT); //Підтягуємо кнопку 35 до HIGH
  
    button0.setPressedHandler(pressed_button_0); //Встановлюємо обробник натискання кнопки 0
    button32.setPressedHandler(pressed_button_32); //Встановлюємо обробник натискання кнопки 32
    button33.setPressedHandler(pressed_button_33); //Встановлюємо обробник натискання кнопки 33
    button34.setPressedHandler(pressed_button_34); //Встановлюємо обробник натискання кнопки 34
    button35.setPressedHandler(pressed_button_35); //Встановлюємо обробник натискання кнопки 35

    
  #endif


  #ifdef TRANSMITTER
    /**
     * @brief инициализируем таймер с помощью функции timerbegin , эта функция получает номер таймера, который
     * мы хотим использовать (от 0 до 3, так как у нас есть 4 аппаратных таймера), значение предварительного
     * делителя и флаг, указывающий, должен ли счетчик считать вверх (истина) или вниз (ложь). счетчик считает,
     * что частота базового сигнала, используемого счетчиками ESP32, составляет 80 МГц . Если мы разделим это
     * значение на 80 (используя 80 в качестве значение прескалера), мы получим сигнал с частотой 1 МГц, который
     * будет увеличивать счетчик таймера на 1 000 000 раз в секунду.
     * Таким образом, таймер 0 будет считать с частотой 1 МГц. Поскольку наша частота составляет 1 МГц, мы знаем,
     * что таймер будет считать 1 000 000 значений за 1 секунду.
     * ДЛя того, чтобы остановить таймер, необходимо вызвать "void timerEnd(hw_timer_t *timer);" Также для старта и
     * остановки таймера есть функции: "void timerStart(hw_timer_t *timer);", "void timerStop(hw_timer_t *timer);", а
     * для рестарта таймера есть функция "void timerRestart(hw_timer_t *timer);"
    */
    myTIMER_0 = timerBegin(0, 80, true); // timer 0, MWDT clock period = 12.5 ns * TIMGn_Tx_WDT_CLK_PRESCALE -> 12.5 ns * 80 -> 1000 ns = 1 us, countUp
    myTIMER_1 = timerBegin(1, 80, true);
    /**
     * @brief Прежде чем включать таймер, нам нужно привязать его к функции обработки, которая будет выполняться при
     * генерации прерывания. Это делается вызовом функции timerAttachInterrupt. Для того, чтобы отключить функцию 
     * обработки прерывания от таймера, необходимо вызвать функцию "void timerDetachInterrupt(hw_timer_t *timer);"
    */
    timerAttachInterrupt(myTIMER_0, &timer_0, true);
    timerAttachInterrupt(myTIMER_1, &timer_1, true);
    /**
     * @brief timerAlarmWrite , чтобы указать значение счетчика, при котором сгенерировано прерывание таймера. Мы предполагаем,
     * что хотим генерировать прерывание каждые 2 секунды, и поэтому мы передаем значение 2 000 000 микросекунд, что равно
     * 2 секундам. Третий аргумент мы будем передавать true , поэтому счетчик будет перезагружаться и, таким образом, будет 
     * периодически генерироваться прерывание.
     * Поскольку наша частота составляет 1 МГц, мы знаем, что таймер будет считать 1 000 000 значений за 1 секунду.
     * 
    */
    timerAlarmWrite(myTIMER_0, periodTimera_0, true);
    timerAlarmWrite(myTIMER_1, periodTimera_1, true);
    

    
    /**
     * @brief Чтобы завершить функцию настройки, вызовем функцию timerAlarmEnable(timer);
     * Эта функция используется для включения генерации тревожных событий таймера. Для того,
     * чтобы отключить эту функцию, необходимо вызвать "void timerAlarmDisable(hw_timer_t *timer);"
     * 
     */
    timerAlarmEnable(myTIMER_0);
    timerAlarmEnable(myTIMER_1);

    if(timerStarted(myTIMER_0)) Serial << "Таймер 1 запущен......" << endl;
    if(timerStarted(myTIMER_1)) Serial << "Таймер 2 запущен......" << endl;
  #endif
}




 


void loop() {
  

  esp_task_wdt_reset(); // Сброс таймера watchdog

  #ifdef BUTTONS
    button0.loop(); //Обробляємо натискання кнопки 0
    button32.loop(); //Обробляємо натискання кнопки 32
    button33.loop(); //Обробляємо натискання кнопки 33
    button34.loop(); //Обробляємо натискання кнопки 34
    button35.loop(); //Обробляємо натискання кнопки 35

  #endif

  digitalWrite(LED_PIN, HIGH); //Выключаем светодиод, сигнализация об окончании передачи/приёма пакета

  #ifdef TRANSMITTER
    /**
     * @brief Основной цикл будет там, где мы фактически обрабатываем прерывание таймера после того,
     * как об этом сигнализирует ISR (процедура обслуживания прерываний, также называемая обработчиком
     * прерываний) . Чтобы проверить значение счетчика прерываний, мы проверим, является липеременная
     * счетчика прерываний больше нуля, и если это так, мы введем код обработки прерывания. Там первое,
     * что мы сделаем, это уменьшим этот счетчик, сигнализируя, что прерывание было подтверждено и будет обработано.
    */
    if(flagTimer_0)
    {
      Serial << " " << endl;
      Serial << "Сработал таймер 0 .............................." << endl;
      Serial << "таймер 0 - остановлен" << endl;
      Serial << "таймер 1 - запущен" << endl;
      Serial << " " << endl;
      radio_TX_loop();
      flagTimer_0 = false;
      timerAlarmEnable(myTIMER_1);
    
    }
    if(flagTimer_1)
    {
      //timerStop(myTIMER_1);
      //timerDetachInterrupt(myTIMER_1);
      timerAlarmDisable(myTIMER_1);
      Serial << " " << endl;
      Serial << "Сработал таймер 1 .............................." << endl;
      Serial << "таймер 1 - остановлен" << endl;
      Serial << "таймер 0 - запущен" << endl;
      Serial << " " << endl;
      flagTimer_1 = false;

    }
  #endif
   
   
  #ifdef RECEIVER   //Если определен модуль как приёмник
    //проверяем, была ли предыдущая передача успешной
    #ifdef DEBUG_PRINT
      Serial.println("..............................................................");
    #endif

    //radio_scan_channel(); //Сканируем канал на наличие LoRa передачи

    if(operationDone) {
      //Сбрасываем сработавший флаг прерывания
      operationDone = false;
      receive_and_print_data(str_read_data); //Принимаем данные и выводим их на экран
    }

    // check CAD result
    //detected_CAD(Radio_1);
    //detectedPreamble(Radio_1);
    

   
  #endif


  

  
}



