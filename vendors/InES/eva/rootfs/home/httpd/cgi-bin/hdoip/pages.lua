#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.network")

local t_rx_tx = {[0] = "Receiver (Source)"; [1] =  "Transmitter (Monitor)";}
local dev_ip0, dev_ip1, dev_ip2, dev_ip3 =  hdoip.network.getDevIp()
local t_eth = {dev_ip0 = 0; dev_ip1 = 0; dev_ip2 = 0; dev_ip3 = 0; rx_tx = "0"; 
            vid_ip0 = 0; vid_ip1 = 0; vid_ip2 = 0; vid_ip3 = 0; vid_port = 0;
            aud_ip0 = 0; aud_ip1 = 0; aud_ip2 = 0; aud_ip3 = 0; aud_port = 0;}

function redirect(url) 
    local str = "<meta http-equiv=\"refresh\" content=\"5; url=" .. url .. "\">\n"
    hdoip.html.Header("Redirect to " .. url, str)  
    hdoip.html.Bottom("") 
end

function status(t, proc)
    hdoip.html.Header(html_side_name .. "Status", t.page)
    hdoip.html.Title("Status")
    hdoip.html.TableHeader(2)
    hdoip.html.Text("Video stream out status");                                 hdoip.html.TableInsElement(1);
    hdoip.html.Text("0x" .. tostring(proc["vso status"]));                      hdoip.html.TableInsElement(1);
    hdoip.html.Text("Video stream in status");                                  hdoip.html.TableInsElement(1);
    hdoip.html.Text("0x" .. tostring(proc["vsi status"]));                      hdoip.html.TableInsElement(1);
    hdoip.html.Text("Audio stream out status");                                 hdoip.html.TableInsElement(1);
    hdoip.html.Text("0x" .. tostring(proc["aso status"]));                      hdoip.html.TableInsElement(1);
    hdoip.html.Text("Audio stream in status");                                  hdoip.html.TableInsElement(1);
    hdoip.html.Text("0x" .. tostring(proc["asi status"]));                      hdoip.html.TableInsElement(1);

    hdoip.html.TableBottom()
    hdoip.html.Bottom(t.err)
end

function eth(t)
    
    if(t.submit == nil) then
        local page = t.page
        local err = t.err
        t = t_eth   
        t.page = page
        t.err = err
        t.dev_ip0, t.dev_ip1, t.dev_ip2, t.dev_ip3 = hdoip.network.getDevIp() 
    else
        err = hdoip.network.setDevIp(t.dev_ip0, t.dev_ip1, t.dev_ip2, t.dev_ip3)
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
        local page = t.page
        local err = t.err
        t = t_eth   
        t.err = err
        t.page = page 
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


