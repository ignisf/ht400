#include "esphome/core/log.h"
#include "ht400_sensor.h"

namespace esphome {
namespace ht400 {

static const char *TAG = "ht400.sensor";

void HT400Sensor::setup() {}

void HT400Sensor::dump_config() {
  LOG_SENSOR("", "HT400 Sensor", this);
}

void HT400Sensor::update() {
  publish_state(this->parent_->get_current_temperature());
}

}  // namespace ht400
}  // namespace esphome
