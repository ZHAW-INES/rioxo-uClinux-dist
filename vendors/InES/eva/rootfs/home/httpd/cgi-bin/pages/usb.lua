#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

REG_MODE_USB_LABEL = "usb_mode"

function reboot(t)
    hdoip.html.Header(t, label.page_name .. "Rebooting", script_path)
    hdoip.html.Title("Reboot")
    hdoip.html.Text("Please wait until device is rebooted.")
    hdoip.html.Text("<br>")
    hdoip.html.Loadbar(0, 30)
    hdoip.html.Bottom(t)
    hdoip.pipe.reboot()
    os.exit(0)
end

-- ------------------------------------------------------------------
-- USB page
-- ------------------------------------------------------------------
function show(t)
    local t_mode =      {[0] = (label.p_usb_host); [1] = (label.p_usb_device); [2] = (label.p_usb_off);}
    local t_mode_conv = {[0] = "host"; [1] = "device"; [2] = "off";}
    local t_mode_reg =  {["host"] = 0; ["device"] = 1; ["off"] = 2;}

    if(t.sent == nil) then
        t.usb_mode = t_mode_reg[hdoip.pipe.getParam(hdoip.pipe.REG_MODE_USB)]
    else
        if(hdoip.pipe.getParam(hdoip.pipe.REG_MODE_USB) ~= t_mode_conv[tonumber(t.usb_mode)]) then
            hdoip.pipe.setParam(hdoip.pipe.REG_TEMP6, t_mode_conv[tonumber(t.usb_mode)])
            pages.restart.show(t)
        end

        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

    if(t.button_restart_yes ~= nil) then
        t.usb_mode_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP6)

        if(t.usb_mode_changed ~= nil) then
            if(t.usb_mode_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_MODE_USB, t.usb_mode_changed)
            end
        end

        hdoip.pipe.setParam(hdoip.pipe.REG_IDENTIFICATION, "17")
        hdoip.pipe.store_cfg()
        reboot(t)
    end

    hdoip.pipe.getUSB(t)

    if(t.show_restart == nil) then
        hdoip.html.Header(t, label.page_name .. label.page_usb, script_path)
        hdoip.html.FormHeader(script_path, main_form)
        hdoip.html.Title(label.page_usb)
        hdoip.html.TableHeader(2)
      
        hdoip.html.Text(label.p_usb_mode);                                                          hdoip.html.TableInsElement(1);
        hdoip.html.FormRadio(REG_MODE_USB_LABEL, t_mode, 3, t.usb_mode);                            hdoip.html.TableInsElement(1);

        if(t.usb_mode == 0) then
            hdoip.html.Text("<br>"..label.p_usb_label_device);                                      hdoip.html.TableInsElement(1);
            hdoip.html.Text("<br>"..t.usb_vendor);                                                  hdoip.html.TableInsElement(1);
            hdoip.html.Text("");                                                                    hdoip.html.TableInsElement(1);
            hdoip.html.Text(t.usb_product);                                                         hdoip.html.TableInsElement(1);
        end

        hdoip.html.TableBottom()
        hdoip.html.FormBottom(t)
        hdoip.html.Bottom(t)
    end
end


