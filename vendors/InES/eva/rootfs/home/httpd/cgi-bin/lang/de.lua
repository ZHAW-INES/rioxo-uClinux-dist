#!/usr/bin/lua
module (..., package.seeall)

label = {
    page_name_rioxo = "Rioxo&reg; : ";
    page_name_emcore = "Emcore : ";    
    page_name_black_box = "Black Box&reg; : ";

    u_decimal = "[dezimal]";
    u_mbps = "[MBit / s]";
    u_ms = "[ms]";
    u_s = "[s]";
    u_percent = "[% von der halben Datenrate]";

    n_none = "Aus";
    n_1_2 = "1/2";
    n_1_3 = "1/3";
    n_1_4 = "1/4";

    device_name = "Ger&auml;tename : ";

    lang_en = "Englisch";
    lang_de = "Deutsch";

    on = "Ein";
    off = "Aus";
    play = "abspielen";
    stop = "anhalten";
    reconnect = "wiederverbinden";
   
    username = "Benutzername";
    password = "Passwort";
    retype_password = "Passwort bestätigen";

    -- Button 
    button_submit = "Best&auml;tigen"; 
    button_default = "Zur&uuml;cksetzen";
    button_save = "Speichern";
    button_dump = "Auslesen";
    button_edit = "Editieren";
    button_logout = "Ausloggen";
    button_login = "Anmelden";
    button_restore = "Wiederherstellen";
    button_yes = "Ja";
    button_no = "Nein";
    button_cancel = "Abbrechen";
    button_save_and_reboot ="Speichern und Neustart";
    button_focus_1080p60 = "focus 1080p60";
    button_focus_1080p24 = "focus 1080p24";
    button_focus_720p60  = "focus  720p60";

    -- Errors
    err_ip_not_valid = "IP-Adresse ist nicht g&uuml;ltig";
    err_subnet_not_valid = "Subnetzmaske ist nicht g&uuml;ltig";
    err_gateway_not_valid = "Netz&uuml;bergang ist nicht g&uuml;ltig";
    err_aud_port_not_in_range = "Audio Port ist nicht im G&uuml;ltigkeitsbereich (0 .. 65535)";
    err_aud_port_not_number = "Audio Port ist keine Nummer";
    err_aud_port_not_even = "Audio Port muss eine Gerade Zahl sein";
    err_vid_port_not_in_range = "Video Port ist nicht im G&uuml;ltigkeitsbereich (0 .. 65535)";
    err_vid_port_not_number = "Video Port ist keine Nummer";
    err_vid_port_not_even = "Video Port muss eine Gerade Zahl sein";
    err_aud_port_cross_vid_port = "Audio Port darf nicht im Bereich (video port - 4) .. (video port + 4) sein";
    err_aud_port_cross_aud_port = "Audio Port darf nicht im Bereich (audio port - 4) .. (audio port + 4) sein";
    err_vid_port_cross_aud_port = "Video Port darf nicht im Bereich (audio port - 4) .. (audio port + 4) sein";
    err_rtsp_port_not_in_range = "RTSP Port ist nicht im G&uuml;ltigkeitsbereich (0 .. 65535)";
    err_hello_port_not_in_range = "HELLO Port ist nicht im G&uuml;ltigkeitsbereich (0 .. 65535)";
    err_rtsp_port_not_number = "RTSP Port ist keine Nummer";
    err_hello_port_not_number = "HELLO Port ist keine Nummer";
    err_datarate_not_number = "Max. Datenrate ist keine Nummer";
    err_datarate_not_in_range = "Max. Datenrate ist nicht im G&uuml;ltigkeitsbereich (1 .. 800)";
    err_datarate_chroma_not_in_range = "Max. Chroma Datenrate ist nicht im G&uuml;ltigkeitsbereich (0 .. 100%)";
    err_net_delay_not_number = "Max. Netzwerkverz&ouml;gerung ist keine Nummer";
    err_av_delay_not_number = "Audio-video Verz&ouml;gerung ist keine Nummer";
    err_net_delay_not_in_range = "Max. Netzwerkverz&ouml;gerung ist nicht im G&uuml;ltigkeitsbereich (1 .. 100)";
    err_av_delay_not_in_range = "Audio-video Verz&ouml;gerung ist nicht im G&uuml;ltigkeitsbereich (-100 .. 100)";
    err_osd_time_not_number = "OSD Zeit ist keine Nummer";
    err_osd_time_not_in_range = "OSD Zeit ist nicht im G&uuml;ltigkeitsbereich (1 .. 100)";
    err_ttl_not_in_range = "TTL ist nicht im G&uuml;ltigkeitsbereich (0 .. 255)";

    -- Page names
    page_streaming = "Stream Konfiguration";
    page_audio = "Audio Konfiguration";
    page_ethernet = "System Konfiguration";
    page_firmware = "Firmware";
    page_status = "Status";
    page_usb = "USB";
    page_fec = "Fehlerkorrektur";
    page_test = "Test";
    page_edid = "EDID - Extended display identification data";
    page_default = "Fabrikeinstellungen wiederherstellen";
    page_restart = "Neustart";
    page_settings = "Einstellungen";
    page_login = "Anmelden";
    tab_streaming = "Stream";
    tab_audio = "Audio";
    tab_ethernet = "Netzwerk";
    tab_firmware = "Firmware";
    tab_status = "Status";
    tab_default = "Zur&uuml;cksetzen";
    tab_settings = "Einstellungen";
    tab_usb = "USB";
    tab_fec = "FEC";
    tab_test = "Test";
    tab_edid = "EDID";

    -- Page default
    p_df_question = "Ger&auml;t auf Werkeinstellungen zur&uuml;cksetzen?";

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

    -- Page USB
    p_usb_mode = "Betriebsart";
    p_usb_host = "Host (ein USB Ger&auml;t (USB-HUB, Maus oder Tastatur) ist mit diesem Ger&auml;t verbunden)";
    p_usb_device = "Device (dieses Ger&auml;t ist mit einem PC verbunden)";
    p_usb_off = "Deaktiviert";
    p_usb_label_device = "Angeschlossenes Ger&auml;t";

    -- Page FEC
    p_fec_transmitter_only = "Nur beim Sender verf&uuml;gbar";
    p_fec_media = "Media Typ";
    p_fec_video = "Video";
    p_fec_audio_emb = "Audio Embedded";
    p_fec_audio_board = "Audio Board";
    p_fec_enable = "Aktivieren";
    p_fec_interleave = "Interleaving";
    p_fec_interleave_off = "Aus";
    p_fec_interleave_style1 = "SMPTE 2022-1 Anhang B";
    p_fec_interleave_style2 = "SMPTE 2022-1 Anhang C";
    p_fec_column_only = "Nur Spalten";
    p_fec_l = "L (Zeilen)";
    p_fec_d = "D (Spalten)";
    p_fec_matrix = "Matrizengr&ouml;sse";
    p_fec_overhead = "Overhead";
    p_fec_received_packets = "Empfangege Pakete";
    p_fec_lost_packets = "Verlorene Pakete";
    p_fec_recovered_packets = "Wiederhergestellte Pakete";
    p_fec_buffer_status = "Buffer Status";
    p_fec_not_available = "Nicht verf&uuml;gbar";

    -- Page TEST
    p_usb_image = "Testbilder";

    -- Page EDID
    p_edid_mode_receiver = "Benutze EDID vom Monitor";
    p_edid_mode_default = "Benutze EDID Parameter vom untenstehenden Menu";
    p_edid_use_advanced = "Benutze erweiterte Einstellungen";
    p_edid_advanced = "Erweiterte Video Einstellungen";
    p_edid_transmitter_only = "Nur beim Sender verf&uuml;gbar";
    p_edid_supported_resolution = "Unterst&uuml;tzte Aufl&ouml;sung";

    -- Page firmware
    p_serial_number = "Seriennummer";
    p_fw_fpga_ver = "Gateware";
    p_fw_sopc_ver = "SOPC";
    p_fw_software_ver = "Software";
    p_fw_software_tag = "Bezeichnung";
    p_fw_select_file = "Datei ausw&auml;hlen";
    p_fw_note = "Bemerkung: Stream wird gestoppt, da das Ger&auml;t Speicherplatz f&uuml;r die heraufgeladene Datei bereitstellen wird";
    p_fw_upload = "Neue Firmware raufladen";
    p_fw_act_firmware = "Aktuelle Firmware-Version";
    p_fw_hw_ver = "Hardware Version";

    -- Page ethernet
    p_eth_vrb = "Empf&auml;nger (Monitor)";
    p_eth_vtb = "Sender (Quelle)";
    p_eth_none = "Passiv";
    p_eth_mac = "MAC-Adresse";
    p_eth_ip = "IP-Adresse";
    p_eth_gateway = "Netz&uuml;bergang";
    p_eth_subnet = "Subnetzmaske";
    p_eth_mode = "Systemmodus";
    p_eth_dev_name = "Hostname";
    p_eth_dev_caption = "Beschreibung";
    p_eth_ip_dhcp = "IP-Adresse von einem DHCP-Server beziehen";
    p_eth_ip_static = "Feste IP-Adresse setzen";
    p_eth_dns_server = "DNS-Server";
    p_eth_led = "Ger&auml;t identifizieren";

    -- Page streaming
    p_st_video = "Video";
    p_st_audio_board = "Audio (Audioboard)";
    p_st_audio_emb = "Audio (Videoboard)";
    p_st_connect = "Verbinden mit";
    p_st_media_sel = "Media Selektierung";
    p_st_vid_port = "Video UDP Port";
    p_st_aud_emb_port = "Audio (Embedded) UDP Port";
    p_st_aud_board_port = "Audio (Audio Board) UDP Port";
    p_st_rtsp_port = "RTSP TCP Port";
    p_st_datarate = "Max. Datenrate";
    p_st_dec_chroma = "Chroma Datenrate";
    p_st_net_delay = "Max. Netzwerkverz&ouml;gerung";
    p_st_av_delay = "Audio-Video Verz&ouml;gerung";
    p_st_fps_divide = "Bildwiederholfrequenz reduzieren";
    p_st_eit_only = "(nur mit EIT Software Decoder)";
    p_st_auto_stream = "Auto Stream";
    p_st_traffic_shaping = "Traffic shaping";
    p_st_force_hdcp = "HDCP erzwingen"; 
    p_st_multicast_en = "Multicast einschalten";
    p_st_multicast_group = "Multicastgruppe";
    p_st_unicast = "Unicast";
    p_st_multicast = "Multicast";
    p_osd_time = "OSD Zeit";
    p_st_ttl = "IP TTL (time to live)";

    -- Page audio
    p_aud_card = "Audiokarte: ";
    p_aud_source_sel = "Audioquelle w&auml;hlen";
    p_aud_mic = "Mikrofon";
    p_aud_line_in = "Line-In";
    p_aud_hpgain = "Lautst&auml;rke Kopfh&ouml;rer (0..100)";
    p_aud_hpgain_warning = "Laust&auml;rke kann nur im Bereich von 0 bis 100 eingestellt werden";
    p_aud_linegain = "Lautst&auml;rke Line-In  (0..100)";
    p_aud_linegain_warning = "Laust&auml;rke kann nur im Bereich von 0 bis 100 eingestellt werden";
    p_aud_mic_label = "Mikrofon Verst&auml;rkung";
    p_aud_mic_boost = "+20dB";
    p_aud_mute = "Stumm";

    -- Page reboot
    p_rb_desc = "Die ge&auml;nderte Einstellung wird erst nach einem Neustart des Ger&auml;ts wirksam.<br>Soll das Ger&auml;t neu gestartet werden?<br>";
}
