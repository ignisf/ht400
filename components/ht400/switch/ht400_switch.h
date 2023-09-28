#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ht400/ht400.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
  namespace ht400 {

    class HT400Switch : public switch_::Switch, public Component {
    public:
      void setup() override;
      void dump_config() override;

      void set_ht400_parent(HT400 *parent) { this->parent_ = parent; }

      optional<bool> get_initial_state();
      void publish_state(bool state);

    protected:
      void write_state(bool state) override;

      HT400 *parent_;
    };

  }  // namespace ht400
}  // namespace esphome
