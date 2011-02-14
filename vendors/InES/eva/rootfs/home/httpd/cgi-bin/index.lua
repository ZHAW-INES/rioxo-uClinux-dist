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

query.err = hdoip.pipe.open()
query.lang = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_LANG)
query.auth_en = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_AUTH_EN)
if(query.auth_en == "") then
    query.auth_en = 1
else
    query.auth_en = tonumber(query.auth_en)
end

if (query.lang == "DE") then
    require("lang.de")
    label = lang.de.label
else
    query.lang = "EN"
    require("lang.en")
    label = lang.en.label
end


if(query.store_cfg ~= nil) then
    hdoip.pipe.store_cfg()
end

if(query.logout ~= nil) then
    hdoip.cookie.delete(query, "username", {})
    hdoip.cookie.delete(query, "password", {})
    query.page = PAGE_ID_LOGIN
end

if(init_err ~= "") then
    hdoip.addError(t, init_err)
end

if(query.auth_en > 0) then
    local user = hdoip.cookie.get("username")
    local pass = hdoip.cookie.get("password")
    query.web_user = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_USER)
    query.web_pass = hdoip.pipe.getParam(hdoip.pipe.REG_WEB_PASS)

    if((user ~= query.web_user) or (pass ~= query.web_pass)) then
        query.page = 20
    end
end

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

hdoip.pipe.close()
