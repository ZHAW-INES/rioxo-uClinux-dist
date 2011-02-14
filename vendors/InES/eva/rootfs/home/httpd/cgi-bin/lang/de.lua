#!/usr/bin/lua
module (..., package.seeall)

label = {
    page_name = "Rioxo : ";

    u_decimal = "[dezimal]";
    u_mbps = "[MBit / s]";
    u_ms = "[ms]";

    device_name = "Ger&auml;tename : ";

    lang_en = "Englisch";
    lang_de = "Deutsch";

    on = "Ein";
    off = "Aus";
    
    username = "Benutzername";
    password = "Passwort";
    retype_password = "Passwort bestätigen";

    -- Button 
    button_submit = "Best&auml;tigen"; 
    button_default = "Undo";
    button_save = "Speichern";
    button_dump = "Auslesen";
    button_edit = "Editieren";
    button_logout = "Ausloggen";
    button_login = "Anmelden";
    button_restore = "Wiederherstellen";

    -- Errors
    err_ip_not_valid = "IP-Adresse ist nicht g&uuml;ltig";
    err_subnet_not_valid = "Subnetzmaske ist nicht g&uuml;ltig";
    err_gateway_not_valid = "Netz&uuml;bergang ist nicht g&uuml;ltig";
    err_aud_port_not_in_range = "Audio port ist nicht im G&uuml;ltigkeitsbereich (0 .. 65535)";
    err_aud_port_not_number = "Audio Port ist keine Nummer";
    err_vid_port_not_in_range = "Video port ist nicht im G&uuml;ltigkeitsbereich (0 .. 65535)";
    err_vid_port_not_number = "Video Port ist keine Nummer";
    err_datarate_not_number = "Datenrate ist keine Nummer";
    err_net_delay_not_number = "Max. Netzwerkverz&ouml;gerung ist keine Nummer";

    -- Page names
    page_streaming = "Stream Konfiguration";
    page_ethernet = "System Konfiguration";
    page_firmware = "Firmware";
    page_status = "Status";
    page_default = "Fabrikeinstellungen wiederherstellen";
    page_settings = "Einstellungen";
    page_login = "Anmelden";
    tab_streaming = "Stream";
    tab_ethernet = "Netzwerk";
    tab_firmware = "Firmware";
    tab_status = "Status";
    tab_default = "Zur&uuml;cksetzen";
    tab_settings = "Einstellungen";

    -- Page default
    p_df_question = "Ger&aumlt auf Werkeinstellungen zur&uuml;cksetzten?";

    -- Page login
    p_lg_success = "Anmelden war erfolgreich";
    p_lg_wrong = "Falscher Benutzername oder Passwort";

    -- Page settings
    p_set_title = "Einstellungen";
    p_set_lang = "Sprache";
    p_set_auth = "Authentifizierung";
    p_set_change_user = "Benutzername & Passwort &auml;ndern";
    p_set_old_password = "Altes Passwort";
    p_set_new_password = "Neues Passwort";
    p_set_new_password_retype = "Neues Passwort best&auml;tigen";
    p_set_err_old_password = "Altes Passwort ist falsch";
    p_set_err_new_password = "Neue Passw&ouml;rter sind nicht identisch";

    -- Page status
    p_stat_title = "Status";

    -- Page firmware
    p_fw_fpga_ver = "Gateware";
    p_fw_sopc_ver = "SOPC";
    p_fw_software_ver = "Software";
    p_fw_upload = "Neue Firmware raufladen";
    p_fw_act_firmware = "Aktuelle Firmware-Version";

    -- Page ethernet
    p_eth_vrb = "Empf&auml;nger (Monitor)";
    p_eth_vtb = "Sender (Quelle)";
    p_eth_none = "Passiv";
    p_eth_mac = "MAC-Adresse";
    p_eth_ip = "IP-Adresse";
    p_eth_gateway = "Netz&uuml;bergang";
    p_eth_subnet = "Subnetzmaske";
    p_eth_mode = "Systemmodus";
    p_eth_dev_name = "Ger&auml;tename";

    -- Page streaming
    p_st_video = "Video";
    p_st_audio = "Audio";
    p_st_connect = "Verbinden mit";
    p_st_media_sel = "Media Selektierung";
    p_st_vid_port = "Video UDP Port";
    p_st_aud_port = "Audio UDP Port";
    p_st_datarate = "Datenrate";
    p_st_net_delay = "Max. Netzwerkverz&ouml;gerung";

}
