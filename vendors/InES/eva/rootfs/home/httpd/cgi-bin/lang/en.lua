#!/usr/bin/lua
module (..., package.seeall)

label = {
    page_name = "Rioxo : ";
    
    u_decimal = "[decimal]";
    u_mbps = "[MBit / s]";
    u_ms = "[ms]";
 
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

    -- Errors
    err_ip_not_valid = "IP is not valid";
    err_subnet_not_valid = "Subnetmask is not valid";
    err_gateway_not_valid = "Gateway is not valid";
    err_aud_port_not_in_range = "Audio port is not in range (0 .. 65535)";
    err_aud_port_not_number = "Audio port is not a number";
    err_vid_port_not_in_range = "Video port is not in range (0 .. 65535)";
    err_vid_port_not_number = "Video port is not a number";
    err_datarate_not_number = "Datarate is not a number";
    err_net_delay_not_number = "Max. network delay is not a number";

    -- Page names
    page_streaming = "Stream configuration";
    page_ethernet = "System configuration";
    page_firmware = "Firmware";
    page_status = "Status";
    page_default = "Factory defaults";
    page_settings = "Settings";
    page_login = "Login";
    tab_streaming = "Stream";
    tab_ethernet = "Ethernet";
    tab_firmware = "Firmware";
    tab_status = "Status";
    tab_default = "Factory defaults";
    tab_settings = "Settings";
    tab_login = "Login";

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

    -- Page firmware
    p_fw_fpga_ver = "Gateware";
    p_fw_sopc_ver = "SOPC";
    p_fw_software_ver = "Software";
    p_fw_upload = "Upload new Firmware";
    p_fw_act_firmware = "Current Firmware version";

    -- Page ethernet
    p_eth_vrb = "Receiver (Monitor)";
    p_eth_vtb = "Transmitter (Source)";
    p_eth_none = "none";
    p_eth_mac = "MAC";
    p_eth_ip = "IP";
    p_eth_gateway = "Gateway";
    p_eth_subnet = "Subnet mask";
    p_eth_mode = "System mode";
    p_eth_dev_name = "Device name";

    -- Page streaming
    p_st_video = "Video";
    p_st_audio = "Audio";
    p_st_connect = "Connect to";
    p_st_media_sel = "Media select";
    p_st_vid_port = "Video UDP Port";
    p_st_aud_port = "Audio UDP Port";
    p_st_datarate = "Datarate";
    p_st_net_delay = "Max. network delay";
    p_st_auto_stream = "Auto stream";
}

