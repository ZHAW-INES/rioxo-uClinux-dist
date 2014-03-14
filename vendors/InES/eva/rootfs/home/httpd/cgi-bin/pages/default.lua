#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

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
    hdoip.html.Loadbar(0, 33, 1)
    hdoip.html.Bottom(t)
    hdoip.pipe.reboot()
    os.exit(0)
end

-- ------------------------------------------------------------------
-- Factory defaults page
-- ------------------------------------------------------------------
function show(t)
    local ret
   
    if(t.restore ~= nil) then
        pages.restart.show(t)
    end
 
    if(t.button_restart_yes ~= nil) then
        hdoip.pipe.factory_default()
        hdoip.pipe.store_cfg()
        reboot(t)
    end

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

    hdoip.html.Header(t, page_name .. label.page_default, script_path)

    hdoip.html.Title(label.page_default);
    hdoip.html.TableHeader(1)

    hdoip.html.Text(label.p_df_question);                                                    hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "restore", label.button_restore)                hdoip.html.TableInsElement(1);
    
    hdoip.html.TableBottom()

    hdoip.html.Bottom(t)
end


