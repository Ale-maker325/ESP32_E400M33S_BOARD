#ifndef SETTINGS_H
#define SETTINGS_H 

#include <Arduino.h>


//#define RECEIVER                //розкомментувати, якщо модуль буде використовуватися як простий приймач
#define TRANSMITTER             //розкоментувати, якщо модуль буде використовуватися як простий передавач

#define DEBUG_PRINT             //Розкментувати, якщо потрібно виводити інформацію в серійний монітор
#define SX1278_MODEM      //Використовувати радіо-модуль SX1278
//#define SX1268_MODEM     //Використовувати радіо-модуль SX1268

#define SPEED_TRANSMIT 900000 //Швидкість передачі даних в мікросекундах (1 000 000 мкс = 1 секунда)

//#define BUTTONS //Розкоментувати, якщо потрібно використовувати кнопки для управління

#define FAN_IS_ACTIVE //Розкоментувати, якщо потрібно використовувати вентилятор для охолодження передавача або закоментувати, якщо не потрібно використовувати вентилятор
#define FAN_ON_POWER 2  //Потужність передавача, при якій вмикається вентилятор (залежить від індивідуальних умов).

static String RADIO_NAME = F("RADIO: "); //Строка для друку назви (імені) радіо-модуля для відображення на дисплеї та в серійному моніторі




//**************************************************** Параметри радіо для компіляції ************************************************

//Завдаємо параметри конфігурації радіо-модуля
//Ці параметри будуть використовуватися для ініціалізації радіо
#define RADIO_FREQ 441          //Встановити частоту радіо (за замовчуванням 441 МГц)
#define RADIO_BANDWIDTH 125     //Встановити ширину смуги пропускання (за замовчуванням 125 кГц)
#define RADIO_SPREAD_FACTOR 9   //Встановити коефіцієнт розширення (за замовчуванням 9)
#define RADIO_CODING_RATE 7     //Встановити коефіцієнт кодування (за замовчуванням 7)
#define RADIO_SYNC_WORD RADIOLIB_LR11X0_LORA_SYNC_WORD_PRIVATE  //Встановити слово синхронізації (за замовчуванням 0x18). УВАГА! Значення 0x34 зарезервовано для мереж LoRaWAN і небажано для використання
#define RADIO_OUTPUT_POWER 2   //Встановити вихідну потужність передавача
#define RADIO_CURRENT_LIMIT 200 //Встановити обмеження струму захисту (за замовчуванням до 80 мА) (допустимий діапазон 45 - 240 мА) УВАГА: встановіть значення 0, щоб вимкнути захист від перевантаження струму
#define RADIO_PREAMBLE_LENGTH 8 //Встановити довжину преамбули (за замовчуванням 8 символів) (допустимий діапазон 6 - 65535)
#define RADIO_GAIN 0            //Встановити регулювання підсилювача (за замовчуванням 1) (допустимий діапазон 1 - 6, де 1 - максимальне збільшення) УВАГА: встановіть значення 0, щоб увімкнути автоматичне регулювання підсилення залиште в 0, якщо ви не знаєте, що ви робите

//************************************************************************************************************************************



/**
 * @brief Піни I2C
 * 
 */
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22



// _______________________ Пін управління вентилятором _______________________
#define FAN 16

// _______________________ Піни управління світлодіодом _______________________
#define LED_PIN 27

// _______________________ Піни управління радіо-модемами _______________________
#define NSS_PIN 17
#define NRST_PIN 14
#define BUSY_PIN 26
#define DIO1_PIN 25
#define TX_EN_PIN 12
#define RX_EN_PIN 13

/**
* @brief Піни SPI для радіо-модулів
* Обрано SPI - VSPI: 
* _________________________________________________________________________
*    СПИ	 *      МОСИ	 *      МИСО	 *     СКЛК	    *      КС     *
*__________________________________________________________________________
*    VSPI	 *    GPIO 23	 *    GPIO 19	 *    GPIO 18	*    GPIO5    *
* ___________*_______________*_______________*______________*_____________*
*    HSPI*	 *    GPIO 13	 *    GPIO 12	 *    GPIO 14	*    GPIO 15  *
* ___________*_______________*_______________*______________*_____________*
*
* *Примітка: стандартні піни HSPI зайняті
*/
#define MOSI_RADIO 23
#define MISO_RADIO 19
#define SCK_RADIO 18



#endif

