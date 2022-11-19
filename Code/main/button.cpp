#include "button.h"

void Button::begin(){
  pinMode(m_pin, INPUT_PULLDOWN);
  m_state = digitalRead(m_pin);
  m_time = millis();
  m_lastState = m_state;
  m_changed = false;
  m_lastChange = m_time;
}

bool Button::read(){
  uint32_t ms = millis();
  bool pinVal = digitalRead(m_pin);
  if (ms - m_lastChange < m_dbTime){
    m_changed = false;
  }
  else{
    m_lastState = m_state;
    m_state = pinVal;
    m_changed = (m_state != m_lastState);
    if (m_changed) { 
      m_lastChange = ms;
      if(m_state) {
        m_released_at = m_pressed_at = ms;
      }
      else {
        m_released_at = ms;
        m_pressed_for = m_released_at - m_pressed_at;
      }
    }
  }
  m_time = ms;
  return m_state;
}

bool Button::pressedFor(uint32_t ms){
  if(m_pressed_for >= ms) {
    m_pressed_for = 0;
    return true;
  }
  else
    return false;
}
