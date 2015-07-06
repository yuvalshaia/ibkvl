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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "../../include/kvl.h"
#include "../../include/vl_trace.h"

MODULE_AUTHOR("Saeed Mahameed <saeedm@mellanox.com>");
MODULE_DESCRIPTION("KVL sample driver");
MODULE_LICENSE("GPL");

#ifndef MODULE_NAME
#define MODULE_NAME "kvl_helloworld"
#endif

int mlnx_command;
module_param_named(command, mlnx_command, int, 0644);
MODULE_PARM_DESC(command, "Enable debug tracing if > 0");

static int init_hello(void);
static void cleanup_hello(void);

int kvl_dummy_test(void *kvldata, void *data, void *userbuff, unsigned int len)
{
	struct kvl_op *op = (struct kvl_op *)kvldata;
	const char *dev = get_param_strval(op, "dev");
	int port = get_param_intval(op, "ibport");

	VL_DATA_TRACE(("Dummy Test dev=%s port=%d\n", dev, port));

	return 0;
}

struct kvl_op *dumy_test_op;

static int init_hello(void)
{
	VL_DATA_TRACE(("init hello kvl\n"));
	dumy_test_op = create_kvlop("dummy_test", "Dummy Test", MODULE_NAME,
				    kvl_dummy_test, NULL, NULL);
	add_str_param(dumy_test_op, "dev", "IB device", "mlx4_0");
	add_int_param(dumy_test_op, "ibport", "IB port", 1);

	return 0;
}


static void cleanup_hello(void)
{
	destroy_kvlop(dumy_test_op);
	printk(KERN_ALERT "Goodbye, KVL\n");
}


module_init(init_hello);
module_exit(cleanup_hello);
