/******************************************************************************
* Sample User Program using TCP/IP for 2-port LAN                             *
*                                                                             *
*  Copyright (c) 2020-2025, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 2020-03-16 Ported to Tessera TS-RX72M-COM having dual Ethernet ports     FY *
* 2020-11-18 Modified dhcp_bynif() to not output message when starting     FY *
* 2020-11-18 Modified dns_command() help message                           FY *
* 2020-11-18 Modified dhcp_bynif() to exclude NIF of sub-channel in r4.4D~ FY *
* 2020-11-18 Aborts MainTask() execution when IP address is not assigned   FY *
* 2020-12-02 Ported to AlphaProject AP-RX72M-0A without change             FY *
* 2022-06-17 Moved add_inet to nonecfg.c renaming to add_inet1             FY *
* 2025-07-22 Placed RAM disk area on external SDRAM when XRAM is defined   FY *
******************************************************************************/

#ifndef NOFILE_VER      /* 1: File System Ver.1 in NORTi sample program */
#define NOFILE_VER  1   /* 2: File System Ver.2 attached to HTTPd for NORTi */
#endif                  /* 4: NORTi File System Ver.4 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "kernel.h"
#include "nosio.h"
#include "nonet.h"
#include "nonitod.h"
#include "nondhcp.h"
#include "nonteln.h"
#include "noncons.h"
#include "nonecho.h"
#include "nontftp.h"
#if (NOFILE_VER != 4)
#include "nonfile.h"
#include "nonftp.h"
#else
#include "nofile.h"
#include "nofftp.h"
#endif

//#define POL

/* Sample Program Configuration */

#define CH_CONS     0   /* serial channel number for console */
#define CH_DUMP     0   /* serial channel number if dump LAN packet */

#ifndef NFILE
#define NFILE       8   /* number of files allowed to be open simultaneously */
#endif

#ifndef NFTP
#define NFTP        1   /* number of clients supported by FTP server */
#endif                  /* (NFTP >= 2 is allowed only for nofftpd.c) */

#define NTP_SRV_MAX 2   /* number of NTP server (0:unused) */
#if (NTP_SRV_MAX != 0)
#include "nonsntp.h"
#include "sntptime.h"
#include "sntpcfg.h"
const char *const sntp_srv[] = { "ntp.nict.jp", "ntp.ring.gr.jp" };
const char dns_srv[] = "192.168.0.1"; /* when DHCP is invalid */
#endif

/* External Functions */

extern int ping_command(int, char **);
extern ER dns_resolver_byname(ID, UW, const char *, T_IPEP *, const char *);
#ifdef POL
extern void lan_intr(void);
#endif

/* Variables and Function for Sub Channel */

extern T_NIF add_inet1;
extern UB ethernet_addr1[];
extern UB default_ipaddr1[];
extern UB subnet_mask1[];
extern UB default_gateway1[];

/* Control Block Structures */

T_TELNETD telnetd;      /* Telnet daemon control block */
T_CONSOLE console;      /* console control block */
T_FTP ftpc;             /* FTP client control block */
T_FTP ftp[NFTP];        /* FTP server control block */
T_FILE file[NFILE];     /* file control block */
T_DISK disk[1];         /* disk drive control block */

/* Task Creation Information */

TASK MainTask(void);
const T_CTSK cMainTask = { TA_HLNG, NULL, (FP)MainTask, 7, 1024, NULL };

/* Address and Size of RAM Disk Area */

#ifdef XRAM /* when external RAM is available */
#define RAMDISK_ADDR    0x08000000  /* map RAM disk to SDRAM top address */
#define RAMDISK_SIZE    0x01000000  /* with SDRAM total size */
#else       /* when using internal RAM only */
#define RAMDISK_ADDR    0           /* map RAM disk to variable area */
#define RAMDISK_SIZE    0x00020000  /* with this size */
#endif

#if (RAMDISK_ADDR == 0)             /* define as array if macro value is 0 */
UW RAMDISK[(RAMDISK_SIZE)/4];       /* UW is for long word alignment */
#undef RAMDISK_ADDR
#define RAMDISK_ADDR    ((UW)RAMDISK)
#endif

/* Local Variable */

BOOL sntp_ini;                      /* SNTP client initialized flag */

/*****************************************************************************
* Telnet Password Check
*
******************************************************************************/

const char *telnet_passwd_check(T_TELNETD *t, const char *name, const char *passwd)
{
  #if 0
    if (strcmp(name, "mispo") == 0 &&
        strcmp(passwd, "12345") == 0)
        return "mispo>";
    else
        return NULL;
  #else
    return ">";
  #endif
}

/*****************************************************************************
* FTP Password Check
*
******************************************************************************/

const char *ftp_passwd_check(T_FTP *ftp, const char *user, const char *pass)
{
  #if 0
    if (strcmp(user, "mispo") == 0 &&
        strcmp(pass, "12345") == 0)
        return "230 Logged in\r\n";
    else
        return NULL;
  #else
    return "230 Logged in\r\n";
  #endif
}

/*****************************************************************************
* Display Error Message
*
******************************************************************************/

void print_errmsg(const char *msg, ER ercd)
{
    char s[7];

    itod(s, (short)ercd, 0);
    print(msg);
    print(" ("); print(s); print(")\r\n");
}

/*****************************************************************************
* Display IP Configuration
*
******************************************************************************/

static void print_ip(UB *ipaddr)
{
    char s[10];
    int i;

    for (i = 0; ; i++) {
        itou(s, (UH)ipaddr[i], itoulen((UH)ipaddr[i]));
        print(s);
        if (i >= 3)
            break;
        print(".");
    }
}

static void print_mac(UB *macaddr)
{
    char s[10];
    int i;

    for (i = 0; ; i++) {
        ito0X(s, (UH)macaddr[i], 2);
        print(s);
        if (i >= 5)
            break;
        print("-");
    }
}

void disp_param(int argc, char *argv[])
{
    T_NIF *nif;

    if (argc <= 1) {
        nif = getnif_default();
        if (nif == NULL) {
            print("Default network interface does not exist.\r\n");
            return;
        }
        do {
            print(nif->name); print(":\t""IP Address = ");
            print_ip(nif->ipaddr); print("\r\n");
        } while ((nif = nif->next) != NULL);

    } else {
        nif = getnif_from_name(argv[1]);
        if (nif == NULL) {
            print("No interface available.\r\n");
            return;
        }
        print(      "Ethernet Address   = "); print_mac(nif->hwaddr);
        print("\r\n""Default IP Address = "); print_ip(nif->ipaddr);
        print("\r\n""Subnet Mask        = "); print_ip(nif->mask);
        print("\r\n""Default Gateway    = "); print_ip(nif->gateway);
        print("\r\n""DHCP Server        = "); print_ip(nif->dhcp_ipaddr);
        print("\r\n""DNS Server         = "); print_ip(nif->dns_ipaddr);
        print("\r\n");
    }
}

/*****************************************************************************
* Display Local Date & Time
*
******************************************************************************/

void print_date(void)
{
    time_t t;
    tm_t *tm;

    time(&t);
    tm = (tm_t *)localtime(&t);
    print(asctime(tm));
}

/*****************************************************************************
* Display UTC Date & Time
*
******************************************************************************/

void print_gmdate(void)
{
    time_t t;
    tm_t *tm;

    time(&t);
    tm = (tm_t *)gmtime(&t);
    print(asctime(tm));
}

/*****************************************************************************
* DHCP Client Test
*
******************************************************************************/

ER dhcp_bynif(T_NIF *nif)
{
    if (byte4_to_long(nif->dhcp_ipaddr) != 0)
        return dhcp_reb_byname(0, nif->name);
    else
        return dhcp_get_byname(0, nif->name);
}

void dhcp_command(void)
{
    T_NIF *nif;
    ER ercd;
    char *argv[2];

    for (nif = getnif_default(); nif != NULL; nif = nif->next) {
        if (nif->type == NI_ETH
          #ifdef SET_VLAN_ID /* Rel.4.4D or later */
            && nif->aif == nif
          #endif
            ) {
            ercd = dhcp_bynif(nif);
            if (ercd != E_OK) {
                print_errmsg("\r\nCould not obtain IP address from DHCP server.", ercd);
            } else {
                print("\r\nSuccessfully assigned by DHCP server (");
                print(nif->name); print(")\r\n");
                argv[1] = nif->name;
                disp_param(2, argv);
            }
        }
    }
}

/*****************************************************************************
* DNS Client Test
*
******************************************************************************/

static const char msg_dns_help[] =
    "Usage : dns domain [-i nif] [dnsip]\r\n"
    "\tdomain - Domain name to be searched\r\n"
    "\tnif    - Network interface name (e.g. eth0, eth1)\r\n"
    "\tdnsip  - IP address of DNS server (e.g. 192.168.1.1)\r\n";

void dns_command(int argc, char *argv[])
{
    T_IPEP ipep;
    T_NIF *nif;
    char *domain;
    UW dnsip;
    ER ercd;
    int i;

    nif = NULL;
    domain = NULL;
    dnsip = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            if (nif != NULL) {
                print("Duplicate -i option.\r\n");
            } else {
                if ((++i < argc) && (nif = getnif_from_name(argv[i])) != NULL)
                    continue;
                print("Invalid interface name.\r\n");
            }
        } else if (argv[i][0] == '-') {
            print("Invalid option.\r\n");
        } else if (domain == NULL) {
            domain = argv[i];
            continue;
        } else if (dnsip == 0) {
            if ((dnsip = ascii_to_ipaddr(argv[i])) != 0)
                continue;
            else
                print("Invalid DNS server address.\r\n");
        } else {
            print("Too many parameters.\r\n");
        }
        return;
    }
    if (domain == NULL) {
        print(msg_dns_help);
        return;
    }
    if (nif == NULL) {
        if (dnsip == 0)
            nif = getnif_default();
        else
            nif = getnif_dstaddr(dnsip);
        if (nif == NULL) {
            print("Default network interface does not exist.\r\n");
            return;
        }
    }
    if (dnsip == 0) {
        if ((dnsip = byte4_to_long(nif->dns_ipaddr)) == 0) {
            print("DNS server is not defined for this interface.\r\n");
            return;
        }
    }

    ercd = dns_resolver_byname(0, dnsip, domain, &ipep, nif->name);
    if (ercd != E_OK) {
        print_errmsg("DNS server or domain name not found.", ercd);
        return;
    }

    print(domain);
    print(" has address ");
    long_to_byte4((UB *)&ipep.ipaddr, ipep.ipaddr);
    print_ip((UB *)&ipep.ipaddr);
    print("\r\n");
}

/*****************************************************************************
* SNTP Client Test
*
******************************************************************************/

void sntp_command(int argc, char *argv[])
{
  #if (NTP_SRV_MAX == 0)
    print("Time server is not defined.\r\n");
  #else
    UW dnsip;
    ER ercd;

    /* initialize SNTP client at first */
    if (!sntp_ini) {
        dnsip = byte4_to_long(getnif_default()->dns_ipaddr);
        if (dnsip == 0)
            dnsip = ascii_to_ipaddr(dns_srv);
        ercd = sntp_start(sntp_srv, dnsip, 1000/MSEC);
        if (ercd < 0) {
            print_errmsg("Time server is not available.", ercd);
            return;
        }
        set_timezone_ofst(DEFAULT_TZ);
        sntp_ini = TRUE;
    }

    /* execute SNTP synchronization */
    ercd = sntp_get_tim(NULL);
    if (ercd < 0)
        print_errmsg("Could not load time from time server.", ercd);
    else
        print_date();
  #endif
}

/*****************************************************************************
* Telnet Command Processing Routine
*
******************************************************************************/

BOOL telnetd_callback(T_TERMINAL *t, char *s)
{
    char *argv[16];
    int argc;

    for (argc = 0; argc < sizeof argv / sizeof (char *);) {
        while (*s == ' ')
            s++;
        if (*s == '\0')
            break;
        argv[argc++] = s;
        s = strchr(s, ' ');
        if (s == NULL)
            break;
        *s++ = '\0';
    }
    if (argc == 0)
        return TRUE;

    if (stricmp(argv[0], "?") == 0 || stricmp(argv[0], "help") == 0)
        print("ip     - display IP configuration\r\n"
              "ping   - ping command\r\n"
              "telnet - start Telnet client\r\n"
              "ftp    - start FTP client\r\n"
              "techo  - start TCP echo client\r\n"
              "uecho  - start UDP echo client\r\n"
              "dhcp   - DHCP client test\r\n"
              "dns    - DNS client test\r\n"
              "sntp   - SNTP client test\r\n"
              "date   - display local time\r\n"
              "gmdate - display UTC time\r\n"
              "help   - display help message\r\n");
    else if (stricmp(argv[0], "ip") == 0)
        disp_param(argc, argv);
    else if (stricmp(argv[0], "ping") == 0)
        ping_command(argc, argv);
    else if (stricmp(argv[0], "telnet") == 0)
        telnet_command(argc, argv);
    else if (stricmp(argv[0], "ftp") == 0)
        ftp_command(argc, argv);
    else if (stricmp(argv[0], "techo") == 0)
        techo_command(argc, argv);
    else if (stricmp(argv[0], "uecho") == 0)
        uecho_command(argc, argv);
    else if (stricmp(argv[0], "dhcp") == 0)
        dhcp_command();
    else if (stricmp(argv[0], "dns") == 0)
        dns_command(argc, argv);
    else if (stricmp(argv[0], "sntp") == 0)
        sntp_command(argc, argv);
    else if (stricmp(argv[0], "date") == 0)
        print_date();
    else if (stricmp(argv[0], "gmdate") == 0)
        print_gmdate();
    else
        print("Unknown or unsupported command.\r\n");
    return TRUE;
}

/*****************************************************************************
* Main Task
*
******************************************************************************/

TASK MainTask(void)
{
    T_NIF_ADDR addr;
    int n;

    /* clear local variable */
    sntp_ini = FALSE;

    /* serial console initialization */
    if (console_ini(&console, 0, 0, CH_CONS, "38400 B8 PN S1") != E_OK)
        goto END;
    console_print(&console, "\r\n*** TCP/IP Sample Program ***");

    /* LAN packets can be sent to serial port for debugging */
  #ifdef DUMP
   #if ((CH_DUMP != -1) && (CH_DUMP != CH_CONS))
    if (ini_sio(CH_DUMP, "38400 B8 PN S1") != E_OK)
        goto END;
    if (ctl_sio(CH_DUMP, TSIO_RXE|TSIO_TXE|TSIO_RTSON) != E_OK)
        goto END;
   #endif
    if (landump_ini(0, 0, 0) != E_OK)
        goto END;
  #endif

    /* protocol stack initialization */
    if (tcp_ini() < 0)
        goto END;

    /* sub channel initialization */
    addr.hwaddr  = ethernet_addr1;
    addr.ipaddr  = default_ipaddr1;
    addr.gateway = default_gateway1;
    addr.mask    = subnet_mask1;
    if (tcp_nif_ini(&add_inet1, "eth1", lan_nif_dev1, &addr) < 0)
        goto END;

    /* get IP address from DHCP server */
  #ifdef DHCP
    dhcp_bynif(getnif_default());
    dhcp_bynif(getnif_from_name("eth1"));
  #endif
    if (byte4_to_long(default_ipaddr) == 0)
        goto END;
    if (byte4_to_long(getnif_from_name("eth1")->ipaddr) == 0)
        goto END;

    /* file system initialization */
    if (file_ini(file, NFILE, 0, 0) != E_OK)
        goto END;

    /* RAM disk initialization */
  #if (NOFILE_VER == 1)
    if (disk_ini(&disk[0], "A:", ramdisk, RAMDISK_ADDR, RAMDISK_SIZE) != E_OK)
        goto END;
    if (dformat("A:") != E_OK)
        goto END;
  #else /* NOFILE_VER >= 2 */
    if (disk_ini(&disk[0], "A:", ramdisk, RAMDISK_ADDR, RAMDISK_SIZE, NULL, 0, 0) != E_OK)
        goto END;
    if (dformat("A:", 0) != E_OK)
        goto END;
  #endif

    /* UDP echo server initialization */
    if (udpecho_ini(0, 0) != E_OK)
        goto END;

    /* TCP echo server initialization */
    if (tcpecho_ini(0, 0, 0) != E_OK)
        goto END;

    /* Telnet daemon initialization */
    if (telnetd_ini(&telnetd, 0, 0, 0, 0) != E_OK)
        goto END;

    /* TFTP server initialization */
    if (tftpsrv_ini(0, 0) != E_OK)
        goto END;

    /* FTP server initialization */
    for (n = 0; n < NFTP; n++) {
        if (ftp_cre_srv(&ftp[n], 0, 0, 0, 0, ftp_passwd_check) != E_OK)
            goto END;
        if (ftp_sta_srv(&ftp[n]) != E_OK)
            goto END;
    }

    /* shell initialization */
    if (shell_ini(&console, 0, 0, telnet_passwd_check) != E_OK)
        goto END;
    if (shell_ini(&telnetd, 0, 0, telnet_passwd_check) != E_OK)
        goto END;

    /* call LAN intrrupt function directly to debug by polling */
  #ifdef POL
    for (;;) {
        chg_pri(TSK_SELF, 1);
        lan_intr();
        chg_pri(TSK_SELF, TPRI_INI);
    }
  #endif
END:
    slp_tsk();
}

/*****************************************************************************
* Main Function (Initialize Handler for NORTi)
*
******************************************************************************/

int main(void)
{
    ID id;

    /* initialize system */
    sysini();

    /* create application task */
    id = acre_tsk(&cMainTask);

    /* start task */
    sta_tsk(id, 0);

    /* start system */
    intsta();                   /* start interval timer interrupt */
    return syssta();            /* enter into multitasking */
}

/* end */
