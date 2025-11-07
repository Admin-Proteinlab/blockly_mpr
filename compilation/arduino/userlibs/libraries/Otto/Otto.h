#ifndef Otto_h
#define Otto_h

#ifdef ARDUINO_ARCH_ESP32
#include <ESP32Servo.h>
#else
#include <Servo.h>
#endif
#include "Oscillator.h"
#include <EEPROM.h>
#include "Otto_sounds.h"
#include "Otto_gestures.h"
#include "Otto_mouths.h"
#include "Otto_matrix.h"


// ===== Otto Debug Config =====
#ifndef OTTO_DBG
  #define OTTO_DBG 1          // 1 = enable Serial debug, 0 = compiled out
#endif

#ifndef OTTO_DBG_BAUD
  #define OTTO_DBG_BAUD 115200
#endif

#ifndef OTTO_VISUAL_DEBUG
  #define OTTO_VISUAL_DEBUG 1   // 1 = trazas de refresco/rr visibles, 0 = silencio
#endif

#if OTTO_DBG && OTTO_VISUAL_DEBUG
  #define OTTO_VIS_PRINT(x)    OTTO_DBG_PRINT(x)
  #define OTTO_VIS_PRINTLN(x)  OTTO_DBG_PRINTLN(x)
#else
  #define OTTO_VIS_PRINT(x)    do {} while(0)
  #define OTTO_VIS_PRINTLN(x)  do {} while(0)
#endif



#if OTTO_DBG
  #define OTTO_DBG_BEGIN()    do { if (!Serial) Serial.begin(OTTO_DBG_BAUD); } while(0)
  #define OTTO_DBG_PRINT(x)   do { Serial.print(x); } while(0)
  #define OTTO_DBG_PRINTLN(x) do { Serial.println(x); } while(0)
#else
  #define OTTO_DBG_BEGIN()    do {} while(0)
  #define OTTO_DBG_PRINT(x)   do {} while(0)
  #define OTTO_DBG_PRINTLN(x) do {} while(0)
#endif

// ===== Default limiter config =====
#ifndef OTTO_LIMITER_DEFAULT_ON
  #define OTTO_LIMITER_DEFAULT_ON 1   // 1=enable SetLimiter() in init()
#endif

#ifndef OTTO_DIFF_LIMIT_DEFAULT
  #define OTTO_DIFF_LIMIT_DEFAULT 3   // degrees per refresh
#endif


//-- Constants
#define FORWARD     1
#define BACKWARD    -1
#define LEFT        1
#define RIGHT       -1
#define SMALL       5
#define MEDIUM      15
#define BIG         30

// -- Servo delta limit default. degree / sec
#define SERVO_LIMIT_DEFAULT 240

class Otto
{
  public:

    //-- Otto initialization
    void init(int YL, int YR, int RL, int RR, bool load_calibration, int Buzzer);
    //-- Attach & detach functions
    void attachServos();
    void detachServos();

    //-- Oscillator Trims
    void setTrims(int YL, int YR, int RL, int RR);
    void saveTrimsOnEEPROM();

    //-- Predetermined Motion Functions
    void _moveServos(int time, int  servo_target[]);
    void _moveSingle(int position,int  servo_number);
    void oscillateServos(int A[4], int O[4], int T, double phase_diff[4], float cycle);

    //-- HOME = Otto at rest position
    void home();
    bool getRestState();
    void setRestState(bool state);

    //-- Predetermined Motion Functions
    void jump(float steps=1, int T = 2000);

    void walk(float steps=4, int T=1000, int dir = FORWARD);
    void turn(float steps=4, int T=2000, int dir = LEFT);
    void bend (int steps=1, int T=1400, int dir=LEFT);
    void shakeLeg (int steps=1, int T = 2000, int dir=RIGHT);

    void updown(float steps=1, int T=1000, int h = 20);
    void swing(float steps=1, int T=1000, int h=20);
    void tiptoeSwing(float steps=1, int T=900, int h=20);
    void jitter(float steps=1, int T=500, int h=20);
    void ascendingTurn(float steps=1, int T=900, int h=20);

    void moonwalker(float steps=1, int T=900, int h=20, int dir=LEFT);
    void crusaito(float steps=1, int T=900, int h=20, int dir=FORWARD);
    void flapping(float steps=1, int T=1000, int h=20, int dir=FORWARD);

    //-- Mouth & Animations
    void putMouth(unsigned long int mouth, bool predefined = true);
    void putAnimationMouth(unsigned long int anim, int index);
    void clearMouth();

    //-- Sounds
    void _tone (float noteFrequency, long noteDuration, int silentDuration);
    void bendTones (float initFrequency, float finalFrequency, float prop, long noteDuration, int silentDuration);
    void sing(int songName);

    //-- Gestures
    void playGesture(int gesture);
    void initMATRIX(int DIN, int CS, int CLK, int rotate);
    void matrixIntensity(int intensity);
    void setLed(byte X, byte Y, byte value);
    void writeText (const char * s, byte scrollspeed);

    // -- Servo limiter
    void enableServoLimit(int speed_limit_degree_per_sec = SERVO_LIMIT_DEFAULT);
    void disableServoLimit();

  private:

    Oscillator servo[4];
    Otto_Matrix ledmatrix;
    int servo_pins[4];
    int servo_trim[4];

    int pinBuzzer;

    unsigned long final_time;
    unsigned long partial_time;
    float increment[4];

    bool isOttoResting;

    unsigned long int getMouthShape(int number);
    unsigned long int getAnimShape(int anim, int index);
    void _execute(int A[4], int O[4], int T, double phase_diff[4], float steps);

};

#endif