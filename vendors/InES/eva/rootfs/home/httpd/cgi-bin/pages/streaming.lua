#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local REG_ST_URI_LABEL = "st_uri"
local REG_ST_BW_LABEL = "st_bw"
local REG_ST_TTL_LABEL = "eth_ttl"
local REG_ST_BW_CHROMA_LABEL = "st_bw_chroma"
local REG_ST_NET_DELAY_LABEL = "net_delay"
local REG_ST_AV_DELAY_LABEL = "av_delay"
local REG_ST_OSD_LABEL = "osd_time"
local REG_ST_VID_PORT_LABEL = "vid_port"
local REG_ST_AUD_EMB_PORT_LABEL = "aud_emb_port"
local REG_ST_AUD_BOARD_PORT_LABEL = "aud_board_port"
local REG_ST_RTSP_PORT_LABEL = "rtsp_port"

local MEDIA_SEL_AUD_BOARD = "audio_board"
local MEDIA_SEL_AUD_EMB = "audio_emb"
local MEDIA_SEL_VID = "video"

function reboot(t)
    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    elseif (t.version_label == "riedel") then
        page_name = label.page_name_riedel
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. "Rebooting", script_path)
    hdoip.html.Title("Reboot")
    hdoip.html.Text("Please wait until device is rebooted.")
    hdoip.html.Text("<br>")
    hdoip.html.Loadbar(0, 33, 0)
    hdoip.html.Bottom(t)
    hdoip.pipe.reboot()
    os.exit(0)
end

-- ------------------------------------------------------------------
-- Streamin page
-- ------------------------------------------------------------------
function show(t)

    if(t.sent == nil) then
        t.st_uri = hdoip.pipe.getParam(hdoip.pipe.REG_ST_URI)
        t.st_uri = string.sub(t.st_uri, string.len("rtsp://")+1, string.len(t.st_uri))
        --t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3 = hdoip.network.text2IpValues(string.sub(str, 8))
        
        t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
        t.aud_emb_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_EMB_PORT)
        t.aud_board_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_BOARD_PORT)
        t.rtsp_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_RTSP_PORT)
        t.edit_vid_port = 0
        t.edit_aud_emb_port = 0
        t.edit_aud_board_port = 0
        t.edit_rtsp_port = 0
        t.eth_ttl = hdoip.pipe.getParam(hdoip.pipe.REG_ST_ETH_TTL)

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
            t.st_bw_chroma = hdoip.pipe.getParam(hdoip.pipe.REG_ST_BW_CHROMA)
            if(tonumber(t.st_bw_chroma) ~= nil) then
                t.st_bw_chroma = tonumber(t.st_bw_chroma);
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

            t.fps_divide = hdoip.pipe.getParam(hdoip.pipe.REG_FPS_DIVIDE)
            if(tonumber(t.fps_divide) ~= nil) then
                t.fps_divide = tonumber(t.fps_divide);
            end

            if(hdoip.pipe.getParam(hdoip.pipe.REG_TRAFFIC_SHAPING) == "true") then
                t.traffic_shaping = 1;
            else 
                t.traffic_shaping = 0;
            end
        end

        if(t.mode_vrb) then
        
	        t.net_delay = hdoip.pipe.getParam(hdoip.pipe.REG_ST_NET_DELAY)
            t.av_delay = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AV_DELAY)
	        t.media_sel = hdoip.pipe.getParam(hdoip.pipe.REG_ST_MODE_MEDIA)
	        t.osd_time = hdoip.pipe.getParam(hdoip.pipe.REG_OSD_TIME)

	        if(string.find(t.media_sel, MEDIA_SEL_AUD_BOARD) ~= nil)and(hdoip.pipe.getParam(hdoip.pipe.REG_AUD_BOARD) ~= "none") then
	            t.cb_audio_board = 1
	        else 
	           t.cb_audio_board = 0
	        end

            if(string.find(t.media_sel, MEDIA_SEL_AUD_EMB) ~= nil) then
	            t.cb_audio_emb = 1
	        else 
	           t.cb_audio_emb = 0
	        end
	
	        if(string.find(t.media_sel, MEDIA_SEL_VID) ~= nil) then
	            t.cb_video = 1
            else 
                t.cb_video = 0
	        end

            if(hdoip.pipe.getParam(hdoip.pipe.REG_AUTO_STREAM) == "true") then
                t.auto_stream = 1;
            else 
                t.auto_stream = 0;
            end
        end

    else
        --if(hdoip.network.checkIp(t.st_uri0, t.st_uri1, t.st_uri2, t.st_uri3) == 1) then


        local st_uri_read = hdoip.pipe.getParam(hdoip.pipe.REG_ST_URI)
        if (st_uri_read == nil) then
            st_uri_read = "rtsp://dummy"
        end
        st_uri_read = string.sub(st_uri_read, 8)

        if((t.st_uri ~= nil) and (t.st_uri ~= st_uri_read))  then
            local uri = "rtsp://"..t.st_uri
            hdoip.pipe.setParam(hdoip.pipe.REG_TEMP4, uri)
            pages.restart.show(t)
        end
        
        t.hdcp_force = tonumber(t.hdcp_force)
        
        if(t.hdcp_force ~= nil) then
            t.hdcp_force_str = "true" 
        else
            t.hdcp_force = 0
            t.hdcp_force_str = "false"
        end 
        hdoip.pipe.setParam(hdoip.pipe.REG_FORCE_HDCP, t.hdcp_force_str)

        if(tonumber(t.eth_ttl) ~= nil) then
            t.eth_ttl = tonumber(t.eth_ttl)
            if((t.eth_ttl >= 0) and (t.eth_ttl <= 255)) then
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_ETH_TTL, t.eth_ttl)
            else
                hdoip.html.AddError(t, label.err_ttl_not_in_range)
            end
        end

        if(t.save_aud_emb_port ~= nil) then 
            if(tonumber(t.aud_emb_port) ~= nil) then
                t.aud_emb_port = tonumber(t.aud_emb_port)
                if((t.aud_emb_port >= 0) and (t.aud_emb_port < 65536)) then
                    if ((t.aud_emb_port % 2) == 0) then
                        t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
                        t.vid_port = tonumber(t.vid_port)
                        if (((t.aud_emb_port - t.vid_port) >= 6) or ((t.vid_port - t.aud_emb_port) >= 6))  then
                            t.aud_board_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_BOARD_PORT)
                            t.aud_board_port = tonumber(t.aud_board_port)
                            if (((t.aud_emb_port - t.aud_board_port) >= 2) or ((t.aud_board_port - t.aud_emb_port) >= 6))  then
                                hdoip.pipe.setParam(hdoip.pipe.REG_TEMP1, t.aud_emb_port)
                                pages.restart.show(t)
                            else
                                hdoip.html.AddError(t, label.err_aud_port_cross_aud_port)
                            end
                        else
                            hdoip.html.AddError(t, label.err_aud_port_cross_vid_port)
                        end
                    else
                        hdoip.html.AddError(t, label.err_aud_port_not_even)
                    end
                else
                    hdoip.html.AddError(t, label.err_aud_port_not_in_range)
                end
            else
                hdoip.html.AddError(t, label.err_aud_port_not_number)
            end

        else
            t.aud_emb_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_EMB_PORT)
        end

        if(t.save_aud_board_port ~= nil) then 
            if(tonumber(t.aud_board_port) ~= nil) then
                t.aud_board_port = tonumber(t.aud_board_port)
                if((t.aud_board_port >= 0) and (t.aud_board_port < 65536)) then
                    if ((t.aud_board_port % 2) == 0) then
                        t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
                        t.vid_port = tonumber(t.vid_port)
                        if (((t.aud_board_port - t.vid_port) >= 6) or ((t.vid_port - t.aud_board_port) >= 2)) then
                            t.aud_emb_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_EMB_PORT)
                            t.aud_emb_port = tonumber(t.aud_emb_port)
                            if (((t.aud_board_port - t.aud_emb_port) >= 6) or ((t.aud_emb_port - t.aud_board_port) >= 2)) then
                                hdoip.pipe.setParam(hdoip.pipe.REG_TEMP5, t.aud_board_port)
                                pages.restart.show(t)
                            else
                                hdoip.html.AddError(t, label.err_aud_port_cross_aud_port)
                            end
                        else
                            hdoip.html.AddError(t, label.err_aud_port_cross_vid_port)
                        end
                    else
                        hdoip.html.AddError(t, label.err_aud_port_not_even)
                    end
                else
                    hdoip.html.AddError(t, label.err_aud_port_not_in_range)
                end
            else
                hdoip.html.AddError(t, label.err_aud_port_not_number)
            end
        else
            t.aud_board_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_BOARD_PORT)
        end

        if(t.save_vid_port ~= nil) then 
            if(tonumber(t.vid_port) ~= nil) then
                t.vid_port = tonumber(t.vid_port)
                if((t.vid_port >= 0) and (t.vid_port < 65536)) then
                    if ((t.vid_port % 2) == 0) then
                        t.aud_emb_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_EMB_PORT)
                        t.aud_emb_port = tonumber(t.aud_emb_port)
                        if (((t.aud_emb_port - t.vid_port) >= 6) or ((t.vid_port - t.aud_emb_port) >= 6)) then
                            t.aud_board_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_AUD_BOARD_PORT)
                            t.aud_board_port = tonumber(t.aud_board_port)
                            if (((t.aud_board_port - t.vid_port) >= 6) or ((t.vid_port - t.aud_board_port) >= 2)) then
                                hdoip.pipe.setParam(hdoip.pipe.REG_TEMP2, t.vid_port)
                                pages.restart.show(t)
                            else
                                hdoip.html.AddError(t, label.err_vid_port_cross_aud_port)
                            end
                        else
                            hdoip.html.AddError(t, label.err_vid_port_cross_aud_port)
                        end
                    else
                        hdoip.html.AddError(t, label.err_vid_port_not_even)
                    end
                else
                    hdoip.html.AddError(t, label.err_vid_port_not_in_range)
                end
            else
                hdoip.html.AddError(t, label.err_vid_port_not_number)
            end
        else
            t.vid_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_VID_PORT)
        end

        if(t.save_rtsp_port ~= nil) then 
            if(tonumber(t.rtsp_port) ~= nil) then
                t.rtsp_port = tonumber(t.rtsp_port)
                if((t.rtsp_port >= 0) and (t.rtsp_port < 65536)) then
                    hdoip.pipe.setParam(hdoip.pipe.REG_TEMP3, t.rtsp_port)
                    pages.restart.show(t)
                else
                    hdoip.html.AddError(t, label.err_rtsp_port_not_in_range)
                end
            else
                hdoip.html.AddError(t, label.err_rtsp_port_not_number)
            end
        else
            t.rtsp_port = hdoip.pipe.getParam(hdoip.pipe.REG_ST_RTSP_PORT)
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
            

            if(tonumber(t.st_bw_chroma) ~= nil) then
                if ((tonumber(t.st_bw_chroma) > 100) or (tonumber(t.st_bw_chroma) < 0)) then
                    hdoip.html.AddError(t, label.err_datarate_chroma_not_in_range)
                else
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_BW_CHROMA, tonumber(t.st_bw_chroma))
                end
            else
                hdoip.html.AddError(t, label.err_datarate_not_number)
            end

            if(t.fps_divide ~= nil) then
               hdoip.pipe.setParam(hdoip.pipe.REG_FPS_DIVIDE, t.fps_divide)
            end

            if(t.traffic_shaping ~= nil) then
               hdoip.pipe.setParam(hdoip.pipe.REG_TRAFFIC_SHAPING, "true")
               t.traffic_shaping = 1
            else
               hdoip.pipe.setParam(hdoip.pipe.REG_TRAFFIC_SHAPING, "false")
               t.traffic_shaping = 0 
            end
            
            fec_setting = hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING)
            fec_setting_modified = string.sub(fec_setting, 1, 3) .. t.traffic_shaping .. string.sub(fec_setting, 5, 8) .. t.traffic_shaping .. string.sub(fec_setting, 10, 10)
            if(hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING) ~= fec_setting_modified) then
                hdoip.pipe.setParam(hdoip.pipe.REG_FEC_SETTING, fec_setting_modified)
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

            if(t.auto_stream ~= nil) then
               hdoip.pipe.setParam(hdoip.pipe.REG_AUTO_STREAM, "true")
               t.auto_stream = 1
            else
               hdoip.pipe.setParam(hdoip.pipe.REG_AUTO_STREAM, "false")
               t.auto_stream = 0 
            end            

	        if(tonumber(t.net_delay) ~= nil) then
                if ((tonumber(t.net_delay) > 100) or (tonumber(t.net_delay) < 1)) then
    	            hdoip.html.AddError(t, label.err_net_delay_not_in_range)
                else
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_NET_DELAY, t.net_delay)
                end
	        else
	            hdoip.html.AddError(t, label.err_net_delay_not_number)
	        end 
	
	        if(tonumber(t.av_delay) ~= nil) then
                if ((tonumber(t.av_delay) > 100) or (tonumber(t.av_delay) < -100)) then
    	            hdoip.html.AddError(t, label.err_av_delay_not_in_range)
                else
                    hdoip.pipe.setParam(hdoip.pipe.REG_ST_AV_DELAY, t.av_delay)
                end
	        else
	            hdoip.html.AddError(t, label.err_av_delay_not_number)
	        end 


	        if(tonumber(t.osd_time) ~= nil) then
                if ((tonumber(t.osd_time) > 100) or (tonumber(t.osd_time) < 0)) then
    	            hdoip.html.AddError(t, label.err_osd_time_not_in_range)
                else
                    hdoip.pipe.setParam(hdoip.pipe.REG_OSD_TIME, t.osd_time)
                end
	        else
	            hdoip.html.AddError(t, label.err_osd_time_not_number)
	        end 

	        t.media = ""
	        if (t.cb_video ~= nil) then
	            t.media = t.media .. "video"
	            t.cb_video = 1
	        else 
                t.cb_video = 0
	        end 
	        if (t.cb_audio_board ~= nil) then
	            if(t.cb_video > 0) then
	                t.media = t.media .. " "
	            end
	            t.media = t.media .. "audio_board" 
	            t.cb_audio_board = 1
	        else 
                t.cb_audio_board = 0
	        end
            if (t.cb_audio_emb ~= nil) then
	            if((t.cb_audio_board > 0) or (t.cb_video > 0)) then
	                t.media = t.media .. " "
	            end
	            t.media = t.media .. "audio_emb" 
	            t.cb_audio_emb = 1
	        else 
                t.cb_audio_emb = 0
	        end        
	        hdoip.pipe.setParam(hdoip.pipe.REG_ST_MODE_MEDIA, t.media)
        end

        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

    if(t.button_restart_yes ~= nil) then
        t.aud_emb_port_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP1)
        t.aud_board_port_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP5)
        t.vid_port_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP2)
        t.rtsp_port_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP3)
        t.st_uri_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP4)

        if(t.aud_emb_port_changed ~= nil) then 
            if(t.aud_emb_port_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_AUD_EMB_PORT, t.aud_emb_port_changed)
            end
        end
        if(t.aud_board_port_changed ~= nil) then 
            if(t.aud_board_port_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_AUD_BOARD_PORT, t.aud_board_port_changed)
            end
        end
        if(t.vid_port_changed ~= nil) then
            if(t.vid_port_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_VID_PORT, t.vid_port_changed)
            end
        end
        if(t.rtsp_port_changed ~= nil) then
            if(t.rtsp_port_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_RTSP_PORT, t.rtsp_port_changed)
            end
        end
        if(t.st_uri_changed ~= nil) then
            if(t.st_uri_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_ST_URI, t.st_uri_changed)
            end
        end

        hdoip.pipe.setParam(hdoip.pipe.REG_IDENTIFICATION, "17")
        hdoip.pipe.store_cfg()
        reboot(t)
    end

    if(t.show_restart == nil) then
        if (t.version_label == "rioxo") then
            page_name = label.page_name_rioxo
        elseif (t.version_label == "emcore") then
            page_name = label.page_name_emcore
        elseif (t.version_label == "black box") then
            page_name = label.page_name_black_box
        elseif (t.version_label == "riedel") then
            page_name = label.page_name_riedel
        else
            page_name = ""
        end

        hdoip.html.Header(t, page_name .. label.page_streaming, script_path)    
        hdoip.html.FormHeader(script_path, main_form)
        hdoip.html.Title(label.page_streaming);
        hdoip.html.TableHeader(3)
        
        -- Common fields
        if(t.mode_vtb) then 
            hdoip.html.FormRadioSingle("multicast_en", "false", label.p_st_unicast, t.unicast_en, 0)    hdoip.html.TableInsElement(3);
            hdoip.html.FormRadioSingle("multicast_en", "true", label.p_st_multicast, t.multicast_en, 0) hdoip.html.TableInsElement(3);
            hdoip.html.Text(label.p_st_multicast_group);                                                hdoip.html.TableInsElement(1);
            hdoip.html.FormText("multicast_group", t.multicast_group, 40, 0);                           hdoip.html.TableInsElement(2);
        elseif(t.mode_vrb) then
            hdoip.html.Text(label.p_st_connect);                                                        hdoip.html.TableInsElement(1);
            hdoip.html.FormText(REG_ST_URI_LABEL, t.st_uri, 40, 0);                                     hdoip.html.TableInsElement(2);
        end
        
        hdoip.html.Text(label.p_st_force_hdcp);                                                         hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("hdcp_force", 1, "", t.hdcp_force, 0)                                   hdoip.html.TableInsElement(2);


        if(t.mode_vrb) then
            hdoip.html.Text(label.p_st_vid_port);                                                       hdoip.html.TableInsElement(1);
            if((t.edit_vid_port ~= nil) and (t.edit_vid_port == "1"))then
                hdoip.html.FormText(REG_ST_VID_PORT_LABEL, t.vid_port, 5, 0);                           hdoip.html.TableInsElement(1);
                hdoip.html.FormHidden("save_vid_port", 1)
                hdoip.html.Text(label.u_decimal);                                                       hdoip.html.TableInsElement(1);
            else
                hdoip.html.Text(t.vid_port)                                                             hdoip.html.TableInsElement(1);  
                hdoip.html.FormCheckbox("edit_vid_port", 1, label.button_edit, t.edit_vid_port, 0)      hdoip.html.TableInsElement(1);
            end

            hdoip.html.Text(label.p_st_aud_emb_port);                                                   hdoip.html.TableInsElement(1);
            if((t.edit_aud_emb_port ~= nil) and (t.edit_aud_emb_port == "1")) then
                hdoip.html.FormText(REG_ST_AUD_EMB_PORT_LABEL, t.aud_emb_port, 5, 0);                   hdoip.html.TableInsElement(1); 
                hdoip.html.FormHidden("save_aud_emb_port", 1)
                hdoip.html.Text(label.u_decimal);                                                       hdoip.html.TableInsElement(1);
            else
                hdoip.html.Text(t.aud_emb_port)                                                         hdoip.html.TableInsElement(1); 
                hdoip.html.FormCheckbox("edit_aud_emb_port", 1, label.button_edit, t.edit_aud_emb_port, 0) hdoip.html.TableInsElement(1);
            end

            hdoip.html.Text(label.p_st_aud_board_port);                                                 hdoip.html.TableInsElement(1);
            if((t.edit_aud_board_port ~= nil) and (t.edit_aud_board_port == "1")) then
                hdoip.html.FormText(REG_ST_AUD_BOARD_PORT_LABEL, t.aud_board_port, 5, 0);               hdoip.html.TableInsElement(1); 
                hdoip.html.FormHidden("save_aud_board_port", 1)
                hdoip.html.Text(label.u_decimal);                                                       hdoip.html.TableInsElement(1);
            else
                hdoip.html.Text(t.aud_board_port)                                                       hdoip.html.TableInsElement(1); 
                hdoip.html.FormCheckbox("edit_aud_board_port", 1, label.button_edit, t.edit_aud_board_port, 0) hdoip.html.TableInsElement(1);
            end

        end

        hdoip.html.Text(label.p_st_rtsp_port);                                                          hdoip.html.TableInsElement(1);
        if((t.edit_rtsp_port ~= nil) and (t.edit_rtsp_port == "1")) then
            hdoip.html.FormText(REG_ST_RTSP_PORT_LABEL, t.rtsp_port, 5, 0);                             hdoip.html.TableInsElement(1);
            hdoip.html.FormHidden("save_rtsp_port", 1)
            hdoip.html.Text(label.u_decimal);                                                           hdoip.html.TableInsElement(1);
        else
            hdoip.html.Text(t.rtsp_port)                                                                hdoip.html.TableInsElement(1);
            hdoip.html.FormCheckbox("edit_rtsp_port", 1, label.button_edit, t.edit_rtsp_port,0)         hdoip.html.TableInsElement(1);
        end

        hdoip.html.Text(label.p_st_ttl);                                                                hdoip.html.TableInsElement(1);
        hdoip.html.FormText(REG_ST_TTL_LABEL, t.eth_ttl, 4, 0);
        hdoip.html.Text(label.u_s);                                                                     hdoip.html.TableInsElement(2);

        -- VTB specific fields
        if(t.mode_vtb) then
            hdoip.html.Text(label.p_st_datarate);                                                       hdoip.html.TableInsElement(1);
            hdoip.html.FormText(REG_ST_BW_LABEL, t.st_bw, 4, 0); 
            hdoip.html.Text(label.u_mbps);                                                              hdoip.html.TableInsElement(2);
            hdoip.html.Text(label.p_st_dec_chroma);                                                     hdoip.html.TableInsElement(1);
            hdoip.html.FormText(REG_ST_BW_CHROMA_LABEL, t.st_bw_chroma, 4, 0); 
            hdoip.html.Text(label.u_percent);                                                           hdoip.html.TableInsElement(2);

            hdoip.html.Text(label.p_st_fps_divide);                                                     hdoip.html.TableInsElement(1);
            hdoip.html.DropdownBox4("fps_divide", label.n_none, label.n_1_2, label.n_1_3, label.n_1_4, t.fps_divide)
            hdoip.html.Text(label.p_st_eit_only);                                                       hdoip.html.TableInsElement(2);

            hdoip.html.Text(label.p_st_traffic_shaping);                                                hdoip.html.TableInsElement(1);
            hdoip.html.FormCheckbox("traffic_shaping", 1, "", t.traffic_shaping, 0)                        hdoip.html.TableInsElement(2);
        end

        -- VRB specific fields
        if(t.mode_vrb) then
            hdoip.html.Text(label.p_st_media_sel);                                                      hdoip.html.TableInsElement(1);
            hdoip.html.FormCheckbox("cb_video", 1, label.p_st_video.."<br>", t.cb_video,0)                hdoip.html.TableInsElement(2);
            hdoip.html.Text("");                                                                            hdoip.html.TableInsElement(1);
        	hdoip.html.FormCheckbox("cb_audio_emb", 1, label.p_st_audio_emb.."<br>", t.cb_audio_emb,0)        hdoip.html.TableInsElement(2);
        	hdoip.html.Text("");                                                                            hdoip.html.TableInsElement(1);
            
            if(hdoip.pipe.getParam(hdoip.pipe.REG_AUD_BOARD) == "none") then
                disabled=1
            else 
                disabled=0 
            end
        	hdoip.html.FormCheckbox("cb_audio_board", 1, label.p_st_audio_board.."<br>", t.cb_audio_board,disabled)  hdoip.html.TableInsElement(2);
	        hdoip.html.Text(label.p_st_net_delay);                                                      hdoip.html.TableInsElement(1);
	        hdoip.html.FormText(REG_ST_NET_DELAY_LABEL, t.net_delay, 4, 0); 
	        hdoip.html.Text(label.u_ms);                                                                hdoip.html.TableInsElement(2);

	        hdoip.html.Text(label.p_st_av_delay);                                                       hdoip.html.TableInsElement(1);
	        hdoip.html.FormText(REG_ST_AV_DELAY_LABEL, t.av_delay, 4, 0); 
	        hdoip.html.Text(label.u_ms);                                                                hdoip.html.TableInsElement(2);

            hdoip.html.Text(label.p_osd_time);                                                          hdoip.html.TableInsElement(1);
            hdoip.html.FormText(REG_ST_OSD_LABEL, t.osd_time, 4, 0); 
            hdoip.html.Text(label.u_s);                                                                 hdoip.html.TableInsElement(2);

            hdoip.html.Text(label.p_st_auto_stream);                                                    hdoip.html.TableInsElement(1);
            hdoip.html.FormCheckbox("auto_stream", 1, "", t.auto_stream, 0)                             hdoip.html.TableInsElement(2);
        end

        hdoip.html.TableBottom()
     
        hdoip.html.FormBottom(t)
        hdoip.html.Bottom(t)
    end
end


