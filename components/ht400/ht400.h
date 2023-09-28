#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace ht400 {

enum class HT400InitState : char {
  HANDSHAKE,
  WIFI_CONFIG,
  IDLE,
  AWAITING_STATE,
  PAIRING_REQUESTED,
  AWAITING_ACK
};

class HT400 : public Component, public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::LATE; }
  void setup() override;
  void loop() override;
  void dump_config() override;

  float get_current_temperature();
  void set_heating_state(bool state);
  void set_led_state(uint8_t state);
  void pair();

 protected:
  void handle_char_(uint8_t c);
  void handle_message_();
  bool validate_wifi_config_(std::string &message);

  void handle_handshake_request_(std::string &message);
  void handle_wifi_config_(std::string &message);
  void handle_ack_(std::string &message);
  void handle_state_report_(std::string &message);
  void handle_temperature_update_(std::string temperature);
  void handle_modem_pairing_request_(std::string rf_pairing_requested);

  void request_state_report_();
  void request_modem_pairing_();
  void request_heating_state_update_();/*  RF_MSG U_now:1S:2# */
  // void request_heating_on_(); /*  RF_MSG U_now:1S:1#  */

  void set_init_state_(HT400InitState new_state);
  void send_message_(const char* message);
  void set_current_temperature_(float temperature);

  bool can_perform_action_();
  bool can_update_heating_state_();

  void action_performed_();

  HT400InitState init_state_ = HT400InitState::HANDSHAKE;
  std::vector<uint8_t> rx_message_;
  float current_temperature_ = 256.256;
  bool heating_state_ = false;
  uint32_t last_action_request_millis_ = 0;
  uint32_t last_heating_state_update_millis_ = 0;
};

}  // namespace ht400
}  // namespace esphome
