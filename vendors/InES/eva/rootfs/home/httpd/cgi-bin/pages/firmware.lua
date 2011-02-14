#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local IMAGE_HDR_SIZE = 20
local IMAGE_HDR_ID = "INES"

function rebooting(t)
    hdoip.html.Header(t, label.page_name .. "Rebooting", script_path)
    hdoip.html.Title("Firmware update")
    hdoip.html.Text("The firmware will update now and reboot after. This takes  3 .. 4 minutes")
    hdoip.html.Text("<br><b> do NOT turn off the device during this process</b>")
 
    hdoip.html.Bottom(t)

end

-- ------------------------------------------------------------------
-- Firmware page
-- ------------------------------------------------------------------
function show(t)

    local post = {}
    local firmware_image = "/tmp/web.tmp"
    local reboot = 0

    if(t.length ~= nil) then
        hdoip.post.readHeader(t)
        hdoip.post.readData(t, firmware_image)

        -- checking image header
        local fd = io.open(firmware_image, "r")

        if(fd ~= nil) then
            local hdr = fd:read(IMAGE_HDR_SIZE)
            fd:close()
            if(string.sub(hdr, 1, 4) ~= IMAGE_HDR_ID) then
                t.err = t.err .. "This file is not an firmware image<br>\n"
                os.execute("/bin/busybox rm "..firmware_image)
            else
                rebooting(t)
                hdoip.pipe.remote_update(firmware_image)
                return
            end
        else
            t.err = t.err .. "Could not open "..firmware_image.."<br>\n"
        end
    end

    hdoip.pipe.getVersion(t)

    hdoip.html.Header(t, label.page_name .. label.page_firmware, script_path)

    if(t.fpga_svn ~= nil) then
        hdoip.html.Title(label.p_fw_act_firmware)
        hdoip.html.TableHeader(2)
        hdoip.html.Text(label.p_fw_fpga_ver);                                        hdoip.html.TableInsElement(1)
        hdoip.html.Text(t.fpga_date_str.." (SVN : "..t.fpga_svn..")");               hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_sopc_ver);                                        hdoip.html.TableInsElement(1)
        hdoip.html.Text(t.sopc_date_str);                                            hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_software_ver);                                    hdoip.html.TableInsElement(1)
        hdoip.html.Text(t.sw_version_str);                                           hdoip.html.TableInsElement(1)
        hdoip.html.TableBottom()
    end

    hdoip.html.Title(label.p_fw_upload)                                             
    hdoip.html.UploadForm(t, script_path, "image_files", "*")                  

    hdoip.html.Bottom(t)
end


