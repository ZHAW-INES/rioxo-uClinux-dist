#include "hoi_cfg_parse.h"

/* Converrt a string to hex value
 *
 * @param str pointer to the strin
 * @param size amount characters of the string
 * @return hex value (32 bit)
 */
uint32_t hoi_cfg_str2hex(char* str, uint32_t size)
{
    uint32_t i, tmp, value=0;

    for(i=0 ; i<size ; i++) {
        // ASCII to value
        if(str[size-1-i]<0x40) {
            tmp = str[size-1-i] - 0x30;
        } else if(str[size-1-i] < 0x47) {
            tmp = str[size-1-i] - 0x37;
        } else {
            tmp = str[size-1-i] - 0x57;
        }
        value = value | (tmp << (4*i));
    }
    return value;
}

/* Convert an ip value to a string (192.168.1.5) 
 *
 * @param ip ip value (32 bit)
 * @param str pointer to the string
 */
void hoi_cfg_prs_ip_str(uint32_t ip, char* str)
{
    sprintf(str, "%d.%d.%d.%d", (ip&0xFF000000)>>24, (ip&0xFF0000)>>16, (ip & 0xFF00)>>8, ip & 0xFF);
}

/* Convert an ip string (192.168.1.5) to a hex value
 *
 * @param str pointer to the string
 * @param ip pointer to the ip value
 */
void hoi_cfg_prs_ip_val(char* str, uint32_t* ip)
{
    uint32_t i;
    char* token;
    *ip = 0; 
    token = strtok(str, ".");
    for(i=0 ; (i<4) && (token != NULL) ; i++) {
        *ip = *ip | ((uint32_t)atoi(token) << (8*(3-i)));
        token = strtok(NULL, ".");
    }
}

/* Convert a MAC address to a string (01:02:03:04:05:06)
 *
 * @param mac pointer to character array (6 elements) of MAC address
 * @param str pointer to the string
 */
void hoi_cfg_prs_mac_str(char* mac, char* str)
{
    sprintf(str,"%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/* Convert a string (01:02:03:04:05:06) to a MAC address
 *
 * @param str pointer to the string
 * @param mac pointer to character array (6 elements) of MAC address
 */
void hoi_cfg_prs_mac_val(char* str, char* mac)
{
    uint32_t i;
    char* token;
    
    token = strtok(str, ":");
    for(i=0 ; (i<6) && (token != NULL); i++) {
        mac[i] = (char) hoi_cfg_str2hex(token, 2);
        token = strtok(NULL, ":");
    }
}

/* Convert an 16 bit value to a string 
 *
 * @param val value of the port (16 bit)
 * @param str pointer to the string
 */
void hoi_cfg_prs_str16(uint16_t val, char* str)
{
    sprintf(str,"%04x", val);
}

/* Convert a string to an 16 bit value
 *
 * @param str pointer to the string
 * @param val pointer to the 16 bit value
 */
void hoi_cfg_prs_val16(char* str, uint16_t* val)
{
    *val = (uint16_t) hoi_cfg_str2hex(str, 4); 
}

/* Convert an 32 bit value to a string 
 *
 * @param val value of the port (32 bit)
 * @param str pointer to the string
 */
void hoi_cfg_prs_str32(uint32_t val, char* str)
{
    sprintf(str,"%08x", val);
}

/* Convert a string to an 32 bit value
 *
 * @param str pointer to the string
 * @param val pointer to the 16 bit value
 */
void hoi_cfg_prs_val32(char* str, uint32_t* val)
{
    *val = hoi_cfg_str2hex(str, 8); 
}

