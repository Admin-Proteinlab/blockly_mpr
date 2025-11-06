//--------------------------------------------------------------
//-- Oscillator.pde
//-- Generate sinusoidal oscillations in the servos
//--------------------------------------------------------------
//-- (c) Juan Gonzalez-Gomez (Obijuan), Dec 2011
//-- GPL license
//--------------------------------------------------------------

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#include <pins_arduino.h>
#endif
#include "Oscillator.h"

//-- This function returns true if another sample
//-- should be taken (i.e. the TS time has passed since
//-- the last sample was taken
bool Oscillator::next_sample() {

  //-- Read current time
  _currentMillis = millis();

  //-- Check if the timeout has passed
  if (_currentMillis - _previousMillis > _samplingPeriod) {
    _previousMillis = _currentMillis;

    return true;
  }

  return false;
}

//-- Attach an oscillator to a servo
//-- Input: pin is the arduino pin were the servo
//-- is connected
void Oscillator::attach(int pin, bool rev) {
  //-- If the oscillator is detached, attach it.
  if (!_servo.attached()) {

    //-- Attach the servo and move it to the home position
    _servo.attach(pin);
    _servo.write(90);

    // corrección sugerida: guardar el pin 
    _pin = pin;

    //-- Initialization of oscilaltor parameters
    _samplingPeriod = 30;
    _period = 3000;  //original:2000
    _numberSamples = _period / _samplingPeriod;
    _inc = 2 * M_PI / _numberSamples;

    _previousMillis = 0;

    //-- Default parameters
    _amplitude = 45;
    _phase = 0;
    _phase0 = 0;
    _offset = 0;
    _stop = false;

    //-- Reverse mode
    _rev = rev;

    // -- corrección sugerida: inicializar variables de control
    _lastCmd = 90;
    _lastMotion = millis();
    _lastUpdate = millis();
  }
}

//-- Detach an oscillator from his servo
void Oscillator::detach() {
  //-- If the oscillator is attached, detach it.
  if (_servo.attached())
    _servo.detach();
}

/*************************************/
/* Set the oscillator period, in ms  */
/*************************************/
void Oscillator::SetT(unsigned int T) {
  //-- Assign the new period
  _period = T;

  //-- Recalculate the parameters
  _numberSamples = _period / _samplingPeriod;
  _inc = 2 * M_PI / _numberSamples;
};

/*******************************/
/* Manual set of the position  */
/******************************/

// version 1 de adaptar posición
// void Oscillator::SetPosition(int position) {
//   _osc_cpp_SetPosition_delay = 1200;  //con 5000 la diferencia de movimiento entre posicion es muy notoria, recomendado: 1000
//   int currentAngle = _servo.read();    // Lee ángulo actual (último comando enviado)
//   int targetAngle = position + _trim;  // Calcula ángulo objetivo con trim

//   if (currentAngle < targetAngle) {
//     // Incrementar gradualmente hacia el objetivo
//     for (int angle = currentAngle; angle <= targetAngle; ++angle) {
//       _servo.write(angle);      // Mueve al siguiente ángulo
//       delayMicroseconds(_osc_cpp_SetPosition_delay);  // Pequeña pausa controla la velocidad
//     }
//   } else if (currentAngle > targetAngle) {
//     // Decrementar gradualmente hacia el objetivo
//     for (int angle = currentAngle; angle >= targetAngle; --angle) {
//       _servo.write(angle);
//       delayMicroseconds(_osc_cpp_SetPosition_delay);
//     }
//   }
//   // Si currentAngle == targetAngle, no hace nada (ya está en posición)

//   Serial.print("osc_cpp_SetPosition_delay!, ");Serial.println(_osc_cpp_SetPosition_delay);
// }
// fin version 1

// Versión previa (bloqueante) comentada en tu código original

void Oscillator::SetPosition(int position) {
  // --- NO BLOQUEANTE + rate limit + anti-spam + auto-detach ---
  unsigned long now = millis();

  // 1) Limitar a 50 Hz por servo
  if ((uint32_t)(now - _lastUpdate) < SERVO_FRAME_MS) return;

  // 2) Calcular comando con trim (coherente con tu SetPosition original)
  int cmd = position + _trim;

  // correccion sugerida:
  cmd = constrain(cmd, 0, 180);



  // Si deseas considerar inversión aquí, descomenta:
  // if (_rev) cmd = 180 - cmd;

  // 3) Evitar escrituras redundantes (tolerancia 1°)
  if (_lastCmd != 255 && abs(cmd - (int)_lastCmd) <= 1) {
    // Sin cambio real: evaluar auto-detach
    if (_servo.attached() && (uint32_t)(now - _lastMotion) > IDLE_DETACH_MS) {
      _servo.detach();
    }
    _lastUpdate = now;
    return;
  }

  // 4) Asegurar attach solo cuando hay que mover
  if (!_servo.attached() && _pin >= 0) {
    _servo.attach(_pin);
  }

  // 5) Escritura directa (la rampa temporal la hace el nivel superior si aplica)
  _servo.write(cmd);
  _lastCmd = (uint8_t)cmd;
  _lastMotion = now;
  _lastUpdate = now;

  // (Si quieres conservar tu debug de delay, puedes imprimir el valor actual,
  // aunque ya no se usa para bloquear)
  // Serial.print("osc_cpp_SetPosition_delay!: ");
  // Serial.println(_osc_cpp_SetPosition_delay);
}

/*******************************************************************/
/* This function should be periodically called                     */
/* in order to maintain the oscillations. It calculates            */
/* if another sample should be taken and position the servo if so  */
/*******************************************************************/
void Oscillator::refresh() {
  //-- Only when TS milliseconds have passed, the new sample is obtained
  if (next_sample()) {
    //-- If the oscillator is not stopped, calculate the servo position
    if (!_stop) {
      //-- Sample the sine function and set the servo pos
      _pos = round(_amplitude * sin(_phase + _phase0) + _offset);
      if (_rev) _pos = -_pos;
      
      //-- CORREGIDO: Aplicar límites y validar
      int servoPos = constrain(_pos + 90 + _trim, 0, 180);
      
      //-- CORREGIDO: Coherencia con anti-spam/auto-detach
      unsigned long now = millis();
      
      // Rate limit check
      if ((uint32_t)(now - _lastUpdate) >= SERVO_FRAME_MS) {
        // Anti-spam: solo escribir si hay cambio significativo
        if (_lastCmd == 255 || abs(servoPos - (int)_lastCmd) > 1) {
          // Re-attach si es necesario
          if (!_servo.attached() && _pin >= 0) {
            _servo.attach(_pin);
          }
          _servo.write(servoPos);
          _lastCmd = (uint8_t)servoPos;
          _lastMotion = now;
        }
        _lastUpdate = now;
      }
    } else {
      // CORREGIDO: Auto-detach cuando está detenido por mucho tiempo
      unsigned long now = millis();
      if (_servo.attached() && (uint32_t)(now - _lastMotion) > IDLE_DETACH_MS) {
        _servo.detach();
      }
    }
    
    //-- Increment the phase
    //-- It is always increased, even when the oscillator is stopped
    //-- so that the coordination is always kept
    // CORREGIDO: Normalizar fase para evitar overflow
    _phase = fmod(_phase + _inc, 2 * M_PI);
  }
}
