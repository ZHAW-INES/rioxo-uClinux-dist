#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local function get_low_byte(value)
    local high_value = math.floor(value/256)
    high_value = high_value*256
    local low_value = value - high_value
    return low_value
end

local function get_low_nibble(value)
    local high_value = math.floor(value/16)
    high_value = high_value*16
    local low_value = value - high_value
    return low_value
end

local function get_high_byte(value)
    local high_value = math.floor(value/256)
    return high_value
end

local function get_high_nibble(value)
    local high_value = math.floor(value/16)
    return high_value
end

local function get_character(string, position)
    return string.sub(string, position, position)
end

local function set_character(string, position, value)
   return string.sub(string, 1, (position - 1)) .. value .. string.sub(string, (position + 1))
end

-- ------------------------------------------------------------------
-- EDID page
-- ------------------------------------------------------------------
function show(t)

    local menu_items_count = 20
    local menu_items = {[0] =  " 640x480p  @ 60Hz";
                        [1] =  " 800x600p  @ 60Hz";
                        [2] =  "1024x768p  @ 60Hz";
                        [3] =  "1280x960p  @ 60Hz";
                        [4] =  "1280x1024p @ 60Hz";
                        [5] =  "1680x1050p @ 60Hz";
                        [6] =  " 720x480p  @ 60Hz";
                        [7] =  " 720x480i  @ 60Hz";
                        [8] =  " 720x576p  @ 50Hz";
                        [9] =  " 720x576i  @ 50Hz";
                        [10] = "1280x720p  @ 60Hz";
                        [11] = "1280x720p  @ 50Hz";
                        [12] = "1920x1080p @ 60Hz";
                        [13] = "1920x1080i @ 60Hz";
                        [14] = "1920x1080p @ 50Hz";
                        [15] = "1920x1080i @ 50Hz";
                        [16] = "1920x1080p @ 30Hz";
                        [17] = "1920x1080p @ 25Hz";
                        [18] = "1920x1080p @ 24Hz";
                        [19] = "   own resolution";
                       }

    local timing = {    [" 640x480p  @ 60Hz"] = { 25175, 0,  640,  480,  160, 45,  16, 10,  96, 2, 1};
                        [" 800x600p  @ 60Hz"] = { 40000, 0,  800,  600,  256, 28,  40,  1, 128, 4, 0};
                        ["1024x768p  @ 60Hz"] = { 65000, 0, 1024,  768,  320, 38,  24,  3, 136, 6, 0};
                        ["1280x960p  @ 60Hz"] = {108000, 0, 1280,  960,  520, 40,  96,  1, 112, 3, 0};
                        ["1280x1024p @ 60Hz"] = {108000, 0, 1280, 1024,  408, 42,  48,  1, 112, 3, 0};
                        ["1680x1050p @ 60Hz"] = {146250, 0, 1680, 1050,  560, 39, 105,  3, 176, 6, 0};
                        [" 720x480p  @ 60Hz"] = { 27000, 0,  720,  480,  138, 45,  16,  9,  62, 6, 2};
                        [" 720x480i  @ 60Hz"] = { 13500, 1,  720,  240,  138, 22,  19,  4,  62, 3, 6};
                        [" 720x576p  @ 50Hz"] = { 27000, 0,  720,  576,  144, 49,  12,  5,  64, 5, 17};
                        [" 720x576i  @ 50Hz"] = { 13500, 1,  720,  288,  144, 24,  12,  2,  63, 3, 21};
                        ["1280x720p  @ 60Hz"] = { 74250, 0, 1280,  720,  370, 30, 110,  5,  40, 5, 4};
                        ["1280x720p  @ 50Hz"] = { 74250, 0, 1280,  720,  700, 30, 440,  5,  40, 5, 19};
                        ["1920x1080p @ 60Hz"] = {148500, 0, 1920, 1080,  280, 45,  88,  4,  44, 5, 16};
                        ["1920x1080i @ 60Hz"] = { 74250, 1, 1920,  540,  280, 22,  88,  2,  44, 5, 5};
                        ["1920x1080p @ 50Hz"] = {148500, 0, 1920, 1080,  720, 45, 528,  4,  44, 5, 31};
                        ["1920x1080i @ 50Hz"] = { 74250, 1, 1920,  540,  720, 22, 528,  2,  44, 5, 20};
                        ["1920x1080p @ 30Hz"] = { 74250, 0, 1920, 1080,  280, 45,  88,  4,  44, 5, 34};
                        ["1920x1080p @ 25Hz"] = { 74250, 0, 1920, 1080,  720, 45, 528,  4,  44, 5, 33};
                        ["1920x1080p @ 24Hz"] = { 74250, 0, 1920, 1080,  830, 45, 638,  4,  44, 5, 32};
                        ["   own resolution"] = {t.own_pixelclock, t.own_interlaced, t.own_horizontal_active, t.own_vertical_active, t.own_horizontal_blanking,t.own_vertical_blanking, t.own_horizontal_offset, t.own_vertical_offset, t.own_horizontal_pulse, t.own_vertical_pulse, 0};
                    }


    local channel_items_count = 8
    local channel_items = { [0] = "1";
                            [1] = "2";
                            [2] = "3";
                            [3] = "4";
                            [4] = "5";
                            [5] = "6";
                            [6] = "7";
                            [7] = "8";
                           }


    local edid = {  0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 
                    0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x29, 0x15, 0x01, 0x03, 0x80, 0x00, 0x00, 0x78, 
                    0x12, 0x0d, 0xc9, 0xa0, 0x57, 0x47, 0x98, 0x27, 
                    0x12, 0x48, 0x4c, 0x00, 0x00, 0x00, 0x01, 0x01, 
                    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
                    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 
                    0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 
                    0x45, 0x00, 0x40, 0x84, 0x63, 0x00, 0x00, 0x1e, 
                    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
                    0x20, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x18, 
                    0x78, 0x0e, 0x46, 0x0f, 0x00, 0x0a, 0x20, 0x20, 
                    0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x10, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 
                    0x02, 0x03, 0x12, 0xb1, 0x67, 0x03, 0x0c, 0x00, 
                    0x10, 0x00, 0x10, 0x1e, 0x41, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18
                 }

    local multicast = hdoip.pipe.getParam(hdoip.pipe.REG_MULTICAST_EN)
    if(multicast == "true") then
        multicast = 1
    else
        multicast = 0
    end

    local vertical_pixels = 0

    if(t.sent == nil) then

        t.resolution_0 = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_RESOLUTION)

        t.own_pixelclock = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_PIXELCLOCK)
        t.own_interlaced = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_INTERLACED)
        t.own_horizontal_active = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_ACTIVE)
        t.own_vertical_active = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_ACTIVE)
        t.own_horizontal_blanking = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_BLANK)
        t.own_vertical_blanking = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_BLANK)
        t.own_horizontal_offset = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_OFFSET)
        t.own_vertical_offset = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_OFFSET)
        t.own_horizontal_pulse = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_PULSE)
        t.own_vertical_pulse = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_PULSE)

        t.audio_channels = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_CHANNELS)

        t.basic_audio = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_BIT), 1)
        t.audio_bit_16 = t.basic_audio
        t.audio_bit_20 = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_BIT), 2)
        t.audio_bit_24 = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_BIT), 3)

        t.audio_frequency_32 = t.basic_audio
        t.audio_frequency_44 = t.basic_audio
        t.audio_frequency_48 = t.basic_audio
        t.audio_frequency_88 = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_FS), 4)
        t.audio_frequency_96 = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_FS), 5)
        t.audio_frequency_176 = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_FS), 6)
        t.audio_frequency_192 = get_character(hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_FS), 7)

        -- if multicast is enabled, default edid is used
        if(multicast == 1) then
            t.edid_mode_default = 1
            t.edid_mode_receiver = 0
        else
            t.edid_mode_default = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_MODE)
            if(t.edid_mode_default == "default") then
                t.edid_mode_default = 1
                t.edid_mode_receiver = 0
            else
                t.edid_mode_default = 0
                t.edid_mode_receiver = 1
            end
        end

    else

        if (t.resolution_0 == nil) then
            t.resolution_0 = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_RESOLUTION)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_RESOLUTION, t.resolution_0)
        end

        if (t.own_pixelclock == nil) then
            t.own_pixelclock = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_PIXELCLOCK)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_PIXELCLOCK, t.own_pixelclock)
        end

        if (t.own_interlaced == nil) then
            t.own_interlaced = 0
        end
        hdoip.pipe.setParam(hdoip.pipe.REG_EDID_INTERLACED, t.own_interlaced)

        if (t.own_horizontal_active == nil) then
            t.own_horizontal_active = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_ACTIVE)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_H_ACTIVE, t.own_horizontal_active)
        end

        if (t.own_vertical_active == nil) then
            t.own_vertical_active = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_ACTIVE)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_V_ACTIVE, t.own_vertical_active)
        end

        if (t.own_horizontal_blanking == nil) then
            t.own_horizontal_blanking = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_BLANK)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_H_BLANK, t.own_horizontal_blanking)
        end

        if (t.own_vertical_blanking == nil) then
            t.own_vertical_blanking = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_BLANK)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_V_BLANK, t.own_vertical_blanking)
        end

        if (t.own_horizontal_offset == nil) then
            t.own_horizontal_offset = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_OFFSET)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_H_OFFSET, t.own_horizontal_offset)
        end

        if (t.own_vertical_offset == nil) then
            t.own_vertical_offset = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_OFFSET)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_V_OFFSET, t.own_vertical_offset)
        end

        if (t.own_horizontal_pulse == nil) then
            t.own_horizontal_pulse = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_H_PULSE)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_H_PULSE, t.own_horizontal_pulse)
        end

        if (t.own_vertical_pulse == nil) then
            t.own_vertical_pulse = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_V_PULSE)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_V_PULSE, t.own_vertical_pulse)
        end

        if (t.audio_channels == nil) then
            t.audio_channels = hdoip.pipe.getParam(hdoip.pipe.REG_EDID_AUDIO_CHANNELS)
        else
            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_AUDIO_CHANNELS, t.audio_channels)
        end

        if (t.basic_audio == nil) then
            t.basic_audio = 0
        end
        t.audio_bit_16 = t.basic_audio

        if ((t.audio_bit_20 == nil) or (t.basic_audio == 0)) then
            t.audio_bit_20 = 0
        end
        if ((t.audio_bit_24 == nil) or (t.basic_audio == 0)) then
            t.audio_bit_24 = 0
        end
        local string = "   "
        string = set_character(string, 1, t.audio_bit_16)
        string = set_character(string, 2, t.audio_bit_20)
        string = set_character(string, 3, t.audio_bit_24)
        hdoip.pipe.setParam(hdoip.pipe.REG_EDID_AUDIO_BIT, string)

        t.audio_frequency_32 = t.basic_audio
        t.audio_frequency_44 = t.basic_audio
        t.audio_frequency_48 = t.basic_audio

        if ((t.audio_frequency_88 == nil) or (t.basic_audio == 0)) then
            t.audio_frequency_88 = 0
        end
        if ((t.audio_frequency_96 == nil) or (t.basic_audio == 0)) then
            t.audio_frequency_96 = 0
        end
        if ((t.audio_frequency_176 == nil) or (t.basic_audio == 0)) then
            t.audio_frequency_176 = 0
        end
        if ((t.audio_frequency_192 == nil) or (t.basic_audio == 0)) then
            t.audio_frequency_192 = 0
        end
        local string = "       "
        string = set_character(string, 1, t.audio_frequency_32)
        string = set_character(string, 2, t.audio_frequency_44)
        string = set_character(string, 3, t.audio_frequency_48)
        string = set_character(string, 4, t.audio_frequency_88)
        string = set_character(string, 5, t.audio_frequency_96)
        string = set_character(string, 6, t.audio_frequency_176)
        string = set_character(string, 7, t.audio_frequency_192)
        hdoip.pipe.setParam(hdoip.pipe.REG_EDID_AUDIO_FS, string)

        -- set manufacturer id and display product name
        if (t.version_label == "rioxo") then
            edid[9]  = 0x4b
            edid[10] = 0x0f
            edid[78] = 0x52
            edid[79] = 0x49
            edid[80] = 0x4f
            edid[81] = 0x58
            edid[82] = 0x4f
            edid[83] = 0x0a
            edid[84] = 0x20
            edid[85] = 0x20
            edid[86] = 0x20
            edid[87] = 0x20
        elseif (t.version_label == "emcore") then
            edid[9] =  0x15
            edid[10] = 0xa3
            edid[78] = 0x45
            edid[79] = 0x4d
            edid[80] = 0x43
            edid[81] = 0x4f
            edid[82] = 0x52
            edid[83] = 0x45
            edid[84] = 0x0a
            edid[85] = 0x20
            edid[86] = 0x20
            edid[87] = 0x20
        elseif (t.version_label == "black box") then
            edid[9] =  0x08
            edid[10] = 0x58
            edid[78] = 0x42
            edid[79] = 0x4c
            edid[80] = 0x41
            edid[81] = 0x43
            edid[82] = 0x4b
            edid[83] = 0x20
            edid[84] = 0x42
            edid[85] = 0x4f
            edid[86] = 0x58
            edid[87] = 0x0a
        end

        -- set manufacturing date and serial number 
        serial = hdoip.pipe.getParam(hdoip.pipe.REG_SERIAL)
        year = string.sub(serial, 1, 4)
        week = string.sub(serial, 5, 6)
        serial = hdoip.convert.num2hex(tonumber(string.sub(serial, 7, 14))) 

        -- expand string to 8 byte length
        len = string.len(serial)
        for i=len+1, 8, 1 do
            serial = '0' .. serial
        end

        edid[13] = tonumber(string.sub(serial, 7, 8), 16)
        edid[14] = tonumber(string.sub(serial, 5, 6), 16)
        edid[15] = tonumber(string.sub(serial, 3, 4), 16)
        edid[16] = tonumber(string.sub(serial, 1, 2), 16)
        edid[17] = tonumber(week)
        edid[18] = tonumber(year)-1990

        -- set detailed timing descriptor
        edid[55] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][1] / 10)
        edid[56] = get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][1] / 10)
        edid[57] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][3])
        edid[58] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][5])
        edid[59] = (get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][3]) * 16) + get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][5])
        edid[60] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][4])
        edid[61] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][6])
        edid[62] = (get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][4]) * 16) + get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][6])
        edid[63] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][7])
        edid[64] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][8])
        edid[65] = (get_low_nibble(timing[menu_items[tonumber(t.resolution_0)]][9]) * 16) + get_low_nibble(timing[menu_items[tonumber(t.resolution_0)]][10])
        edid[66] = (get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][7]) * 64) + (get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][8]) * 16) + (get_high_nibble(timing[menu_items[tonumber(t.resolution_0)]][9]) * 4) + get_low_nibble(timing[menu_items[tonumber(t.resolution_0)]][10])

        -- display size (aspect ratio = vertical pixels / horizontal pixels)
        if ((timing[menu_items[tonumber(t.resolution_0)]][2]) == 1) then
            -- if interlaced, vertical size = 2 * field
            vertical_pixels = timing[menu_items[tonumber(t.resolution_0)]][4] * 2
        else
            vertical_pixels = timing[menu_items[tonumber(t.resolution_0)]][4]
        end
        edid[67] = get_low_byte(timing[menu_items[tonumber(t.resolution_0)]][3])
        edid[68] = get_low_byte(vertical_pixels)
        edid[69] = (get_high_byte(timing[menu_items[tonumber(t.resolution_0)]][3]) * 16) + get_high_byte(vertical_pixels)

        if ((timing[menu_items[tonumber(t.resolution_0)]][2]) == 1) then
            edid[72] = 0x9e
        else
            edid[72] = 0x1e
        end

        -- basic audio support
        if (tonumber(t.basic_audio) == 1) then
            -- adjust size (+4 (size of audio block))
            edid[131] = 0x12
            -- set basic audio flag
            edid[132] = 0xb1 + 64
        else
            edid[131] = 0x0e
            edid[132] = 0xb1
        end

        -- short video descriptor (same as preferred timing)
        edid[142] = timing[menu_items[tonumber(t.resolution_0)]][11]

        -- audio data block (only if basic audio is supported)
        if (tonumber(t.basic_audio) == 1) then
            edid[143] = 0x23
            -- audio count of channels
            edid[144] = t.audio_channels + 8
            -- audio sampling frequencies
            edid[145] = (t.audio_frequency_32) + (t.audio_frequency_44 * 2) + (t.audio_frequency_48 * 4) + (t.audio_frequency_88 * 8) + (t.audio_frequency_96 * 16) + (t.audio_frequency_176 * 32) + (t.audio_frequency_192 * 64)
            -- audio bitrate
            edid[146] = (t.audio_bit_16) + (t.audio_bit_20 * 2) + (t.audio_bit_24 * 4)
        end

        -- write generated EDID to flash
        file = io.open("/mnt/config/edid.bin", "w")
        for i=1, (256), 1 do
            file:write(string.char(edid[i]))
        end
        file:close()

        -- reload EDID
        hdoip.pipe.reload_edid()

        if(t.edid_mode_default == nil) then
            t.edid_mode_default =  hdoip.pipe.getParam(hdoip.pipe.REG_EDID_MODE)
        end

        -- set EDID mode only if unicast (multicast = default EDID)
        if(multicast == 0) then
            if(t.edid_mode_default == "default") then
	            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_MODE, "default")
	            t.edid_mode_default = 1
	            t.edid_mode_receiver = 0
	        else
	            hdoip.pipe.setParam(hdoip.pipe.REG_EDID_MODE, "receiver")
	            t.edid_mode_default = 0
	            t.edid_mode_receiver = 1 
	        end
        end
    end

    if (tonumber(t.basic_audio) == 0) then
        basic_audio_inverted = 1
    else
        basic_audio_inverted = 0
    end

    if (t.version_label == "rioxo") then
        page_name = label.page_name_rioxo
    elseif (t.version_label == "emcore") then
        page_name = label.page_name_emcore
    elseif (t.version_label == "black box") then
        page_name = label.page_name_black_box
    else
        page_name = ""
    end

    hdoip.html.Header(t, page_name .. label.page_edid, script_path)
    hdoip.html.FormHeader(script_path, main_form)

    if(t.mode_vtb) then 
        hdoip.html.TableHeader(2)
        hdoip.html.Title("EDID")                                                                                                                        hdoip.html.TableInsElement(2);
        hdoip.html.FormRadioSingle("edid_mode_default", "receiver", label.p_edid_mode_receiver, tonumber(t.edid_mode_receiver), tonumber(multicast))    hdoip.html.TableInsElement(2);
        hdoip.html.FormRadioSingle("edid_mode_default", "default", label.p_edid_mode_default, tonumber(t.edid_mode_default), tonumber(multicast))
      
        if(multicast == 1) then
            hdoip.html.Text(" (enabled because of multicast)");
        end
                                                                                                                                                        hdoip.html.TableInsElement(2);
        hdoip.html.Title("Video")                                                                                                                       hdoip.html.TableInsElement(2);
        hdoip.html.Text("Supported resolution");                                                                                                        hdoip.html.TableInsElement(1);
        hdoip.html.DropdownBoxEdid("resolution_0", menu_items, menu_items_count, t.resolution_0, t.edid_mode_receiver)                                  hdoip.html.TableInsElement(1);
        hdoip.html.TableBottom()

        hdoip.html.TableHeader(3)
        hdoip.html.Title("Own resolution:");                                                                                                            hdoip.html.TableInsElement(3);
        hdoip.html.Text("");                                                                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.Text("Horizontal");                                                                                                                  hdoip.html.TableInsElement(1);
        hdoip.html.Text("Vertical");                                                                                                                    hdoip.html.TableInsElement(1);
        hdoip.html.Text("Active pixels/lines");                                                                                                         hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_horizontal_active", t.own_horizontal_active, 4, t.edid_mode_receiver);                                                 hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_vertical_active", t.own_vertical_active, 4, t.edid_mode_receiver);                                                     hdoip.html.TableInsElement(1);
        hdoip.html.Text("Blanking pixels/lines");                                                                                                       hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_horizontal_blanking", t.own_horizontal_blanking, 4, t.edid_mode_receiver);                                             hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_vertical_blanking", t.own_vertical_blanking, 4, t.edid_mode_receiver);                                                 hdoip.html.TableInsElement(1);
        hdoip.html.Text("Sync offset");                                                                                                                 hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_horizontal_offset", t.own_horizontal_offset, 4, t.edid_mode_receiver);                                                 hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_vertical_offset", t.own_vertical_offset, 4, t.edid_mode_receiver);                                                     hdoip.html.TableInsElement(1);
        hdoip.html.Text("Pulse width");                                                                                                                 hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_horizontal_pulse", t.own_horizontal_pulse, 4, t.edid_mode_receiver);                                                   hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_vertical_pulse", t.own_vertical_pulse, 4, t.edid_mode_receiver);                                                       hdoip.html.TableInsElement(1);
        hdoip.html.Text("Pixel clock");                                                                                                                 hdoip.html.TableInsElement(1);
        hdoip.html.FormText("own_pixelclock", t.own_pixelclock, 6, t.edid_mode_receiver);
        hdoip.html.Text("kHz");                                                                                                                         hdoip.html.TableInsElement(2);
        hdoip.html.Text("Interlaced");                                                                                                                  hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("own_interlaced", 1, "", tonumber(t.own_interlaced), t.edid_mode_receiver)                                              hdoip.html.TableInsElement(2);
        hdoip.html.TableBottom()

        hdoip.html.TableHeader(2)
        hdoip.html.Title("Audio")                                                                                                                       hdoip.html.TableInsElement(2);
        hdoip.html.Text("Basic audio support");                                                                                                         hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("basic_audio", 1, "", tonumber(t.basic_audio), t.edid_mode_receiver)                                                    hdoip.html.TableInsElement(1);

        hdoip.html.Text("Channels");                                                                                                                    hdoip.html.TableInsElement(1);
        hdoip.html.DropdownBoxEdid("audio_channels", channel_items, channel_items_count, t.audio_channels, t.edid_mode_receiver)                        hdoip.html.TableInsElement(1);
        hdoip.html.Text("Bit rates");                                                                                                                   hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("audio_bit_16", 1, "", tonumber(t.audio_bit_16), 1)
        hdoip.html.Text("16");
        hdoip.html.FormCheckbox("audio_bit_20", 1, "", tonumber(t.audio_bit_20), t.edid_mode_receiver + basic_audio_inverted)
        hdoip.html.Text("20");
        hdoip.html.FormCheckbox("audio_bit_24", 1, "", tonumber(t.audio_bit_24), t.edid_mode_receiver + basic_audio_inverted)
        hdoip.html.Text("24");                                                                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text("Sampling frequencies");                                                                                                        hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("audio_frequency_32", 1, "", tonumber(t.audio_frequency_32), 1)
        hdoip.html.Text("32kHz");
        hdoip.html.FormCheckbox("audio_frequency_44", 1, "", tonumber(t.audio_frequency_44), 1)
        hdoip.html.Text("44.1kHz");
        hdoip.html.FormCheckbox("audio_frequency_48", 1, "", tonumber(t.audio_frequency_48), 1)
        hdoip.html.Text("48kHz");
        hdoip.html.FormCheckbox("audio_frequency_88", 1, "", tonumber(t.audio_frequency_88), t.edid_mode_receiver + basic_audio_inverted)
        hdoip.html.Text("88.2kHz");
        hdoip.html.FormCheckbox("audio_frequency_96", 1, "", tonumber(t.audio_frequency_96), t.edid_mode_receiver + basic_audio_inverted)
        hdoip.html.Text("96kHz");
        hdoip.html.FormCheckbox("audio_frequency_176", 1, "", tonumber(t.audio_frequency_176), t.edid_mode_receiver + basic_audio_inverted)
        hdoip.html.Text("176.4kHz");
        hdoip.html.FormCheckbox("audio_frequency_192", 1, "", tonumber(t.audio_frequency_192), t.edid_mode_receiver + basic_audio_inverted)
        hdoip.html.Text("192kHz");                                                                                                                      hdoip.html.TableInsElement(1);
        hdoip.html.TableBottom()
    else
        hdoip.html.TableHeader(2)
        hdoip.html.Title("EDID")                                                                                                                        hdoip.html.TableInsElement(2);
        hdoip.html.Text(label.p_edid_transmitter_only)                                                                                                  hdoip.html.TableInsElement(2);
        hdoip.html.TableBottom()
    end

    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end

