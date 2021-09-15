/* (c) 2013  All rights reserved for TES Electronic Solutions
 *************************************************************************************************************
 *  Project       : Warping Engine
 *  Module        : Driver Core
 *  Purpose       : internal types and function declarations
 *  Author        : Serhiy Bodnar
 *  Creation Date : 2013-07-18
 *************************************************************************************************************
 * Version Control Information
 *  $Id: warping_engine_base.h 70 2014-07-24 08:06:22Z hh74071 $
 *  $HeadURL: https://svn.hamburg.tes/svn/tes_warping_engine/devel/driver/include/warping_engine_base.h $
 *  $Revision: 70 $
 *  $Date: 2014-07-24 10:06:22 +0200 (Do, 24 Jul 2014) $
 *  $LastChangedBy: hh74071 $
 ************************************************************************************************************/

#ifndef WARPING_ENGINE_BASE_H_
#define WARPING_ENGINE_BASE_H_

#include "warping_engine.h"

#define WARPING_ENGINE_DRIVER_VERSION 0x00000001
#define MIN_H_W_VERSION 0x00000000
#define MAX_H_W_VERSION 0x00000000

#ifndef NULL
  #define NULL ((void *) 0)
#endif

#define BYTES_PER_PIXEL      ( 4 )

// registers
#define WARPING_ENGINE_COORDINATES_ADDRESS_REG         ( 0 )
#define WARPING_ENGINE_COORDINATES_COUNT_REG           ( 1 )
#define WARPING_ENGINE_INPUT_ADDRESS_REG               ( 2 )
#define WARPING_ENGINE_INPUT_SIZE_REG                  ( 3 )
#define WARPING_ENGINE_INPUT_PITCH_REG                 ( 4 )
#define WARPING_ENGINE_INPUT_BYTE_PITCH_REG            ( 5 )
#define WARPING_ENGINE_OUTSIDE_COLOR_REG               ( 6 )
#define WARPING_ENGINE_OUTPUT_ADDRESS_REG              ( 7 )
#define WARPING_ENGINE_OUTPUT_SIZE_REG                 ( 8 )
#define WARPING_ENGINE_OUTPUT_PITCH_REG                ( 9 )
#define WARPING_ENGINE_STRIPE_WIDTH_REG                ( 10)
#define WARPING_ENGINE_H_W_REVISION_REG                ( 11)
#define WARPING_ENGINE_CONFIG_REG                      ( 12)
#define WARPING_ENGINE_IRQ_ENABLE_REG                  ( 13)
#define WARPING_ENGINE_IRQ_STATUS_REG                  ( 14)
#define WARPING_ENGINE_IRQ_CLEAR_REG                   ( 15)
#define WARPING_ENGINE_RESET_PIPE_REG                  ( 22)


#define WARPING_ENGINE_STREAM_BUFFER_PIXEL_COUNT_REG   ( 23)
#define WARPING_ENGINE_PFC_ENABLE_REG                  ( 30)
#define WARPING_ENGINE_PFC_CLEAR_REG                   ( 31)
#define WARPING_ENGINE_PFC_EVENT_SELECT_REG_BASE       ( 32)
#define WARPING_ENGINE_PFC_VALUE_REG_BASE              ( 64)


typedef struct warping_engine_context_tag
{
  warping_engine_uint32 m_size;
  warping_engine_platform_settings m_platform;
  
  // warping engine configuration registers
  warping_engine_uint32 m_hw_revision;
  warping_engine_uint32 m_config;
  warping_engine_bool m_enabled;
  
  // isr callbacks and isr callback data
  warping_engine_isr_callback m_irq;
  warping_engine_uint32 m_irq_data;
  
} warping_engine_context;

// functions defined in warping engine platform code:

warping_engine_bool warping_engine_arch_init(warping_engine_context* a_base, warping_engine_platform_settings a_platform);
void warping_engine_arch_exit(warping_engine_context* a_base);
warping_engine_bool warping_engine_arch_initIRQ(warping_engine_context *a_context);
void warping_engine_arch_deinitIRQ(warping_engine_context *a_context);
warping_engine_ptr warping_engine_arch_malloc(warping_engine_uint32 a_size);
void warping_engine_arch_free(warping_engine_ptr a_ptr);
warping_engine_uint32 warping_engine_arch_readReg(warping_engine_context *a_context, warping_engine_uint32 a_regAddress);
void warping_engine_arch_writeReg(warping_engine_context *a_context, warping_engine_uint32 a_regAddress, warping_engine_uint32 a_value);

// internal functions
warping_engine_context *warping_engine_validateContext(warping_engine_ptr a_context);
void warping_engine_resetRegisters(warping_engine_context *a_context);

#endif // WARPING_ENGINE_BASE_H_
