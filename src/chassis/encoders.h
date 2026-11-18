// --- Квадратурные энкодеры: интерфейс --------------------------------------
// Encoder инкапсулирует состояние одного колеса: счётчик тиков, подавление
// дребезга, переходы квадратурной таблицы и EMA-фильтр измеренной скорости.
// Encoders — группа порта PORTC: включает PCINT1 и раздаёт один снимок PINC
// двум энкодерам.
//
// Экземпляры Encoder и группа encoders создаются в main.cpp: его адрес нужен
// ISR. Реализация и ISR — в encoders.cpp.

#ifndef ENCODERS_H
#define ENCODERS_H

#include <Arduino.h>

class Encoder {
 public:
  // Пины каналов A и B. Оба обязаны быть на PORTC (A0..A5): ISR обслуживает
  // PCINT1_vect. Порядок A/B задаёт направление счёта.
  Encoder(uint8_t pinA, uint8_t pinB);
  Encoder(const Encoder &) = delete;
  Encoder &operator=(const Encoder &) = delete;

  // Перебазирует опорную точку после паузы: следующая итерация считает тики
  // только с этого момента.
  void rebase();

  // Сбрасывает фильтр, чтобы после остановки первый замер был не сглаженным.
  void resetFilter();

  // Возвращает сглаженную скорость в мм/с в системе энкодера: положительное
  // значение — направление счёта энкодера. Знак колеса применяет Motor.
  float measureSpeed(float dtSec);

 private:
  friend class Encoders;

  // Настраивает пины как входы с подтяжкой и маску PCMSK1 своего порта.
  void initPins();

  // Обрабатывает снимок порта. Вызывается только из
  // Encoders::handleInterrupt(), то есть из ISR.
  void updateFromPort(uint8_t portBits);

  // Обрабатывает новое состояние каналов A/B (младшие 2 бита). Вызывается
  // только из Encoders::handleInterrupt(), то есть из ISR.
  void update(uint8_t state);

  // Атомарный снимок 32-битного счётчика. На AVR volatile не делает чтение
  // четырёх байт атомарным: ISR может обновить ticks_ между байтами.
  int32_t takeTicks() const;

  static const int8_t kQuadTable[16];

  const uint8_t pinA_;
  const uint8_t pinB_;
  const uint8_t bitA_;    // номер бита канала A в PORTC (PCINT8..PCINT13)
  const uint8_t bitB_;    // номер бита канала B в PORTC
  const uint8_t pcMask_;  // маска каналов в PCMSK1

  volatile int32_t ticks_ = 0;
  int32_t prevTicks_ = 0;
  uint8_t prevState_ = 0;
  uint32_t lastTickMicros_ = 0;
  float measuredMmS_ = 0.0f;
  bool filterInit_ = false;
};

class Encoders {
 public:
  Encoders(Encoder &left, Encoder &right);
  Encoders(const Encoders &) = delete;
  Encoders &operator=(const Encoders &) = delete;

  // Настраивает пины обоих энкодеров и разрешает прерывание по PCINT1.
  void init();

  // Обрабатывает фронт на любой из линий A0-A5. Вызывается из ISR.
  void handleInterrupt();

 private:
  Encoder &left_;
  Encoder &right_;
};

// Экземпляр создаётся в основном скетче; ISR из encoders.cpp обращается к нему
// через это объявление.
extern Encoders encoders;

#endif
