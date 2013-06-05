#!/usr/bin/lua
module (..., package.seeall)
require ("hdoip.convert")

REG_SYS_UPDATE = "system-update"
REG_SYS_IF = "system-ifname"
REG_SYS_HOST_NAME = "system-hostname"
REG_SYS_DEV_CAPTION = "system-dev-caption"
REG_SYS_IP = "system-ip"
REG_SYS_DNS1 = "system-dns1"
REG_SYS_DNS2 = "system-dns2"
REG_SYS_MAC = "system-mac"
REG_SYS_SUB = "system-subnet"
REG_SYS_GW = "system-gateway"
REG_SYS_DHCP = "system-dhcp"
REG_AUTO_STREAM = "auto-stream"
REG_TRAFFIC_SHAPING = "traffic-shaping"
REG_IDENTIFICATION = "led_instruction"
REG_OSD_TIME = "osd-time"
REG_MODE_START = "mode-start"
REG_ST_MODE_MEDIA = "mode-media"
REG_ST_URI = "remote-uri"
REG_ST_RTSP_PORT = "rtsp-server-port"
REG_ST_BW = "bandwidth"
REG_ST_BW_CHROMA = "chroma-bandwidth"
REG_ST_NET_DELAY = "network-delay"
REG_ST_AV_DELAY = "av-delay"
REG_ST_VID_PORT = "video-port"
REG_ST_AUD_EMB_PORT = "audio-port"
REG_ST_AUD_BOARD_PORT = "audio-port-board"
REG_AUD_SOURCE = "audio-source"
REG_AUD_BOARD  = "audio-board"
REG_AUD_HPGAIN = "audio-hpgain"
REG_AUD_LINEGAIN = "audio-linegain"
REG_AUD_MIC_BOOST = "audio-mic-boost"
REG_AUD_MUTE = "audio-mute"
REG_STATUS_DRIVER = "daemon-driver"
REG_STATUS_STATE = "daemon-state"
REG_STATUS_VTB = "daemon-vtb-state"
REG_STATUS_VRB = "daemon-vrb-state"
REG_STATUS_RSC = "daemon-rsc"
REG_STATUS_ETH = "eth-status"
REG_STATUS_FEC = "fec-status"
REG_STATUS_VSO = "vso-status"
REG_VRB_IS_PLAYING = "vrb_is_playing"
REG_FORCE_HDCP = "hdcp-force"
REG_STATUS_HDCP = "hdcp-status"
REG_STATUS_SYSTEM = "system-state"
REG_STATUS_STREAM = "stream-state"
REG_WEB_LANG = "web-lang"
REG_WEB_AUTH_EN = "web-auth-en"
REG_WEB_USER = "web-user"
REG_WEB_PASS = "web-pass"
REG_AMX_EN = "amx-en"
REG_AMX_HELLO_IP = "amx-hello-ip"
REG_AMX_HELLO_PORT = "amx-hello-port"
REG_AMX_HELLO_MSG = "amx-hello-msg"
REG_AMX_HELLO_INTERVAL = "amx-hello-interval"
REG_MULTICAST_EN = "multicast_en"
REG_MULTICAST_GROUP = "multicast_group"
REG_SERIAL = "serial-number"
REG_MODE_USB = "usb-mode"
REG_FEC_SETTING = "fec_setting"
REG_MODE_TEST_IMAGE = "test-image"
REG_EDID_MODE = "edid-mode"
REG_FPS_DIVIDE = "fps_divide"
REG_TEMP1 = "temp1"
REG_TEMP2 = "temp2"
REG_TEMP3 = "temp3"
REG_TEMP4 = "temp4"
REG_TEMP5 = "temp5"
REG_TEMP6 = "temp6"

local PIPE_CMD = "/tmp/web.cmd"
local PIPE_RSP = "/tmp/web.rsp"

local PIPE_CMD_READY            = "31000008"
local PIPE_CMD_VRB_PLAY         = "31000007"
local PIPE_CMD_GET_PARAM        = "3800000a"
local PIPE_CMD_SET_PARAM        = "3800000b"
local PIPE_CMD_STORE_CFG        = "32000007"
local PIPE_CMD_REMOTE_UPDATE    = "3100000C"
local PIPE_CMD_FREE_BUFFER      = "3400000B"
local PIPE_CMD_GETVERSION       = "3100000D"
local PIPE_CMD_FACTORY_DEFAULT  = "3100000F"
local PIPE_CMD_REBOOT           = "31000009"
local PIPE_CMD_GETUSB           = "31000010"
local PIPE_CMD_GET_HDCP_STATE   = "31000020"

local fd_cmd, fd_rsp = nil, nil

local function fileExist(file)
    local fd = io.open(file)
    if(fd) then
        fd:close()
        return true
    else 
        return false
    end
end


function open()
    fd_cmd = io.open(PIPE_CMD, "w");
    if(fd_cmd ~= nil) then
        fd_rsp = io.open(PIPE_RSP, "r+");
        if(fd_rsp == nil) then
            fd_cmd:close()
            fd_cmd = nil
            return "could not open "..PIPE_RSP.."<br>\n"
        end
        return ""
    end
    return "could not open "..PIPE_CMD.."<br>\n"
end

function close()
    if(fd_rsp ~= nil) then
        fd_rsp:close()

        if(fd_cmd ~= nil) then
            fd_cmd:close()
        end
    end
 
end

local function createCmdHeader(cmd, payload_size)
    str = hdoip.convert.Str2LE_32(cmd)
    str = str .. hdoip.convert.Str2LE_32(string.format("%08x", (payload_size + 8)))
    return str
end

local function simple_cmd(cmd_id)
    if(fd_cmd ~= nil) then
        str = hdoip.convert.Str2HexFile(createCmdHeader(cmd_id, 0))
        fd_cmd:write(str)
        fd_cmd:flush()
    end
end

function setParam(key, value)
    if(fd_cmd ~= nil) then
        key_len = string.len(key)
        value_len = string.len(value)

        str = createCmdHeader(PIPE_CMD_SET_PARAM, (value_len + key_len + 2 + 4))
        str = str .. hdoip.convert.Str2LE_32(string.format("%08x", key_len + 1))
        str = hdoip.convert.Str2HexFile(str) .. key .. string.char(0) .. value .. string.char(0)    
        fd_cmd:write(str)
        fd_cmd:flush()
    end
end

function getParam(key)
    if((fd_cmd ~= nil) and (fd_rsp ~= nil)) then
        local key_len = string.len(key)

        str = createCmdHeader(PIPE_CMD_GET_PARAM, (key_len + 1 + 4))
        str = str .. hdoip.convert.Str2LE_32(string.format("%08x", key_len + 1))
        str = hdoip.convert.Str2HexFile(str) .. key .. string.char(0)

        fd_cmd:write(str)
        fd_cmd:flush()

        -- Read command header
        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(8)
        end
        
        -- cmd = hdoip.convert.bin2hex(string.sub(ret,1,4),4)       NOT USED YET
        -- cmd = hdoip.convert.Str2LE_32(cmd)                       NOT USED YET
        size = hdoip.convert.bin2dec(string.sub(ret,5,8),4)
  
        -- Read payload
        ret = nil 
        while(ret == nil) do
            ret = fd_rsp:read(size - 8)
        end

        offset = hdoip.convert.bin2dec(string.sub(ret,1,4),4)
        value = string.format("%s",string.sub(ret,5,5 + offset - 1))

        return value
    end

    return ""
end

function getVersion(t)
    if((fd_cmd ~= nil) and (fd_rsp ~= nil)) then
        str = createCmdHeader(PIPE_CMD_GETVERSION, 70)
        str = str .. string.format("%08x%08x%08x%08x%08x",0,0,0,0,0)
        
        str = hdoip.convert.Str2HexFile(str) .. '                                                  '

        fd_cmd:write(str)
        fd_cmd:flush()

        -- Read command header
        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(8)
        end
        
        size = hdoip.convert.bin2dec(string.sub(ret,5,8),4)

        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(size - 8)
        end
        
        t.fpga_date = hdoip.convert.bin2dec(string.sub(ret,1,4),4)
        t.fpga_date_str = os.date("%x %X",t.fpga_date)
   
        t.fpga_svn  = hdoip.convert.bin2dec(string.sub(ret,5,8),4)
        t.sopc_date = hdoip.convert.bin2dec(string.sub(ret,9,12),4)
        t.sopc_date_str = os.date("%x %X",t.sopc_date)
        t.sopc_id   = hdoip.convert.bin2dec(string.sub(ret,13,16),4)
        t.sw_version= hdoip.convert.bin2dec(string.sub(ret,17,20),4)

        local tmp  = hdoip.convert.bin2dec(string.sub(ret,17,18),2)
        local tmp2 = hdoip.convert.bin2dec(string.sub(ret,19,20),2) 
        t.sw_version_str = string.format("%d.%d",tmp2, tmp) 
        t.sw_tag = string.format("%s",string.sub(ret,21))
    end
end

function getUSB(t)
    if((fd_cmd ~= nil) and (fd_rsp ~= nil)) then
        str = createCmdHeader(PIPE_CMD_GETUSB, 100)
        
        str = hdoip.convert.Str2HexFile(str) .. '                                                                                                    '

        fd_cmd:write(str)
        fd_cmd:flush()

        -- Read command header
        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(8)
        end
        
        size = hdoip.convert.bin2dec(string.sub(ret,5,8),4)

        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(size - 8)
        end
        
        t.usb_vendor = string.format("%s",string.sub(ret,1,49))
        t.usb_product = string.format("%s",string.sub(ret,50,99))
    end
end

function getHDCP(t)
    if((fd_cmd ~= nil) and (fd_rsp ~= nil)) then
        str = createCmdHeader(PIPE_CMD_GET_HDCP_STATE, 100)
        
        str = hdoip.convert.Str2HexFile(str) .. '                                                                                                    '

        fd_cmd:write(str)
        fd_cmd:flush()

        -- Read command header
        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(8)
        end
        
        size = hdoip.convert.bin2dec(string.sub(ret,5,8),4)

        ret = nil
        while(ret == nil) do
            ret = fd_rsp:read(size - 8)
        end
        
        t.hdcp_state = string.format("%s",string.sub(ret,1,49))
    end
end

function remote_update(file)
    if(fd_cmd ~= nil) then
        file_len = string.len(file)

        str = createCmdHeader(PIPE_CMD_REMOTE_UPDATE, (file_len + 1 + 4))
        str = str .. hdoip.convert.Str2LE_32(string.format("%08x", 0))
        str = hdoip.convert.Str2HexFile(str) .. file .. string.char(0)    
        fd_cmd:write(str)
        fd_cmd:flush()
    end
end

function store_cfg()
    simple_cmd(PIPE_CMD_STORE_CFG)
end

function reboot()
    simple_cmd(PIPE_CMD_REBOOT)
end

function factory_default()
    simple_cmd(PIPE_CMD_FACTORY_DEFAULT)
end

function ready()
    simple_cmd(PIPE_CMD_READY)
end

function play()
    simple_cmd(PIPE_CMD_VRB_PLAY)
end

function free_buffer()
    simple_cmd(PIPE_CMD_FREE_BUFFER)
end
