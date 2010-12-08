#!/usr/bin/lua
require("hdoip.pages")
require("hdoip.html")
require("hdoip.pipe")

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

query.err = query.err .. hdoip.pipe.open()

if(query.page == 0) then
    hdoip.pages.status(query)
elseif(query.page == 1) then
    hdoip.pages.eth(query)
elseif(query.page == 2) then
    hdoip.pages.streaming(query)
else 
    hdoip.pages.status(query)
end


hdoip.pipe.close()
