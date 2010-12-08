#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local REG_DEV_IP = "dev_ip"
local REG_DEV_MAC = "dev_mac"
local REG_DEV_MODE = "dev_mode"
local REG_AUD_IP = "aud_ip"
local REG_AUD_PORT = "aud_port"
local REG_VID_IP = "vid_ip"
local REG_VID_PORT = "vid_port"



local t_rx_tx = {[0] = "Receiver (Source)"; [1] =  "Transmitter (Monitor)";}

local function text2IpValues(str)
    local t = {}
    local pos_start = 0
    local pos_end

    for i = 0, 3, 1 do 
        pos_end = string.find(str, '.', pos_start, true)
        if(pos_end == nil) then
            t[i] = string.sub(str, pos_start, string.len(str))
            break;
        end
        t[i] = string.sub(str, pos_start, pos_end-1)
        pos_start = pos_end + 1 
    end

    return t[0], t[1], t[2], t[3]
end


function redirect(url) 
    local str = "<meta http-equiv=\"refresh\" content=\"5; url=" .. url .. "\">\n"
    hdoip.html.Header("Redirect to " .. url, str)  
    hdoip.html.Bottom("") 
end

function status(t)
    t.dev_ip = hdoip.pipe.getParam(REG_DEV_IP)
    t.rx_tx = hdoip.pipe.getParam(REG_DEV_MODE)
    t.vid_port = hdoip.pipe.getParam(REG_VID_PORT)
    t.vid_ip = hdoip.pipe.getParam(REG_VID_IP)

    hdoip.html.Header(html_side_name .. "Status", t.page)
    hdoip.html.Title("Status")
    hdoip.html.Text(t.vid_port .. "<br>")
    hdoip.html.Text(t.dev_ip .. "<br>")
    hdoip.html.Text(t.rx_tx .. "<br>")
    hdoip.html.Text(t.vid_ip .. "<br>")
    hdoip.html.Bottom(t.err)
end

function eth(t)
    
    if(t.submit == nil) then
        str = hdoip.pipe.getParam(REG_DEV_IP)
        t.dev_ip0, t.dev_ip1, t.dev_ip2, t.dev_ip3 = text2IpValues(str)
        
    end

    hdoip.html.Header(html_side_name .. "Device configuration", t.page)
    hdoip.html.FormHeader()
    hdoip.html.Title("Device configuration");           
    hdoip.html.TableHeader(2)
    hdoip.html.Text("IP");                                                      hdoip.html.TableInsElement(1);
    hdoip.html.FormIP("dev_ip", t.dev_ip0, t.dev_ip1, t.dev_ip2, t.dev_ip3);    hdoip.html.TableInsElement(1);
    hdoip.html.TableBottom()
    
    hdoip.html.FormBottom(t.page)
    hdoip.html.Bottom(t.err)
end

function streaming(t)
    
    if(t.submit == nil) then
        t.rx_tx = hdoip.pipe.getParam(REG_DEV_MODE)
        t.vid_port = hdoip.pipe.getParam(REG_VID_PORT)
        str = hdoip.pipe.getParam(REG_AUD_IP)
        t.vid_ip0, t.vid_ip1, t.vid_ip2, t.vid_ip3 = text2IpValues(str)
    end

    hdoip.html.Header(html_side_name .. "Streaming", t.page)
    
    hdoip.html.FormHeader()
    hdoip.html.Title("Stream configuration");
    hdoip.html.TableHeader(2)
    hdoip.html.FormRadio("rx_tx", t_rx_tx, 2, t.rx_tx)                          hdoip.html.TableInsElement(2);
    hdoip.html.Text("<br>");                                                    hdoip.html.TableInsElement(2);
    hdoip.html.Text("IP from / to");                                            hdoip.html.TableInsElement(1);
    hdoip.html.FormIP("vid_ip", t.vid_ip0, t.vid_ip1, t.vid_ip2, t.vid_ip3);    hdoip.html.TableInsElement(1);
    hdoip.html.Text("UDP Port");                                                hdoip.html.TableInsElement(1);
    hdoip.html.FormText("vid_port", t.vid_port, 4);                             hdoip.html.TableInsElement(1);
    hdoip.html.TableBottom()
    
    hdoip.html.FormBottom(t.page)
    hdoip.html.Bottom(t.err)
end


