#include "ht400.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ht400 {

static const char *TAG = "ht400";
static const int COMMAND_DELAY = 50;
static const char SEPARATOR = '#';
static const uint32_t POLLING_INTERVAL = 2000; /* millis */
static const uint32_t HEATING_TX_INTERVAL = 1 * 60 * 1000; /* millis */

void HT400::setup() {
  this->last_heating_state_update_millis_ = millis() - HEATING_TX_INTERVAL;
}

void HT400::loop() {
  while(this->available()) {
    uint8_t c;
    this->read_byte(&c);
    this->handle_char_(c);
  }

  if (!this->can_perform_action_()) {
    return;
  }

  if (this->init_state_ == HT400InitState::IDLE) {
    if (this->can_update_heating_state_()) {
      this->request_heating_state_update_();
    } else {
      this->request_state_report_();
    }
  } else if (this->init_state_ == HT400InitState::AWAITING_STATE) {
    this->request_state_report_();
  } else if (this->init_state_== HT400InitState::PAIRING_REQUESTED) {
    this->request_modem_pairing_();
  }
}

void HT400::set_heating_state(bool state) {
  this->heating_state_ = state;
  this->last_heating_state_update_millis_ = millis() - HEATING_TX_INTERVAL;
}

float HT400::get_current_temperature() {
  return this->current_temperature_;
}

void HT400::pair() {
  ESP_LOGD(TAG, "Pairing requested");
  this->set_init_state_(HT400InitState::PAIRING_REQUESTED);
}

void HT400::action_performed_() {
  this->last_action_request_millis_ = millis();
}

bool HT400::can_perform_action_() {
  return millis() - this->last_action_request_millis_ > POLLING_INTERVAL;
}

bool HT400::can_update_heating_state_() {
  ESP_LOGVV(TAG, "%d", millis() - this->last_heating_state_update_millis_);
  return millis() - this->last_heating_state_update_millis_ > HEATING_TX_INTERVAL;
}

void HT400::dump_config() {
  ESP_LOGCONFIG(TAG, "HT400:");
  this->check_uart_settings(9600);
}

void HT400::handle_char_(uint8_t c) {
  if (c == SEPARATOR) {
    this->handle_message_();
  } else {
    this->rx_message_.push_back(c);
  }
}

void HT400::set_init_state_(HT400InitState new_state) {
  this->init_state_ = new_state;
  ESP_LOGD(TAG, "HT400 Init State changed to %d", new_state);
}

void HT400::send_message_(const char* message) {
  ESP_LOGD(TAG, "ESP->MCU: %s", message);
  this->action_performed_();
  this->write_str(message);
}

void HT400::set_current_temperature_(float temperature) {
  this->current_temperature_ = temperature;
  // ESP_LOGD(TAG, "New temperature: %f", temperature);
}

void HT400::handle_handshake_request_(std::string &message) {
  if (message.compare("HelloESP") == 0) {
    this->set_init_state_(HT400InitState::WIFI_CONFIG);
    this->send_message_("HelloArduino#");
  } else {
    ESP_LOGE(TAG, "Unexpected message for state %d: %s", this->init_state_, message.c_str());
  }
}

void HT400::handle_state_report_(std::string &message) {
  /*
    Example message:
    TEMP:24.25RF:0
   */
  if (message.substr(0, 5).compare("TEMP:") == 0 &&
      message.substr(10, 3).compare("RF:") == 0) {
    this->send_message_("SUCCESS#");
    this->send_message_("ESPISALIVE#");
    this->set_init_state_(HT400InitState::IDLE);
    this->handle_temperature_update_(message.substr(5, 5));
    this->handle_modem_pairing_request_(message.substr(13, 1));
  } else {
    this->set_init_state_(HT400InitState::IDLE);
    ESP_LOGE(TAG, "Unexpected message for state %d: %s", this->init_state_, message.c_str());
  }
}

void HT400::handle_temperature_update_(std::string temperature) {
  auto maybe_temperature = parse_number<float>(temperature);
  if (maybe_temperature.has_value()) {
    this->current_temperature_ = maybe_temperature.value();
     ESP_LOGD(TAG, "Temperature updated to: %f", this->current_temperature_);
  } else {
    ESP_LOGE(TAG, "Invalid temperature: %s", temperature.c_str());
  }
}

void HT400::handle_modem_pairing_request_(std::string rf_pairing_requested) {
  if (rf_pairing_requested.compare("1") == 0) {
    this->set_init_state_(HT400InitState::PAIRING_REQUESTED);
  }
  ESP_LOGD(TAG, "RF Pairing: %s", rf_pairing_requested.c_str());
}

void HT400::request_modem_pairing_() {
  this->set_init_state_(HT400InitState::AWAITING_ACK);
  this->send_message_("RF_PAIRING#");
}

void HT400::handle_ack_(std::string &message) {
  if(message.compare("SUCCESS") == 0) {
    this->set_init_state_(HT400InitState::IDLE);
    ESP_LOGD(TAG, "Acknowledgement received");
  } else {
    this->set_init_state_(HT400InitState::IDLE);
    ESP_LOGE(TAG, "Unexpected message for state %d: %s", message.c_str());
  }
}

void HT400::handle_wifi_config_(std::string &message) {
  /*
    Example message:
    id=foo bar&pw=baz&uid=FEB59FZkkJ8&rid=DAFD&rfp=1
  */
  if (str_startswith(message, "id=")) {
    this->set_init_state_(HT400InitState::IDLE);
    this->send_message_("SUCCESS#");
  } else {
    this->set_init_state_(HT400InitState::HANDSHAKE);
    ESP_LOGE(TAG, "Unexpected message for state %d: %s", this->init_state_, message.c_str());
  }
}

void HT400::handle_message_() {
  std::string message(this->rx_message_.begin(), this->rx_message_.end());
  ESP_LOGD(TAG, "MCU->ESP: %s", message.c_str());

  if (this->init_state_ == HT400InitState::HANDSHAKE) {
    this->handle_handshake_request_(message);
  } else if (this->init_state_ == HT400InitState::WIFI_CONFIG) {
    this->handle_wifi_config_(message);
  } else if (this->init_state_ == HT400InitState::IDLE) {
    ESP_LOGW(TAG, "Unexpected message for state %d: %s", this->init_state_, message.c_str());
  } else if (this->init_state_ == HT400InitState::AWAITING_STATE) {
    this->handle_state_report_(message);
  } else if (this->init_state_ == HT400InitState::AWAITING_ACK) {
    this->handle_ack_(message);
  }
  this->rx_message_.clear();
}

void HT400::request_state_report_() {
  this->set_init_state_(HT400InitState::AWAITING_STATE);
  this->send_message_("LOOPDATA#");
}

void HT400::request_heating_state_update_() {
  this->last_heating_state_update_millis_ = millis();
  if (this->heating_state_) {
    this->send_message_("RF_MSG U_now:0S:1#");
  } else {
    this->send_message_("RF_MSG U_now:0S:2#");
  }
}

}  // namespace ht400
}  // namespace esphome
