#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")
require("hdoip.download")

-- ------------------------------------------------------------------
-- Factory defaults page
-- ------------------------------------------------------------------
function show(t)
    local ret
   
    if(t.restore ~= nil) then
        hdoip.pipe.factory_default()
        hdoip.pipe.store_cfg()
    end
 
    hdoip.html.Header(t, label.page_name .. label.page_default, script_path)

    hdoip.html.Title(label.page_default);
    hdoip.html.TableHeader(2)

    hdoip.html.Text(label.p_df_question);                                                    hdoip.html.TableInsElement(1);
    hdoip.html.OneButtonForm(t, script_path, "restore", label.button_restore)                hdoip.html.TableInsElement(1);
    
    hdoip.html.TableBottom()

    hdoip.html.Bottom(t)
end


