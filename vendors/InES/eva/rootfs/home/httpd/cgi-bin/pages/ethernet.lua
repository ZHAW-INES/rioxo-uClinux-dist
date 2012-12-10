#!/usr/bin/lua
module (..., package.seeall)
require("hdoip.html")
require("hdoip.pipe")
require("hdoip.network")

REG_SYS_IF_LABEL = "sys_if"
REG_SYS_IP_LABEL = "sys_ip"
REG_SYS_GATEWAY_LABEL = "sys_gw"
REG_SYS_DNS1_LABEL = "sys_dns1_"
REG_SYS_DNS2_LABEL = "sys_dns2_"
REG_SYS_HOST_NAME_LABEL = "sys_hostname"
REG_SYS_DEV_CAPTION_LABEL = "sys_caption"
REG_SYS_MAC_LABEL = "sys_mac"
REG_SYS_SUB_LABEL = "sys_sub"
REG_SYS_DHCP = "sys_dhcp"
REG_MODE_START_LABEL = "sys_mode"

function redirect(t, url) 
    hdoip.html.Header(t, "Redirect to " .. url, script_path) 
    hdoip.html.Title(label.page_ethernet);  
    hdoip.html.Text("Redirect to " .. url .. " in 2 seconds")
    hdoip.html.redirect(url)
    hdoip.html.Bottom(t) 
end

function reboot(t)
    hdoip.html.Header(t, label.page_name .. "Rebooting", script_path)
    hdoip.html.Title("Reboot")
    hdoip.html.Text("Please wait until device is rebooted.")
    hdoip.html.Text("<br>")
    hdoip.html.Loadbar(0, 33, 0)
    hdoip.html.Bottom(t)
    hdoip.pipe.reboot()
    os.exit(0)
end

local function get_network(t) 
    local str
    
    str = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_IP)
    t.sys_ip0, t.sys_ip1, t.sys_ip2, t.sys_ip3 = hdoip.network.text2IpValues(str)
    str = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_SUB)
    t.sys_sub0, t.sys_sub1, t.sys_sub2, t.sys_sub3 = hdoip.network.text2IpValues(str)
    str = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_GW)
    t.sys_gw0, t.sys_gw1, t.sys_gw2, t.sys_gw3 = hdoip.network.text2IpValues(str)
    
    str = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_DNS1)
    if(str == "") then
        t.sys_dns1_0 = ""; t.sys_dns1_1 = ""; t.sys_dns1_2 = ""; t.sys_dns1_3 = ""
    else 
        t.sys_dns1_0, t.sys_dns1_1, t.sys_dns1_2, t.sys_dns1_3 = hdoip.network.text2IpValues(str)
    end 
    
    str = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_DNS2)
    if(str == "") then
        t.sys_dns2_0 = ""; t.sys_dns2_1 = ""; t.sys_dns2_2 = ""; t.sys_dns2_3 = ""
    else
        t.sys_dns2_0, t.sys_dns2_1, t.sys_dns2_2, t.sys_dns2_3 = hdoip.network.text2IpValues(str)
    end 
end


-- ------------------------------------------------------------------
-- Device configuration page
-- ------------------------------------------------------------------
function show(t)
    local new_ip = 0
    local err = 0
    local tmp0,tmp1,tmp2,tmp3
    local t_sys_mode =      {[0] = (label.p_eth_none); [1] = (label.p_eth_vrb); [2] = (label.p_eth_vtb);}
    local t_sys_mode_conv = {[0] = "none"; [1] = "vrb";               [2] = "vtb";}
    local t_sys_mode_reg = {["none"] = 0; ["vrb"] = 1; ["vtb"] = 2}
    local t_sys_ip = {[0] = (label.p_eth_ip_static); [1] = (label.p_eth_ip_dhcp)}

       
    t.sys_mac = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_MAC)

    if(t.sent == nil) then
        t.sys_mode = t_sys_mode_reg[hdoip.pipe.getParam(hdoip.pipe.REG_MODE_START)]
        t.sys_hostname = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_HOST_NAME)
        t.sys_caption = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_DEV_CAPTION)
        
        get_network(t)
        
        t.sys_dhcp = hdoip.pipe.getParam(hdoip.pipe.REG_SYS_DHCP)
        if(t.sys_dhcp == "true") then
            t.sys_dhcp = 1
        else 
            t.sys_dhcp = 0
        end 
        
        if(hdoip.pipe.getParam(hdoip.pipe.REG_IDENTIFICATION) == "14") then
            t.identification = 1;
        else 
            t.identification = 0;
        end

    else
        t.sys_hostname = hdoip.html.unescape(t.sys_hostname)
        hdoip.pipe.setParam(hdoip.pipe.REG_SYS_HOST_NAME, t.sys_hostname)
        
        t.sys_caption = hdoip.html.unescape(t.sys_caption)
        hdoip.pipe.setParam(hdoip.pipe.REG_SYS_DEV_CAPTION, t.sys_caption)
        
        if(t.sys_dhcp == "1") then
            t.sys_dhcp_str = "true"
            t.sys_dhcp = 1
        else
            t.sys_dhcp_str = "false"
            t.sys_dhcp = 0
        end
        
        if(t.identification ~= nil) then
           hdoip.pipe.setParam(hdoip.pipe.REG_IDENTIFICATION, "14")
           t.identification = 1
        else
           hdoip.pipe.setParam(hdoip.pipe.REG_IDENTIFICATION, "15")
           t.identification = 0 
        end

        hdoip.pipe.setParam(hdoip.pipe.REG_SYS_DHCP, t.sys_dhcp_str)

        -- Set network parameter if not in DHCP mode
        if(t.sys_dhcp == 0) then
            -- Set system ip
            if(hdoip.network.checkIp(t.sys_ip0, t.sys_ip1, t.sys_ip2, t.sys_ip3) == 1) then   
	            tmp0,tmp1,tmp2,tmp3 = hdoip.network.text2IpValues(hdoip.pipe.getParam(hdoip.pipe.REG_SYS_IP))
	            if((tmp0 ~= t.sys_ip0) or (tmp1 ~= t.sys_ip1) or (tmp2 ~= t.sys_ip2) or (tmp3 ~= t.sys_ip3)) then
	                new_ip = 1;
                    -- TODO: page must be reloaded first before ip can be changed
	                hdoip.pipe.setParam(hdoip.pipe.REG_SYS_IP, t.sys_ip0.."."..t.sys_ip1.."."..t.sys_ip2.."."..t.sys_ip3)      
	            end
	        else 
	            hdoip.html.AddError(t, label.err_ip_not_valid)
	            err = err + 1
	        end
	        
	        -- Set subnet mask
	        if(hdoip.network.checkIp(t.sys_sub0, t.sys_sub1, t.sys_sub2, t.sys_sub3) == 1) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_SYS_SUB, t.sys_sub0.."."..t.sys_sub1.."."..t.sys_sub2.."."..t.sys_sub3) 
	        else
	            hdoip.html.AddError(t, label.err_subnet_not_valid)
	            err = err + 1
	        end
	
	        -- Set gateway
	        if(hdoip.network.checkIp(t.sys_gw0, t.sys_gw1, t.sys_gw2, t.sys_gw3) == 1) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_SYS_GW, t.sys_gw0.."."..t.sys_gw1.."."..t.sys_gw2.."."..t.sys_gw3) 
	        else
	            hdoip.html.AddError(t, label.err_gateway_not_valid)
	            err = err + 1
	        end
	        
	        -- Set DNS1
	        if(hdoip.network.checkIp(t.sys_dns1_0, t.sys_dns1_1, t.sys_dns1_2, t.sys_dns1_3) == 1) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_SYS_DNS1, t.sys_dns1_0.."."..t.sys_dns1_1.."."..t.sys_dns1_2.."."..t.sys_dns1_3) 
	        else
	            hdoip.pipe.setParam(hdoip.pipe.REG_SYS_DNS1, "")
	        end
	        
	        -- Set DNS2
	        if(hdoip.network.checkIp(t.sys_dns2_0, t.sys_dns2_1, t.sys_dns2_2, t.sys_dns2_3) == 1) then
	            hdoip.pipe.setParam(hdoip.pipe.REG_SYS_DNS2, t.sys_dns2_0.."."..t.sys_dns2_1.."."..t.sys_dns2_2.."."..t.sys_dns2_3) 
	        else
	            hdoip.pipe.setParam(hdoip.pipe.REG_SYS_DNS2, "")
	        end
	    else 
	       get_network(t)
        end

        if(hdoip.pipe.getParam(hdoip.pipe.REG_MODE_START) ~= t_sys_mode_conv[tonumber(t.sys_mode)]) then
            hdoip.pipe.setParam(hdoip.pipe.REG_TEMP5, t_sys_mode_conv[tonumber(t.sys_mode)])
            pages.restart.show(t)
        end

        hdoip.pipe.getParam(hdoip.pipe.REG_SYS_UPDATE)
    end

    if(t.button_restart_yes ~= nil) then
        t.sys_mode_changed = hdoip.pipe.getParam(hdoip.pipe.REG_TEMP5)

        if(t.sys_mode_changed ~= nil) then
            if(t.sys_mode_changed ~= "") then
                hdoip.pipe.setParam(hdoip.pipe.REG_MODE_START, t.sys_mode_changed)
            end
        end

        hdoip.pipe.setParam(hdoip.pipe.REG_IDENTIFICATION, "17")
        hdoip.pipe.store_cfg()
        reboot(t)
    end

    if(t.show_restart == nil) then
        if((new_ip == 0) or (err > 0))then
            hdoip.html.Header(t, label.page_name .. label.page_ethernet, script_path)
            hdoip.html.FormHeader(script_path, main_form)
            hdoip.html.Title(label.page_ethernet);           
            hdoip.html.TableHeader(2)
            hdoip.html.FormRadio(REG_SYS_DHCP, t_sys_ip, 2, t.sys_dhcp)                                             hdoip.html.TableInsElement(2);
            hdoip.html.Text(label.p_eth_mac);                                                                       hdoip.html.TableInsElement(1);
            hdoip.html.Text(t.sys_mac);                                                                             hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_ip);                                                                        hdoip.html.TableInsElement(1);
            hdoip.html.FormIP(REG_SYS_IP_LABEL,t.sys_ip0,t.sys_ip1,t.sys_ip2,t.sys_ip3, t.sys_dhcp);                hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_subnet);                                                                    hdoip.html.TableInsElement(1);
            hdoip.html.FormIP(REG_SYS_SUB_LABEL,t.sys_sub0,t.sys_sub1,t.sys_sub2,t.sys_sub3, t.sys_dhcp);           hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_gateway);                                                                   hdoip.html.TableInsElement(1);
            hdoip.html.FormIP(REG_SYS_GATEWAY_LABEL,t.sys_gw0,t.sys_gw1,t.sys_gw2,t.sys_gw3, t.sys_dhcp);           hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_dns_server.." 1");                                                          hdoip.html.TableInsElement(1);
            hdoip.html.FormIP(REG_SYS_DNS1_LABEL,t.sys_dns1_0,t.sys_dns1_1,t.sys_dns1_2,t.sys_dns1_3, t.sys_dhcp);  hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_dns_server.." 2");                                                          hdoip.html.TableInsElement(1);
            hdoip.html.FormIP(REG_SYS_DNS2_LABEL,t.sys_dns2_0,t.sys_dns2_1,t.sys_dns2_2,t.sys_dns2_3, t.sys_dhcp);  hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_dev_name);                                                                  hdoip.html.TableInsElement(1);
            hdoip.html.FormText(REG_SYS_HOST_NAME_LABEL, t.sys_hostname, 30, 0);                                    hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_dev_caption);                                                               hdoip.html.TableInsElement(1);
            hdoip.html.FormText(REG_SYS_DEV_CAPTION_LABEL, t.sys_caption, 30, 0);                                   hdoip.html.TableInsElement(1);
            hdoip.html.Text(label.p_eth_mode);                                                                      hdoip.html.TableInsElement(1);
            hdoip.html.FormRadio(REG_MODE_START_LABEL, t_sys_mode, 3, t.sys_mode)                                   hdoip.html.TableInsElement(1);

            hdoip.html.Text(label.p_eth_led);                                                                       hdoip.html.TableInsElement(1);
            hdoip.html.FormCheckbox("identification", 1, "", t.identification)                                      hdoip.html.TableInsElement(2);

            hdoip.html.TableBottom()
        
            hdoip.html.FormBottom(t)
            hdoip.html.Bottom(t)
        else
            redirect(t, "http://"..t.sys_ip0.."."..t.sys_ip1.."."..t.sys_ip2.."."..t.sys_ip3)
        end
    end
end


