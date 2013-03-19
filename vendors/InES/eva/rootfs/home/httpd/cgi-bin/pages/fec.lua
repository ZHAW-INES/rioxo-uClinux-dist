#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")

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
        t.fec_video_interleave = string.sub(temp_reg, 4, 4)
        t.fec_video_column_only = string.sub(temp_reg, 5, 5)

        t.fec_audio_en = string.sub(temp_reg, 6, 6)
       -- t.fec_audio_l = string.sub(temp_reg, 7, 7)
       -- t.fec_audio_d = string.sub(temp_reg, 8, 8)
        t.fec_audio_matrix = string.sub(temp_reg, 7, 7)
        t.fec_audio_interleave = string.sub(temp_reg, 9, 9)
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
        if(t.fec_video_interleave == nil) then
            t.fec_video_interleave = 0
        end
        if(t.fec_audio_interleave == nil) then
            t.fec_audio_interleave = 0
        end


        temp_reg = t.fec_video_en .. t.fec_video_matrix .. t.fec_video_matrix .. t.fec_video_interleave .. t.fec_video_column_only .. t.fec_audio_en .. t.fec_audio_matrix .. t.fec_audio_matrix .. t.fec_audio_interleave .. t.fec_audio_column_only

        if(hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING) ~= temp_reg) then
            hdoip.pipe.setParam(hdoip.pipe.REG_FEC_SETTING, temp_reg)
        end

        hdoip.pipe.getParam(hdoip.pipe.REG_FEC_SETTING)
    end

    if (tonumber(t.fec_video_column_only) == 1) then
        overhead_video = 1 / (tonumber(t.fec_video_matrix) + 1) * 100
    else
        numerator_video = (tonumber(t.fec_video_matrix) + 1) + (tonumber(t.fec_video_matrix) + 1)
        denominator_video = (tonumber(t.fec_video_matrix) + 1) * (tonumber(t.fec_video_matrix) + 1)
        overhead_video = numerator_video / denominator_video * 100
    end

    if (tonumber(t.fec_audio_column_only) == 1) then
        overhead_audio = 1 / (tonumber(t.fec_audio_matrix) + 1) * 100
    else
        numerator_audio = (tonumber(t.fec_audio_matrix) + 1) + (tonumber(t.fec_audio_matrix) + 1)
        denominator_audio = (tonumber(t.fec_audio_matrix) + 1) * (tonumber(t.fec_audio_matrix) + 1)
        overhead_audio = numerator_audio / denominator_audio * 100
    end


    hdoip.html.Header(t, label.page_name .. label.page_fec, script_path)
    hdoip.html.FormHeader(script_path, main_form)
    hdoip.html.Title(label.page_fec)
    hdoip.html.TableHeader(3)

    if(t.mode_vtb) then   
                                                                                                                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.Text(label.p_fec_video)                                                                                                                                  hdoip.html.TableInsElement(1);
        hdoip.html.Text(label.p_fec_audio)                                                                                                                                  hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_enable);                                                                                                                                hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_video_en", 1, "", tonumber(t.fec_video_en))                                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_audio_en", 1, "", tonumber(t.fec_audio_en))                                                                                            hdoip.html.TableInsElement(1);

       -- hdoip.html.Text(label.p_fec_l);                                                                                                                                     hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_video_l", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_video_l)                                                                               hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_audio_l", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_audio_l)                                                                               hdoip.html.TableInsElement(1);

       -- hdoip.html.Text(label.p_fec_d);                                                                                                                                     hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_video_d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_video_d)                                                                               hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox10("fec_audio_d", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_audio_d)                                                                               hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_matrix);                                                                                                                                hdoip.html.TableInsElement(1);
        hdoip.html.DropdownBox10("fec_video_matrix", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_video_matrix)                                                                     hdoip.html.TableInsElement(1);
        hdoip.html.DropdownBox10("fec_audio_matrix", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, t.fec_audio_matrix)                                                                     hdoip.html.TableInsElement(1);

       -- hdoip.html.Text(label.p_fec_interleave);                                                                                                                            hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox3("fec_video_interleave", label.p_fec_interleave_off, label.p_fec_interleave_style1, label.p_fec_interleave_style2, t.fec_video_interleave)   hdoip.html.TableInsElement(1);
       -- hdoip.html.DropdownBox3("fec_audio_interleave", label.p_fec_interleave_off, label.p_fec_interleave_style1, label.p_fec_interleave_style2, t.fec_audio_interleave)   hdoip.html.TableInsElement(1);
        hdoip.html.Text(label.p_fec_interleave);                                                                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_video_interleave", 2, "", tonumber(t.fec_video_interleave))                                                                            hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_audio_interleave", 2, "", tonumber(t.fec_audio_interleave))                                                                            hdoip.html.TableInsElement(1);



        hdoip.html.Text(label.p_fec_column_only);                                                                                                                           hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_video_column_only", 1, "", tonumber(t.fec_video_column_only))                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.FormCheckbox("fec_audio_column_only", 1, "", tonumber(t.fec_audio_column_only))                                                                          hdoip.html.TableInsElement(1);

        hdoip.html.Text(label.p_fec_overhead);                                                                                                                              hdoip.html.TableInsElement(1);
        hdoip.html.Text(math.floor(overhead_video + 0.5) .. " %");                                                                                                          hdoip.html.TableInsElement(1);
        hdoip.html.Text(math.floor(overhead_audio + 0.5) .. " %");                                                                                                          hdoip.html.TableInsElement(1);

    else
        hdoip.html.Text(label.p_fec_transmitter_only)                                                                                                                       hdoip.html.TableInsElement(4);
    end

    hdoip.html.TableBottom()
    hdoip.html.FormBottom(t)
    hdoip.html.Bottom(t)
end

