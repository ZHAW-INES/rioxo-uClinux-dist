#!/usr/bin/lua
require("pages.status")
require("pages.ethernet")
require("pages.streaming")
require("pages.firmware")
require("pages.default")
require("pages.debug")
require("pages.settings")
require("pages.login")
require("hdoip.html")
require("hdoip.pipe")
require("hdoip.post")
require("hdoip.cookie")
require("hdoip.download")

script_path = "/cgi-bin/index.lua"
img_path = "/img/"
main_form = "mainform"
PAGE_ID_ETH = 0
PAGE_ID_STREAM = 1
PAGE_ID_STATUS = 2
PAGE_ID_FIRMWARE = 3
PAGE_ID_DEFAULT = 4
PAGE_ID_SETTING = 5
PAGE_ID_LOGIN = 20
PAGE_START = 0

local init_err = ""
local query = {}
local proc = {}

request_method = os.getenv("REQUEST_METHOD")
if(request_method == nil) then
    request_method = "GET"
end

if(request_method == "POST") then
    init_err = hdoip.post.init(query)
    query.page = PAGE_ID_FIRMWARE
else
    query_string = os.getenv("QUERY_STRING")
    if(query_string ~= nil) then
        query = hdoip.html.getQueryData(query_string)
        if(query.page == nil) then
            query.page = PAGE_START
        else 
            query.page = tonumber(query.page)
        end
    else
        query.page = PAGE_START
    end
end

-----------------------------------------------
-- System parameters
query.err = hdoip.pipe.open()
query.lang = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_LANG)
query.auth_en = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_AUTH_EN)
query.daemon_state = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_STATE)
query.system_mode = hdoip.pipe.getParam(hdoip.pipe.REG_MODE_START)

if(query.system_mode == "vtb") then
    query.mode_vtb = true;
else 
    query.mode_vtb = false;
end

if(query.system_mode == "vrb") then
    query.mode_vrb = true;
else 
    query.mode_vrb = false;
end  

-----------------------------------------------
-- Language select
if (query.lang == "DE") then
    require("lang.de")
    label = lang.de.label
else
    query.lang = "EN"
    require("lang.en")
    label = lang.en.label
end

-----------------------------------------------
-- Web authentification
if(query.auth_en == "false") then
    query.auth_en = false
    query.login = true
else
    query.auth_en = true
    
    local user = hdoip.cookie.get("username")
    local pass = hdoip.cookie.get("password")
    query.web_user = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_USER)
    query.web_pass = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_PASS)

    if((user ~= query.web_user) or (pass ~= query.web_pass)) then
        query.page = 20
        query.login = false
    else 
        query.login = true
    end
end

-----------------------------------------------
-- General functions
if(query.logout ~= nil) then
    hdoip.cookie.delete(query, "username", {})
    hdoip.cookie.delete(query, "password", {})
    query.page = PAGE_ID_LOGIN
    query.login = false
end

if(query.login) then
	if(query.stop ~= nil) then
	    hdoip.pipe.ready()
	end
	
	if(query.play ~= nil) then
	    hdoip.pipe.play()
	end
	
	if(query.refresh ~= nil) then
	    hdoip.pipe.ready()
	    hdoip.pipe.play()
	end
	
	if(init_err ~= "") then
	    hdoip.html.AddError(query, init_err)
	end
end
-----------------------------------------------
-- Page select
if(query.page == 0) then
    pages.ethernet.show(query)
elseif(query.page == 1) then
    pages.streaming.show(query)
elseif(query.page == 2) then
    pages.status.show(query)
elseif(query.page == 3) then 
    pages.firmware.show(query)
elseif(query.page == 4) then 
    pages.default.show(query)
elseif(query.page == 5) then 
    pages.settings.show(query)
elseif(query.page == 20) then
    pages.login.show(query)
else 
    pages.debug.show(query)
end

if(query.store == "1")then
    hdoip.pipe.store_cfg()
end

hdoip.pipe.close()
