/*
 * debug.h
 *
 *  Created on: Aug 24, 2011
 *      Author: jerry.yu
 */

#ifndef _AMLOGIC_DEBUG_H_
#define _AMLOGIC_DEBUG_H_

#ifdef CONFIG_ENABLE_NAND_DEBUG
#define nanddebug(a...) printf(a)
#else
#define nanddebug(a...)
#endif



#endif /* DEBUG_H_ */
