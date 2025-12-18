/**
 *******************************************************************************
 * @file  adv_cli.c
 * @brief ADV7280/ADV7391 command-line interface for register debugging
 *
 * Commands:
 *   r  AA RR        - Read register from device
 *   rm AA MM RR     - Read register from map (ADV7280)
 *   w  AA RR VV     - Write value to register
 *   wm AA MM RR VV  - Write value to map register (ADV7280)
 *   d  AA RR QQ     - Dump QQ registers starting from RR
 *   dm AA MM RR QQ  - Dump from map (ADV7280)
 *
 * Where:
 *   AA = I2C device address (42=ADV7280, 56=ADV7391)
 *   MM = Map select (00=Main, 20=Int/VDP, 40=Map2, 80=VPP)
 *   RR = Register address
 *   VV = Value to write
 *   QQ = Count for dump
 *******************************************************************************
 */

#include "adv_cli.h"
#include "debug_uart.h"
#include "main.h"
#include <stdio.h>

#define CLI_BUF_SIZE   64
#define CLI_TIMEOUT    1000
#define MAP_SELECT_REG 0x0E

static char    cli_buf[CLI_BUF_SIZE];
static uint8_t cli_idx = 0;

/*============================================================================
 * Helper Functions
 *===========================================================================*/

static uint8_t hex_to_byte(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static uint8_t parse_hex(const char *s)
{
    return (hex_to_byte(s[0]) << 4) | hex_to_byte(s[1]);
}

static int32_t switch_map(uint8_t dev, uint8_t map)
{
    uint8_t cmd[2] = {MAP_SELECT_REG, map};
    return I2C_Transmit(dev, cmd, 2, CLI_TIMEOUT);
}

static void print_ok(void)  { printf("OK\r\n"); }
static void print_err(void) { printf("ERR\r\n"); }

/*============================================================================
 * Command Handlers
 *===========================================================================*/

static void cmd_read(uint8_t dev, uint8_t reg)
{
    uint8_t val = 0;
    if (I2C_Receive(dev, &reg, &val, 1, CLI_TIMEOUT) == 0)
        printf("%02X\r\n", val);
    else
        print_err();
}

static void cmd_read_map(uint8_t dev, uint8_t map, uint8_t reg)
{
    uint8_t val = 0;

    if (switch_map(dev, map) != 0) { print_err(); return; }
    if (I2C_Receive(dev, &reg, &val, 1, CLI_TIMEOUT) != 0) { print_err(); return; }
    switch_map(dev, 0x00);

    printf("%02X\r\n", val);
}

static void cmd_write(uint8_t dev, uint8_t reg, uint8_t val)
{
    uint8_t cmd[2] = {reg, val};
    if (I2C_Transmit(dev, cmd, 2, CLI_TIMEOUT) == 0)
        print_ok();
    else
        print_err();
}

static void cmd_write_map(uint8_t dev, uint8_t map, uint8_t reg, uint8_t val)
{
    if (switch_map(dev, map) != 0) { print_err(); return; }

    uint8_t cmd[2] = {reg, val};
    if (I2C_Transmit(dev, cmd, 2, CLI_TIMEOUT) != 0) { print_err(); return; }

    if (switch_map(dev, 0x00) == 0)
        print_ok();
    else
        print_err();
}

static void cmd_dump(uint8_t dev, uint8_t start, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t reg = start + i, val = 0;
        if (I2C_Receive(dev, &reg, &val, 1, CLI_TIMEOUT) == 0)
            printf("%02X%c", val, ((i + 1) % 16) ? ' ' : '\n');
    }
    printf("\r\n");
}

static void cmd_dump_map(uint8_t dev, uint8_t map, uint8_t start, uint8_t count)
{
    if (switch_map(dev, map) != 0) { print_err(); return; }

    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t reg = start + i, val = 0;
        if (I2C_Receive(dev, &reg, &val, 1, CLI_TIMEOUT) == 0)
            printf("%02X%c", val, ((i + 1) % 16) ? ' ' : '\n');
    }

    switch_map(dev, 0x00);
    printf("\r\n");
}

/*============================================================================
 * Command Parser
 *===========================================================================*/

static void cli_execute(char *cmd)
{
    uint8_t args[8], argc = 0;
    char   *p = cmd + (cmd[1] == 'm' ? 3 : 2);  // Skip command

    // Parse hex arguments
    while (*p && argc < 8)
    {
        while (*p == ' ') p++;
        if (!*p) break;
        args[argc++] = parse_hex(p);
        p += 2;
    }

    // Dispatch command
    if (cmd[0] == 'r' && cmd[1] != 'm' && argc == 2)
        cmd_read(args[0], args[1]);
    else if (cmd[0] == 'r' && cmd[1] == 'm' && argc == 3)
        cmd_read_map(args[0], args[1], args[2]);
    else if (cmd[0] == 'w' && cmd[1] != 'm' && argc == 3)
        cmd_write(args[0], args[1], args[2]);
    else if (cmd[0] == 'w' && cmd[1] == 'm' && argc == 4)
        cmd_write_map(args[0], args[1], args[2], args[3]);
    else if (cmd[0] == 'd' && cmd[1] != 'm' && argc == 3)
        cmd_dump(args[0], args[1], args[2]);
    else if (cmd[0] == 'd' && cmd[1] == 'm' && argc == 4)
        cmd_dump_map(args[0], args[1], args[2], args[3]);
    else
        printf("?\r\n");
}

/*============================================================================
 * Public Functions
 *===========================================================================*/

void ADVCLI_Task(void)
{
    DebugUart_ClearErrors();

    char ch;
    if (DebugUart_ReceiveChar(&ch) != 0)
        return;

    if (ch == '\r' || ch == '\n')
    {
        printf("\r\n");
        if (cli_idx > 0)
        {
            cli_buf[cli_idx] = '\0';
            cli_execute(cli_buf);
        }
        printf(">");
        cli_idx = 0;
    }
    else if (ch == 8 && cli_idx > 0)  // Backspace
    {
        cli_idx--;
        printf("\b \b");
    }
    else if (ch >= 32 && cli_idx < CLI_BUF_SIZE - 1)
    {
        cli_buf[cli_idx++] = ch;
        printf("%c", ch);
    }
}

void ADVCLI_Init(void)
{
    printf("\r\n");
    printf("================ ADV CLI ================\r\n");
    printf("r  AA RR        - Read register\r\n");
    printf("w  AA RR VV     - Write register\r\n");
    printf("d  AA RR QQ     - Dump registers\r\n");
    printf("-----------------------------------------\r\n");
    printf("rm AA MM RR     - Read from map\r\n");
    printf("wm AA MM RR VV  - Write to map\r\n");
    printf("dm AA MM RR QQ  - Dump from map\r\n");
    printf("MM: 00=Main 20=Int/VDP 40=Map2 80=VPP\r\n");
    printf("=========================================\r\n");
    printf(">");
}
