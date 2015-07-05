/*
 * Copyright (c) 2011 Mellanox Technologies. All rights reserved.
 * Copyright (c) 2012 Oracle.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _kvl_common_h_
#define _kvl_common_h_

#define IN
#define OUT

#define KVL_MODULE_NAME "ibkvl"
#define PROCFS_NAME	"ibkvl"

/*
 * Macros to help debugging
 */

#define print_info(fmt, args...) \
	printk(KERN_INFO  "[%s]: " fmt, KVL_MODULE_NAME, ## args)
#define print_warn(fmt, args...) \
	printk(KERN_WARNING "[%s] warning: " fmt, KVL_MODULE_NAME, ## args)
#define print_err(fmt, args...) \
	printk(KERN_ERR "[%s] error: " fmt, KVL_MODULE_NAME, ## args)

/* undef it, just in case */
/* #undef GMOD_DEBUG */
#ifdef GMOD_DEBUG
#define print_dbg(fmt, args...) \
	printk(KERN_DEBUG "[%s] debug %s:%d : " fmt, KVL_MODULE_NAME, \
	       __FILE__, __LINE__, ## args)
#else
#define print_dbg(fmt, args...)
#endif

#endif

