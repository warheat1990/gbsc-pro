/*
 * Код кнопок пульта (DEC)
 * Remote Control Button Code (DEC)
 *
 * Created in 10.09.2023
 * By Karabanov Aleksandr
 * https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw
 *
 */



const uint32_t IRKeyMenu = 0xEA52609F; // (Menu)        3931267231

const uint32_t IRKeySave = 0xEA5200FF; // (Load/ Save)  3931242751

const uint32_t IRKeyInfo = 0xEA5210EF; // (Info)        3931246831

const uint32_t IRKeyRight = 0xEA5240BF; // 向右          3931259071

const uint32_t IRKeyLeft = 0xEA52807F; // 向左          3931275391

const uint32_t IRKeyUp = 0xEA52C03F; // 向上          3931291711

const uint32_t IRKeyDown = 0xEA5220DF; // 向下          3931250911

const uint32_t IRKeyOk = 0xEA52A05F; // OK            3931283551

const uint32_t IRKeyExit = 0xEA52E01F; // Выход (Exit)  3931299871

const uint32_t IRKeyMute = 0xEA52D02F; // MUTE_ON       3931295791

const uint32_t kRecv2 = 0xEA52906F; // + Volume      3931279471

const uint32_t kRecv3 = 0xEA5250AF; // - Volume      3931263151


// const uint32_t kRecv14    = 0xEA522279; // 重新启动      3931251321
