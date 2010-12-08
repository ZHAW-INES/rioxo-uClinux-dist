#!/usr/bin/lua

t_hex2num = {["f"]=15; ["F"]=15; ["e"]=14; ["E"]=14; ["d"]=13; ["D"]=13; ["c"]=12; ["C"]=12; ["b"]=11; ["B"]=11; ["a"]=10; ["A"]=10;
            ["9"]=9; ["8"]=8; ["7"]=7; ["6"]=6; ["5"]=5; ["4"]=4; ["3"]=3; ["2"]=2; ["1"]=1;["0"]=0}

function convStr2HexFile(str) 
    local len = string.len(str)
    local i, hex = 1, ""
    
    while(len >= 2) do
        hex = hex .. string.char((t_hex2num[string.sub(str,i,i)]*16 + t_hex2num[string.sub(str,i+1,i+1)]))
        len = len - 2
        i = i + 2
    end

    if(len == 1) then
        hex = hex .. string.char(t_hex2num[string.sub(str,i,i)]*16)
    end

    return hex
end

function conStr2LE_32(str)
    return string.sub(str,7,8)..string.sub(str,5,6)..string.sub(str,3,4)..string.sub(str,1,2)
end

function conStr2LE_16(str)
    return string.sub(str,3,4)..string.sub(str,1,2)
end

function createCmdHeader(cmd, payload_size)
    str = conStr2LE_32(cmd)
    str = str .. conStr2LE_32(string.format("%08x", payload_size + 8))

    return str
end

PIPE_CMD_GET_PARAM = "3800000a"
PIPE_CMD_SET_PARAM = "3800000b"

function setParameter(key, value)
    key_len = string.len(key)
    value_len = string.len(value)

    str = createCmdHeader(PIPE_CMD_SET_PARAM, value_len + key_len + 2 + 4)
    str = str .. conStr2LE_32(string.format("%08x", value_len+1))
    str = (convStr2HexFile(str) .. key .. string.char(0) .. value .. string.char(0))
    return str
end


rsp_fd = io.open("/tmp/web.rsp", "r")
cmd_fd = io.open("/tmp/web.cmd", "w")

if((cmd_fd == nil) or (rsp_fd == nil)) then
    print("could not open file");
else
    str = setParameter("0123", "4567")
    print("str : " .. str)
    cmd_fd:write(str)
    print("command sent")

    rsp = ""
--    while (rsp == nil) do
    rsp = rsp_fd:read("*a")
--    end

    print("response : " .. rsp)
end
cmd_fd:close()
rsp_fd:close()
print("pipes closed")
