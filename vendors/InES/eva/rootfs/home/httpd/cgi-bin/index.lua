#!/usr/bin/lua
require("hdoip.pages")
require("hdoip.network")
require("hdoip.html")
require("hdoip.command")

html_side_name = "HD over IP : "
html_menu_item_cnt = 3
html_menu_items = {[0] = "Status"; [1] = "Device configuration"; [2] = "Streaming";}
local err
local query = {}
local proc = {}

query_string = os.getenv("QUERY_STRING")

if(query_string ~= nil) then
    query = hdoip.html.getQueryData(query_string)
    if(query.page == nil) then
        query.page = 0
    else 
        query.page = tonumber(query.page)
    end
else
    query.page = 0
end

query.err = ""

fd = io.open("/proc/hdoip_core", "r")
if(fd == nil) then
    query.err = tostring(query.err .. "could not open \"/proc/hdoip_core\"<br>");
    proc.cnt = 0
else

    local pos = 0, 0
    local name, value = "", ""
    line = fd:read("*l");
    
    while line ~= nil do
        pos = string.find(line, ":") 
        proc[string.sub(line, 0, pos-1)] = string.sub(line, pos+1, string.len(line))
        line = fd:read("*l")
    end
    proc.cnt = line_cnt
end

if(query.page == 0) then
    hdoip.pages.status(query, proc)
elseif(query.page == 1) then
    hdoip.pages.eth(query)
elseif(query.page == 2) then
    hdoip.pages.streaming(query)
else 
    hdoip.pages.status(query, proc)
end

--hdoip.command.setDebug()

--os.exit()

