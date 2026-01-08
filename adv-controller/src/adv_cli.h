/**
 *******************************************************************************
 * @file  adv_cli.h
 * @brief ADV7280/ADV7391 command-line interface for register debugging
 *******************************************************************************
 */

#ifndef ADV_CLI_H
#define ADV_CLI_H

#include <stdint.h>

void ADVCLI_Init(void);
void ADVCLI_Task(void);

#endif /* ADV_CLI_H */
