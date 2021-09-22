/******************************************************************************
 * @file       font.h
 * @brief      5x5 LEDマトリクス フォントデータ ヘッダファイル
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    5x5 LEDマトリクス用 英数記号フォントデータのヘッダファイル
 * @date       2021/09/09 v1.00 新規作成
 * @par     
 * @copyright  Copyright ©︎ 2021 SONODA Takehiko All rights reserved.
 ******************************************************************************/

//#include "M5Atom.h"

#ifndef _FONT_H_
#define _FONT_H_

//#ifdef __cplusplus
//extern "C" {
//#endif

#define FONT5X5_NUM     96
#define FONT5X5_ROW     5
#define FONT5X5_COL     5
#define FONT5X5_START_CODE  0x20
#define FONT5X5_END_CODE    0x7F

extern const unsigned char    Font5x5[FONT5X5_NUM][FONT5X5_ROW];

//#ifdef __cplusplus
//}
//#endif

#endif /* _FONT_H_ */
