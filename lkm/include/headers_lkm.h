#ifndef _HEADERS_LKM_H
#define _HEADERS_LKM_H

////////////////////////////////////////////////////////////////////////////////
//
//  File           : lkm/include/headers_lkm.h
//  Description    : This is the header file for the kernel-mode driver. It
//                   contains all the necessary header files for the driver.
//
//   Author        : Thomason Zhao
//   Last Modified : Dec 03, 2023
//

// Include Files

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/errno.h>
#include <linux/fortify-string.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/notifier.h>
#include <linux/preempt.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

#include <asm/msr.h>
#include <asm/msr-index.h>
#include <asm/processor.h>

// TODO: Check if these are necessary
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/ioctl.h>

#endif // _HEADERS_LKM_H
