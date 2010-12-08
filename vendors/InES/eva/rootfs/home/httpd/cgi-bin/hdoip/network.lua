module (..., package.seeall)

local function text2IpValues(str)
    local t = {}
    local pos_start = 0
    local pos_end

    report("str : " .. str)

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

