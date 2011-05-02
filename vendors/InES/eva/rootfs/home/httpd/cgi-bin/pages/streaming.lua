#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local REG_ST_URI_LABEL = "st_uri"
local REG_ST_RSCP_PORT_LABEL = "st_rscp_port"
local REG_ST_BW_LABEL = "st_bw"
local REG_ST_NET_DELAY_LABEL = "net_delay"
local REG_ST_VID_PORT_LABEL = "vid_port"
local REG_ST_AUD_PORT_LABEL = "aud_port"

local MEDIA_SEL_AUD = "audio"
local MEDIA_SEL_VID = "video"

-- ------------------------------------------------------------------
-- Streamin page
-- ------------------------------------------------------------------
function show(t)

    if(t.sent == nil) then
        local str = hdoip.pipe.getParam(hdoip.pipe.REG_ST_URI)
        t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3 = hdoip.network.text2IpValues(string.sub(str, 8))
        
        t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
        t.aud_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_PORT)
       
        if(t.mode_vtb) then 
            t.st_bw = hdoip.pipe.getParam(hdoip.pipe.REG_ST_BW)
            if(tonumber(t.st_bw) ~= nil) then
                t.st_bw = 8 * tonumber(t.st_bw) / 2^20 -- Bytes/s => MBit/s
            end
        end

        if(t.mode_vrb) then
            if(hdoip.pipe.getParam(hdoip.pipe.REG_AUTO_STREAM) == "true") then
                t.auto_stream = true;
            end
        
	        t.net_delay = hdoip.pipe.getParam(hdoip.pipe.REG_ST_NET_DELAY)
	        t.media_sel = hdoip.pipe.getParam(hdoip.pipe.REG_ST_MODE_MEDIA)
	        
	        if(string.find(t.media_sel, MEDIA_SEL_AUD) ~= nil) then
	            t.cb_audio = 1
	        end
	
	        if(string.find(t.media_sel, MEDIA_SEL_VID) ~= nil) then
	            t.cb_video = 1
	        end
        end

    else
        if(hdoip.network.checkIp(t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3) == 1) then
            local uri = "rscp://"..t.st_uri0.."."..t.st_uri1.."."..t.st_uri2.."."..t.st_uri3
            hdoip.pipe.setParam(hdoip.pipe.REG_ST_URI, uri)
            hdoip.pipe.setParam(hdoip.pipe.REG_ST_HELLO_URI, uri)
        else
            hdoip.html.AddError(t, label.err_ip_not_valid)
        end
 
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
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_BW, (tonumber(t.st_bw) / 8 * 2^20))
            else
                hdoip.html.AddError(t, label.err_datarate_not_number)
            end
        end

        if(t.mode_vrb) then
	        if(tonumber(t.net_delay) ~= nil) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_ST_NET_DELAY, t.net_delay)
	        else
	            hdoip.html.AddError(t, label.err_net_delay_not_number)
	        end 
	
	        t.media = ""
	        if (t.cb_video ~= nil) then
	            t.media = t.media .. "video"
	        end 
	        if (t.cb_audio ~= nil) then
	            if(t.cb_video ~= nil) then
	                t.media = t.media .. " "
	            end
	            t.media = t.media .. "audio" 
	        end
	        hdoip.pipe.setParam(hdoip.pipe.REG_ST_MODE_MEDIA, t.media)
	        
	        if(t.auto_stream ~= nil) then
	           hdoip.pipe.setParam(hdoip.pipe.REG_AUTO_STREAM, "true")
	        else
	           hdoip.pipe.setParam(hdoip.pipe.REG_AUTO_STREAM, "false") 
	        end
        end
        
        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

    hdoip.html.Header(t, label.page_name .. label.page_streaming, script_path)
    
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_streaming);
    hdoip.html.TableHeader(3)
    
    -- Common fields
    hdoip.html.Text(label.p_st_connect);                                                    hdoip.html.TableInsElement(1);
    hdoip.html.FormIP(REG_ST_URI_LABEL, t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3);        hdoip.html.TableInsElement(2);

    hdoip.html.Text(label.p_st_vid_port);                                                   hdoip.html.TableInsElement(1);
    if(t.edit_vid_port ~= nil) then
        hdoip.html.FormText(REG_ST_VID_PORT_LABEL, t.vid_port, 5, 0);                       hdoip.html.TableInsElement(1);
        hdoip.html.FormHidden("save_vid_port", 1)
        hdoip.html.Text(label.u_decimal);                                                   hdoip.html.TableInsElement(1);
    else
        hdoip.html.Text(t.vid_port)                                                         hdoip.html.TableInsElement(1);  
        hdoip.html.FormCheckbox("edit_vid_port", 1, label.button_edit, t.edit_vid_port)     hdoip.html.TableInsElement(1);
    end

    hdoip.html.Text(label.p_st_aud_port);                                                   hdoip.html.TableInsElement(1);
    if(t.edit_aud_port ~= nil) then
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
	    hdoip.html.Text("Auto stream");                                                         hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("auto_stream", 1, "", t.auto_stream)                            hdoip.html.TableInsElement(2);
    end

    hdoip.html.TableBottom()
 
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end


