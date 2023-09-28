#include "esphome/core/log.h"
#include "ht400_switch.h"

namespace esphome {
  namespace ht400 {

    static const char *TAG = "ht400.switch";

    void HT400Switch::setup() {
    }

    void HT400Switch::write_state(bool state) {
      this->parent_->set_heating_state(state);
      ESP_LOGD(TAG, "Setting switch: %s", ONOFF(state));

      this->publish_state(state);
    }

    void HT400Switch::dump_config() {
      LOG_SWITCH("", "HT400 Heating Switch", this);
    }


    optional<bool> HT400Switch::get_initial_state() {
      return false;
    }

    void HT400Switch::publish_state(bool state) {
      if (!this->publish_dedup_.next(state))
        return;
      this->state = state != this->inverted_;

      ESP_LOGD(TAG, "'%s': Sending state %s", this->name_.c_str(), ONOFF(state));
      this->state_callback_.call(this->state);
    }
  }  // namespace ht400
}  // namespace esphome
