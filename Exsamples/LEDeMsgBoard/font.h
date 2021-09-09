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
