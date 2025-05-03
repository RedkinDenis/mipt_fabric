// Подключение необходимых библиотек
#include <Arduino.h>
#include <app.hpp>

// Подключение заголовочных файлов игр
#include <doom.hpp>
#include <menu.hpp>
#include <pong.hpp>
#include <tetris.hpp>

// Подключение библиотеки для работы с клавиатурой
#include <kbd/sdl_keyboard.hpp>

// Условная компиляция для симуляции и реального устройства
#ifndef SIM
#include <TFT_eSPI.h>  // Библиотека для работы с TFT дисплеем
#else
#include <sdl/sim_display.hpp>  // Библиотека для симуляции дисплея
#endif // SIM
#include <etl/to_string.h>

// Создание объекта клавиатуры с указанием пинов
sdl::Keyboard<A0, D8, D4> keyboard;

// Массив приложений (игр) с указанием их основных функций
App gApps[] = {[kPong] = {PongStep},
               [kTetris] = {TetrisStep},
               [kMenu] = {MenuStep},
               [kDoom] = {DoomStep}};

// Время между кадрами в миллисекундах
constexpr int kFrameTick = 50; // ms

// Функция инициализации
void setup() {
#ifndef SIM
  Serial.begin(9600);  // Инициализация последовательного порта для отладки
#endif // SIM

  keyboard.init();  // Инициализация клавиатуры
  PongInit();      // Инициализация игры Pong
  DoomInit();      // Инициализация игры Doom
  // sdl::AllocSprite(sdl::kWidth, 110);
}

// Основной цикл программы
void loop() {
  static uint64_t next_game_tick = 0;  // Время следующего обновления
  static enum AppsList sCurrentApp = kMenu;  // Текущее приложение (начинаем с меню)

  static sdl::Key key_buffer = sdl::KeyNone;  // Буфер для хранения нажатой клавиши

  // Получение нажатой клавиши
  auto key = keyboard.getKey();
  if (key != sdl::KeyNone) {
    key_buffer = key;

#ifndef SIM
    // Вывод информации о нажатой клавише в последовательный порт
    if (key_buffer.pressed()) {
      Serial.printf("pressed %d\n", key_buffer.code());
    } else {
      Serial.printf("clicked %d\n", key_buffer.code());
    }
#endif // SIM
  }

  // Проверка необходимости обновления кадра
  if (millis() - next_game_tick < kFrameTick) {
    return;
  }

  next_game_tick = millis();

  // Вызов основного цикла текущего приложения
  sCurrentApp = gApps[sCurrentApp].main_(key_buffer);
  key_buffer = sdl::KeyNone;  // Очистка буфера клавиш
}

// Код для симуляции (только для ПК)
#ifdef SIM
namespace sdl {
sf::Event key_buff;  // Буфер для хранения событий клавиатуры в симуляции
}

// Точка входа для симуляции
int main() {
  setup();

  long last_time = 0;
  auto &&display = sim::Display::GetOrInit();  // Инициализация дисплея симуляции

  // Основной цикл симуляции
  while (display->isOpen()) {
    sf::Event event;

    while (true) {
      display->pollEvent(event);
      // Проверка на закрытие окна или нажатие Escape
      if (event.type == sf::Event::Closed ||
          event.type == sf::Event::KeyPressed &&
              event.key.code == sf::Keyboard::Escape) {
        display->close();
        break;
      }

      // Обработка нажатий клавиш
      if (event.type == sf::Event::KeyPressed)
        sdl::key_buff = event;

      loop();
      sdl::key_buff.key.code = sf::Keyboard::Unknown;

      // Обновление дисплея каждые 3 мс
      if (millis() - last_time > 3) {
        display->flush();
        last_time = millis();
      }
    }
  }

  return 0;
}
#endif // SIM