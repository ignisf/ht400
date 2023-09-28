from esphome.components import switch
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import CONF_ID
from .. import ht400_ns, CONF_HT400_ID, HT400

DEPENDENCIES = ['ht400']

HT400Switch = ht400_ns.class_('HT400Switch', switch.Switch, cg.Component)

CONFIG_SCHEMA = switch.SWITCH_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(HT400Switch),
    cv.GenerateID(CONF_HT400_ID): cv.use_id(HT400),
}).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield switch.register_switch(var, config)

    paren = yield cg.get_variable(config[CONF_HT400_ID])
    cg.add(var.set_ht400_parent(paren))
