#!/usr/bin/lua

module (..., package.seeall)

require("hdoip.html")

local function optional (key, value)
    if ((key ~= nil) and (name ~= nil)) then  
        return string.format("; %s=%s", key, value)
    end
    return ""
end

local function build (name, value, options)
    if (name == nil) or (value == nil) then
        error("cookie needs a name and a value")
    end
    
    local cookie = name .. "=" .. hdoip.html.escape(value)
    options = options or {}
    if (options.expires) then
        local t = os.date("!%A, %d-%b-%Y %H:%M:%S GMT", options.expires)
        cookie = cookie .. optional("expires", t)
    end
    cookie = cookie .. optional("path", options.path)
    cookie = cookie .. optional("domain", options.domain)
    cookie = cookie .. optional("secure", options.secure)
    return cookie
end

function set(t, name, value, options)
    if(t.cookie == nil) then
        t.cookie = ""
    end
    t.cookie = t.cookie .. string.format('<meta http-equiv="Set-Cookie" content="%s">\n', build(name, value, options)) 
end

function get(name)
    local cookies = os.getenv("HTTP_COOKIE") or ""
    cookies = ";" .. cookies .. ";"
    cookies = string.gsub(cookies, "%s*;%s*", ";")
    local pattern = ";" .. name .. "=(.-);"
    local _, __, value = string.find(cookies, pattern)

    if(value == nil) then
        return nil
    end

    return hdoip.html.unescape(value)
end

function delete (t, name, options)
    options = options or {}
    options.expires = 1
    set(t, name, "xxx", options)
end
