#define LEDPIN 13
#define NUMLEDS 32 // Количество светодиодов
#define Colors 255 // Количество простых режимов
#define Modes 2 // Количество дополнительных режимов
#include <microLED.h>
#include <FastLEDsupport.h>
microLED<NUMLEDS, LEDPIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> strip;
#include <EncButton.h>
EncButton encoder(2, 3, 4);

// Режим иммитации пламени
int count = 0;
void Fire() {
  mGradient<4> myGrad;
  myGrad.colors[0] = mBlack;
  myGrad.colors[1] = mRed;
  myGrad.colors[2] = mYellow;
  myGrad.colors[3] = mWhite;

  for (int i = 0; i < NUMLEDS; i++) {
    // проходим по всей ленте
    // inoise8 вернёт 0-255
    // градиент будет брать значение шума, размер градиента 255 (как максимум шума)
    // i*50 - шаг шума по горизонтали
    // count - общее движение шума по вертикали
    strip.leds[i] = myGrad.get(inoise8(i * 50, count), 255);
  }
  count += 20;
  strip.show();
  delay(40);
}

void Gradient(unsigned char color1, unsigned char color2) {
  mGradient<2> myGrad;  // создать градиент с именем myGrad на 2 точки
  // указываем цвет каждой точки
  myGrad.colors[0] = mWheel8(color1);
  myGrad.colors[1] = mWheel8(color2);
  for (int i = 0; i < NUMLEDS / 2; i++) {
    strip.set(i, myGrad.get(i, NUMLEDS / 2));
  }
  for (int i = 0; i < NUMLEDS / 2; i++) {
    strip.set(NUMLEDS / 2 + i, myGrad.get(NUMLEDS / 2 - i, NUMLEDS / 2));
  }
  strip.show();
  delay(40);
}

// Кратковременный сигнал ленты
void Signal(enum COLORS color = mWhite) {
  strip.setBrightness(64);
  strip.fill(color);
  strip.show();
  delay(100);
}

// Отключение ленты
void Clear() {
  strip.fill(mBlack);
  strip.setBrightness(0);
  strip.show();
  delay(1);
}

unsigned char mode = 0; // Режим
unsigned char brightness = 100;
unsigned char color = 0;
unsigned char color2 = 0;
bool ColorOrBrightness = 0;
bool OnOff = 0;
bool levelOfModes = 0; // Уровень режимов
bool GradColorNum = 0;

void setup() {
  encoder.setEncReverse(0);
  encoder.setEncType(EB_STEP4_LOW); // Тип энкодера
  pinMode(5, 2);
  pinMode(6, 2);
  Clear();
}

void loop() {
  encoder.tick();
  if (OnOff) {
    if (levelOfModes == 0) {
      strip.setBrightness(brightness);
      // Обработка поворота энкодера
      if (!ColorOrBrightness) {
        if (encoder.turn()){
          if (encoder.pressing()) {
            color += encoder.dir(); // Медленное пролистывание
          }
          else {color += 5 * encoder.dir();}

          if (color >= Colors){
            color = 0;
          }
          if (color < 0){
            color = Colors - 5;
          }
        }
      }
      else {
        if (encoder.turn()){
          if (encoder.pressing()) {
            brightness += encoder.dir(); // Медленное пролистывание
          }
          else {brightness += 5 * encoder.dir();}

          if (brightness >= 255){
            brightness = 250;
          }
          if (brightness <= 0){
            brightness = 5;
          }
        }
      }

      strip.fill(mWheel8(color));
      strip.show();
      delay(1);
    }
    else if (levelOfModes == 1) {
      strip.setBrightness(brightness);
      // Обработка поворота энкодера
      if (!ColorOrBrightness) {
        if (GradColorNum == 0) {
          if (encoder.turn()){
            if (encoder.pressing()) {
              color += encoder.dir(); // Медленное пролистывание
            }
            else {color += 5 * encoder.dir();}

            if (color >= Colors){
              color = 0;
            }
            if (color < 0){
              color = Colors - 5;
            }
          }
        }
        else if (GradColorNum == 1) {
          if (encoder.turn()){
            if (encoder.pressing()) {
              color2 += encoder.dir(); // Медленное пролистывание
            }
            else {color2 += 5 * encoder.dir();}

            if (color2 >= Colors){
              color2 = 0;
            }
            if (color2 < 0){
              color2 = Colors - 5;
            }
          }
        }
      }
      else {
        if (encoder.turn()){
          if (encoder.pressing()) {
            brightness += encoder.dir(); // Медленное пролистывание
          }
          else {brightness += 5 * encoder.dir();}

          if (brightness >= 255){
            brightness = 250;
          }
          if (brightness <= 0){
            brightness = 5;
          }
        }
      }

      if (mode == 0) {
        Fire();
      }
      else if (mode == 1) {
        Gradient(color, color2);
      }
    }
  }

  if (encoder.click()) {
    Signal(mYellow);
    if (!levelOfModes) {
      ColorOrBrightness = !ColorOrBrightness;
    }
    else if (levelOfModes) {
      if (!GradColorNum) {
        Signal(mPurple);
      }
      else if (GradColorNum) {
        Signal(mAqua);
      }
      GradColorNum = !GradColorNum;
    }
  }

  if (encoder.hold()) {
    Signal(mGreen);
    levelOfModes = !levelOfModes; // Смена уровня режимов
    mode = 0;
  }

  if (digitalRead(5) != 1) {
    Signal();
    if (OnOff) {
      Clear();
    }
    OnOff = !OnOff;
    delay(500);
  }

  if (digitalRead(6) != 1) {
    if (levelOfModes) {
      mode = !mode;
      delay(500);
    }
  }
}
