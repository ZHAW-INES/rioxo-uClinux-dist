#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local REG_ST_URI_LABEL = "st_uri"
local REG_ST_RSCP_PORT_LABEL = "st_rscp_port"
local REG_ST_BW_LABEL = "st_bw"
local REG_ST_NET_DELAY_LABEL = "net_delay"
local REG_ST_OSD_LABEL = "osd_time"
local REG_ST_VID_PORT_LABEL = "vid_port"
local REG_ST_AUD_PORT_LABEL = "aud_port"

local MEDIA_SEL_AUD = "audio"
local MEDIA_SEL_VID = "video"

-- ------------------------------------------------------------------
-- Streamin page
-- ------------------------------------------------------------------
function show(t)
    local t_osd_mode =      {[0] = (label.p_osd_off); [1] = (label.p_osd_tmp); [2] = (label.p_osd_on);}
    local t_osd_mode_conv = {[0] = "off"; [1] = "tmp"; [2] = "on";}
    local t_osd_mode_reg = {["off"] = 0; ["tmp"] = 1; ["on"] = 2}


    if(t.sent == nil) then
        t.st_uri = hdoip.pipe.getParam(hdoip.pipe.REG_ST_URI)
        t.st_uri = string.sub(t.st_uri, string.len("rscp://")+1, string.len(t.st_uri))
        --t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3 = hdoip.network.text2IpValues(string.sub(str, 8))
        
        t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
        t.aud_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_PORT)
        t.edit_vid_port = 0
        t.edit_aud_port = 0
        
        if(hdoip.pipe.getParam(hdoip.pipe.REG_FORCE_HDCP) == "true") then
            t.hdcp_force = 1
        else 
            t.hdcp_force = 0
        end 
       
        if(t.mode_vtb) then 
            t.st_bw = hdoip.pipe.getParam(hdoip.pipe.REG_ST_BW)
            if(tonumber(t.st_bw) ~= nil) then
                t.st_bw = 8 * tonumber(t.st_bw) / 2^20 -- Bytes/s => MBit/s
            end
            
            t.multicast_en = hdoip.pipe.getParam(hdoip.pipe.REG_MULTICAST_EN)
            if(t.multicast_en == "true") then
                t.multicast_en = 1
                t.unicast_en = 0
            else
                t.multicast_en = 0
                t.unicast_en = 1
            end
            t.multicast_group = hdoip.pipe.getParam(hdoip.pipe.REG_MULTICAST_GROUP)
            
            if(hdoip.pipe.getParam(hdoip.pipe.REG_AUTO_STREAM) == "true") then
                t.auto_stream = 1;
            else 
                t.auto_stream = 0;
            end
        end

        if(t.mode_vrb) then
        
	        t.net_delay = hdoip.pipe.getParam(hdoip.pipe.REG_ST_NET_DELAY)
	        t.media_sel = hdoip.pipe.getParam(hdoip.pipe.REG_ST_MODE_MEDIA)
	        
	        if(string.find(t.media_sel, MEDIA_SEL_AUD) ~= nil) then
	            t.cb_audio = 1
	        else 
	           t.cb_audio = 0
	        end
	
	        if(string.find(t.media_sel, MEDIA_SEL_VID) ~= nil) then
	            t.cb_video = 1
            else 
                t.cb_video = 0
	        end
        end

    else
        --if(hdoip.network.checkIp(t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3) == 1) then
        if(t.st_uri ~= nil) then
            local uri = "rscp://"..t.st_uri
            hdoip.pipe.setParam(hdoip.pipe.REG_ST_URI, uri)
            hdoip.pipe.setParam(hdoip.pipe.REG_ST_HELLO_URI, uri)
        else
            hdoip.html.AddError(t, label.err_ip_not_valid)
        end
        
        t.hdcp_force = tonumber(t.hdcp_force)
        
        if(t.hdcp_force ~= nil) then
            t.hdcp_force_str = "true" 
        else
            t.hdcp_force = 0
            t.hdcp_force_str = "false"
        end 
        hdoip.pipe.setParam(hdoip.pipe.REG_FORCE_HDCP, t.hdcp_force_str)
 
        if(t.save_aud_port ~= nil) then 
            if(tonumber(t.aud_port) ~= nil) then
                t.aud_port = tonumber(t.aud_port)
                if((t.aud_port >= 0) and (t.aud_port < 65536)) then
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_AUD_PORT, t.aud_port)
                else
                    hdoip.html.AddError(t, label.err_aud_port_not_in_range)
                end
            else
                hdoip.html.AddError(t, label.err_aud_port_not_number)
            end

        else
            t.aud_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_PORT)
        end

        if(t.save_vid_port ~= nil) then 
            if(tonumber(t.vid_port) ~= nil) then
                t.vid_port = tonumber(t.vid_port)
                if((t.vid_port >= 0) and (t.vid_port < 65536)) then
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_VID_PORT, t.vid_port)
                else
                    hdoip.html.AddError(t, label.err_vid_port_not_in_range)
                end
            else
                hdoip.html.AddError(t, label.err_vid_port_not_number)
            end
        else
            t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
        end

        if(t.mode_vtb) then
            if(tonumber(t.st_bw) ~= nil) then
                if ((tonumber(t.st_bw) > 800) or (tonumber(t.st_bw) < 1)) then
                    hdoip.html.AddError(t, label.err_datarate_not_in_range)
                else
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_BW, (tonumber(t.st_bw) / 8 * 2^20))
                end
            else
                hdoip.html.AddError(t, label.err_datarate_not_number)
            end
            
            if(t.auto_stream ~= nil) then
               hdoip.pipe.setParam(hdoip.pipe.REG_AUTO_STREAM, "true")
               t.auto_stream = 1
            else
               hdoip.pipe.setParam(hdoip.pipe.REG_AUTO_STREAM, "false")
               t.auto_stream = 0 
            end
            
            if((t.multicast_en ~= nil) and (t.multicast_en == "true")) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_MULTICAST_EN, "true")
	            t.multicast_en = 1
	            t.unicast_en = 0
	        else
	            hdoip.pipe.setParam(hdoip.pipe.REG_MULTICAST_EN, "false")
	            t.multicast_en = 0
	            t.unicast_en = 1 
	        end
	        
	        if(t.multicast_group ~= nil) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_MULTICAST_GROUP, t.multicast_group)
	        end
        end

        if(t.mode_vrb) then
	        if(tonumber(t.net_delay) ~= nil) then
                if ((tonumber(t.net_delay) > 100) or (tonumber(t.net_delay) < 1)) then
    	            hdoip.html.AddError(t, label.err_net_delay_not_in_range)
                else
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_NET_DELAY, t.net_delay)
                end
	        else
	            hdoip.html.AddError(t, label.err_net_delay_not_number)
	        end 
	
	        t.media = ""
	        if (t.cb_video ~= nil) then
	            t.media = t.media .. "video"
	            t.cb_video = 1
	        else 
                t.cb_video = 0
	        end 
	        if (t.cb_audio ~= nil) then
	            if(t.cb_video > 0) then
	                t.media = t.media .. " "
	            end
	            t.media = t.media .. "audio" 
	            t.cb_audio = 1
	        else 
                t.cb_audio = 0
	        end
	        hdoip.pipe.setParam(hdoip.pipe.REG_ST_MODE_MEDIA, t.media)
        end
        
        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

    hdoip.html.Header(t, label.page_name .. label.page_streaming, script_path)
    
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_streaming);
    hdoip.html.TableHeader(3)
    
    -- Common fields
    if(t.mode_vtb) then 
        hdoip.html.FormRadioSingle("multicast_en", "false", label.p_st_unicast, t.unicast_en)       hdoip.html.TableInsElement(3);
        hdoip.html.Text(label.p_st_connect);                                                        hdoip.html.TableInsElement(1);
        hdoip.html.FormText(REG_ST_URI_LABEL, t.st_uri, 40, 0);                                     hdoip.html.TableInsElement(2);
    
        hdoip.html.FormRadioSingle("multicast_en", "true", label.p_st_multicast, t.multicast_en)    hdoip.html.TableInsElement(3);
        hdoip.html.Text(label.p_st_multicast_group);                                                hdoip.html.TableInsElement(1);
        hdoip.html.FormText("multicast_group", t.multicast_group, 40, 0);                           hdoip.html.TableInsElement(2);
    elseif(t.mode_vrb) then
        hdoip.html.Text(label.p_st_connect);                                                        hdoip.html.TableInsElement(1);
        hdoip.html.FormText(REG_ST_URI_LABEL, t.st_uri, 40, 0);                                     hdoip.html.TableInsElement(2);
    end
    
    hdoip.html.Text(label.p_st_force_hdcp);                                                 hdoip.html.TableInsElement(1);
    hdoip.html.FormCheckbox("hdcp_force", 1, "", t.hdcp_force)                              hdoip.html.TableInsElement(2);

    hdoip.html.Text(label.p_st_vid_port);                                                   hdoip.html.TableInsElement(1);
    if((t.edit_vid_port ~= nil) and (t.edit_vid_port == "1"))then
        hdoip.html.FormText(REG_ST_VID_PORT_LABEL, t.vid_port, 5, 0);                       hdoip.html.TableInsElement(1);
        hdoip.html.FormHidden("save_vid_port", 1)
        hdoip.html.Text(label.u_decimal);                                                   hdoip.html.TableInsElement(1);
    else
        hdoip.html.Text(t.vid_port)                                                         hdoip.html.TableInsElement(1);  
        hdoip.html.FormCheckbox("edit_vid_port", 1, label.button_edit, t.edit_vid_port)     hdoip.html.TableInsElement(1);
    end

    hdoip.html.Text(label.p_st_aud_port);                                                   hdoip.html.TableInsElement(1);
    if((t.edit_aud_port ~= nil) and (t.edit_aud_port == "1")) then
        hdoip.html.FormText(REG_ST_AUD_PORT_LABEL, t.aud_port, 5, 0);                       hdoip.html.TableInsElement(1); 
        hdoip.html.FormHidden("save_aud_port", 1)
        hdoip.html.Text(label.u_decimal);                                                   hdoip.html.TableInsElement(1);
    else
        hdoip.html.Text(t.aud_port)                                                         hdoip.html.TableInsElement(1); 
        hdoip.html.FormCheckbox("edit_aud_port", 1, label.button_edit, t.edit_aud_port)     hdoip.html.TableInsElement(1);
    end

    -- VTB specific fields
    if(t.mode_vtb) then
        hdoip.html.Text(label.p_st_datarate);                                               hdoip.html.TableInsElement(1);
        hdoip.html.FormText(REG_ST_BW_LABEL, t.st_bw, 4, 0); 
        hdoip.html.Text(label.u_mbps);                                                      hdoip.html.TableInsElement(2);
        hdoip.html.Text(label.p_st_auto_stream);                                                hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("auto_stream", 1, "", t.auto_stream)                            hdoip.html.TableInsElement(2);
    end

    -- VRB specific fields
    if(t.mode_vrb) then
        hdoip.html.Text(label.p_st_media_sel);                                                  hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("cb_video", 1, label.p_st_video.."<br>", t.cb_video)            hdoip.html.TableInsElement(2);
        hdoip.html.Text("");                                                                    hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("cb_audio", 1, label.p_st_audio.."<br>", t.cb_audio)            hdoip.html.TableInsElement(2);
	    hdoip.html.Text(label.p_st_net_delay);                                                  hdoip.html.TableInsElement(1);
	    hdoip.html.FormText(REG_ST_NET_DELAY_LABEL, t.net_delay, 4, 0); 
	    hdoip.html.Text(label.u_ms);                                                            hdoip.html.TableInsElement(2);
  --      hdoip.html.Text(label.p_osd_time);                                                      hdoip.html.TableInsElement(1);
  --      hdoip.html.FormRadio(REG_ST_OSD_LABEL, t_osd_mode, 3, t.osd_mode)                       hdoip.html.TableInsElement(1);
    end

    hdoip.html.TableBottom()
 
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end


