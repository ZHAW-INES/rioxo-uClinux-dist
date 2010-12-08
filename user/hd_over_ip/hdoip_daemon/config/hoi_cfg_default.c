#include "hdoipd.h"
#include "hoi_cfg_default.h"

void hoi_cfg_default()
{
    reg_set("dev_ip", "192.168.1.200");
    reg_set("dev_mac", "00:15:12:00:00:42");
    reg_set("dev_mode", "0");
    reg_set("aud_ip", "192.168.1.201");
    reg_set("aud_port", "5000");
    reg_set("vid_ip", "192.168.1.202");
    reg_set("vid_port", "6000");
}
