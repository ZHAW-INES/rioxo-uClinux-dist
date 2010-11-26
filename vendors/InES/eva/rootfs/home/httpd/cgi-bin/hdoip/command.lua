#!/usr/bin/lua
module (..., package.seeall)

require("hdoip.convert")

WEB_PIPE_TO_DAEMON = "/tmp/web_from"

function setDebug()
    fd = assert(io.open(WEB_PIPE_TO_DAEMON, "wb"))

    cmd = hdoip.convert.Str2LE_32("35000001") .. hdoip.convert.Str2LE_32("00000008") -- command id & command size
    
    fd:write(hdoip.convert.Str2HexFile(cmd))
    fd:close()
end
