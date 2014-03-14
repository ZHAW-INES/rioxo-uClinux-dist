#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

-- ------------------------------------------------------------------
-- TEST page
-- ------------------------------------------------------------------
function show(t)
    if(t.test_focus_1080p60 ~= nil) then
        hdoip.pipe.setParam(hdoip.pipe.REG_MODE_TEST_IMAGE, "1")
    end

    if(t.test_focus_1080p24 ~= nil) then
        hdoip.pipe.setParam(hdoip.pipe.REG_MODE_TEST_IMAGE, "2")
    end

    if(t.test_focus_720p60 ~= nil) then
        hdoip.pipe.setParam(hdoip.pipe.REG_MODE_TEST_IMAGE, "3")
    end

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

    hdoip.html.Header(t, page_name .. label.page_test, script_path)
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.p_usb_image)

    hdoip.html.OneButtonForm(t, script_path, "test_focus_1080p60",label.button_focus_1080p60);
    hdoip.html.OneButtonForm(t, script_path, "test_focus_1080p24",label.button_focus_1080p24);
    hdoip.html.OneButtonForm(t, script_path, "test_focus_720p60",label.button_focus_720p60);

    hdoip.html.Bottom(t)
end


