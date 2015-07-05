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
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/uaccess.h>

#include "kvl_common.h"
#include "kvl.h"

MODULE_AUTHOR("Saeed Mahameed - Mellanox Technologies");
MODULE_DESCRIPTION("Testing module & VL library for linux kernel");
MODULE_LICENSE("GPL");

#define IS_EMPTY_STR(str) (!strlen(str) || !strcmp(str, "\0") || \
			  !strcmp(str, "") || !strcmp(str, "\n"))

/* Procfs root directory */
static struct proc_dir_entry *gmod_proc_root;

/* Registered operations list */
LIST_HEAD(kvl_op_list);

/* List mutex */
DEFINE_SEMAPHORE(kvl_list_mutex);

/* Print info about current registerd operation */
int info_run(void *kvldata, void *data, void *userbuff, unsigned int len)
{
	struct list_head *cursor;
	int res;

	print_info("KVL loaded modules\n");
	res = down_interruptible(&kvl_list_mutex);
	if (res) {
		print_info("Aborting operation ... (killed by user)\n");
		return -1;
	}
	list_for_each(cursor, &kvl_op_list) {
		struct kvl_op *op = (struct kvl_op *)cursor;
		if (op->user_app)
			print_info("[%s]\t/proc/%s/%s [user_app = %s]: %s\n",
				   op->module, PROCFS_NAME, op->name,
				   op->user_app, op->descr);
		else
			print_info("[%s]\t/proc/%s/%s: %s\n", op->module,
				   PROCFS_NAME, op->name, op->descr);
	}
	up(&kvl_list_mutex);

	return 0;
}


/**
 * This function is called when the /proc/ibkvl/... file is read
 */
int procfile_read(char *buffer, char **buffer_location, off_t offset,
		  int buffer_length, int *eof, void *data)
{
	int ret = -1;
	struct kvl_op *op = (struct kvl_op *)data;

	print_dbg("procfile_read (/proc/%s/%s)\n", PROCFS_NAME, op->name);
	ret = strlen(op->descr);
	memcpy(buffer, op->descr, ret);

	return ret;
}

int kvl_parse_args(char **argv, struct kvl_op *op)
{
	int rc = 0;
	char *arg = NULL;
	char *delems = " =\n";
	struct list_head *cursor;

	if ((*argv)[strlen(*argv)-1] == '\n')
		(*argv)[strlen(*argv)-1] = '\0';

	list_for_each(cursor, &op->param_list.list) {
		struct argument *arg = (struct argument *)cursor;
		arg->value = NULL;
	}

	arg = strsep(argv, delems);
	while (arg != NULL) {
		if (IS_EMPTY_STR(arg)) {
			arg = strsep(argv, delems);
			continue;
		}
		if (!strcmp(arg, "--help")) {
			print_info("Info :-\n");
			print_info("\t%s : %s\n", op->name, op->descr);
			print_info("params:\n");
			list_for_each(cursor, &op->param_list.list) {
				struct argument *param =
					(struct argument *)cursor;
				switch (param->type) {
				case INT:
					print_info("\t%s\t: %s (default %d)\n",
						   param->name,
						   param->descreption,
						   param->intdefval);
					break;
				case STR:
					print_info("\t%s\t: %s (default %s)\n",
						   param->name,
						   param->descreption,
						   param->strdefval);
					break;
				default:
					print_info("\t%s\t: %s\n", param->name,
						   param->descreption);
				}
			}
			return -1;
		}
		list_for_each(cursor, &op->param_list.list) {
			struct argument *param = (struct argument *)cursor;
			if (!strcmp(param->name, arg)) {
				arg = strsep(argv, delems);
				print_info("Got param %s = %s\n", param->name,
					   arg);
				param->value = arg;
			}
		}
		arg = strsep(argv, delems);
	}

	return rc;
}

/**
 * This function is called when the /proc/ibkvl/... file is written
 */
int procfile_write(struct file *file, const char *buffer, unsigned long count,
		   void *data)
{
	struct kvl_op *op = (struct kvl_op *)data;
	char *input_buffer = NULL;
	int input_size = 0;
	int ret;

	print_dbg("procfile_write (/proc/%s/%s) [%ld]\n", PROCFS_NAME, op->name,
		  count);
	/* Write data to the buffer */
	input_size = count > MAX_INPUT_SZ ? MAX_INPUT_SZ : count;
	input_buffer = kmalloc(input_size + 1, GFP_KERNEL);
	if (!input_buffer)
		return -ENOMEM;

	if (copy_from_user(input_buffer, buffer, input_size)) {
		print_err("[%s] Failed to copy user buffer\n", op->module);
		kfree(input_buffer);
		return -EFAULT;
	}
	if (!op->user_app) {
		input_buffer[input_size] = '\0';
		print_info("Executing using sysfs command parse:\n");
	} else
		print_info("Executing using: %s\n", op->user_app);

	print_info("***************** Executing %s *****************\n",
		   op->name);
	if (op->op_run) {
		if (input_size && (op->user_app == NULL)) {
			char *argv = input_buffer;
			print_info("Params: %s\n", input_buffer);
			ret = kvl_parse_args(&argv, op);
			if (ret != 0) {
				ret = -EFAULT;
				print_err("[%s] [%s] Failed to parse args\n",
					  op->module, op->name);
				goto exit;
			}
		}
		ret = op->op_run(op, op->data, input_buffer, input_size);
	} else {
		print_err("[%s] [%s] run function pointer is set to null\n",
			  op->module, op->name);
		ret = -EINVAL;
	}
exit:
	print_info("************* Done %s rc=[%d] *************\n",
		   op->name, ret);

	kfree(input_buffer);
	if (ret != 0)
		ret = -EFAULT;
	else
		ret = count;

	return ret;
}

int register_kvl_op(struct kvl_op *operation)
{
	struct proc_dir_entry *new_proc_file;

	if (operation == NULL) {
		print_warn("Trying to register a NULL operation");
		return -1;
	}

	print_info("[%s] Registering operation: %s\n", operation->module,
		   operation->name);
	new_proc_file = create_proc_entry(operation->name, 0777,
					  gmod_proc_root);
	if (!new_proc_file) {
		print_err("Error: Could not initialize /proc/%s/%s\n",
			  PROCFS_NAME, operation->name);
		return -1;
	}
	print_info("[%s] Created procfs: /proc/%s/%s\n", operation->module,
		   PROCFS_NAME, operation->name);
	new_proc_file->data = operation;
	new_proc_file->read_proc = procfile_read;
	new_proc_file->write_proc = procfile_write;
	new_proc_file->mode = S_IFREG | S_IRUGO;

	/* Add the operation to the list */
	down(&kvl_list_mutex);
	list_add_tail(&operation->list, &kvl_op_list);
	up(&kvl_list_mutex);

	return 0;
}
EXPORT_SYMBOL(register_kvl_op);

int unregister_kvl_op(struct kvl_op *operation)
{
	if (operation == NULL) {
		print_warn("Trying to unregister a NULL operation");
		return -1;
	}
	print_info("[%s] Unregistering operation: %s\n", operation->module,
		   operation->name);
	remove_proc_entry(operation->name, gmod_proc_root);
	print_info("[%s] Destroyed procfs: /proc/%s/%s\n", operation->module,
		   PROCFS_NAME, operation->name);

	/* Remove the operation from the list */
	down(&kvl_list_mutex);
	list_del_init(&operation->list);
	up(&kvl_list_mutex);

	return 0;
}
EXPORT_SYMBOL(unregister_kvl_op);

struct kvl_op *kvl_info;

/**
 * Driver entry function
 */
int ibkvl_driver_init(void)
{
	print_info("Loading module %s\n", KVL_MODULE_NAME);
#ifdef GMOD_DEBUG
	print_warn("running in debug mode\n");
#endif

	/* Create the /proc file */
	gmod_proc_root = proc_mkdir(PROCFS_NAME, NULL);
	if (gmod_proc_root == NULL) {
			print_err("Fail to initialize /proc/%s\n", PROCFS_NAME);
			return -ENOMEM;
	}
	print_dbg("Created /proc/%s\n", PROCFS_NAME);
	kvl_info = create_kvlop("kvlinfo", "kvl info", "ibkvl", info_run,
				NULL, NULL);
	print_dbg("Module Loaded\n");

	return 0;
}

/**
 * Driver exit function
 */
void ibkvl_driver_exit(void)
{
	print_info("Unloading module %s\n", KVL_MODULE_NAME);
	destroy_kvlop(kvl_info);

	remove_proc_entry(PROCFS_NAME, NULL);
	print_dbg("/proc/%s removed\n", PROCFS_NAME);
	print_dbg("Module unloaded\n");
}

module_init(ibkvl_driver_init);
module_exit(ibkvl_driver_exit);

