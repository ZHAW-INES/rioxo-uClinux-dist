#!/usr/bin/lua
module (..., package.seeall)

label = {
    page_name = "Black Box&reg; : ";
    
    u_decimal = "[decimal]";
    u_mbps = "[MBit / s]";
    u_ms = "[ms]";
    u_s = "[s]";
    u_percent = "[% of half datarate]";

    n_none = "off";
    n_1_2 = "1/2";
    n_1_3 = "1/3";
    n_1_4 = "1/4";

    device_name = "Device name : ";

    lang_en = "English";
    lang_de = "German";

    on = "on";
    off = "off";
    play = "play";
    stop = "stop";
    reconnect = "reconnect";

    username = "User name";
    password = "Password";
    retype_password = "Retype password";

    -- Button
    button_submit = "Apply"; 
    button_default = "Undo";
    button_save = "Save";
    button_dump = "Dump";
    button_edit = "Edit";
    button_logout = "Logout";
    button_login = "Login";
    button_restore = "Restore";
    button_yes = "yes";
    button_no = "no";
    button_cancel = "cancel";
    button_save_and_reboot = "save and reboot";
    button_focus_1080p60 = "focus 1080p60";
    button_focus_1080p24 = "focus 1080p24";
    button_focus_720p60  = "focus  720p60";

    -- Errors
    err_ip_not_valid = "IP is not valid";
    err_subnet_not_valid = "Subnetmask is not valid";
    err_gateway_not_valid = "Gateway is not valid";
    err_aud_port_not_in_range = "Audio port is not in range (0 .. 65535)";
    err_aud_port_not_number = "Audio port is not a number";
    err_aud_port_not_even = "Audio port must be even";
    err_vid_port_not_in_range = "Video port is not in range (0 .. 65535)";
    err_vid_port_not_number = "Video port is not a number";
    err_vid_port_not_even = "Video port must be even";
    err_aud_port_cross_vid_port = "Audio port must not be in range (video port - 4) .. (video port + 4)";
    err_aud_port_cross_aud_port = "Audio port must not be in range (audio port - 4) .. (audio port + 4)";
    err_vid_port_cross_aud_port = "Video port must not be in range (audio port - 4) .. (audio port + 4)";
    err_rscp_port_not_in_range = "RSCP port is not in range (0 .. 65535)";
    err_hello_port_not_in_range = "HELLO port is not in range (0 .. 65535)";
    err_rscp_port_not_number = "RSCP port is not a number";
    err_hello_port_not_number = "HELLO port is not a number";
    err_datarate_not_number = "Max. datarate is not a number";
    err_datarate_not_in_range = "Max. datarate is not in range (1 .. 800)";
    err_datarate_chroma_not_in_range = "Max. chroma datarate is not in range (0 .. 100%)";
    err_net_delay_not_number = "Max. network delay is not a number";
    err_av_delay_not_number = "Audio-video delay is not a number";
    err_net_delay_not_in_range = "Max. network delay is not in range (1 .. 100)";
    err_av_delay_not_in_range = "Audio-video delay is not in range (-100 .. 100)";
    err_osd_time_not_number = "OSD time is not a number";
    err_osd_time_not_in_range = "OSD time is not in range (0 .. 100)";

    -- Page names
    page_streaming = "Stream configuration";
    page_audio = "Stream configuration";
    page_ethernet = "System configuration";
    page_firmware = "Firmware";
    page_status = "Status";
    page_usb = "USB";
    page_fec = "Forward Error Correction";
    page_test = "Test";
    page_edid = "EDID - Extended display identification data";
    page_default = "Factory defaults";
    page_restart = "Reboot";
    page_settings = "Settings";
    page_login = "Login";
    tab_streaming = "Stream";
    tab_audio = "Audio";
    tab_ethernet = "Ethernet";
    tab_firmware = "Firmware";
    tab_status = "Status";
    tab_default = "Factory defaults";
    tab_settings = "Settings";
    tab_login = "Login";
    tab_usb = "USB";
    tab_fec = "FEC";
    tab_test = "Test";
    tab_edid = "EDID";

    -- Page default
    p_df_question = "Restore factory defaults?";

    -- Page login
    p_lg_success = "Login was successful";
    p_lg_wrong = "Wrong user name or password";
   
    -- Page settings 
    p_set_title = "Settings";
    p_set_lang = "Language";
    p_set_auth = "Authentication";
    p_set_change_user = "Change user name & password";
    p_set_old_password = "Old password";
    p_set_new_password = "New password";
    p_set_new_password_retype = "Retype new password";
    p_set_err_old_password = "Old password is wrong";
    p_set_err_new_password = "New passwords do not match";

    -- Page status
    p_stat_title = "Status";

    -- Page USB
    p_usb_mode = "Mode";
    p_usb_host = "Host (A device is connected to this unit (USB-HUB, Mouse or Keyboard))";
    p_usb_device = "Device (PC is connected to this unit)";
    p_usb_off = "Disabled";
    p_usb_label_device = "Connected Device";

    -- Page FEC
    p_fec_transmitter_only = "only available on transmitter";
    p_fec_media = "Media type";
    p_fec_video = "Video";
    p_fec_audio_emb = "Audio embedded";
    p_fec_audio_board = "Audio board";
    p_fec_enable = "Enable";
    p_fec_interleave = "Interleaving";
    p_fec_interleave_off = "Off";
    p_fec_interleave_style1 = "SMPTE 2022-1 annex B";
    p_fec_interleave_style2 = "SMPTE 2022-1 annex C";
    p_fec_column_only = "Column only";
    p_fec_l = "L (rows)";
    p_fec_d = "D (columns)";
    p_fec_matrix = "Size of matrix";
    p_fec_overhead = "Overhead";
    p_fec_received_packets = "Received packets";
    p_fec_lost_packets = "Lost packets";
    p_fec_recovered_packets = "Recovered packets";
    p_fec_buffer_status = "Buffer status";
    p_fec_not_available = "Not available";

    -- Page TEST
    p_usb_image = "Test Images";

    -- Page EDID
    p_edid_mode = "EDID - Extended display identification data";
    p_edid_mode_receiver = "Use EDID of receiver";
    p_edid_mode_default = "Use default EDID";
    p_edid_transmitter_only = "only available on transmitter";

    -- Page firmware
    p_serial_number = "Serial number";
    p_fw_fpga_ver = "Gateware";
    p_fw_sopc_ver = "SOPC";
    p_fw_software_ver = "Software";
    p_fw_software_tag = "Tag";
    p_fw_upload = "Upload new Firmware";
    p_fw_act_firmware = "Current Firmware version";
    p_fw_hw_ver = "Hardware version";

    -- Page ethernet
    p_eth_vrb = "Receiver (Monitor)";
    p_eth_vtb = "Transmitter (Source)";
    p_eth_none = "none";
    p_eth_mac = "MAC";
    p_eth_ip = "IP";
    p_eth_gateway = "Gateway";
    p_eth_subnet = "Subnet mask";
    p_eth_mode = "System mode";
    p_eth_dev_name = "Host name";
    p_eth_dev_caption = "Device caption";
    p_eth_ip_dhcp = "Obtain an IP address from a DHCP server";
    p_eth_ip_static = "Specify an IP address";
    p_eth_dns_server = "DNS server";
    p_eth_led = "Identify device";

    -- Page streaming
    p_st_video = "Video";
    p_st_audio_board = "Audio (Audioboard)";
    p_st_audio_emb = "Audio (Videoboard)";
    p_st_connect = "Connect to";
    p_st_media_sel = "Media select";
    p_st_vid_port = "Video UDP port";
    p_st_aud_emb_port = "Audio (embedded) UDP port";
    p_st_aud_board_port = "Audio (audio board) UDP port";
    p_st_rscp_port = "RSCP TCP Port";
    p_st_hello_port = "HELLO UDP Port";
    p_st_datarate = "Max. datarate";
    p_st_dec_chroma = "Chroma datarate";
    p_st_net_delay = "Max. network delay";
    p_st_av_delay = "Audio-video delay";
    p_st_fps_divide = "Reduce refresh rate";
    p_st_eit_only = "(only with EIT software decoder)";
    p_st_auto_stream = "Auto stream";
    p_st_traffic_shaping = "Traffic shaping";
    p_st_force_hdcp = "Force HDCP"; 
    p_st_multicast_en = "Multicast enable";
    p_st_multicast_group = "Multicast group";
    p_st_unicast = "Unicast";
    p_st_multicast = "Multicast";
    p_osd_time = "OSD time";

    -- Page audio
    p_aud_source_sel = "Audio source select";
    p_aud_mic = "Microphone";
    p_aud_line_in = "Line In";
    p_aud_hpgain = "Headphones gain (0..100)";
    p_aud_hpgain_warning = "Gain can just be defined in the range of 0 to 100";
    p_aud_linegain = "Line in gain (0..100)";
    p_aud_linegain_warning = "Gain can just be defined in the range of 0 to 100";
    p_aud_mic_label = "Microphone boost";
    p_aud_mic_boost = "+20dB";
    p_aud_board = "Audio card: ";
    p_aud_mute = "Mute";

    -- Page reboot
    p_rb_desc = "The new setting will take effect only after a restart of the device.<br>Should the device be restarted?<br>";
}

