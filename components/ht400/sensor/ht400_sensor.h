#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ht400/ht400.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
  namespace ht400 {

    class HT400Sensor : public sensor::Sensor, public PollingComponent {
    public:
      void setup() override;
      void dump_config() override;

      void set_ht400_parent(HT400 *parent) { this->parent_ = parent; }
      void update() override;
      
    protected:
      HT400 *parent_;
    };

  }  // namespace ht400
}  // namespace esphome
