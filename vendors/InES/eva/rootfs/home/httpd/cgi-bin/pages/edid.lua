#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

-- ------------------------------------------------------------------
-- EDID page
-- ------------------------------------------------------------------
function show(t)

    if(t.sent == nil) then
        if(t.mode_vtb) then 
            t.edid_mode_default = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_MODE)
            if(t.edid_mode_default == "default") then
                t.edid_mode_default = 1
                t.edid_mode_receiver = 0
            else
                t.edid_mode_default = 0
                t.edid_mode_receiver = 1
            end
        end
    else
        if(t.mode_vtb) then 
            if((t.edid_mode_default ~= nil) and (t.edid_mode_default == "default")) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_MODE, "default")
	            t.edid_mode_default = 1
	            t.edid_mode_receiver = 0
	        else
	            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_MODE, "receiver")
	            t.edid_mode_default = 0
	            t.edid_mode_receiver = 1 
	        end
        end
    end

    hdoip.html.Header(t, label.page_name .. label.page_edid, script_path)
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_edid)
    hdoip.html.TableHeader(2)

    if(t.mode_vtb) then   
        hdoip.html.FormRadioSingle("edid_mode_default", "receiver", label.p_edid_mode_receiver, t.edid_mode_receiver)       hdoip.html.TableInsElement(2);
        hdoip.html.FormRadioSingle("edid_mode_default", "default", label.p_edid_mode_default, t.edid_mode_default)          hdoip.html.TableInsElement(2);
    else
        hdoip.html.Text(label.p_edid_transmitter_only)                                                                      hdoip.html.TableInsElement(1);
    end

    hdoip.html.TableBottom()
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end


