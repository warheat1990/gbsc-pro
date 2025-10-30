/**
 * adv_cli.c - ADV CLI interface for ADV7280 and ADV7391
 * Commands: r(read), rm(read from map), w(write), wm(write to map), d(dump),dm(dump from map)
 */

#include "main.h"
#include "adv_cli.h"
#include "debug_uart.h"
#include <stdio.h>

#define BUF_SIZE 64
static char buf[BUF_SIZE];
static uint8_t idx = 0;

static uint8_t h2b(char c) {
    return (c>='0'&&c<='9')?(c-'0'):(c>='a'&&c<='f')?(c-'a'+10):(c-'A'+10);
}

static uint8_t parse(const char *s) {
    return (h2b(s[0])<<4)|h2b(s[1]);
}

static void exec(char *cmd) {
    uint8_t d[8], n=0;
    char *p = cmd+2;
    
    while(*p && n<8) {
        while(*p==' ') p++;
        if(!*p) break;
        d[n++] = parse(p);
        p += 2;
    }

    if(cmd[0]=='r' && n==2) {
        // r: Read
        // Usage: r AA RR (device, value, register)
        uint8_t v=0;
        // Read register
        if(Chip_Receive(d[0],&d[1],&v,1,1000)!=0) {
            printf("ERR\r\n");
            return;
        }
        printf("%02X\r\n",v);
    }
    else if(cmd[0]=='r' && cmd[1]=='m' && n==3) {
        // rm: Read from Map
        // Usage: rm AA MM RR (device, map value, register)
        uint8_t t[2], v=0;
        // Switch to map
        t[0]=0x0E; t[1]=d[1];
        if(I2C_Master_Transmit(d[0],t,2,1000)!=0) {
            printf("ERR\r\n");
            return;
        }
        // Read register
        if(Chip_Receive(d[0],&d[2],&v,1,1000)!=0) {
            printf("ERR\r\n");
            return;
        }
        // Switch back to User Sub Map
        t[0]=0x0E; t[1]=0x00;
        I2C_Master_Transmit(d[0],t,2,1000);
        printf("%02X\r\n",v);
    }
    else if(cmd[0]=='w' && n==3) {
        // w: Write
        // Usage: w AA RR VV (device, register, value)
        uint8_t t[2];
        // Write register
        t[0]=d[1]; t[1]=d[2];
        if(I2C_Master_Transmit(d[0],t,2,1000)==0)
            printf("OK\r\n");
        else
            printf("ERR\r\n");
    }
    else if(cmd[0]=='w' && cmd[1]=='m' && n==4) {
        // wm: Write to Map
        // Usage: wm AA MM RR VV (device, map value, register, value)
        uint8_t t[2];
        // Switch to map
        t[0]=0x0E; t[1]=d[1];
        if(I2C_Master_Transmit(d[0],t,2,1000)!=0) {
            printf("ERR\r\n");
            return;
        }
        // Write register
        t[0]=d[2]; t[1]=d[3];
        if(I2C_Master_Transmit(d[0],t,2,1000)!=0) {
            printf("ERR\r\n");
            return;
        }
        // Switch back to User Sub Map
        t[0]=0x0E; t[1]=0x00;
        if(I2C_Master_Transmit(d[0],t,2,1000)==0)
            printf("OK\r\n");
        else
            printf("ERR\r\n");
    }
    else if(cmd[0]=='d' && n==3) {
        // d: Dump
        // Usage: d AA RR QQ (device, start register, count)
        uint8_t t[2];
        // Dump registers
        for(uint8_t i=0;i<d[2];i++) {
            uint8_t r=d[1]+i,v=0;
            if(Chip_Receive(d[0],&r,&v,1,1000)==0)
                printf("%02X%c",v,(i+1)%16?' ':'\n');
        }
        printf("\r\n");
    }
    else if(cmd[0]=='d' && cmd[1]=='m' && n==4) {
        // dm: Dump from Map
        // Usage: dm AA MM RR QQ (device, map value, start register, count)
        uint8_t t[2];
        // Switch to map
        t[0]=0x0E; t[1]=d[1];
        if(I2C_Master_Transmit(d[0],t,2,1000)!=0) {
            printf("ERR\r\n");
            return;
        }
        // Dump registers
        for(uint8_t i=0;i<d[3];i++) {
            uint8_t r=d[2]+i,v=0;
            if(Chip_Receive(d[0],&r,&v,1,1000)==0)
                printf("%02X%c",v,(i+1)%16?' ':'\n');
        }
        // Switch back to User Sub Map
        t[0]=0x0E; t[1]=0x00;
        I2C_Master_Transmit(d[0],t,2,1000);
        printf("\r\n");
    }
    else {
        printf("?\r\n");
    }
}

void ADVCLI_Task(void) {
    DebugUart_ClearErrors();

    char ch;
    if(DebugUart_ReceiveChar(&ch)==0) {
        if(ch=='\r'||ch=='\n') {
            printf("\r\n");
            if(idx>0) {
                buf[idx]=0;
                exec(buf);
            }
            printf(">");
            idx=0;
        }
        else if(ch==8 && idx>0) {
            idx--;
            printf("\b \b");
        }
        else if(ch>=32 && idx<BUF_SIZE-1) {
            buf[idx++]=ch;
            printf("%c",ch);
        }
    }
}

void ADVCLI_Init(void) {
    printf("\r\n");
    printf("================ ADV CLI ================\r\n");
    printf("r AA RR         - Read (ADV7391)\r\n");
    printf("w AA RR VV      - Write (ADV7391)\r\n");
    printf("d AA RR QQ      - Dump (ADV7391)\r\n");
    printf("-----------------------------------------\r\n");
    printf("rm AA MM RR     - Read from Map (ADV7280)\r\n");
    printf("wm AA MM RR VV  - Write to Map (ADV7280)\r\n");
    printf("dm AA MM RR QQ  - Dump from Map (ADV7280)\r\n");
    printf("MM: 00=Main - 20=Int/VDP - 40=Map2\r\n");
    printf("=========================================\r\n");
    printf(">");
}

void ADVCLI_ExecuteCommand(const char *cmd) {
    char b[BUF_SIZE];
    uint8_t i=0;
    while(*cmd && i<BUF_SIZE-1) b[i++]=*cmd++;
    b[i]=0;
    exec(b);
}