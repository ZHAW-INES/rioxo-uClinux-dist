#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local IMAGE_HDR_SIZE = 32
local IMAGE_HDR_ID = "INES"
local HW_VERSION_TAG = 1

function rebooting(t)
    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    elseif (t.version_label == "riedel") then
        page_name = label.page_name_riedel
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. "Rebooting", script_path)
    hdoip.html.Title("Firmware update")
    hdoip.html.Text("The firmware will update now and reboot after. This takes  3 .. 4 minutes")
    hdoip.html.Text("<br><b> do NOT turn off the device during this process</b>")
    hdoip.html.Bottom(t)
end

function rebooting_load_bar(t, t_load, t_restart)
    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    elseif (t.version_label == "riedel") then
        page_name = label.page_name_riedel
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. "Rebooting", script_path)
    hdoip.html.Title("Firmware update")
    hdoip.html.Text("The firmware will update now and reboot after.")
    hdoip.html.Text("<br><b> do NOT turn off the device during this process</b>")
    hdoip.html.Loadbar(t_load, t_restart, 0)
    hdoip.html.Bottom(t)
end

-- ------------------------------------------------------------------
-- Firmware page
-- ------------------------------------------------------------------
function show(t)

    local post = {}
    local firmware_image = "/tmp/web.tmp"
    local reboot = 0

    if(t.length ~= nil)then
        hdoip.post.readHeader(t)
        t.err = t.err .. hdoip.post.readData(t, firmware_image)

        -- checking image header
        local fd = io.open(firmware_image, "r")

        if(fd ~= nil) then
            local hdr = fd:read(IMAGE_HDR_SIZE)
            fd:close()
            if(hdr ~= nil) then
                if(string.sub(hdr, 1, 4) ~= IMAGE_HDR_ID) then
                    t.err = t.err .. "This file is not a firmware image<br>\n"
                    os.execute("/bin/busybox rm "..firmware_image)
                else
                    if(string.byte(hdr, 5) == 24) then
                        loadtime = string.byte(hdr, 21)
                        loadtime = loadtime + string.byte(hdr, 22) * 256
                        restarttime = string.byte(hdr, 25)
                        hw_version = string.byte(hdr, 29)
                        if (hw_version == HW_VERSION_TAG) then
                            rebooting_load_bar(t, loadtime, restarttime)
                            hdoip.pipe.remote_update(firmware_image)
                            return
                        else
                            t.err = t.err .. "This file is not valid for this hardware version<br>\n"
                            os.execute("/bin/busybox rm "..firmware_image)
                        end
                    else
                        t.err = t.err .. "This firmware image is too old<br>\n"
                        os.execute("/bin/busybox rm "..firmware_image)
                    end
                end
            end
        else
            t.err = t.err .. "Could not open "..firmware_image.."<br>\n"
        end
    end

    hdoip.pipe.getVersion(t)

    serial = hdoip.pipe.getParam(hdoip.pipe.REG_SERIAL)

    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    elseif (t.version_label == "riedel") then
        page_name = label.page_name_riedel
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. label.page_firmware, script_path)

    if(t.fpga_svn ~= nil) then
        hdoip.html.Title(label.p_fw_act_firmware)
        hdoip.html.TableHeader(2)


        hdoip.html.Text(label.p_serial_number);                                      hdoip.html.TableInsElement(1)
        hdoip.html.Text(serial);                                                     hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_fpga_ver);                                        hdoip.html.TableInsElement(1)
        hdoip.html.Text(t.fpga_date_str.." (SVN : "..t.fpga_svn..")");               hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_sopc_ver);                                        hdoip.html.TableInsElement(1)
        hdoip.html.Text(t.sopc_date_str);                                            hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_hw_ver);                                          hdoip.html.TableInsElement(1)
        hdoip.html.Text(HW_VERSION_TAG);                                             hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_software_ver);                                    hdoip.html.TableInsElement(1)        
        hdoip.html.Text(t.sw_version_str);                                           hdoip.html.TableInsElement(1)
        hdoip.html.Text(label.p_fw_software_tag);                                    hdoip.html.TableInsElement(1)
        hdoip.html.Text(t.sw_tag);                                                   hdoip.html.TableInsElement(1)

        hdoip.html.TableBottom()
    end

    hdoip.html.Title(label.p_fw_upload)
                 
    if (t.upload_en ~= nil) then
        hdoip.pipe.free_buffer()
        hdoip.html.UploadForm(t, script_path, "image_files", "*")        
    else
        hdoip.html.FormHeader(script_path, main_form)
        hdoip.html.TableHeader(2)
        hdoip.html.Text(label.p_fw_select_file);                                     hdoip.html.TableInsElement(1)
        hdoip.html.FormCheckbox("upload_en", 1, "", 0)                               hdoip.html.TableInsElement(1)
        hdoip.html.TableBottom()
        hdoip.html.TableHeader(1)
        hdoip.html.Text(label.p_fw_note);                                            hdoip.html.TableInsElement(1)
        hdoip.html.TableBottom()
        hdoip.html.FormBottom(t)
    end

    hdoip.html.Bottom(t)
end


