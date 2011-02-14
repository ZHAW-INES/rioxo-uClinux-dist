#!/usr/bin/lua
module (..., package.seeall)

--local POST_READ = "/dev/stdin"
local POST_READ = "/proc/self/fd/0"
local POST_CHUNK_SIZE = 1000
local POST_MAX_SIZE = 4*2^20 -- 4 MB

function getLength(post)
    post.length = tonumber(os.getenv("CONTENT_LENGTH"))
end

function getBoundary(post)
    post.content_type =  os.getenv("CONTENT_TYPE")
    
    if(post.content_type ~=nil) then
        _,_,post.boundary = string.find(post.content_type, "boundary%=(.-)$") 
        post.boundary = "--" .. post.boundary
        post.boundary_len = string.len(post.boundary)
    end
end

function readHeader(post)
    local cnt = 0
    local char
    post.header = ""

    if(post.fd ~= nil) then
        -- skip boundary
        post.fd:read(post.boundary_len) 
        post.length = post.length - post.boundary_len

        -- read header
        while(cnt < 4) do
            char = post.fd:read(1)
            post.length = post.length - 1
            post.header = post.header .. char

            if((char == "\n") or (char == "\r")) then 
                cnt = cnt + 1
            else
                cnt = 0
            end
        end

        -- parse header
        string.gsub(post.header, '([^%c%s:]+):%s+([^\n]+)', function (attr, val) attr = string.lower(attr); post[attr] = val end)
        string.gsub(post["content-disposition"], ';%s*([^%s=]+)="(.-)"', function (attr, val) post[attr] = val end)
    end
end

function readData(post, file)
    if(post.fd ~= nil) then
        if(post.length < POST_MAX_SIZE) then
            local fd = io.open(file, "w+")

            if(fd ~= nil) then
                post.data_len = post.length - post.boundary_len - 6 -- 6 = 2 x "\r\n" + 1 x "--"

                -- Write upload in chunks to file
                while(post.data_len ~= 0) do
                    if(post.data_len >= POST_CHUNK_SIZE) then
                        fd:write(post.fd:read(POST_CHUNK_SIZE))
                        post.data_len = post.data_len - POST_CHUNK_SIZE
                    else
                        fd:write(post.fd:read(post.data_len))
                        post.data_len = 0
                    end 
                end
                fd:close()
            else
                return "Could not open ".. file .."<br>\n"
            end
        else
            return "File is too large (max size : "..POST_MAX_SIZE.." bytes)<br>\n"
        end
    end
    return ""
end

function init(post)
    post.fd = io.open(POST_READ, "r")
    if(post.fd ~= nil) then
        getBoundary(post)
        getLength(post)
        return ""
    end
    return "Could not open "..POST_READ.."<br>\n"
end

