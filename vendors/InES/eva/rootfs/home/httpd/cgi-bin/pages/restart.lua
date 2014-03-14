#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")


-- ------------------------------------------------------------------
-- Restart page
-- ------------------------------------------------------------------
function show(t)

    t.show_restart = 1

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

    hdoip.html.Header(t, page_name .. label.page_restart, script_path)
    hdoip.html.Title(label.page_restart)

    hdoip.html.Text(label.p_rb_desc)
    hdoip.html.CarriageReturn()
    hdoip.html.TableHeader(2)

    hdoip.html.OneButtonForm(t, script_path, "button_restart_no", label.button_cancel);             hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "button_restart_yes", label.button_save_and_reboot);   hdoip.html.TableInsElement(1);

    hdoip.html.TableBottom()
    hdoip.html.Bottom(t)
end
