
#ifndef __LOX_H__
#define __LOX_H__ 1

#include <u.h>
#include <libc.h>
#include "myth.h"


/* The following are offsets in 0x7F00 page
 */
#define ARG 0xF7 /*Current Argument address*/
#define POS 0xF8 /*Output text cursor pos*/
#define VTP 0xF9 /*VOCAB top page*/
#define VTO 0xFA /*VOCAB top page*/
#define DESTP 0xFB /*Destination address page*/
#define DESTO 0xFC /*Destination address offs*/
#define SRCP 0xFD /*Source address page*/
#define SRCO 0xFE /*Source address offs*/
#define ECODE 0xFF /*Exit Code*/


/* Application specific opcode
*/
#define END 0x81


#endif
