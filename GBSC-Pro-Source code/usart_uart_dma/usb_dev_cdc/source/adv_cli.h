/**
 * adv_cli.h - Interactive command-line interface for ADV testing
 */

#ifndef ADV_CLI_H
#define ADV_CLI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void ADVCLI_Init(void);
void ADVCLI_Task(void);
void ADVCLI_ExecuteCommand(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif /* ADV_CLI_H */