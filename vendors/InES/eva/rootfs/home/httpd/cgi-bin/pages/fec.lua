#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

local function get_str(str, beginstr, endstr)
    local ret_str = str
    local len = string.len(str)
    local len_begin = string.len(beginstr)
    local len_end = string.len(endstr)
    local offset = string.find(str, beginstr)

    if (offset == nil) then
        offset = 0
    end

    ret_str = string.sub(str, offset+len_begin, len)

    offset = string.find(ret_str, endstr)

    if (offset ~=nil) then
        ret_str = string.sub(ret_str, 0, offset-len_end)
    end

    return ret_str
end

-- ------------------------------------------------------------------
-- FEC page
-- ------------------------------------------------------------------
function show(t)

    if(t.sent == nil) then
        temp_reg = hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING)

        t.fec_video_en = string.sub(temp_reg, 1, 1)
       -- t.fec_video_l = string.sub(temp_reg, 2, 2)
       -- t.fec_video_d = string.sub(temp_reg, 3, 3)
        t.fec_video_matrix = string.sub(temp_reg, 2, 2)
       -- t.fec_video_interleave = string.sub(temp_reg, 4, 4)
        t.fec_video_column_only = string.sub(temp_reg, 5, 5)

        t.fec_audio_en = string.sub(temp_reg, 6, 6)
       -- t.fec_audio_l = string.sub(temp_reg, 7, 7)
       -- t.fec_audio_d = string.sub(temp_reg, 8, 8)
        t.fec_audio_matrix = string.sub(temp_reg, 7, 7)
       -- t.fec_audio_interleave = string.sub(temp_reg, 9, 9)
        t.fec_audio_column_only = string.sub(temp_reg, 10, 10)
    else
        if(t.fec_video_en == nil) then
            t.fec_video_en = 0
        end
        if(t.fec_audio_en == nil) then
            t.fec_audio_en = 0
        end
        if(t.fec_video_column_only == nil) then
            t.fec_video_column_only = 0
        end
        if(t.fec_audio_column_only == nil) then
            t.fec_audio_column_only = 0
        end

        temp_reg = hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING)
        temp_reg = t.fec_video_en .. t.fec_video_matrix .. t.fec_video_matrix .. string.sub(temp_reg, 4, 4) .. t.fec_video_column_only .. t.fec_audio_en .. t.fec_audio_matrix .. t.fec_audio_matrix .. string.sub(temp_reg, 9, 9) .. t.fec_audio_column_only

        if(hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING) ~= temp_reg) then
            hdoip.pipe.setParam(hdoip.pipe.REG_FEC_SETTING, temp_reg)
        end

    end

    if (tonumber(t.fec_video_column_only) == 1) then
        overhead_video = 1 / (tonumber(t.fec_video_matrix) + 4) * 100
    else
        numerator_video = (tonumber(t.fec_video_matrix) + 4) + (tonumber(t.fec_video_matrix) + 4)
        denominator_video = (tonumber(t.fec_video_matrix) + 4) * (tonumber(t.fec_video_matrix) + 4)
        overhead_video = numerator_video / denominator_video * 100
    end

    if (tonumber(t.fec_audio_column_only) == 1) then
        overhead_audio = 1 / (tonumber(t.fec_audio_matrix) + 4) * 100
    else
        numerator_audio = (tonumber(t.fec_audio_matrix) + 4) + (tonumber(t.fec_audio_matrix) + 4)
        denominator_audio = (tonumber(t.fec_audio_matrix) + 4) * (tonumber(t.fec_audio_matrix) + 4)
        overhead_audio = numerator_audio / denominator_audio * 100
    end

    local str = hdoip.pipe.getParam(hdoip.pipe.REG_STATUS_FEC)
    local vid_str = get_str(str, "VID", ")")
    local aud_emb_str = get_str(str, "AUD_EMB", ")")
    local aud_board_str = get_str(str, "AUD_BOARD", ")")

    local stat_vid_pkg = get_str(vid_str, "pkg: ", " ")
    local stat_vid_mis = get_str(vid_str, "mis: ", " ")
    local stat_vid_fix = get_str(vid_str, "fix: ", " ")
    local stat_vid_buf = get_str(vid_str, "buf: ", " ")
    local stat_vid_buf_entries = get_str(stat_vid_buf, " ", "/")
    local stat_vid_buf_total = get_str(stat_vid_buf, "/", " ")
    local stat_vid_buf_percent = math.floor(tonumber(stat_vid_buf_entries) * 100 / tonumber(stat_vid_buf_total))

    local stat_aud_emb_pkg = get_str(aud_emb_str, "pkg: ", " ")
    local stat_aud_emb_mis = get_str(aud_emb_str, "mis: ", " ")
    local stat_aud_emb_fix = get_str(aud_emb_str, "fix: ", " ")
    local stat_aud_emb_buf = get_str(aud_emb_str, "buf: ", " ")
    local stat_aud_emb_buf_entries = get_str(stat_aud_emb_buf, " ", "/")
    local stat_aud_emb_buf_total = get_str(stat_aud_emb_buf, "/", " ")
    local stat_aud_emb_buf_percent = math.floor(tonumber(stat_aud_emb_buf_entries) * 100 / tonumber(stat_aud_emb_buf_total))

    local stat_aud_board_pkg = get_str(aud_board_str, "pkg: ", " ")
    local stat_aud_board_mis = get_str(aud_board_str, "mis: ", " ")
    --local stat_aud_board_fix = get_str(aud_board_str, "fix: ", " ")
    local stat_aud_board_fix = label.p_fec_not_available
    local stat_aud_board_buf = get_str(aud_board_str, "buf: ", " ")
    local stat_aud_board_buf_entries = get_str(stat_aud_board_buf, " ", "/")
    local stat_aud_board_buf_total = get_str(stat_aud_board_buf, "/", " ")
    local stat_aud_board_buf_percent = math.floor(tonumber(stat_aud_board_buf_entries) * 100 / tonumber(stat_aud_board_buf_total))

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

    hdoip.html.Header(t, page_name .. label.page_fec, script_path)
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_fec)
    hdoip.html.TableHeader(4)

    hdoip.html.Text(label.p_fec_media)                                                                                                                                      hdoip.html.TableInsElement(1);
    hdoip.html.Text(label.p_fec_video)                                                                                                                                      hdoip.html.TableInsElement(1);
    hdoip.html.Text(label.p_fec_audio_emb)                                                                                                                                  hdoip.html.TableInsElement(1);
    hdoip.html.Text(label.p_fec_audio_board)                                                                                                                                hdoip.html.TableInsElement(1);

    if(t.mode_vtb) then   

        hdoip.html.Text(label.p_fec_enable);                                                                                                                                hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_video_en", 1, "", tonumber(t.fec_video_en))                                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_audio_en", 1, "", tonumber(t.fec_audio_en))                                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.Text("-");                                                                                                                                               hdoip.html.TableInsElement(1);

       -- hdoip.html.Text(label.p_fec_l);                                                                                                                                     hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_video_l", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_video_l)                                                                               hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_audio_l", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_audio_l)                                                                               hdoip.html.TableInsElement(1);

       -- hdoip.html.Text(label.p_fec_d);                                                                                                                                     hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_video_d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_video_d)                                                                               hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_audio_d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_audio_d)                                                                               hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_matrix);                                                                                                                                hdoip.html.TableInsElement(1);
        hdoip.html.DropdownBox7("fec_video_matrix", 4, 5, 6, 7, 8, 9, 10, t.fec_video_matrix)                                                                               hdoip.html.TableInsElement(1);
        hdoip.html.DropdownBox7("fec_audio_matrix", 4, 5, 6, 7, 8, 9, 10, t.fec_audio_matrix)                                                                               hdoip.html.TableInsElement(1);
        hdoip.html.Text("-");                                                                                                                                               hdoip.html.TableInsElement(1);

       -- hdoip.html.Text(label.p_fec_interleave);                                                                                                                            hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox3("fec_video_interleave", label.p_fec_interleave_off, label.p_fec_interleave_style2, label.p_fec_interleave_style1, t.fec_video_interleave)   hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox3("fec_audio_interleave", label.p_fec_interleave_off, label.p_fec_interleave_style2, label.p_fec_interleave_style1, t.fec_audio_interleave)   hdoip.html.TableInsElement(1);
       -- hdoip.html.Text(label.p_fec_interleave);                                                                                                                            hdoip.html.TableInsElement(1);
       -- hdoip.html.FormCheckbox("fec_video_interleave", 1, "", tonumber(t.fec_video_interleave))                                                                            hdoip.html.TableInsElement(1);
       -- hdoip.html.FormCheckbox("fec_audio_interleave", 1, "", tonumber(t.fec_audio_interleave))                                                                            hdoip.html.TableInsElement(1);


        hdoip.html.Text(label.p_fec_column_only);                                                                                                                           hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_video_column_only", 1, "", tonumber(t.fec_video_column_only))                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_audio_column_only", 1, "", tonumber(t.fec_audio_column_only))                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text("-");                                                                                                                                               hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_overhead);                                                                                                                              hdoip.html.TableInsElement(1);
        hdoip.html.Text(math.floor(overhead_video + 0.5) .. " %");                                                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text(math.floor(overhead_audio + 0.5) .. " %");                                                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text("-");                                                                                                                                               hdoip.html.TableInsElement(1);

    else

        hdoip.html.Text(label.p_fec_received_packets)                                                                                                                       hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_vid_pkg)                                                                                                                                       hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_emb_pkg)                                                                                                                                   hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_board_pkg)                                                                                                                                 hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_lost_packets)                                                                                                                           hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_vid_mis)                                                                                                                                       hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_emb_mis)                                                                                                                                   hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_board_mis)                                                                                                                                 hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_recovered_packets)                                                                                                                      hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_vid_fix)                                                                                                                                       hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_emb_fix)                                                                                                                                   hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_board_fix)                                                                                                                                 hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_buffer_status)                                                                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_vid_buf_percent .. " %")                                                                                                                       hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_emb_buf_percent .. " %")                                                                                                                   hdoip.html.TableInsElement(1);
        hdoip.html.Text(stat_aud_board_buf_percent .. " %")                                                                                                                 hdoip.html.TableInsElement(1);

    end

    hdoip.html.TableBottom()
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end

