#!/usr/bin/lua
module (..., package.seeall)

html_script_path = "/cgi-bin/index.lua"
html_str = "<b>html error</b>"
html_table = 0
html_table_str = "<b>table error</b>"
html_table_backup = ""
html_table_col = 0
html_table_col_cnt = 0

local function extractArg(t, s)
    sep_pos = string.find(s, "=")
    local arg_value = string.sub(s, sep_pos+1, string.len(s))
    local arg_name = string.sub(s, 0, sep_pos-1)
    t[arg_name] = arg_value
end
    
function getQueryData(query_string)
    local arg_str
    local arg_start = 0
    local arg_end = 0
    local t= {}
    
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
    html_str = html_str .. str .. "\n"
end

function Title(str)
    html_str = html_str .. "<h2>" .. str .. "</h2>\n"
end

function Error(str)
    if(str ~= "") then
        html_str = html_str .. "<b><p style=\"color:#FF0000;\">Error : " .. str .. "</p></b>\n"
    end
end

function Header(title, page, addon)
    local menu_class = ""
    local css = "<link rel=\"stylesheet\" type=\"text/css\" href=\"/main.css\">\n"

    if(addon == nil) then
        addon = ""
    end
    print ("Content-type: text/html\n")
    html_str = "<html>\n<head>\n<title>" .. title .. "</title>\n" .. addon .. css .. "</head>\n<body>\n"
    
    html_str = html_str .. "<ul id=\"globalnav\">\n"
    
    for i=0, (html_menu_item_cnt-1), 1 do
        if(page == i) then
            menu_class = " class=\"here\""
        else 
            menu_class = ""
        end
        html_str = html_str .. "    <li><a" .. menu_class .." href=\"" .. html_script_path .. "?page=" .. i .. "\">" .. html_menu_items[i] .. "</a></li>\n"
    end
    html_str = html_str .. "</ul>\n<br><br><br>\n"
end

function Bottom(err)
    Error(err)    
    html_str = html_str .. "</body>\n</html>\n"
    print(html_str)
end

function FormHidden(name, value)
    html_str = html_str .. "<input name=\"" .. name .. "\" type=\"hidden\" value=\"" .. value .. "\">\n"
end

function FormText(name, value, size)
    html_str = html_str .. "<input name=\"" .. name .. "\" type=\"text\" size=\"" .. size .. "\" maxlength=\"" .. size .. "\" value=\"" .. value .. "\">\n"
end

function FormRadio(name, values, size, selected)
    local cnt = 0
    local checked

    while(cnt < size) do
        if(string.format(cnt) == selected) then
            checked = " checked"
        else 
            checked = ""
        end
        html_str = html_str .. "<input type=\"radio\" name=\"" .. name .. "\" value=\"" .. cnt .. "\"".. checked .. ">" .. values[cnt] .. "<br>\n"
        cnt = cnt + 1
    end
end 

function FormIP(name, value0, value1, value2, value3)
    FormText(name .. "0", value0, 3)
    html_str = html_str .. "."
    FormText(name .. "1", value1, 3)
    html_str = html_str .. "."
    FormText(name .. "2", value2, 3)
    html_str = html_str .. "."
    FormText(name .. "3", value3, 3)
end

function FormHeader()
    html_str = html_str .. "<form action=\"" .. html_script_path .. "\" method=\"get\">\n"
end

function FormBottom(page)
    FormHidden("page", page)
    FormHidden("submit", "1")
    html_str = html_str .. "<br>\n<input type=\"submit\" value=\"Save\">\n"
    html_str = html_str .. "<input type=\"reset\" value=\"Default\">\n"
    html_str = html_str .. "</form>\n"
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
