#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")


-- ------------------------------------------------------------------
-- Restart page
-- ------------------------------------------------------------------
function show(t)

    hdoip.html.Header(t, label.page_name .. label.page_restart, script_path)
    hdoip.html.Title(label.page_restart)

    hdoip.html.Text(label.p_rb_desc)
    hdoip.html.TableHeader(2)

    hdoip.html.OneButtonForm(t, script_path, "button_restart_yes", label.button_yes);        hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "button_restart_no", label.button_no);          hdoip.html.TableInsElement(1);

    hdoip.html.TableBottom()
    hdoip.html.Bottom(t)
end
