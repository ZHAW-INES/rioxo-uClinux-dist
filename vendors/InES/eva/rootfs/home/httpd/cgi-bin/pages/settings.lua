#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")
require("hdoip.download")

-- ------------------------------------------------------------------
-- Status page
-- ------------------------------------------------------------------
function show(t)
    local lang_cnt = 2
    local t_lang        = {[0] = (label.lang_en); [1] = (label.lang_de);}
    local t_lang_conv   = {[0] = "EN"; [1] = "DE";}
    local t_lang_short  = {["EN"] = 0; ["DE"] = 1;}
    local t_auth_en     = {[0] = label.off; [1] = label.on;}
    local t_amx_en     = {[0] = label.off; [1] = label.on;}

    if(t.sent ~= nil) then
        t.lang_sel = tonumber(t.lang_sel)
        t.auth_en_checkbox = tonumber(t.auth_en_sel)
        t.amx_en_checkbox = tonumber(t.amx_en_sel)

        hdoip.pipe.setParam(hdoip.pipe.REG_WEB_LANG, t_lang_conv[t.lang_sel])
           
        if(t.auth_en_checkbox > 0) then
            t.auth_en_str = "true"
            t.auth_en = true
        else 
            t.auth_en_str = "false"
            t.auth_en = false
        end
        hdoip.pipe.setParam(hdoip.pipe.REG_WEB_AUTH_EN, t.auth_en_str)
        
        if(t.amx_en_checkbox > 0) then
            t.amx_en_str = "true"
            t.amx_en = true
        else
            t.amx_en_str = "false"
            t.amx_en = false
        end
        hdoip.pipe.setParam(hdoip.pipe.REG_AMX_EN, t.amx_en_str)       

        if(t.auth_en) then
	        if(t.web_user == nil)then
	            t.web_user = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_USER)
	        end
	        
	        if((t.pass_old ~= "") and (t.new_pass ~= nil) and (t.new_user ~= nil))then
	            if(t.pass_old == t.web_pass) then
	                if((t.new_pass == t.new_pass2) and (t.new_pass ~= ""))then
	                    hdoip.pipe.setParam(hdoip.pipe.REG_WEB_PASS, t.new_pass)
	                    hdoip.pipe.setParam(hdoip.pipe.REG_WEB_USER, t.new_user)
	                    hdoip.cookie.delete(t, "username", {})
	                    hdoip.cookie.delete(t, "password", {})
	                else
	                    hdoip.html.AddError(t, label.p_set_err_new_password)
	                end
	            else
	                hdoip.html.AddError(t, label.p_set_err_old_password)
	            end
	        end
        end
        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    else 
        if(t.auth_en) then
            t.auth_en_checkbox = 1
        else 
            t.auth_en_checkbox = 0
        end
        
        t.amx_en = hdoip.pipe.getParam(hdoip.pipe.REG_AMX_EN)
        if(t.amx_en == "true") then
            t.amx_en_checkbox = 1
        else 
            t.amx_en_checkbox = 0
        end 
        
        t.lang_sel = t_lang_short[t.lang]
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

    hdoip.html.Header(t, page_name .. label.page_settings, script_path)

    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.p_set_lang);                              
    hdoip.html.FormRadio("lang_sel", t_lang, lang_cnt, t.lang_sel);
    hdoip.html.Title("AMX");                              
    hdoip.html.FormRadio("amx_en_sel", t_amx_en, 2, t.amx_en_checkbox);           
    
    hdoip.html.Title(label.p_set_auth);                              
    hdoip.html.FormRadio("auth_en_sel", t_auth_en, 2, t.auth_en_checkbox);   
    
    if(t.auth_en) then
	    hdoip.html.Title(label.p_set_change_user);                              
	    hdoip.html.TableHeader(2)
	    hdoip.html.Text(label.username);                        hdoip.html.TableInsElement(1);
	    hdoip.html.FormText("new_user", t.web_user, 30, 0)      hdoip.html.TableInsElement(1);
	    hdoip.html.Text(label.p_set_old_password);              hdoip.html.TableInsElement(1);
	    hdoip.html.FormPassword("pass_old", "", 30, 0)          hdoip.html.TableInsElement(1);
	    hdoip.html.Text(label.p_set_new_password);              hdoip.html.TableInsElement(1);
	    hdoip.html.FormPassword("new_pass", "", 30, 0)          hdoip.html.TableInsElement(1);
	    hdoip.html.Text(label.p_set_new_password_retype);       hdoip.html.TableInsElement(1);
	    hdoip.html.FormPassword("new_pass2", "", 30, 0)         hdoip.html.TableInsElement(1);
	    hdoip.html.TableBottom()
    end
 
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end


