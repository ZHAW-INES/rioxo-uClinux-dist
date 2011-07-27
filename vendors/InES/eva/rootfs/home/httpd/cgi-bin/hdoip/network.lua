module (..., package.seeall)

function checkIp(ip0, ip1, ip2, ip3)
    if((tonumber(ip0) ~= nil) and (tonumber(ip1) ~= nil) and (tonumber(ip2) ~= nil) and (tonumber(ip3) ~= nil)) then
        return 1
    end
    return 0
end

function text2IpValues(str)
    local t = {[0] = ""; [1] = ""; [2] = ""; [3] = ""}
    local pos_start = 0
    local pos_end

    for i = 0, 3, 1 do 
        pos_end = string.find(str, '.', pos_start, true)
        if(pos_end == nil) then
            t[i] = string.sub(str, pos_start, string.len(str))
            break;
        end
        t[i] = string.sub(str, pos_start, pos_end-1)
        pos_start = pos_end + 1 
    end

    return t[0], t[1], t[2], t[3]
end

