#pragma once
#include "Output.h"

class I2SOutput : public Output {
  private:
    i2s_pin_config_t m_i2s_pins;

  public:
    I2SOutput(i2s_port_t i2s_port, i2s_pin_config_t &i2s_pins);
    void start(int sample_rate);
};
