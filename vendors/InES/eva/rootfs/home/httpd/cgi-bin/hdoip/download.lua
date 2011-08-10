#!/usr/bin/lua
module(..., package.seeall)

-- TO DO: remove LF at end of file
function file(file, filename)
    fd = io.open(file, "rb")

    if(fd ~= nil) then
        print ("Content-type:application/x-download")
        print ("Content-Disposition:attachment;filename="..filename.."\n")
        print (fd:read("*a"))
        return ""
    end

    return "Could not open "..file.."<br>\n"
end
