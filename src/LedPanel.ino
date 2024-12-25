#define LEDPIN 13 // Информационный пин ленты
#define NUMLEDS 32 // Количество светодиодов

#define Modes 4 // Количество дополнительных режимов
#define StaticMode 0 // Первый режим
#define TemperatureMode 1 // Второй режим
#define GradientMode 2 // Третий режим
#define FireMode 3 // Четвёртый режим
#include <microLED.h>
#include <FastLEDsupport.h>
microLED<NUMLEDS, LEDPIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> strip; // Инициализация ленты
#include <EncButton.h>
EncButton encoder(2, 3, 4); // Инициализация энкодера

int temp = 3500; // Цветовая температура
unsigned char mode = 0; // Режим
unsigned char brightness = 100;
unsigned char color = 0;
unsigned char color2 = 0;
bool ColorOrBrightness = 0; // Выбор цвета или яркости
bool OnOff = 0; // Вкл/Выкл ленту
bool levelOfModes = 3; // Уровень режимов
bool GradColorNum = 0; // Изменение разных цветов градиента
bool modeSelect = 0; // Меню выбора режима
int count = 0; // Переменная для анимации пламени

void StaticColor(unsigned char color = 0);
void Temperature(int temp = 3500);
void Gradient(unsigned char color1, unsigned char color2);
void Fire();
void SetBrightness(unsigned char *brightness);
void SetColor(unsigned char *color, int speed = 1);
void SetTemperature(int *temp);
void ModeSelect(unsigned char *mode);
void Signal(enum COLORS color = mWhite);
void Clear();

void setup() {
  encoder.setEncReverse(0);
  encoder.setEncType(EB_STEP4_LOW); // Тип энкодера
  Serial.begin(115200);
  pinMode(5, 2);
  Clear();
}

void loop() {
  encoder.tick(); // Обработка энокдера
  if (OnOff) { // Проверка включения ленты
    if (modeSelect) { // Меню выбора режимов
      ModeSelect(&mode);
    }
    else if (mode == StaticMode) { // Статичный цвет
      if (!ColorOrBrightness) {
        SetColor(&color);
      }
      else {
        SetBrightness(&brightness);
      }

      StaticColor(color);
    }
    else if (mode == TemperatureMode) { // Холодный/Тёплый свет
      if (!ColorOrBrightness) {
        SetTemperature(&temp);
      }
      else {
        SetBrightness(&brightness);
      }

      Temperature(temp);
    }
    else if (mode == GradientMode) { // Градиент
      if (!ColorOrBrightness) {
        if (encoder.pressing()) {
          SetColor(&color2, 5); // Выбор первого цвета
        }
        else {
          SetColor(&color); // Выбор второго цвета
        }
      }
      else {
        SetBrightness(&brightness);
      }

      Gradient(color, color2);
    }
    else if (mode == FireMode) { // Режим иммитации пламени
      if (ColorOrBrightness) { // Временное отключение анимации для выбора яркости
        SetBrightness(&brightness);
        strip.fill(mYellow);
        strip.show();
      }
      else {
        Fire();
      }
    }
  }

  // Переключение в режим смены яркости
  if (encoder.click()) {
    Signal(mYellow);
    ColorOrBrightness = !ColorOrBrightness;
  }

  // Переключение в меню выбора режимов
  if (encoder.hold()) {
    Signal(mGreen);
    modeSelect = !modeSelect; // Смена уровня режимов
  }

  // Вкл/Выкл ленту
  if (digitalRead(5) != 1) {
    Signal();
    if (OnOff) {
      Clear();
    }
    OnOff = !OnOff;

    delay(500);
  }
}

// Статичный цвет
void StaticColor(unsigned char color = 0) {
  strip.setBrightness(brightness);
  strip.fill(mWheel8(color));
  strip.show();
  delay(1);
}

// Холодный/Тёплый свет
void Temperature(int temp = 3500) {
  strip.setBrightness(brightness);
  strip.fill(mKelvin(temp));
  strip.show();
  Serial.println(temp);
  delay(1);
}

// Градиент
void Gradient(unsigned char color1, unsigned char color2) {
  strip.setBrightness(brightness);
  strip.fillGradient(0, NUMLEDS/2, mWheel8(color1), mWheel8(color2)); // Заливка первого ряда ленты
  strip.fillGradient(NUMLEDS/2, NUMLEDS, mWheel8(color2), mWheel8(color1)); // Заливка второго ряда ленты
  strip.show();
  Serial.println(color1);
  Serial.print('\t');
  Serial.print(color2);
  delay(1);
}

// Режим иммитации пламени
void Fire() {
  strip.setBrightness(brightness);
  mGradient<4> myGrad; // Палитра градиента из 4 точек
  myGrad.colors[0] = mBlack;
  myGrad.colors[1] = mRed;
  myGrad.colors[2] = mYellow;
  myGrad.colors[3] = mWhite;

  for (int i = 0; i < NUMLEDS; i++) {
    strip.leds[i] = myGrad.get(inoise8(i * 50, count), 255); // Случайный градиент
  }
  count += 20; // Смещение
  strip.show();
  delay(40);
}

// Смена уровня яркости
void SetBrightness(unsigned char *brightness) {
  if (encoder.turn()) {
    if (encoder.pressing()) {
      *brightness += encoder.dir(); // Медленное пролистывание
    }
    else {
      *brightness += 5 * encoder.dir();
    }

    // Ограничение диапазона
    if (*brightness >= 255) {
      *brightness = 250;
    }
    if (*brightness <= 0) {
      *brightness = 5;
    }
  }

  strip.setBrightness(*brightness); // Применение выбранной яркости
  Serial.println("brightness = " + *brightness);
}

// Смена цвета
void SetColor(unsigned char *color, int speed = 1) {
  if (encoder.turn()) {
    if (encoder.pressing()) {
      *color += speed * encoder.dir(); // Медленное пролистывание
    }
    else {
      *color += 5 * encoder.dir();
    }

    // Ограничение диапазона
    if (*color > 255) {
      *color = 0;
    }
    if (*color < 0) {
      *color = 255;
    }
  }

  Serial.println(*color);
}

// Смена температуры
void SetTemperature(int *temp) {
  if (encoder.turn()) {
    if (encoder.pressing()) {
      *temp += 50 * encoder.dir(); // Медленное пролистывание
    }
    else {
      *temp += 200 * encoder.dir();
    }

    // Ограничение диапазона
    if (*temp > 7000) {
      *temp = 7000;
    }
    if (*temp < 0) {
      *temp = 0;
    }
  }
}

// Меню выбора режимов
void ModeSelect(unsigned char *mode) {
  if (encoder.turn()){
    *mode += encoder.dir();

    // Ограничение диапазона
    if (*mode >= Modes){
      *mode = 0;
    }
    if (*mode < 0){
      *mode = Modes;
    }
  }

  // Отображение выбранного режима
  strip.fill((NUMLEDS/Modes) * *mode, (NUMLEDS/Modes) * (*mode + 1) - 1, mWheel8(50 * *mode));
  strip.show();
  delay(10);
  strip.fill(mBlack);
  strip.show();
  delay(1);
}

// Кратковременный сигнал ленты
void Signal(enum COLORS color = mWhite) {
  strip.setBrightness(100);
  strip.fill(color);
  strip.show();
  delay(100);
  strip.fill(mBlack);
  strip.show();
  delay(1);
}

// Очистка ленты
void Clear() {
  strip.fill(mBlack);
  strip.show();
  delay(1);
}