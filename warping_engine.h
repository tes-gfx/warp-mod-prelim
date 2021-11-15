/* (c) 2013  All rights reserved for TES Electronic Solutions
 *************************************************************************************************************
 *  Project       : Warping Engine
 *  Module        : Driver Core
 *  Purpose       : public types and function declarations
 *  Author        : Serhiy Bodnar
 *  Creation Date : 2013-07-18
 *************************************************************************************************************
 * Version Control Information
 *  $Id: warping_engine.h 70 2014-07-24 08:06:22Z hh74071 $
 *  $HeadURL: https://svn.hamburg.tes/svn/tes_warping_engine/devel/driver/include/warping_engine.h $
 *  $Revision: 70 $
 *  $Date: 2014-07-24 10:06:22 +0200 (Do, 24 Jul 2014) $
 *  $LastChangedBy: hh74071 $
 ************************************************************************************************************/

 /*--------------------------------------------------------------------------
 *
 * Title: Types
 *
 *-------------------------------------------------------------------------- */
 
#ifndef WARPING_ENGINE_H_
#define WARPING_ENGINE_H_

#define WARPING_ENGINE_FALSE 0
#define WARPING_ENGINE_TRUE 1



/*--------------------------------------------------------------------------
 * Enum: warping_engine_irq_type
 *  WARPING_ENGINE_IRQ_WARPING_FINISHED          - Indicates a finished warping 
 */
typedef enum {
  WARPING_ENGINE_IRQ_WARPING_FINISHED = 0x01,
} warping_engine_irq_type;


/* 
 * Type: warping_engine_bool 
 *  boolean type, can hold WARPING_ENGINE_TRUE or WARPING_ENGINE_FALSE
 */
typedef unsigned warping_engine_bool;

/* 
 * Type: warping_engine_uint8 
 *  8 bit unsigned integer type
 */
typedef unsigned char warping_engine_uint8;

/* 
 * Type: warping_engine_uint16 
 *  16 bit unsigned integer type
 */
typedef unsigned short warping_engine_uint16;

/* 
 * Type: warping_engine_sint16 
 *  16 bit signed integer type
 */
typedef short warping_engine_sint16;

/* 
 * Type: warping_engine_uint32 
 *  32 bit unsigned integer type
 */
typedef unsigned warping_engine_uint32;

/* 
 * Type: warping_engine_sint32 
 *  32 bit signed integer type
 */
typedef unsigned warping_engine_sint32;

/* 
 * Type: warping_engine_ptr 
 *  Multipurpose pointer (e.g. for malloc or free)
 */
typedef void     *warping_engine_ptr;

/* 
 * Type: warping_engine_frame_ptr
 *  Pointer for a framebuffer address.
 */
typedef warping_engine_uint32 warping_engine_frame_ptr;

/* 
 * Type: warping_engine_handle 
 *  External context handle of a warping engine context
 */
typedef void     *warping_engine_handle;

/* Type: warping_engine_platform_settings 
 *  Platform dependend settings for warping_engine_init
 */
typedef void     *warping_engine_platform_settings;


/* 
 * Type: warping_engine_float 
 *  32 bit IEEE 754 floating point number
 */
typedef float    warping_engine_float;

/* 
 * Type: warping_engine_isr_callback(warping_engine_uint32)
 *  Signature of an interrupt callback.
 *  The argument ist the a_data parameter of warping_engine_registerISR.
 *
 * See also:
 *  <warping_engine_registerISR>
 */
typedef void (*warping_engine_isr_callback)(warping_engine_uint32);


/* Type: warping_engine_status
 * This struct holds the curent status of the warping engine (retrieved with <warping_engine_getStatus>)
 *
 * m_warping_finished   - Indicates finished warping
 */
typedef struct warping_engine_status_tag
{
  warping_engine_uint32 m_warping_finished:1;
} warping_engine_status;

/* Type: warping_engine_config
 * This struct holds the warping engine configuration options (retrieved with <warping_engine_getConfig>)
 *
 * m_revision_major                - Major version of the warping engine core
 * m_revision_minor                - Minor version of the warping engine core
 * m_status_registers              - If set, the warping engine status can be queried with <warping_engine_getStatus>

 */
 
/* IOCTL Commands */
#define WARPING_ENGINE_IOCTL_TYPE 'B'
#define WARPING_ENGINE_IOCTL_REG_PREFIX     (0x80)
#define WARPING_ENGINE_IOCTL_NR_SETTINGS    (0x01)
#define WARPING_ENGINE_IOCTL_MAKE_REG(reg)  (reg|WARPING_ENGINE_IOCTL_REG_PREFIX)
#define WARPING_ENGINE_IOCTL_GET_REG(nr)    (nr&(~WARPING_ENGINE_IOCTL_REG_PREFIX))
#define WARPING_ENGINE_IOCTL_WREG(reg)      (_IOW(WARPING_ENGINE_IOCTL_TYPE,WARPING_ENGINE_IOCTL_MAKE_REG(reg),unsigned long))
#define WARPING_ENGINE_IOCTL_RREG(reg)      (_IOR(WARPING_ENGINE_IOCTL_TYPE,WARPING_ENGINE_IOCTL_MAKE_REG(reg),unsigned long))
#define WARPING_ENGINE_IOCTL_GET_SETTINGS   (_IOR(WARPING_ENGINE_IOCTL_TYPE,WARPING_ENGINE_IOCTL_NR_SETTINGS,warping_engine_settings))

/* warping_engine physical memory layout */
typedef struct
{
  unsigned long base_phys;      /* base start address               */
  unsigned long span;           /* last base offset                 */
  unsigned long mem_base_phys;  /* video memory start address       */
  unsigned long mem_span;       /* last video memory cell offset    */
} warping_engine_settings;
typedef struct warping_engine_config_tag
{
  warping_engine_uint32 m_revision_major:8;
  warping_engine_uint32 m_revision_minor:8;
  warping_engine_uint32 m_status_registers:1;
} warping_engine_config;

//enumeration of performance counter events
typedef enum {
  WARPING_ENGINE_PFC_EVENT_COORDINATES_READ_BURST_READ  = 0,      //Coordinates reader did a burst read on MBI
  WARPING_ENGINE_PFC_EVENT_COORDINATES_READ_WORD_READ,            //Coordinates reader did a word read on MBI (1 event for every word in a burst)
  WARPING_ENGINE_PFC_EVENT_TXS_PREFETCH_MISS,                     //Cache miss in the in the prefetching texture cache scheduler
  WARPING_ENGINE_PFC_EVENT_TXS_REFRESH,                           //Line refresh job sent from texel schedule to texture cache
  WARPING_ENGINE_PFC_EVENT_TXC_BURST_READ,                        //Texture cache did a burst read on MBI
  WARPING_ENGINE_PFC_EVENT_TXC_WORD_READ,                         //Texture cache received a word from MBI (1 event for every word in a burst)
  WARPING_ENGINE_PFC_EVENT_TXC_PIXEL_READ,                        //Texture cache processed a pixel read (i.e. potentially multiple texel reads, still only a single event)
  WARPING_ENGINE_PFC_EVENT_TXC_PIXEL_READ_HIT,                    //Texture cache processed a pixel read without the need to wait for a fetch.
  WARPING_ENGINE_PFC_EVENT_TXC_PIXEL_READ_FETCH_WAIT,             //Wait cycles at pipeline read interface because of wait for line to be fetched.
  WARPING_ENGINE_PFC_EVENT_TXC_PIXEL_READ_RAM_WAIT,               //Wait cycles inserted in pipeline read path because read RAM needed to be accessed more than once and prefetch was not successful.
  WARPING_ENGINE_PFC_EVENT_TXC_JOB_LINE_READY,                    //A job was taken from the prefetch queue without the need to wait for the access count to reach the replace count.
  WARPING_ENGINE_PFC_EVENT_TXC_JOB_LINE_WAIT,                     //Wait cycles at prefetch queue interface because job could not be fetched as the access count had not yet reached the replace count.
  WARPING_ENGINE_PFC_EVENT_WRITE_ASSEMBLY_BURST_WRITE,            //Write assembly did a burst write on MBI
  WARPING_ENGINE_PFC_EVENT_WRITE_ASSEMBLY_WORD_WRITE,             //Write assembly did a word write on MBI (1 event for every word in a burst)
  WARPING_ENGINE_PFC_EVENT_CLOCK,                                 //Every clock cycle
} warping_engine_pfc_event_t;

/******************************************************************************
 *         functions                                                          *
 ******************************************************************************/
warping_engine_handle warping_engine_init(warping_engine_platform_settings a_platform);
void warping_engine_exit(warping_engine_handle a_handle);

warping_engine_config warping_engine_getConfig(warping_engine_handle a_handle);

void warping_engine_registerISR(warping_engine_handle a_handle, warping_engine_irq_type a_type, warping_engine_isr_callback a_callback, warping_engine_uint32 a_data);

void warping_engine_setEnabled(warping_engine_handle a_handle, warping_engine_bool a_enable);


void warping_engine_setCoordinatesAddress(warping_engine_handle a_handle, warping_engine_uint32 a_address);
void warping_engine_setCoordinatesCount(warping_engine_handle a_handle, warping_engine_uint32 a_count);
void warping_engine_setInputImageAddress(warping_engine_handle a_handle, warping_engine_uint32 a_address);
void warping_engine_setInputImageSize(warping_engine_handle a_handle, warping_engine_uint16 a_width, warping_engine_uint16 a_height);
void warping_engine_setInputImagePitch(warping_engine_handle a_handle, warping_engine_uint16 a_pitch);
void warping_engine_setOutsideColor(warping_engine_handle a_handle, warping_engine_uint32 a_color);
void warping_engine_setOutputImageAddress(warping_engine_handle a_handle, warping_engine_uint32 a_address);
void warping_engine_setOutputImageSize(warping_engine_handle a_handle, warping_engine_uint16 a_width, warping_engine_uint16 a_height);
void warping_engine_setOutputImagePitch(warping_engine_handle a_handle, warping_engine_uint16 a_pitch);
void warping_engine_setStripeWidth(warping_engine_handle a_handle, warping_engine_uint8 a_stripe_width);
void warping_engine_resetPipeline(warping_engine_handle a_handle);

void warping_engine_enablePerformanceCounters(warping_engine_handle a_handle, warping_engine_uint32 a_enable_mask);
void warping_engine_clearPerformanceCounters(warping_engine_handle a_handle, warping_engine_uint32 a_clear_mask);
void warping_engine_enablePerformanceCounter(warping_engine_handle a_handle, warping_engine_uint8 a_pfc_number);
void warping_engine_clearPerformanceCounter(warping_engine_handle a_handle, warping_engine_uint8 a_pfc_number);
void warping_engine_setPerformanceCounterEvent(warping_engine_handle a_handle, warping_engine_uint8 a_pfc_number, warping_engine_pfc_event_t a_pfc_event);
warping_engine_uint32 warping_engine_getPerformanceCounterValue(warping_engine_handle a_handle, warping_engine_uint8 a_pfc_number);

 
#endif // WARPING_ENGINE_H_
