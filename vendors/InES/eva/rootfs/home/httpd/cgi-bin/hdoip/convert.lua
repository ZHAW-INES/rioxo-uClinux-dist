#!/usr/bin/lua
module (..., package.seeall)

local t_hex2num = {["f"]=15; ["F"]=15; ["e"]=14; ["E"]=14; ["d"]=13; ["D"]=13; ["c"]=12; ["C"]=12; ["b"]=11; ["B"]=11; ["a"]=10; ["A"]=10;
            ["9"]=9; ["8"]=8; ["7"]=7; ["6"]=6; ["5"]=5; ["4"]=4; ["3"]=3; ["2"]=2; ["1"]=1;["0"]=0}

function Str2LE_32(str)
    return string.sub(str,7,8)..string.sub(str,5,6)..string.sub(str,3,4)..string.sub(str,1,2)
end

function Str2LE_16(str)
    return string.sub(str,3,4)..string.sub(str,1,2)
end

function Str2HexFile(str) 
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

function bin2hex(str, size)
    local len = 0
    local ret = ""

    while(size ~= len) do
        ret = ret .. string.format("%02x", string.byte(str,len+1))
        len = len + 1 
    end 
    return ret
end

function bin2dec(str, size)
    local len = 0
    local ret = 0

    while(size ~= len) do
        tmp = string.byte(str, len+1)
        ret = ret + (tmp * 16^(2*len))
        len = len + 1
    end
    return ret
end

function checkHex(str)
    local len = 0
    local char = 0
    local size

    if(str == nil) then
        return false
    end

    size = string.len(str)
    
    while(size ~= len) do
        char = tonumber(string.byte(str, len+1))
        len = len + 1 

        -- A - F  => 65 - 70
        -- a - f  => 97 - 102
        -- 0 - 9  => 48 - 57
        if((((char > 47) and (char < 58)) or ((char > 96) and (char < 103)) or ((char > 64) and (char < 71))) == false) then
            return false
        end
    end 
    
    return true
end
