#
# (C) Tenable Network Security
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(16750);
 script_version ("$Revision: 1.2 $");

 name["english"] = "HP-UX Security patch : PHCO_27882";
 
 script_name(english:name["english"]);
 
 desc["english"] = '
The remote host is missing HP-UX Security Patch number PHCO_27882 .
(SSRT2336 Rev.5 XDR library)

Solution : ftp://ftp.itrc.hp.com/hp-ux_patches/s700_800/10.X/PHCO_27882
See also : HPUX security bulletin 215
Risk factor : High';

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for patch PHCO_27882";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2005 Tenable Network Security");
 family["english"] = "HP-UX Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/HP-UX/swlist");
 exit(0);
}

include("hpux.inc");

if ( ! hpux_check_ctx ( ctx:"800:10.24 700:10.24 " ) )
{
 exit(0);
}

if ( hpux_patch_installed (patches:"PHCO_27882 ") )
{
 exit(0);
}

if ( hpux_check_patch( app:"OS-Core.C-MIN", version:NULL) )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"OS-Core.CORE-SHLIBS", version:NULL) )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"ProgSupport.PROG-AUX", version:NULL) )
{
 security_hole(0);
 exit(0);
}
if ( hpux_check_patch( app:"ProgSupport.PROG-MIN", version:NULL) )
{
 security_hole(0);
 exit(0);
}
