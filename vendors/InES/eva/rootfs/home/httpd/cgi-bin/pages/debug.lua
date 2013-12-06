#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")
require("hdoip.download")

-- ------------------------------------------------------------------
-- Debug page
-- ------------------------------------------------------------------
function show(t)
    local ret

    if(t.dl_hdoipd0_log ~= nil) then
        ret = hdoip.download.file("/var/log/hdoipd0.log", "hdoipd0.log")
        if(ret == "") then
            return
        end        
        t.err = t.err .. ret 
    end
    
    if(t.dl_hdoipd1_log ~= nil) then
        ret = hdoip.download.file("/var/log/hdoipd1.log", "hdoipd1.log")
        if(ret == "") then
            return
        end        
        t.err = t.err .. ret 
    end

    if(t.dl_rtsp0_log ~= nil) then
        ret = hdoip.download.file("/var/log/hdoipd.rtsp0.log", "rtsp0.log")
        if(ret == "") then
            return
        end       
        t.err = t.err .. ret 
    end
    
    if(t.dl_rtsp1_log ~= nil) then
        ret = hdoip.download.file("/var/log/hdoipd.rtsp1.log", "rtsp1.log")
        if(ret == "") then
            return
        end       
        t.err = t.err .. ret 
    end
    
    if(t.dl_kernel_log ~= nil) then
        os.execute("/bin/busybox dmesg >> /var/log/kernel.log")
        ret = hdoip.download.file("/var/log/kernel.log", "kernel.log")
        os.execute("/bin/busybox rm /var/log/kernel.log")
        if(ret == "") then
            return
        end       
        t.err = t.err .. ret 
    end

    if(t.dl_boa_access_log ~= nil) then
        ret = hdoip.download.file("/var/log/boa.access.log", "boa.access.log")
        if(ret == "") then
            return
        end       
        t.err = t.err .. ret 
    end
    
    if(t.dl_boa_cgi_log ~= nil) then
        ret = hdoip.download.file("/var/log/boa.cgi.log", "boa.cgi.log")
        if(ret == "") then
            return
        end       
        t.err = t.err .. ret 
    end

    if(t.dl_boa_error_log ~= nil) then
        ret = hdoip.download.file("/var/log/boa.error.log", "boa.error.log")
        if(ret == "") then
            return
        end       
        t.err = t.err .. ret 
    end

    t.driver = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_DRIVER)
    t.state = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_STATE)
    t.system_state = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_SYSTEM)
    t.vtb = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_VTB)
    t.vrb = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_VRB)
    t.rsc = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_RSC)
    t.eth = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_ETH)
    t.vso = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_VSO)

    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. "Debug", script_path)
    hdoip.html.Title("Debug")
    
    hdoip.html.TableHeader(2)
    hdoip.html.Text("System status");                                                   hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.system_state);                                                    hdoip.html.TableInsElement(1);
    hdoip.html.Text("Daemon driver");                                                   hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.driver);                                                          hdoip.html.TableInsElement(1);
    hdoip.html.Text("Daemon state");                                                    hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.state);                                                           hdoip.html.TableInsElement(1);
    hdoip.html.Text("VTB state");                                                       hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.vtb);                                                             hdoip.html.TableInsElement(1);
    hdoip.html.Text("VRB state");                                                       hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.vrb);                                                             hdoip.html.TableInsElement(1);
    hdoip.html.Text("RSC state");                                                       hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.rsc);                                                             hdoip.html.TableInsElement(1);
    hdoip.html.Text("ETH status");                                                      hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.eth);                                                             hdoip.html.TableInsElement(1);
    hdoip.html.Text("VSO status");                                                      hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.vso);                                                             hdoip.html.TableInsElement(1);
    hdoip.html.TableBottom()
    
    hdoip.html.Title("<br>Log files");                                                   

    hdoip.html.TableHeader(4)
    hdoip.html.Text("hdoipd0.log");                                                     hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_hdoipd0_log",label.button_dump);       hdoip.html.TableInsElement(1);
    hdoip.html.Text("hdoipd1.log");                                                     hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_hdoipd1_log",label.button_dump);       hdoip.html.TableInsElement(1);
    hdoip.html.Text("rtsp0.log");                                                       hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_rtsp0_log",label.button_dump);         hdoip.html.TableInsElement(1);
    hdoip.html.Text("rtsp1.log");                                                       hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_rtsp1_log",label.button_dump);         hdoip.html.TableInsElement(1);
    hdoip.html.Text("kernel.log");                                                      hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_kernel_log",label.button_dump);        hdoip.html.TableInsElement(1);
    hdoip.html.Text("boa.access.log");                                                  hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_boa_access_log",label.button_dump);    hdoip.html.TableInsElement(1);
    hdoip.html.Text("boa.cgi.log");                                                     hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_boa_cgi_log",label.button_dump);       hdoip.html.TableInsElement(1);
    hdoip.html.Text("boa.error.log");                                                   hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "dl_boa_error_log",label.button_dump);     hdoip.html.TableInsElement(1);
    hdoip.html.TableBottom()

    hdoip.html.Bottom(t)
end


