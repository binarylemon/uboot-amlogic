/*
 * debug.h
 *
 *  Created on: Aug 24, 2011
 *      Author: jerry.yu
 */

#ifndef _AMLOGIC_DEBUG_H_
#define _AMLOGIC_DEBUG_H_

#if CONFIG_ENABLE_NAND_DEBUG
#define nanddebug(level,a...) if(level<=CONFIG_ENABLE_NAND_DEBUG){ \
		printf("%s +%d func=%s:",__FILE__,__LINE__,__func__);	\
		printf(a);printf("\n");}
#undef assert
#define assert(a)	if((a)==0){printf("%s +%d func=%s: %s fail\n",__FILE__,__LINE__,__func__,#a);while(1);};

#else
#define nanddebug(a...)
#endif



#endif /* DEBUG_H_ */
