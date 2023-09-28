from esphome.components import sensor
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID, UNIT_CELSIUS, ICON_THERMOMETER
from .. import ht400_ns, CONF_HT400_ID, HT400

DEPENDENCIES = ['ht400']

HT400Sensor = ht400_ns.class_('HT400Sensor', sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    icon=ICON_THERMOMETER,
    accuracy_decimals=1).extend({
    cv.GenerateID(): cv.declare_id(HT400Sensor),
    cv.GenerateID(CONF_HT400_ID): cv.use_id(HT400),
}).extend(cv.polling_component_schema('2s'))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)

    paren = yield cg.get_variable(config[CONF_HT400_ID])
    cg.add(var.set_ht400_parent(paren))
