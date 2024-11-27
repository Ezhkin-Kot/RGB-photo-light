#define LEDPIN 13
#define NUMLEDS 32 // Количество светодиодов

#define Modes 4 // Количество дополнительных режимов
#define StaticMode 0 // Первый режим
#define TemperatureMode 1 // Второй режим
#define GradientMode 2 // Третий режим
#define FireMode 3 // Четвёртый режим
#include <microLED.h>
#include <FastLEDsupport.h>
microLED<NUMLEDS, LEDPIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_AVER> strip;
#include <EncButton.h>
EncButton encoder(2, 3, 4);

int temp = 3500;
unsigned char mode = 0; // Режим
unsigned char brightness = 100;
unsigned char color = 0;
unsigned char color2 = 0;
bool ColorOrBrightness = 0;
bool OnOff = 0;
bool levelOfModes = 3; // Уровень режимов
bool GradColorNum = 0;
bool modeSelect = 0;

void StaticColor(unsigned char color = 0) {
  strip.fill(mWheel8(color));
  strip.show();
  delay(1);
}

// Холодный/Тёплый свет
void Temperature(int temp = 3500) {
  strip.fill(mKelvin(temp));
  strip.show();
  Serial.println(temp);
  delay(1);
}

// Градиент
void Gradient(unsigned char color1, unsigned char color2) {
  strip.fillGradient(0, NUMLEDS/2, mWheel8(color1), mWheel8(color2));
  strip.fillGradient(NUMLEDS/2, NUMLEDS, mWheel8(color2), mWheel8(color1));
  strip.show();
  Serial.println(color1);
  Serial.print('\t');
  Serial.print(color2);
  delay(1);
}

// Режим иммитации пламени
int count = 0;
void Fire() {
  strip.setBrightness(brightness);
  mGradient<4> myGrad;
  myGrad.colors[0] = mBlack;
  myGrad.colors[1] = mRed;
  myGrad.colors[2] = mYellow;
  myGrad.colors[3] = mWhite;

  for (int i = 0; i < NUMLEDS; i++) {
    strip.leds[i] = myGrad.get(inoise8(i * 50, count), 255);
  }
  count += 20;
  strip.show();
  delay(40);
}

void SetBrightness(unsigned char *brightness) {
  if (encoder.turn()) {
    if (encoder.pressing()) {
      *brightness += encoder.dir(); // Медленное пролистывание
    }
    else {
      *brightness += 5 * encoder.dir();
    }

    if (*brightness >= 255) {
      *brightness = 250;
    }
    if (*brightness <= 0) {
      *brightness = 5;
    }
  }

  strip.setBrightness(*brightness);
  Serial.println("brightness = " + *brightness);
}

void SetColor(unsigned char *color, int speed = 1) {
  if (encoder.turn()) {
    if (encoder.pressing()) {
      *color += speed * encoder.dir(); // Медленное пролистывание
    }
    else {
      *color += 5 * encoder.dir();
    }

    if (*color > 255) {
      *color = 0;
    }
    if (*color < 0) {
      *color = 255;
    }
  }

  Serial.println(*color);
}

void ModeSelect(unsigned char *mode) {
  if (encoder.turn()){
    *mode += encoder.dir();

    if (*mode >= Modes){
      *mode = 0;
    }
    if (*mode < 0){
      *mode = Modes;
    }
  }

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

// Отключение ленты
void Clear() {
  strip.fill(mBlack);
  strip.show();
  delay(1);
}

void setup() {
  encoder.setEncReverse(0);
  encoder.setEncType(EB_STEP4_LOW); // Тип энкодера
  Serial.begin(115200);
  pinMode(5, 2);
  pinMode(6, 2);
  Clear();
}

void loop() {
  encoder.tick();
  if (OnOff) {
    if (modeSelect) {
      ModeSelect(&mode);
    }
    else if (mode == StaticMode) {
      if (!ColorOrBrightness) {
        SetColor(&color);
      }
      else {
        SetBrightness(&brightness);
      }

      StaticColor(color);
    }
    else if (mode == TemperatureMode) {
      if (!ColorOrBrightness) {
        if (encoder.turn()){
          if (encoder.pressing()) {
            temp += 50 * encoder.dir(); // Медленное пролистывание
          }
          else {temp += 200 * encoder.dir();}

          if (temp > 7000) {
            temp = 7000;
          }
          if (temp < 0) {
            temp = 0;
          }
        }
      }
      else {
        SetBrightness(&brightness);
      }

      Temperature(temp);
    }
    else if (mode == GradientMode) {
      if (!ColorOrBrightness) {
        if (encoder.pressing()) {
          SetColor(&color2, 5);
        }
        else {
          SetColor(&color);
        }
      }
      else {
        SetBrightness(&brightness);
      }

      Gradient(color, color2);
    }
    else if (mode == FireMode) {
      if (ColorOrBrightness) {
        SetBrightness(&brightness);
        strip.fill(mYellow);
        strip.show();
      }
      else {
        Fire();
      }
    }
  }

  if (encoder.click()) {
    Signal(mYellow);
    ColorOrBrightness = !ColorOrBrightness;
  }

  if (encoder.hold()) {
    Signal(mGreen);
    modeSelect = !modeSelect; // Смена уровня режимов
  }

  if (digitalRead(5) != 1) {
    Signal();
    if (OnOff) {
      Clear();
    }
    OnOff = !OnOff;

    delay(500);
  }
}
