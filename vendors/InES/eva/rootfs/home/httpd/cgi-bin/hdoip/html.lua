#!/usr/bin/lua
module (..., package.seeall)

require ("hdoip.pipe")

local html_css_file = "/main.css"
local html_css_ie = "/ieonly.css"
local html_favicon = "/favicon.ico"
local html_str = "<b>html error</b>"
local html_table = 0
local html_table_str = "<b>table error</b>"
local html_table_backup = ""
local html_table_col = 0
local html_table_col_cnt = 0
 
function escape(str)
    str = string.gsub(str, "\n", "\r\n")
    str = string.gsub(str, "([^0-9a-zA-Z ])", function (c) return string.format("%%%02X", string.byte(c)) end)
    str = string.gsub(str, " ", "+")
    return str
end

function unescape(str)
    if(str ~= nil) then
        str = string.gsub(str, "+", " ")
        str = string.gsub(str, "%%(%x%x)", function(h) return string.char(tonumber(h, 16)) end)
        str = string.gsub(str, "\r\n", "\n")
        return str
    end
    return ""
end

local function extractArg(t, s)
    local sep_pos = string.find(s, "=")
    local arg_value = string.sub(s, sep_pos+1, string.len(s))
    local arg_name = string.sub(s, 0, sep_pos-1)
    t[arg_name] = arg_value
end
    
function getQueryData(query_string)
    local arg_str
    local arg_start = 0
    local arg_end = 0
    local t= {}
   
    query_string = unescape(query_string)
     
    while(arg_end ~= nil) do
        arg_end = string.find(query_string, "&", arg_start)
        if(arg_end ~= nil) then
            arg_str = string.sub(query_string, arg_start, arg_end-1)
            extractArg(t, arg_str)
            arg_start = arg_end + 1	
        end
    end
    arg_str = string.sub(query_string, arg_start, string.len(query_string))
    extractArg(t, arg_str)
    return t
end

function Text(str)
    if(str ~= nil) then
        html_str = html_str .. str .. "\n"
    end
end

function Title(str)
    if(str ~= nil) then
        html_str = html_str .. '<h2 class="contentheading">' .. str .. '</h2>\n'
    end
end

function Image(path, tooltip) 
    if(tooltip == nil) then
        tooltip = ""
    end
    if(path ~= nil) then
        html_str = html_str .. "<img src=\""..path.."\" alt=\""..path.."\" title=\"".. tooltip .."\">\n"
    end
end

function ImageLink(dest, img_path, tooltip)
    if(dest ~= nil) then
        html_str = html_str .. "<a href=\""..dest.."\">"
        Image(img_path, tooltip)
        html_str = html_str .. "</a>\n"
    end
end

function Link(dest, label)
    html_str = html_str .. "<a href=\""..dest.."\">"..label.."</a>\n"
end

function AddError(t, str)
    t.err = t.err .. str .. "<br>\n"
end

function Error(str)
    if(str ~= "") then
        html_str = html_str .. "<b><p style=\"color:#FF0000;\">Error : <br>" .. str .. "</p></b>\n"
    end
end
function FormHidden(name, value)
    html_str = html_str .. "<input style=\"visibility:hidden\" name=\"" .. name .. "\" type=\"hidden\" value=\"" .. value .. "\">\n"
end

function FormPassword(name, value, size, disabled)
    if(value == nil) then
        value = ""
    end

    if((disabled == 0) and (disabled ~= nil)) then
        html_str = html_str .. "<input class=\"inputbox\" name=\"" .. name .. "\" type=\"password\" size=\"" .. size .. "\" maxlength=\"" .. size .. "\" value=\"" .. value.."\">\n" 
    else
        Text(value)
    end

end


function FormText(name, value, size, disabled)
    if(value == nil) then
        value = ""
    end

    if((disabled == 0) and (disabled ~= nil)) then
        html_str = html_str .. "<input class=\"inputbox\" name=\"" .. name .. "\" type=\"text\" size=\"" .. size .. "\" maxlength=\"" .. size .. "\" value=\"" .. value.."\">\n" 
    else
        Text(value)
    end

end

function FormRadioSingle(name, value, label, checked)
    if((checked ~= nil) and (checked > 0)) then
        checked = ' checked'
    else 
        checked = ''
    end

    html_str = html_str .. "<input type=\"radio\" name=\"" .. name .. "\" value=\"" .. value .. "\"".. checked .. ">" .. label .. "\n"
end

function FormRadio(name, values, size, selected)
    local cnt = 0
    local checked

    while(cnt < size) do
        if(cnt == tonumber(selected)) then
            checked = 1
        else 
            checked = 0
        end
        
        FormRadioSingle(name, cnt, values[cnt], checked)
        html_str = html_str .. '<br>\n'
        cnt = cnt + 1
    end
end 

function FormIP(name, value0, value1, value2, value3, disable)
    if((disable ~= nil) and (disable > 0))then
        if((value0 ~= "") and (value1 ~= "") and (value2 ~= "") and (value3 ~= "")) then
            Text(value0..'.'..value1..'.'..value2..'.'..value3)
        else 
            Text("")
        end
    else
	    FormText(name .. "0", value0, 3, 0)
	    html_str = html_str .. " . "
	    FormText(name .. "1", value1, 3, 0)
	    html_str = html_str .. " . "
	    FormText(name .. "2", value2, 3, 0)
	    html_str = html_str .. " . "
	    FormText(name .. "3", value3, 3, 0)
    end
end

function FormCheckbox(name, value, label, checked)
    html_str = html_str .. "<input type=\"checkbox\" name=\""..name.."\" value=\""..value.."\""
    if((checked ~= nil) and (checked > 0))then
        html_str = html_str .." checked"
    end
    html_str = html_str ..">"..label
end

function OneButtonForm(t, script_path, value, button_label)
    FormHeader(script_path, "OneButtonForm")
    FormHidden(value, 1)
    FormHidden("page", t.page)
    html_str = html_str .. "<input type=\"submit\" value=\""..button_label.."\">\n</form>\n"
end


function FormHeader(script_path, name)
    html_str = html_str .. "<form action=\"" .. script_path .. "\" name=\""..name.."\" method=\"get\" accept-charset=\"UTF-8\">\n"
end

function OnclickButton(label, event)
    html_str = html_str .. "<input type=\"button\" value=\""..label.."\" name=\"Button\" onclick=\""..event.."\">\n"
end
 
function FormButton(button_type, label)
    html_str = html_str .. "<input type=\"".. button_type .."\" value=\""..label.."\">\n"
end

function FormEnd(t)
    FormHidden("store", 0)
    FormHidden("page", t.page)
    FormHidden("sent", 1)
   
    html_str = html_str .. "</form>\n"
end

function FormBottom(t)
    html_str = html_str .. "<br>\n"
    FormButton("submit", label.button_submit)
    FormButton("reset", label.button_default)
    FormHidden("store", 0)
    FormHidden("page", t.page)
    FormHidden("sent", 1)
 
    OnclickButton(label.button_save,"document.mainform.store.value=1;document.mainform.submit()")
    html_str = html_str .. "</form>\n"
 
    --FormEnd(t)
end

function Header(t, title, script_path, addon)
    local menu_item_cnt = 6
    local menu_items = {[0] = label.tab_ethernet; [1] = label.tab_streaming; [2] = label.tab_status; [3] = label.tab_firmware; [4] = label.tab_default; [5] = label.tab_settings;}
    local dev_name = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_HOST_NAME)
    local dev_caption = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_DEV_CAPTION)
    

    local menu_class = ""
    local css = "<link rel=\"stylesheet\" type=\"text/css\" href=\"".. html_css_file .."\">\n"
    local css = css .. '<!--[if IE ]><link href="'.. html_css_ie ..'" rel="stylesheet" type="text/css" /><![endif]-->'
    local css = css .. "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\""..html_favicon.."\">\n"

    if(t.cookie == nil) then
        t.cookie = ""
    end

    if(addon == nil) then
        addon = ""
    end
    print ("Content-type: text/html\n")
    html_str = "<html>\n<head>\n".. t.cookie .."<title>" .. title .. "</title>\n" .. addon .. css .. "</head>\n<body>\n"
   
    html_str = html_str .. '<div id="wrapper">\n<div id="main">\n<div id="box">\n<div id="header">\n'
    html_str = html_str .. '<div id="headerright"><b>'..dev_name..'</b><br>'..dev_caption..'</div><h1 id="logo"><a href="http://www.rioxo.ch"><img src="/img/rioxo_logo.png" alt="rioxo&reg;"></a></h1>\n'
    

    html_str = html_str .. '<div id="mainmenu">\n<ul class=\"menu\">\n'
    
    if((t.mode_vrb) and (t.login))then
        html_str = html_str .. '<li id="right">'
        ImageLink(script_path.."?page=".. t.page .."&refresh=1", img_path.."refresh.png", label.reconnect)
        html_str = html_str .. '</li>'

        if(((t.vrb_playing == false) and (t.refresh == nil) and (t.play == nil)) or (t.stop ~= nil))then    
            html_str = html_str .. '<li id="right">'
            ImageLink(script_path.."?page=".. t.page, img_path.."stop_gray.png", label.stop)
            html_str = html_str .. '</li>'
    
            html_str = html_str .. '<li id="right">'
            ImageLink(script_path.."?page=".. t.page .."&play=1", img_path.."play.png", label.play)
            html_str = html_str .. '</li>'
        else
            html_str = html_str .. '<li id="right">'
            ImageLink(script_path.."?page=".. t.page .."&stop=1", img_path.."stop.png", label.stop)
            html_str = html_str .. '</li>'
    
            html_str = html_str .. '<li id="right">'
            ImageLink(script_path.."?page=".. t.page, img_path.."play_gray.png", label.play)
            html_str = html_str .. '</li>'
        end
    end
    
    if(t.login and t.auth_en) then
        html_str = html_str .. '<li id="right">'
        OneButtonForm(t, script_path, "logout", label.button_logout) 
        html_str = html_str .. '</li>'
    end
    
    for i=0, (menu_item_cnt-1), 1 do
        menu_class = "passiv"
        if(t.page == i) then
            menu_class = 'current'
        end
        html_str = html_str .. '    <li id="'..menu_class..'"><a href="' .. script_path .. '?page=' .. i .. '"><span>' .. menu_items[i] .. '</span></a></li>\n'
    end



    --html_str = html_str .. "</ul><br><br><b>"..label.device_name..dev_name.."</b>"
    html_str = html_str .. '</ul>\n</div>\n</div>\n' 
end

function Bottom(t)
    Error(t.err)
    html_str = html_str .. '</div>\n</div>\n</div>\n'
    html_str = html_str .. '<div id="bottom">\n<div id="footer">\n<div id="footerleft">\n'
    -- TODO: insert footer left
    --html_str = html_str .. '</div><div id="footermain"><div class="moduletable_address"><p>rioxo GmbH | <a href="mailto:mail@rioxo.net">mail@rioxo.net</a></p>\n'
    html_str = html_str .. '</div><div id="footermain"><div class="moduletable_address">\n'
    html_str = html_str .. "</div>\n</div>\n</div>\n</div>\n</div>\n</div>\n</body>\n</html>\n"
    print(html_str)
end



function TableHeader(col_size)
    html_table = 1
    html_table_col = col_size
    html_table_col_cnt = 0
    html_str = html_str .. "<table>\n"
    html_table_backup = html_str 
    html_table_str = ""
    html_str = "" 
end

function TableInsElement(colspan)
    if(html_table_col_cnt == 0) then
        html_table_str = html_table_str .. "    <tr>\n"
    end
    html_table_str = html_table_str .. "        <td colspan=\"" .. colspan .. "\">\n            " .. html_str .. "        </td>\n"
    if(html_table_col_cnt == html_table_col-1) then
        html_table_str = html_table_str .. "    </tr>\n"
    end

    html_table_col_cnt = html_table_col_cnt + colspan
    if(html_table_col_cnt >= html_table_col) then
        html_table_col_cnt = 0
    end
    html_str = ""
end

function TableBottom()
    html_str = html_table_backup .. html_table_str .. "</table>"
    html_table = 0
end

function UploadForm(t, script_path, label, accept)
    html_str = html_str .. "<form action=\""..script_path.."\" method=\"post\" enctype=\"multipart/form-data\">\n" 
--    html_str = html_str .. "<p>"..str.."<input name=\""..label.."\" type=\"file\" size=\"50\" accept=\""..accept.."\">\n"
    html_str = html_str .. "<p><input name=\""..label.."\" type=\"file\" size=\"50\">\n"
    html_str = html_str .. "<input type=\"submit\" value=\"Upload\"></p>\n</form>\n"
end
