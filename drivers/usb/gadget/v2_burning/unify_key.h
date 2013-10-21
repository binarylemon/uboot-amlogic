/*
 * \file        unify_key.h
 * \brief       User interface from drivers/keymanage
 *
 * \version     1.0.0
 * \date        2013/9/4
 * \author      Sam.Wu <yihui.wu@amlgic.com>
 *
 * Copyright (c) 2013 Amlogic Inc.. All Rights Reserved.
 *
 */

/*
 * function name: key_unify_init
 * buf : input 
 * len  : > 0
 * return : >=0: ok, other: fail
 * */
int key_unify_init(char *buf,unsigned int len);

/* function name: key_unify_uninit
 * functiion : uninit 
 * return : >=0 ok, <0 fail
 * */
int key_unify_uninit(void);


/* funtion name: key_unify_write
 * keyname : key name is ascii string
 * keydata : key data buf
 * datalen : key buf len
 * return  0: ok, -0x1fe: no space, other fail
 * */
int key_unify_write(char *keyname,unsigned char *keydata,unsigned int datalen);

/*
 *function name: key_unify_read
 * keyname : key name is ascii string
 * keydata : key data buf
 * datalen : key buf len
 * reallen : key real len
 * return : <0 fail, >=0 ok
 * */
int key_unify_read(char *keyname,unsigned char *keydata,unsigned int datalen,unsigned int *reallen);

/*
*    key_unify_query - query whether key was burned.
*    @keyname : key name will be queried.
*    @keystate: query state value, 0: key was NOT burned; 1: key was burned; others: reserved.
*     keypermit: read permit: bit0~bit3
*                write permit: bit4~bit7
*     if it return failed, keypermit is invalid; kerpermit is valid,when it return successful only
*    return: >=0: successful; others: failed. 
*/
int key_unify_query(char *keyname,unsigned int *keystate,unsigned int *keypermit);


