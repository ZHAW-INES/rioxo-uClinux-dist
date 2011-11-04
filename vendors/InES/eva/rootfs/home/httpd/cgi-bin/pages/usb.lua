#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

REG_MODE_USB_LABEL = "usb_mode"

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
            hdoip.pipe.setParam(hdoip.pipe.REG_MODE_USB, t_mode_conv[tonumber(t.usb_mode)])
            pages.restart.show(t)
        end

        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

    if(t.button_restart_yes ~= nil) then
        hdoip.pipe.reboot()
    end

    hdoip.pipe.getUSB(t)

    hdoip.html.Header(t, label.page_name .. label.page_usb, script_path)
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_usb)
    hdoip.html.TableHeader(2)
  
        hdoip.html.Text(label.p_usb_mode);                                                          hdoip.html.TableInsElement(1);
        hdoip.html.FormRadio(REG_MODE_USB_LABEL, t_mode, 3, t.usb_mode);                            hdoip.html.TableInsElement(1);
        hdoip.html.Text("");                                                                        hdoip.html.TableInsElement(1);
        hdoip.html.Text(label.p_usb_reboot_note);                                                   hdoip.html.TableInsElement(1);

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


