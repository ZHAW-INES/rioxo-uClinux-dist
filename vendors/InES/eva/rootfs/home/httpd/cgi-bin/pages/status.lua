#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")
require("hdoip.download")

local line_cnt = 0
local lines = {}
local vid_res = nil

local function checkline(str)
    if(str ~= nil) then
        local begin = string.sub(str, 0, 14)    

        if(string.find(begin, "video") or string.find(begin, "stream")) then
            lines[line_cnt] = {}
            lines[line_cnt] = str
            line_cnt = line_cnt + 1   
        elseif(string.find(begin, "resolution")) then
             
            vid_res = string.sub(str, string.find(str, ":")+1, string.len(str))
        end
    end
end

local function get_stream_param(str)
    local value, type, dir = "", nil, nil;
    string.gsub(str, "[%b  ]%a+", function(c) value = value .. c end)
    string.gsub(str, "(%a+)", function(c) type = c end)
    string.gsub(str, "{(%a+)}", function(c) dir = c end)
    return value, type, dir
end

local function parse_stream_state(t, str)
    local off 

    if((str ~= "") and (str ~= nil))then
        string.gsub(str, "([^\n]+)", checkline)
    
        string.gsub(lines[0], "([^:]+)", function(c) t.vid_out = c end)
        string.gsub(lines[1], "([^:]+)", function(c) t.vid_in = c end)
    
        off = 2
        t.stream_cnt = line_cnt - off
        for i=off,line_cnt-1 do
            t[i-off] = {}
            t[i-off].state, t[i-off].type, t[i-off].dir = get_stream_param(lines[i])
        end
    else
        t.stream_cnt = 0
    end
end

-- ------------------------------------------------------------------
-- Status page
-- ------------------------------------------------------------------
function show(t)
    local str = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_STREAM)
    parse_stream_state(t, str)
    t.daemon_state = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_STATE)

    if(t.daemon_state == "vtb") then
        t.daemon_state = "Video transmitter box"
    elseif(t.daemon_state == "vrb") then
        t.daemon_state = "Video receiver box"
    else
        t.daemon_state = "none"
    end
    
    hdoip.html.Header(t, label.page_name .. label.page_status, script_path)
    hdoip.html.Title(label.p_stat_title)

    hdoip.html.TableHeader(2)
    hdoip.html.Text("Operating mode");                                          hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.daemon_state);                                            hdoip.html.TableInsElement(1);

    hdoip.html.Text("Video sink");                                              hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.vid_out);                                                 hdoip.html.TableInsElement(1);
    hdoip.html.Text("Video source");                                            hdoip.html.TableInsElement(1);
    hdoip.html.Text(t.vid_in);                                                  hdoip.html.TableInsElement(1);

    if(vid_res ~= nil) then
        hdoip.html.Text("Resolution");                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text(vid_res);                                               hdoip.html.TableInsElement(1);
    end

    hdoip.html.Text("");                                                        hdoip.html.TableInsElement(1);
    hdoip.html.Text("");                                                        hdoip.html.TableInsElement(1);

    if(t.stream_cnt == 0) then
        hdoip.html.Text("");                                                    hdoip.html.TableInsElement(1);
        hdoip.html.Text("<b>Could not establish a stream</b>");                 hdoip.html.TableInsElement(1);
    else
        for i=0, t.stream_cnt-1 do
            hdoip.html.Text("Stream "..i.." ["..t[i].type.." "..t[i].dir.."] :");   hdoip.html.TableInsElement(1);
            hdoip.html.Text(t[i].state);                                            hdoip.html.TableInsElement(1);   
        end
    end
    hdoip.html.TableBottom()
    hdoip.html.Bottom(t)
end


