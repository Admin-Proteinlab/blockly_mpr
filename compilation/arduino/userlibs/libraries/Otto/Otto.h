#ifndef Otto_h
#define Otto_h

// === DEBUG OSCILLATOR ===
// 1 = imprime cada N frames, 0 = silencia
#define OTTO_DEBUG_OSC 1
#if OTTO_DEBUG_OSC
#define OTTO_OSC_DBG(msg) Serial.println(F(msg))
#else
#define OTTO_OSC_DBG(msg)
#endif

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

//-- Constants
#define FORWARD 1
#define BACKWARD -1
#define LEFT 1
#define RIGHT -1
#define SMALL 5
#define MEDIUM 15
#define BIG 30

// === Parámetros de oscilación y ahorro ===
static const uint16_t OTTO_FRAME_MS = 20;           // ~50 Hz
static const uint16_t OTTO_DBG_EVERY_N_FRAMES = 5;  // imprime cada 5 frames
static const uint8_t OTTO_SLEW_MAX_DEG_FRAME = 5;   // Δmáx de A/O por frame
static const uint8_t OTTO_DEADBAND_DEG = 2;         // reservado (si luego mides Δθ real)
static const uint16_t OTTO_IDLE_MS_DETACH = 200;    // ms quieto antes de detach

// === OscState para habilitar proceso no-bloqueante
struct OscState {
  bool active = false;

  // Objetivo:
  int A_tgt[4] = { 0, 0, 0, 0 };
  int O_tgt[4] = { 0, 0, 0, 0 };
  int T_ms = 1000;
  double Ph[4] = { 0, 0, 0, 0 };
  float cycles = 1.0;

  // Estado actual (rampa de A/O):
  int A_cur[4] = { 0, 0, 0, 0 };
  int O_cur[4] = { 0, 0, 0, 0 };

  // Timing:
  unsigned long ref_ms = 0;
  unsigned long end_ms = 0;
  unsigned long next_ms = 0;
  uint32_t frames = 0;

  // Inactividad (para detach):
  unsigned long last_motion_ms = 0;
};
// === fin preparación OscState

class Otto {
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
  void _moveServos(int time, int servo_target[]);
  void _moveSingle(int position, int servo_number);
  //  void oscillateServos(int A[4], int O[4], int T, double phase_diff[4], float cycle); //se reemplazará al final, para connotar el cambio

  //-- HOME = Otto at rest position
  void home();
  bool getRestState();
  void setRestState(bool state);

  //-- Predetermined Motion Functions
  void jump(float steps = 1, int T = 2000);

  void walk(float steps = 4, int T = 1000, int dir = FORWARD);
  void turn(float steps = 4, int T = 2000, int dir = LEFT);
  void bend(int steps = 1, int T = 1400, int dir = LEFT);
  void shakeLeg(int steps = 1, int T = 2000, int dir = RIGHT);

  void updown(float steps = 1, int T = 1000, int h = 20);
  void swing(float steps = 1, int T = 1000, int h = 20);
  void tiptoeSwing(float steps = 1, int T = 900, int h = 20);
  void jitter(float steps = 1, int T = 500, int h = 20);
  void ascendingTurn(float steps = 1, int T = 900, int h = 20);

  void moonwalker(float steps = 1, int T = 900, int h = 20, int dir = LEFT);
  void crusaito(float steps = 1, int T = 900, int h = 20, int dir = FORWARD);
  void flapping(float steps = 1, int T = 1000, int h = 20, int dir = FORWARD);

  //-- Mouth & Animations
  void putMouth(unsigned long int mouth, bool predefined = true);
  void putAnimationMouth(unsigned long int anim, int index);
  void clearMouth();

  //-- Sounds
  void _tone(float noteFrequency, long noteDuration, int silentDuration);
  void bendTones(float initFrequency, float finalFrequency, float prop, long noteDuration, int silentDuration);
  void sing(int songName);

  //-- Gestures
  void playGesture(int gesture);
  void initMATRIX(int DIN, int CS, int CLK, int rotate);
  void matrixIntensity(int intensity);
  void setLed(byte X, byte Y, byte value);
  void writeText(const char* s, byte scrollspeed);

  // === inclusión de etapa no bloqueante
  // Nueva API no-bloqueante (opcional):
  bool oscillateServosStep(int A[4], int O[4], int T, double phase_diff[4], float cycle = 1.0f);

  // La API original se mantiene:
  void oscillateServos(int A[4], int O[4], int T, double phase_diff[4], float cycle = 1.0f);


private:
  // === para activar OscState ===

  OscState osc;

  void oscApplyTargets_(const int A[4], const int O[4], int T, const double Ph[4], float cycles);
  bool oscStep_();            // ejecuta un frame si corresponde; true = sigue activo
  bool oscAllAZero_() const;  // todos A en cero (cur y tgt)
  void oscRampAO_();          // rampa (slew-rate) A_cur/O_cur -> A_tgt/O_tgt
  bool oscIsMoving_() const;  // placeholder; útil si luego calculas Δθ real

  Oscillator servo[4];
  Otto_Matrix ledmatrix;
  int servo_pins[4];
  int servo_trim[4];
  int servo_position[4];

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
