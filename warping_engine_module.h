/****************************************************************************
 *  License : All rights reserved for TES Electronic Solutions GmbH
 *        See included /docs/license.txt for details
 *  Project : WARPING_ENGINE
 *  Purpose : Kernel module internal definitions
 ****************************************************************************
 * Version Control Information :
 *  $Revision: 188 $
 *  $Date: 2021-09-09 15:31:21 +0200 (Do, 09 Sep 2021) $
 *  $LastChangedBy: markus.hertel $
 ****************************************************************************/

#ifndef TES_WE_MODULE_H_
#define TES_WE_MODULE_H_

#include <linux/wait.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/spinlock.h>

/* Linux character device config */
#define WARPING_ENGINE_DEVICE_NAME          "warpingengine"
#define WARPING_ENGINE_DEVICE_CLASS       "warpingengine"
#define WARPING_ENGINE_DEVICE_CNT         1u

/* device tree node */
#define WARPING_ENGINE_OF_COMPATIBLE        "tes,tes_warpingengine-1.0"

/* Register access macros */
#define WARPING_ENGINE_IO_WREG(addr,data)       iowrite32(data,addr)
#define WARPING_ENGINE_IO_RREG(addr)        ioread32(addr)
#define WARPING_ENGINE_IO_RADDR(base,reg)     ((void*)((unsigned long)base|((unsigned long)reg)<<2))

struct warping_engine_dev
{
  unsigned long base_phys;
  unsigned long span;
  void *base_virt;
  unsigned int irq_no;
  unsigned int irq_stat;
  spinlock_t irq_slck;
  wait_queue_head_t irq_waitq;
  dev_t dev;
  struct cdev cdev;
  struct device *device;
};

#endif /* TES_WE_MODULE_H_ */
