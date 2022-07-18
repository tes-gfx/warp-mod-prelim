/****************************************************************************
 *  License : All rights reserved for TES Electronic Solutions GmbH
 *        See included /docs/license.txt for details
 *  Project : WARPING_ENGINE
 *  Purpose : Main linux kernel module. Handles initialization and fops.
 ****************************************************************************
 * Version Control Information :
 *  $Revision: 67 $
 *  $Date: 2014-07-22 08:22:47 +0200 (Di, 22 Jul 2014) $
 *  $LastChangedBy: hh04075 $
 ****************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "warping_engine_module.h"
#include "warping_engine_base.h"

/* store class globally. */
struct class *warping_engine_class;

/* fops functions */
static int warping_engine_open(struct inode *ip, struct file *fp)
{
  struct warping_engine_dev *dev;

  /* extract the device structure and add it to the file pointer for easier
   * access */
  dev = container_of(ip->i_cdev, struct warping_engine_dev, cdev);
  fp->private_data = dev;

  return 0;
}

static int warping_engine_mmap(struct file *fp, struct vm_area_struct *vma)
{
  struct warping_engine_dev *dev = fp->private_data;
  int ret;

  vma->vm_flags &= ~VM_PFNMAP;
  vma->vm_pgoff = 0;

  ret = dma_mmap_wc(dev->device, vma,
            dev->mem_base_virt, dev->mem_base_phys,
            vma->vm_end - vma->vm_start);

  return ret;
}

static long warping_engine_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
  struct warping_engine_dev *dev = fp->private_data;
  warping_engine_settings wpset;
  unsigned int cmd_nr;

  if (_IOC_TYPE(cmd) != WARPING_ENGINE_IOCTL_TYPE)
    return -ENOTTY;

  cmd_nr = _IOC_NR(cmd);
  if (_IOC_DIR(cmd) == _IOC_WRITE)
  {
    if (cmd_nr & WARPING_ENGINE_IOCTL_REG_PREFIX)
    {
      /* direct register write: Register value in argument */
    WARPING_ENGINE_IO_WREG( WARPING_ENGINE_IO_RADDR(dev->base_virt,
                            WARPING_ENGINE_IOCTL_GET_REG(cmd_nr)),
                            arg);
      return 0;
    }
  }
  else if (_IOC_DIR(cmd) == _IOC_READ)
  {
    if (cmd_nr & WARPING_ENGINE_IOCTL_REG_PREFIX)
    {  /* direct register read: Argument is a pointer */
      if(!put_user(WARPING_ENGINE_IO_RREG(WARPING_ENGINE_IO_RADDR(dev->base_virt, 
                                          WARPING_ENGINE_IOCTL_GET_REG(cmd_nr))), 
                                          (unsigned long*) arg))
        return -EFAULT;
      return 0;
    }

    switch(cmd_nr)
    {
      case WARPING_ENGINE_IOCTL_NR_SETTINGS:
        wpset.base_phys = dev->base_phys;
        wpset.span = dev->span;
        wpset.mem_base_phys = dev->mem_base_phys;
        wpset.mem_span = dev->mem_span;
      
        if(copy_to_user(  (void*) arg, 
                          (void*) &wpset,
                          sizeof(warping_engine_settings))  ) 
        {
          dev_err(fp->private_data,
            "error while copying settings to user space\n");
          return -EFAULT;
        }
        return 0;

      default:
        return -EINVAL;
    }
  }
  return -EINVAL;
}

ssize_t warping_engine_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
  struct warping_engine_dev *dev = filp->private_data;
  unsigned long flags;
  int temp;

  wait_event_interruptible(dev->irq_waitq, dev->irq_stat);

  spin_lock_irqsave(&dev->irq_slck, flags);
  temp = dev->irq_stat;
  dev->irq_stat = 0;
  spin_unlock_irqrestore(&dev->irq_slck, flags);

  if(count==sizeof(temp))
  {
    put_user(temp, buff);
    return sizeof(temp);
  }

  return 0;
}

static struct file_operations warping_engine_fops = {
  .owner = THIS_MODULE,
  .open = warping_engine_open,
  .mmap = warping_engine_mmap,
  .unlocked_ioctl = warping_engine_ioctl,
  .read = warping_engine_read,
};

static irqreturn_t std_irq_handler(int irq, void *dev_id)
{
  unsigned long flags;
  struct warping_engine_dev *warping_engined = dev_id;
  int status;

  status = WARPING_ENGINE_IO_RREG(WARPING_ENGINE_IO_RADDR(warping_engined->base_virt,WARPING_ENGINE_IRQ_STATUS_REG));
  WARPING_ENGINE_IO_WREG(WARPING_ENGINE_IO_RADDR(warping_engined->base_virt, WARPING_ENGINE_IRQ_CLEAR_REG), status);

  spin_lock_irqsave(&warping_engined->irq_slck, flags);
  warping_engined->irq_stat |= status;
  spin_unlock_irqrestore(&warping_engined->irq_slck, flags);

  wake_up_interruptible(&warping_engined->irq_waitq);

  return IRQ_HANDLED;
}

static int register_irq(struct warping_engine_dev *dev)
{
  if(request_irq(dev->irq_no, std_irq_handler, 0, "TES WARPING_ENGINE", (void*) dev))
  {
    dev_err(dev->device, "irq cannot be registered\n");
    return -EBUSY;
  }

  return 0;
}

static void unregister_irq(struct warping_engine_dev *dev)
{
  free_irq(dev->irq_no, (void*) dev);
}

/* create character class and devices */
static int warping_engine_setup_device(struct warping_engine_dev *dev)
{
  int result = 0;

  result = alloc_chrdev_region(&dev->dev, 0, WARPING_ENGINE_DEVICE_CNT,
      WARPING_ENGINE_DEVICE_NAME);

  if (result < 0) {
    dev_err(dev->device, "can't alloc_chrdev_region\n");
    goto CHR_FAILED;
  }

  warping_engine_class = class_create(THIS_MODULE, WARPING_ENGINE_DEVICE_CLASS);
  if(!warping_engine_class)
  {
    dev_err(dev->device, "cannot create class %s\n", WARPING_ENGINE_DEVICE_CLASS);
    result = -EBUSY;
    goto CLASS_FAILED;
  }

  dev->device = device_create(warping_engine_class, NULL, dev->dev, dev, WARPING_ENGINE_DEVICE_NAME);
  if(!dev->device)
  {
    dev_err(dev->device, "cannot create device %s\n", WARPING_ENGINE_DEVICE_NAME);
    result = -EBUSY;
    goto DEVICE_FAILED;
  }

  cdev_init(&dev->cdev, &warping_engine_fops);
  dev->cdev.owner = THIS_MODULE;

  result = cdev_add(&dev->cdev, dev->dev, WARPING_ENGINE_DEVICE_CNT);
  if(result)
  {
    dev_err(dev->device, "can't register char device\n");
    goto DEV_FAILED;
  }

  return 0;

DEV_FAILED:
  device_destroy(warping_engine_class, dev->dev);
DEVICE_FAILED:
  class_destroy(warping_engine_class);
CLASS_FAILED:
  unregister_chrdev_region(dev->dev, WARPING_ENGINE_DEVICE_CNT);
CHR_FAILED:

  return result;
}

static void warping_engine_log_params(struct warping_engine_dev *dev)
{
  dev_info(dev->device, "Base address:\t0x%08lx - 0x%08lx\n",
      dev->base_phys, dev->base_phys + dev->span);
  dev_info(dev->device, "Video RAM:\t%pad\n",
      &dev->mem_base_phys);
  dev_info(dev->device, "IRQ:\t%d\n", dev->irq_no);
}

static void warping_engine_shutdown_device(struct warping_engine_dev *dev)
{
  device_destroy(warping_engine_class, dev->dev);
  class_destroy(warping_engine_class);
  unregister_chrdev_region(dev->dev, WARPING_ENGINE_DEVICE_CNT);
  cdev_del(&dev->cdev);
}

/* platform device functions:
 * on probe (new device), copy all neccessary data from device tree description
 * to local data structure and initialize the driver part.
 * */
static int warping_engine_probe(struct platform_device *pdev)
{
  struct device_node *np;
  struct resource rsrc;
  int result = 0;
  struct warping_engine_dev *warping_engine;

  warping_engine = devm_kzalloc(&pdev->dev, sizeof(struct warping_engine_dev), GFP_KERNEL);
  if(!warping_engine)
  {
    dev_err(&pdev->dev,
      "Memory allocation for driver struct failed!\n");
    return -ENOMEM;
  }
  platform_set_drvdata(pdev, warping_engine);
  warping_engine->device = &pdev->dev;
  np = pdev->dev.of_node;
  if(!np)
  {
    dev_err(&pdev->dev,
      "driver should only be instanciated over device tree!\n");
    return -ENODEV;
  }

  of_address_to_resource(np, 0, &rsrc);
  warping_engine->base_phys = rsrc.start;
  warping_engine->span = rsrc.end - rsrc.start;
  warping_engine->irq_no = of_irq_to_resource(np, 0, &rsrc);

  warping_engine_log_params(warping_engine);

  spin_lock_init(&warping_engine->irq_slck);
  init_waitqueue_head(&warping_engine->irq_waitq);

  if (!request_mem_region(warping_engine->base_phys, warping_engine->span, "TES WARPING_ENGINE"))
  {
    dev_err(&pdev->dev, "memory region already in use\n");
    return -EBUSY;
  }

  warping_engine->base_virt = ioremap_nocache(warping_engine->base_phys, warping_engine->span);
  if (!warping_engine->base_virt)
  {
    dev_err(&pdev->dev, "ioremap failed\n");
    result = -EBUSY;
    goto IO_FAILED;
  }

  result = WARPING_ENGINE_IO_RREG(WARPING_ENGINE_IO_RADDR(warping_engine->base_virt,
        WARPING_ENGINE_H_W_REVISION_REG));
  dev_info(&pdev->dev, "Found WARPING_ENGINE rev. 0x%08X\n", result);

  /* Allocate Vidmem */
  warping_engine->mem_span = 16*1024*1024;
  warping_engine->mem_base_virt = dma_alloc_wc(warping_engine->device, warping_engine->mem_span, &warping_engine->mem_base_phys,  GFP_KERNEL | __GFP_NOWARN);
  if(!warping_engine->mem_base_virt) {
    dev_err(&pdev->dev, "allocating video memory failed\n");
    goto IO_VID_FAILED;
  }

  warping_engine_log_params(warping_engine);

  result = warping_engine_setup_device(warping_engine);
  if(result)
  {
    goto DEV_FAILED;
  }

  /* register warping_engine IRQs */
  result = register_irq(warping_engine);
  if(result)
  {
    dev_err(&pdev->dev, "can't register irq %d\n", warping_engine->irq_no);
    goto IRQ_FAILED;
  }
  dev_warn(&pdev->dev, "This driver is PRELIMINARY. Do NOT use in production environment!\n");

  return 0;

IRQ_FAILED:
  warping_engine_shutdown_device(warping_engine);
DEV_FAILED:
  dma_free_wc(warping_engine->device, warping_engine->mem_span, warping_engine->mem_base_virt, warping_engine->mem_base_phys);
IO_VID_FAILED:
  iounmap(warping_engine->base_virt);
IO_FAILED:
  release_mem_region(warping_engine->base_phys, warping_engine->span);

  return -EBUSY;
}

static int warping_engine_remove(struct platform_device *pdev)
{
  struct warping_engine_dev *warping_engine = platform_get_drvdata(pdev);
  unregister_irq(warping_engine);
  iounmap(warping_engine->mem_base_virt);
  iounmap(warping_engine->base_virt);
  release_mem_region(warping_engine->base_phys, warping_engine->span);
  release_mem_region(warping_engine->mem_base_phys, warping_engine->mem_span);
  warping_engine_shutdown_device(warping_engine);
  devm_kfree(&pdev->dev, warping_engine);
  return 0;
}

static const struct of_device_id warping_engine_of_ids[] = {
  {
    .compatible = WARPING_ENGINE_OF_COMPATIBLE,
  },
  { }
};

static struct platform_driver warping_engine_driver = {
  .driver = {
    .name = WARPING_ENGINE_DEVICE_NAME,
    .owner = THIS_MODULE,
    .of_match_table = of_match_ptr(warping_engine_of_ids),
  },
  .probe = warping_engine_probe,
  .remove = warping_engine_remove,
};

static int __init _warping_engine_init(void)
{
  int result = 0;

  result = platform_driver_register(&warping_engine_driver);
  if(result)
    printk(KERN_ALERT "%s: failed to register platform driver\n", __func__);

  return result;

}

static void __exit _warping_engine_exit(void)
{
  platform_driver_unregister(&warping_engine_driver);
}

module_init(_warping_engine_init);
module_exit(_warping_engine_exit);
MODULE_AUTHOR("TES Electronic Solutions GmbH");
MODULE_DESCRIPTION("Kernel driver for WARPING_ENGINE display controller.");
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(of, warping_engine_of_ids);
