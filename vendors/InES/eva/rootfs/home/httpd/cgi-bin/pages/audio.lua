#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local AUD_SOURCE = "aud_source"
local AUD_HPGAIN = "aud_hpgain"
local AUD_LINEGAIN = "aud_linegain"
local AUD_MIC_BOOST = "aud_mic_boost"

local AUD_MIC = "mic"
local AUD_LINEIN = "linein"

local AUD_LINEGAIN_MIN = 0;
local AUD_LINEGAIN_MAX = 100;
local AUD_HPGAIN_MIN = 0;
local AUD_HPGAIN_MAX = 100;

function reboot(t)

    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
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

    local t_aud_source =      {[0] = (label.p_aud_mic);   [1] = (label.p_aud_line_in);}
    local t_aud_source_conv = {[0] = AUD_MIC;             [1] = AUD_LINEIN;}
    local t_aud_source_reg =  {[AUD_MIC] = 0;             [AUD_LINEIN] = 1;}


    if(t.sent == nil) then
--load data from register

        --if(t.mode_vrb) then
--audio hpgain
            t.aud_hpgain = hdoip.pipe.getParam(hdoip.pipe.REG_AUD_HPGAIN)

--audio mute
--            t.aud_mute = tonumber(hdoip.pipe.getParam(hdoip.pipe.REG_AUD_MUTE));
        --end
        
        --if(t.mode_vtb) then
--audio source select
            t.aud_source = t_aud_source_reg[hdoip.pipe.getParam(hdoip.pipe.REG_AUD_SOURCE)]

--audio linegain
            t.aud_linegain = hdoip.pipe.getParam(hdoip.pipe.REG_AUD_LINEGAIN)

--audio mic boost
            t.aud_mic_boost = hdoip.pipe.getParam(hdoip.pipe.REG_AUD_MIC_BOOST);            
        --end

    else
--load data from interface
        --if(t.mode_vrb) then
--audio hpgain
            if(tonumber(t.aud_hpgain)~=nil and tonumber(t.aud_hpgain)<=AUD_HPGAIN_MAX and tonumber(t.aud_hpgain)>=AUD_HPGAIN_MIN) then
                hdoip.pipe.setParam(hdoip.pipe.REG_AUD_HPGAIN, t.aud_hpgain)
            else
                t_aud_hpgain_warning = 1
                t.aud_hpgain = hdoip.pipe.getParam(hdoip.pipe.REG_AUD_HPGAIN)
            end
        --end

        --if(t.mode_vtb) then
--audio source select            
            if(hdoip.pipe.getParam(hdoip.pipe.REG_AUD_SOURCE) ~= t_aud_source_conv[tonumber(t.aud_source)]) then
                hdoip.pipe.setParam(hdoip.pipe.REG_AUD_SOURCE, t_aud_source_conv[tonumber(t.aud_source)])
                reboot_flag = 1
            end

--audio linegain
            if(tonumber(t.aud_linegain)~=nil and tonumber(t.aud_linegain)<=AUD_LINEGAIN_MAX and tonumber(t.aud_linegain)>=AUD_LINEGAIN_MIN) then
                hdoip.pipe.setParam(hdoip.pipe.REG_AUD_LINEGAIN, t.aud_linegain)
            else
                t_aud_linegain_warning = 1
                t.aud_linegain = hdoip.pipe.getParam(hdoip.pipe.REG_AUD_LINEGAIN)
            end

--audio mic boost
            if(tonumber(t.aud_mic_boost)==20) then
                hdoip.pipe.setParam(hdoip.pipe.REG_AUD_MIC_BOOST, "20")
            else
                hdoip.pipe.setParam(hdoip.pipe.REG_AUD_MIC_BOOST, "0")
            end
        --end

        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

--start to visualise page
    if(t.button_restart_yes ~= nil) then
        reboot(t)
    end

    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. label.page_audio, script_path)
    
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_audio);
    hdoip.html.TableHeader(2)

-- Common fields

-- VTB specific fields
    --if(t.mode_vtb) then
        hdoip.html.Text(label.p_aud_source_sel);                                                        hdoip.html.TableInsElement(1);
        hdoip.html.FormRadio(AUD_SOURCE, t_aud_source, 2, t.aud_source);                                hdoip.html.TableInsElement(1);

        if(t_aud_linegain_warning==1) then
            hdoip.html.Text(label.p_aud_linegain_warning);                                              hdoip.html.TableInsElement(2);
            t_aud_linegain_warning = 0;
        end
        hdoip.html.Text(label.p_aud_linegain);                                                          hdoip.html.TableInsElement(1);
        --hdoip.html.FormText(AUD_LINEGAIN, t.aud_linegain, 3, 0);                                        hdoip.html.TableInsElement(1);
        hdoip.html.FormSlide(AUD_LINEGAIN, AUD_LINEGAIN_MIN, AUD_LINEGAIN_MAX, 1, 3, t.aud_linegain)
                                                                                                        hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_aud_mic_label);                                                         hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox(AUD_MIC_BOOST, 20, label.p_aud_mic_boost, tonumber(t.aud_mic_boost),0); hdoip.html.TableInsElement(1);
    --end

-- VRB specific fields
    --if(t.mode_vrb) then
        if(t_aud_hpgain_warning==1) then
                hdoip.html.Text(label.p_aud_hpgain_warning);                                            hdoip.html.TableInsElement(2);
                t_aud_hpgain_warning = 0;
        end
        hdoip.html.Text(label.p_aud_hpgain);                                                            hdoip.html.TableInsElement(1);
        --hdoip.html.FormText(AUD_HPGAIN, t.aud_hpgain, 3, 0);                                            hdoip.html.TableInsElement(1);
        hdoip.html.FormSlide(AUD_HPGAIN, AUD_HPGAIN_MIN, AUD_HPGAIN_MAX, 1, 3, t.aud_hpgain)
                                                                                                        hdoip.html.TableInsElement(1);
    --end

    hdoip.html.TableBottom()
 
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end


