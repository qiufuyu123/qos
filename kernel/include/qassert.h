/*
 * @Description: 断言实现
 * @Author: QIUFUYU
 * @Date: 2021-09-30 19:53:29
 * @LastEditTime: 2021-10-09 20:30:24
 */
#ifndef _H_ASSERT
#define _H_ASSERT
#include"console.h"
#define ASSERT(expr) if(!(expr)) {while(1){console_debug("ASSERT PANIC!",DEBUG_PANIC);}}


#endif