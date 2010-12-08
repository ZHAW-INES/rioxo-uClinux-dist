#!/usr/bin/lua
module (..., package.seeall)
require ("hdoip.convert")

local PIPE_CMD = "/tmp/web.cmd"
local PIPE_RSP = "/tmp/web.rsp"

local PIPE_CMD_GET_PARAM = "3800000a"
local PIPE_CMD_SET_PARAM = "3800000b"

local fd_cmd, fd_rsp = nil, nil

function open()
    fd_cmd = io.open(PIPE_CMD, "w");
    if(fd_cmd == nil) then
        return ("could not open "..PIPE_CMD.."\n")
    end


    fd_rsp = io.open(PIPE_RSP, "r+");
    if(fd_rsp == nil) then
        return ("could not open "..PIPE_RSP.."\n")
        
    end

    return "" 
end

function close()
    if(fd_rsp ~= nil) then
        fd_rsp:close()
    end
 
    if(fd_cmd ~= nil) then
        fd_cmd:close()
    end
end

local function createCmdHeader(cmd, payload_size)
    str = hdoip.convert.Str2LE_32(cmd)
    str = str .. hdoip.convert.Str2LE_32(string.format("%08x", (payload_size + 8)))
    return str
end


function setParam(key, value)
    if(fd_cmd ~= nil) then
        key_len = string.len(key)
        value_len = string.len(value)

        str = createCmdHeader(PIPE_CMD_SET_PARAM, (value_len + key_len + 2 + 4))
        str = str .. hdoip.convert.Str2LE_32(string.format("%08x", key_len + 1))
        debug = str .. key .. string.char(0) .. value .. string.char(0)
        str = hdoip.convert.Str2HexFile(str) .. key .. string.char(0) .. value .. string.char(0)    
        fd_cmd:write(str)
        fd_cmd:flush() 
    end
end

function getParam(key)
    if((fd_cmd ~= nil) and (fd_rsp ~= nil)) then
        key_len = string.len(key)

        str = createCmdHeader(PIPE_CMD_GET_PARAM, (key_len + 1 + 4))
        str = str .. hdoip.convert.Str2LE_32(string.format("%08x", key_len + 1))
        str = hdoip.convert.Str2HexFile(str) .. key .. string.char(0)

        fd_cmd:write(str)
        fd_cmd:flush()

        -- Read command header
        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(8)
        end
        
        cmd = hdoip.convert.bin2hex(string.sub(ret,1,4),4)
        cmd = hdoip.convert.Str2LE_32(cmd)
        size = hdoip.convert.bin2dec(string.sub(ret,5,8),4)
  
        -- Read payload
        ret = nil 
        while(ret == nil) do
            ret = fd_rsp:read(size - 8)
        end

        offset = hdoip.convert.bin2dec(string.sub(ret,1,4),4)
        value = string.format("%s",string.sub(ret,5,5 + offset - 1))

        return value
    end

    return ""
end
