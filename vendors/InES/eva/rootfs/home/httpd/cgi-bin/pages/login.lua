#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.cookie")

-- ------------------------------------------------------------------
-- Status page
-- ------------------------------------------------------------------
function show(t)
    local login = 0
 
    if(t.submit ~= nil) then
        if((t.in_web_user == t.web_user) and (t.in_web_pass == t.web_pass)) then
            hdoip.cookie.set(t, "username", t.web_user, {})        
            hdoip.cookie.set(t, "password", t.web_pass, {})       
            login = 1 
        else 
            hdoip.html.AddError(t, label.p_lg_wrong)
        end
    end
 
    hdoip.html.Header(t, label.page_name .. label.page_login, script_path)
    hdoip.html.Title(label.page_login)

    if(login == 1) then
        hdoip.html.Text(label.p_lg_success)
    else 
        hdoip.html.TableHeader(2)
        hdoip.html.FormHeader(script_path)
    
        hdoip.html.Text(label.username);                        hdoip.html.TableInsElement(1);
        hdoip.html.FormText("in_web_user", "", 30, 0)           hdoip.html.TableInsElement(1);
        hdoip.html.Text(label.password);                        hdoip.html.TableInsElement(1);
        hdoip.html.FormPassword("in_web_pass", "", 30, 0)       hdoip.html.TableInsElement(1);
        hdoip.html.FormButton("submit", label.button_login)     hdoip.html.TableInsElement(2);
        hdoip.html.TableBottom()
    
        hdoip.html.FormEnd(t)
    end

    hdoip.html.Bottom(t)
end


