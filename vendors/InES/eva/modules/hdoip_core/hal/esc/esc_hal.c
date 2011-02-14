
#include "esc_hal.h"

void esc_set_type (void* p, uint32_t type) {
    uint32_t tmp = esc_get_control(p);
    tmp = tmp & ~(ESC_CONTROL_TYPE_MASK << ESC_CONTROL_TYPE_SHIFT);
    tmp = tmp | (type << ESC_CONTROL_TYPE_SHIFT);
    esc_set_control_reg(p, tmp);
}


void esc_set_param (void* p, uint32_t param) {
    uint32_t tmp = esc_get_control(p);
    tmp = tmp & ~(ESC_CONTROL_PARAM_MASK << ESC_CONTROL_PARAM_SHIFT);
    tmp = tmp | (param << ESC_CONTROL_PARAM_SHIFT);
    esc_set_control_reg(p, tmp);
}


void esc_set_data_in (void* p, uint32_t data_in) {
    uint32_t tmp = esc_get_control(p);
    tmp = tmp & ~(ESC_CONTROL_DATA_IN_MASK << ESC_CONTROL_DATA_IN_SHIFT);
    tmp = tmp | (data_in << ESC_CONTROL_DATA_IN_SHIFT);
    esc_set_control_reg(p, tmp);
}


uint32_t esc_get_data_out (void* p) {
    uint32_t tmp = esc_get_status(p);
    tmp = tmp & (ESC_STATUS_DATA_OUT_MASK << ESC_STATUS_DATA_OUT_SHIFT);
    tmp = tmp >> ESC_STATUS_DATA_OUT_SHIFT;
    tmp = tmp & ESC_STATUS_DATA_OUT_MASK;
    return tmp;
}


uint32_t esc_get_link_speed (void* p) {
    uint32_t tmp = esc_get_phy_spec_status_reg(p);
    tmp = tmp >> PHY_SPEC_STATUS_SPEED_SHIFT;
    tmp = tmp & PHY_SPEC_STATUS_SPEED_MASK;
    if (tmp == PHY_SPEC_STATUS_SPEED_1000)
        return 1000;
    if (tmp == PHY_SPEC_STATUS_SPEED_100)
        return 100;
    if (tmp == PHY_SPEC_STATUS_SPEED_10)
        return 10;
    return 0;
}
