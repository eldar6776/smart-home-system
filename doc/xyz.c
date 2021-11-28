/**
 * UTILS FUNCTIONS
 **/

#define MODBUS_GET_HIGH_BYTE(data) (((data) >> 8) & 0xFF)
#define MODBUS_GET_LOW_BYTE(data) ((data) & 0xFF)
#define MODBUS_GET_INT64_FROM_INT16(tab_int16, index) \
    (((int64_t)tab_int16[(index)    ] << 48) + \
     ((int64_t)tab_int16[(index) + 1] << 32) + \
     ((int64_t)tab_int16[(index) + 2] << 16) + \
      (int64_t)tab_int16[(index) + 3])
#define MODBUS_GET_INT32_FROM_INT16(tab_int16, index) ((tab_int16[(index)] << 16) + tab_int16[(index) + 1])
#define MODBUS_GET_INT16_FROM_INT8(tab_int8, index) ((tab_int8[(index)] << 8) + tab_int8[(index) + 1])
#define MODBUS_SET_INT16_TO_INT8(tab_int8, index, value) \
    do { \
        tab_int8[(index)] = (value) >> 8;  \
        tab_int8[(index) + 1] = (value) & 0xFF; \
    } while (0)
#define MODBUS_SET_INT32_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 16; \
        tab_int16[(index) + 1] = (value); \
    } while (0)
#define MODBUS_SET_INT64_TO_INT16(tab_int16, index, value) \
    do { \
        tab_int16[(index)    ] = (value) >> 48; \
        tab_int16[(index) + 1] = (value) >> 32; \
        tab_int16[(index) + 2] = (value) >> 16; \
        tab_int16[(index) + 3] = (value); \
    } while (0)

#include <stdlib.h>

#ifndef _MSC_VER
#  include <stdint.h>
#else
#  include "stdint.h"
#endif

#include <string.h>
#include <assert.h>

#if defined(_WIN32)
#  include <winsock2.h>
#elif defined(ARDUINO)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htonl(x) bswap_32(x)
#define htons(x) bswap_16(x)
#define ntohl(x) bswap_32(x)
#define ntohs(x) bswap_32(x)
#else
#define htonl(x) (x)
#define htons(x) (x)
#define ntohl(x) (x)
#define ntohs(x) (x)
#endif
#else
#  include <arpa/inet.h>
#endif

#ifndef ARDUINO
#include <config.h>
#endif

#include "modbus.h"

#if defined(HAVE_BYTESWAP_H)
#  include <byteswap.h>
#endif

#if defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#  define bswap_16 OSSwapInt16
#  define bswap_32 OSSwapInt32
#  define bswap_64 OSSwapInt64
#endif

#if defined(__GNUC__)
#  define GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ * 10)
#  if GCC_VERSION >= 430
// Since GCC >= 4.30, GCC provides __builtin_bswapXX() alternatives so we switch to them
#    undef bswap_32
#    define bswap_32 __builtin_bswap32
#  endif
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  define bswap_32 _byteswap_ulong
#  define bswap_16 _byteswap_ushort
#endif

#if !defined(__CYGWIN__) && !defined(bswap_16)
#ifndef ARDUINO
#  warning "Fallback on C functions for bswap_16"
#endif
static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}
#endif

#if !defined(bswap_32)
#ifndef ARDUINO
#  warning "Fallback on C functions for bswap_32"
#endif
static inline uint32_t bswap_32(uint32_t x)
{
    return (bswap_16(x & 0xffff) << 16) | (bswap_16(x >> 16));
}
#endif
#define _GPS_MPH_PER_KNOT 1.15077945
#define _GPS_MPS_PER_KNOT 0.51444444
#define _GPS_KMPH_PER_KNOT 1.852
#define _GPS_MILES_PER_METER 0.00062137112
#define _GPS_KM_PER_METER 0.001
#define _GPS_FEET_PER_METER 3.2808399
#define _GPS_MAX_FIELD_SIZE 15

#define CHARISNUM(x)                        ((x) >= '0' && (x) <= '9')
#define CHARISHEXNUM(x)                     (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define CHARTONUM(x)                        ((x) - '0')
#define CHARHEXTONUM(x)                     (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (((x) >= 'a' && (x) <= 'z') ? ((x) - 'a' + 10) : (((x) >= 'A' && (x) <= 'Z') ? ((x) - 'A' + 10) : 0)))
#define FROMMEM(x)                          ((const char *)(x))

#define UART_SEND_STR(str)                  do { Send.Data = (const uint8_t *)(str); Send.Count = strlen((const char *)(str)); GSM_LL_Callback(GSM_LL_Control_Send, &Send, &Send.Result); } while (0)
#define UART_SEND(str, len)                 do { Send.Data = (const uint8_t *)(str); Send.Count = (len); GSM_LL_Callback(GSM_LL_Control_Send, &Send, &Send.Result); } while (0)
#define UART_SEND_CH(ch)                    do { Send.Data = (const uint8_t *)(ch); Send.Count = 1; GSM_LL_Callback(GSM_LL_Control_Send, &Send, &Send.Result); } while (0)
typedef struct {
    uint8_t Length;
    uint8_t Data[128];
} Received_t;
#define RECEIVED_ADD(c)                     do { Received.Data[Received.Length++] = (c); Received.Data[Received.Length] = 0; } while (0)
#define RECEIVED_RESET()                    do { Received.Length = 0; Received.Data[0] = 0; } while (0)
#define RECEIVED_LENGTH()                   Received.Length
/**
 * \defgroup LOWLEVEL_Macros
 * \brief    GSM Low-Level macros
 * \{
 */
#define GSM_OK                              FROMMEM("OK\r\n")
#define GSM_ERROR                           FROMMEM("ERROR\r\n")
#define GSM_BUSY                            FROMMEM("BUSY\r\n")
#define GSM_NO_CARRIER                      FROMMEM("NO CARRIER\r\n")
#define GSM_RING                            FROMMEM("RING\r\n")
#define GSM_CRLF                            FROMMEM("\r\n")


#define GSM_RTS_SET         1       /*!< RTS should be set high */
#define GSM_RTS_CLR         0       /*!< RTS should be set low */
#define GSM_RESET_SET       1       /*!< Reset pin should be set */
#define GSM_RESET_CLR       0       /*!< Reset pin should be cleared */
/**
 * \brief   Loe-level control enumeration with supported callbacks
 * 
 * This enumeration is used in \ref GSM_LL_Callback function with param and result parameters to function
 */
typedef enum _GSM_LL_Control_t {
    /**
     * \brief       Called to initialize low-level part of device, such as UART and GPIO configuration
     *
     * \param[in]   *param: Pointer to \ref GSM_LL_t structure with baudrate setup
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_Init = 0x00,     /*!< Initialization control */
    
    /**
     * \brief       Called to send data to GSM device
     *
     * \param[in]   *param: Pointer to \ref GSM_LL_Send_t structure with data to send
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_Send,            /*!< Send data control */
    
    /**
     * \brief       Called to set software RTS pin when necessary
     *
     * \param[in]   *param: Pointer to \ref uint8_t variable with RTS info. This parameter can be a value of \ref GSM_RTS_SET or \ref GSM_RTS_CLR macros
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_SetRTS,          /*!< Set software RTS control */
    
    /**
     * \brief       Called to set reset pin when necessary
     *
     * \param[in]   *param: Pointer to \ref uint8_t variable with RESET info. This parameter can be a value of \ref GSM_RESET_SET or \ref GSM_RESET_CLR macros
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_SetReset,        /*!< Set reset control */
    
    /**
     * \brief       Called to create system synchronization object on RTOS support
     *
     * \param[in]   *param: Pointer to \ref GSM_RTOS_SYNC_t variable with sync object
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_SYS_Create,      /*!< Creates a synchronization object */
    
    /**
     * \brief       Called to delete system synchronization object on RTOS support
     *
     * \param[in]   *param: Pointer to \ref GSM_RTOS_SYNC_t variable with sync object
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_SYS_Delete,      /*!< Deletes a synchronization object */
    
    /**
     * \brief       Called to grant access to sync object
     *
     * \param[in]   *param: Pointer to \ref GSM_RTOS_SYNC_t variable with sync object
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_SYS_Request,     /*!< Requests grant for specific sync object */
    
    /**
     * \brief       Called to release access from sync object
     *
     * \param[in]   *param: Pointer to \ref GSM_RTOS_SYNC_t variable with sync object
     * \param[out]  *result: Pointer to \ref uint8_t variable with result. Set to 0 when OK, or non-zero on ERROR.
     */
    GSM_LL_Control_SYS_Release,     /*!< Releases grant for specific sync object */
} GSM_LL_Control_t;

/**
 * \brief   Structure for sending data to low-level part
 */
typedef struct _GSM_LL_Send_t {
    const uint8_t* Data;            /*!< Pointer to data to send */
    uint16_t Count;                 /*!< Number of bytes to send */
    uint8_t Result;                 /*!< Result of last send */
} GSM_LL_Send_t;

/**
 * \brief   Low level structure for driver
 * \note    For now it has basic settings only without hardware flow control.
 */
typedef struct _GSM_LL_t {
    uint32_t Baudrate;              /*!< Baudrate to be used for UART */
} GSM_LL_t;
/**
 * \brief       Low-level callback for interaction with device specific section
 * \param[in]   ctrl: Control to be done
 * \param[in]   *param: Pointer to parameter setup, depends on control type
 * \param[out]  *result: Optional result parameter in case of commands
 * \retval      1: Control command has been processed
 * \retval      0: Control command has not been processed
 */
uint8_t GSM_LL_Callback(GSM_LL_Control_t ctrl, void* param, void* result);
uint8_t GSM_LL_Callback(GSM_LL_Control_t ctrl, void* param, void* result) {
    switch (ctrl) {
        case GSM_LL_Control_Init: {                 /* Initialize low-level part of communication */
            GSM_LL_t* LL = (GSM_LL_t *)param;       /* Get low-level value from callback */
            
            /************************************/
            /*  Device specific initialization  */
            /************************************/
            TM_USART_Init(USART1, TM_USART_PinsPack_1, LL->Baudrate);
            TM_GPIO_Init(GPIOA, GPIO_PIN_1, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low);
            
            if (result) {
                *(uint8_t *)result = 0;             /* Successfully initialized */
            }
            return 1;                               /* Return 1 = command was processed */
        }
        case GSM_LL_Control_Send: {
            GSM_LL_Send_t* send = (GSM_LL_Send_t *)param;   /* Get send parameters */
            
            /* Send actual data to UART, implement function to send data */
        	TM_USART_Send(USART1, (uint8_t *)send->Data, send->Count);
            
            if (result) {
                *(uint8_t *)result = 0;             /* Successfully send */
            }
            return 1;                               /* Command processed */
        }
        case GSM_LL_Control_SetReset: {             /* Set reset value */
            uint8_t state = *(uint8_t *)param;      /* Get state packed in uint8_t variable */
            if (state == GSM_RESET_SET) {
                TM_GPIO_SetPinLow(GPIOA, GPIO_PIN_1);
            } else {
                TM_GPIO_SetPinHigh(GPIOA, GPIO_PIN_1);
            }
            return 1;                               /* Command has been processed */
        }
        case GSM_LL_Control_SetRTS: {               /* Set RTS value */
            uint8_t state = *(uint8_t *)param;      /* Get state packed in uint8_t variable */
            (void)state;                            /* Prevent compiler warnings */
            return 1;                               /* Command has been processed */
        }
#if GSM_RTOS
        case GSM_LL_Control_SYS_Create: {           /* Create system synchronization object */
            GSM_RTOS_SYNC_t* Sync = (GSM_RTOS_SYNC_t *)param;   /* Get pointer to sync object */
            id = osMutexCreate(Sync);               /* Create mutex */
            
            if (result) {
                *(uint8_t *)result = id == NULL;    /*!< Set result value */
            }
            return 1;                               /* Command processed */
        }
        case GSM_LL_Control_SYS_Delete: {           /* Delete system synchronization object */
            GSM_RTOS_SYNC_t* Sync = (GSM_RTOS_SYNC_t *)param;   /* Get pointer to sync object */
            osMutexDelete(Sync);                    /* Delete mutex object */
            
            if (result) {
                *(uint8_t *)result = id == NULL;    /*!< Set result value */
            }
            return 1;                               /* Command processed */
        }
        case GSM_LL_Control_SYS_Request: {          /* Request system synchronization object */
            GSM_RTOS_SYNC_t* Sync = (GSM_RTOS_SYNC_t *)param;   /* Get pointer to sync object */
            (void)Sync;                             /* Prevent compiler warnings */
            
            *(uint8_t *)result = osMutexWait(id, 1000) == osOK ? 0 : 1; /* Set result according to rGSMonse */
            return 1;                               /* Command processed */
        }
        case GSM_LL_Control_SYS_Release: {          /* Release system synchronization object */
            GSM_RTOS_SYNC_t* Sync = (GSM_RTOS_SYNC_t *)param;   /* Get pointer to sync object */
            (void)Sync;                             /* Prevent compiler warnings */
            
            *(uint8_t *)result = osMutexRelease(id) == osOK ? 0 : 1;    /* Set result according to rGSMonse */
            return 1;                               /* Command processed */
        }
#endif /* GSM_RTOS */
        default: 
            return 0;
    }
}
/**
   \brief Convert an integer to a string.

   The function itoa() converts the integer value from \c val into an
   ASCII representation that will be stored under \c s.  The caller
   is responsible for providing sufficient storage in \c s.

   \note The minimal size of the buffer \c s depends on the choice of
   radix. For example, if the radix is 2 (binary), you need to supply a buffer
   with a minimal length of 8 * sizeof (int) + 1 characters, i.e. one
   character for each bit plus one for the string terminator. Using a larger
   radix will require a smaller minimal buffer size.

   \warning If the buffer is too small, you risk a buffer overflow.

   Conversion is done using the \c radix as base, which may be a
   number between 2 (binary conversion) and up to 36.  If \c radix
   is greater than 10, the next digit after \c '9' will be the letter
   \c 'a'.
    
    If radix is 10 and val is negative, a minus sign will be prepended.

   The itoa() function returns the pointer passed as \c s.
*/
#ifdef  __DOXYGEN__
extern char *itoa(int val, char *s, int radix);
#else
extern __inline__ __ATTR_GNU_INLINE__
char *itoa (int __val, char *__s, int __radix)
{
    if (!__builtin_constant_p (__radix)) {
	extern char *__itoa (int, char *, int);
	return __itoa (__val, __s, __radix);
    } else if (__radix < 2 || __radix > 36) {
	*__s = 0;
	return __s;
    } else {
	extern char *__itoa_ncheck (int, char *, unsigned char);
	return __itoa_ncheck (__val, __s, __radix);
    }
}
#endif

/**
 \ingroup avr_stdlib
 
   \brief Convert a long integer to a string.

   The function ltoa() converts the long integer value from \c val into an
   ASCII representation that will be stored under \c s.  The caller
   is responsible for providing sufficient storage in \c s.

   \note The minimal size of the buffer \c s depends on the choice of
   radix. For example, if the radix is 2 (binary), you need to supply a buffer
   with a minimal length of 8 * sizeof (long int) + 1 characters, i.e. one
   character for each bit plus one for the string terminator. Using a larger
   radix will require a smaller minimal buffer size.

   \warning If the buffer is too small, you risk a buffer overflow.

   Conversion is done using the \c radix as base, which may be a
   number between 2 (binary conversion) and up to 36.  If \c radix
   is greater than 10, the next digit after \c '9' will be the letter
   \c 'a'.

   If radix is 10 and val is negative, a minus sign will be prepended.

   The ltoa() function returns the pointer passed as \c s.
*/
#ifdef  __DOXYGEN__
extern char *ltoa(long val, char *s, int radix);
#else
extern __inline__ __ATTR_GNU_INLINE__
char *ltoa (long __val, char *__s, int __radix)
{
    if (!__builtin_constant_p (__radix)) {
	extern char *__ltoa (long, char *, int);
	return __ltoa (__val, __s, __radix);
    } else if (__radix < 2 || __radix > 36) {
	*__s = 0;
	return __s;
    } else {
	extern char *__ltoa_ncheck (long, char *, unsigned char);
	return __ltoa_ncheck (__val, __s, __radix);
    }
}
#endif

/**
 \ingroup avr_stdlib

   \brief Convert an unsigned integer to a string.

   The function utoa() converts the unsigned integer value from \c val into an
   ASCII representation that will be stored under \c s.  The caller
   is responsible for providing sufficient storage in \c s.

   \note The minimal size of the buffer \c s depends on the choice of
   radix. For example, if the radix is 2 (binary), you need to supply a buffer
   with a minimal length of 8 * sizeof (unsigned int) + 1 characters, i.e. one
   character for each bit plus one for the string terminator. Using a larger
   radix will require a smaller minimal buffer size.

   \warning If the buffer is too small, you risk a buffer overflow.

   Conversion is done using the \c radix as base, which may be a
   number between 2 (binary conversion) and up to 36.  If \c radix
   is greater than 10, the next digit after \c '9' will be the letter
   \c 'a'.

   The utoa() function returns the pointer passed as \c s.
*/
#ifdef  __DOXYGEN__
extern char *utoa(unsigned int val, char *s, int radix);
#else
extern __inline__ __ATTR_GNU_INLINE__
char *utoa (unsigned int __val, char *__s, int __radix)
{
    if (!__builtin_constant_p (__radix)) {
	extern char *__utoa (unsigned int, char *, int);
	return __utoa (__val, __s, __radix);
    } else if (__radix < 2 || __radix > 36) {
	*__s = 0;
	return __s;
    } else {
	extern char *__utoa_ncheck (unsigned int, char *, unsigned char);
	return __utoa_ncheck (__val, __s, __radix);
    }
}
#endif

/**
 \ingroup avr_stdlib
   \brief Convert an unsigned long integer to a string.

   The function ultoa() converts the unsigned long integer value from
   \c val into an ASCII representation that will be stored under \c s.
   The caller is responsible for providing sufficient storage in \c s.

   \note The minimal size of the buffer \c s depends on the choice of
   radix. For example, if the radix is 2 (binary), you need to supply a buffer
   with a minimal length of 8 * sizeof (unsigned long int) + 1 characters,
   i.e. one character for each bit plus one for the string terminator. Using a
   larger radix will require a smaller minimal buffer size.

   \warning If the buffer is too small, you risk a buffer overflow.

   Conversion is done using the \c radix as base, which may be a
   number between 2 (binary conversion) and up to 36.  If \c radix
   is greater than 10, the next digit after \c '9' will be the letter
   \c 'a'.

   The ultoa() function returns the pointer passed as \c s.
*/
#ifdef  __DOXYGEN__
extern char *ultoa(unsigned long val, char *s, int radix);
#else
extern __inline__ __ATTR_GNU_INLINE__
char *ultoa (unsigned long __val, char *__s, int __radix)
{
    if (!__builtin_constant_p (__radix)) {
	extern char *__ultoa (unsigned long, char *, int);
	return __ultoa (__val, __s, __radix);
    } else if (__radix < 2 || __radix > 36) {
	*__s = 0;
	return __s;
    } else {
	extern char *__ultoa_ncheck (unsigned long, char *, unsigned char);
	return __ultoa_ncheck (__val, __s, __radix);
    }
}
#endif
/* UART receive interrupt handler */
void TM_USART1_ReceiveHandler(uint8_t ch) {
	/* Send received character to GSM stack */
	GSM_DataReceived(&ch, 1);
}

/* Sets many bits from a single byte value (all 8 bits of the byte value are
   set) */
void modbus_set_bits_from_byte(uint8_t *dest, int idx, const uint8_t value)
{
    int i;

    for (i=0; i < 8; i++) {
        dest[idx+i] = (value & (1 << i)) ? 1 : 0;
    }
}
/* Sets many bits from a table of bytes (only the bits between idx and
   idx + nb_bits are set) */
void modbus_set_bits_from_bytes(uint8_t *dest, int idx, unsigned int nb_bits,
                                const uint8_t *tab_byte)
{
    unsigned int i;
    int shift = 0;

    for (i = idx; i < idx + nb_bits; i++) {
        dest[i] = tab_byte[(i - idx) / 8] & (1 << shift) ? 1 : 0;
        /* gcc doesn't like: shift = (++shift) % 8; */
        shift++;
        shift %= 8;
    }
}
/* Gets the byte value from many bits.
   To obtain a full byte, set nb_bits to 8. */
uint8_t modbus_get_byte_from_bits(const uint8_t *src, int idx,
                                  unsigned int nb_bits)
{
    unsigned int i;
    uint8_t value = 0;

    if (nb_bits > 8) {
        /* Assert is ignored if NDEBUG is set */
        assert(nb_bits < 8);
        nb_bits = 8;
    }

    for (i=0; i < nb_bits; i++) {
        value |= (src[idx+i] << i);
    }

    return value;
}
/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = ntohl(((uint32_t)src[0] << 16) + src[1]);
    memcpy(&f, &i, sizeof(float));

    return f;
}
/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = ntohl(bswap_32((((uint32_t)src[0]) << 16) + src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}
/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint32_t i;

#if defined(ARDUINO) && defined(__AVR__)
    i = ntohl((uint32_t)((uint32_t)bswap_16(src[0]) << 16) + bswap_16(src[1]));
#else
    i = ntohl((uint32_t)(bswap_16(src[0]) << 16) + bswap_16(src[1]));
#endif
    memcpy(&f, &i, sizeof(float));

    return f;
}
/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = ntohl((((uint32_t)src[1]) << 16) + src[0]);
    memcpy(&f, &i, sizeof(float));

    return f;
}
/* DEPRECATED - Get a float from 4 bytes in sort of Modbus format */
float modbus_get_float(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = (((uint32_t)src[1]) << 16) + src[0];
    memcpy(&f, &i, sizeof(float));

    return f;
}
/* Set a float to 4 bytes for Modbus w/o any conversion (ABCD) */
void modbus_set_float_abcd(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = htonl(i);
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}
/* Set a float to 4 bytes for Modbus with byte and word swap conversion (DCBA) */
void modbus_set_float_dcba(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = bswap_32(htonl(i));
    dest[0] = (uint16_t)(i >> 16);
    dest[1] = (uint16_t)i;
}
/* Set a float to 4 bytes for Modbus with byte swap conversion (BADC) */
void modbus_set_float_badc(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = htonl(i);
    dest[0] = (uint16_t)bswap_16(i >> 16);
    dest[1] = (uint16_t)bswap_16(i & 0xFFFF);
}
/* Set a float to 4 bytes for Modbus with word swap conversion (CDAB) */
void modbus_set_float_cdab(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    i = htonl(i);
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}
/* DEPRECATED - Set a float to 4 bytes in a sort of Modbus format! */
void modbus_set_float(float f, uint16_t *dest)
{
    uint32_t i;

    memcpy(&i, &f, sizeof(uint32_t));
    dest[0] = (uint16_t)i;
    dest[1] = (uint16_t)(i >> 16);
}
/**
 * @defgroup TM_GENERAL_Macros
 * @brief    Library defines
 * @{
 */
/**
 * @brief  System speed in MHz
 */
extern uint16_t GENERAL_SystemSpeedInMHz;
/* System speed in MHz */
uint16_t GENERAL_SystemSpeedInMHz = 0;
static uint16_t InterruptDisabledCount = 0;
/* Private functions */
static uint32_t x_na_y(uint32_t x, uint8_t y) {
	uint32_t output = 1;
	
	/* Make a "power" multiply */
	while (y--) {
		output *= x;
	}
	
	/* Return output value */
	return output;
}
/**
 * @brief  Clock speed enumeration
 */
typedef enum {
	TM_GENERAL_Clock_HSI,    /*!< High speed internal clock */
	TM_GENERAL_Clock_HSE,    /*!< High speed external clock */
	TM_GENERAL_Clock_SYSCLK, /*!< System core clock */
	TM_GENERAL_Clock_PCLK1,  /*!< PCLK1 (APB1) peripheral clock */
	TM_GENERAL_Clock_PCLK2,  /*!< PCLK2 (APB2) peripheral clock */
	TM_GENERAL_Clock_HCLK    /*!< HCLK (AHB1) high speed clock */
} TM_GENERAL_Clock_t;
/**
 * @brief  All possible reset sources
 */
typedef enum {
	TM_GENERAL_ResetSource_None = 0x00,     /*!< No reset source detected. Flags are already cleared */
	TM_GENERAL_ResetSource_LowPower = 0x01, /*!< Low-power management reset occurs */
	TM_GENERAL_ResetSource_WWDG = 0x02,     /*!< Window watchdog reset occurs */
	TM_GENERAL_ResetSource_IWDG = 0x03,     /*!< Independent watchdog reset occurs */
	TM_GENERAL_ResetSource_Software = 0x04, /*!< Software reset occurs */
	TM_GENERAL_ResetSource_POR = 0x05,      /*!< POR/PDR reset occurs */
	TM_GENERAL_ResetSource_PIN = 0x06,      /*!< NRST pin is set to low by hardware reset, hardware reset */
	TM_GENERAL_ResetSource_BOR = 0x07,      /*!< BOR reset occurs */
} TM_GENERAL_ResetSource_t;
/**
 * @brief  Float number operation structure
 */
typedef struct {
	int32_t Integer;  /*!< Integer part of float number */
	uint32_t Decimal; /*!< Decimal part of float number */
} TM_GENERAL_Float_t;
/**
 * @}
 */
/**
 * @defgroup TM_GENERAL_Functions
 * @brief    Library Functions
 * @{
 */
/**
 * @brief  Performs a system reset
 * @note   Before system will be reset, @ref TM_GENERAL_SoftwareResetCallback() will be called,
 *         where you can do important stuff if necessary
 * @param  None
 * @retval None
 */
void TM_GENERAL_SystemReset(void);
/**
 * @brief  Gets reset source why system was reset
 * @param  reset_flags: After read, clear reset flags
 *            - 0: Flags will stay untouched
 *            - > 0: All reset flags will reset
 * @retval Member of @ref TM_GENERAL_ResetSource_t containing reset source
 */
TM_GENERAL_ResetSource_t TM_GENERAL_GetResetSource(uint8_t reset_flags);
/**
 * @brief  Disables all interrupts in system
 * @param  None
 * @retval None
 */
void TM_GENERAL_DisableInterrupts(void);
/**
 * @brief  Enables interrupts in system.
 * @note   This function has nesting support. This means that if you call @ref TM_GENERAL_DisableInterrupts() 4 times,
 *         then you have to call this function also 4 times to enable interrupts.
 * @param  None
 * @retval Interrupt enabled status:
 *            - 0: Interrupts were not enabled
 *            - > 0: Interrupts were enabled
 */
uint8_t TM_GENERAL_EnableInterrupts(void);
/**
 * @brief  Checks if code execution is inside active IRQ
 * @param  None
 * @retval IRQ Execution status:
 *            - 0: Code execution is not inside IRQ, thread mode
 *            - > 0: Code execution is inside IRQ, IRQ mode
 * @note   Defines as macro for faster execution
 */
#define TM_GENERAL_IsIRQMode()               (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk)
/**
 * @brief  Gets specific clock speed value from STM32F4xx device
 * @param  clock: Clock type you want to know speed for. This parameter can be a value of @ref TM_GENERAL_Clock_t enumeration
 * @retval Clock speed in units of hertz
 */
uint32_t TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_t clock);
/**
 * @brief  Gets system clock speed in units of MHz
 * @param  None
 * @retval None
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_GetSystemClockMHz()       ((uint16_t)(SystemCoreClock * (float)0.000001))
/**
 * @brief  Enables DWT counter in Cortex-M4 core
 * @param  None
 * @retval DWT Status:
 *            - 0: DWT has not started, hardware/software reset is required
 *            - > 0: DWT has started and is ready to use
 * @note   It may happen, that DWT counter won't start after reprogramming device.
 *         This happened to me when I use onboard ST-Link on Discovery or Nucleo boards.
 *         When I used external debugger (J-Link or ULINK2) it worked always without problems.
 *         If your DWT doesn't start, you should perform software/hardware reset by yourself.
 */
uint8_t TM_GENERAL_DWTCounterEnable(void);
/**
 * @brief  Disables DWT counter in Cortex-M4 core
 * @param  None
 * @retval None
 * @note   Defined as macro for faster execution
 */
#if !defined(STM32F0xx)
#define TM_GENERAL_DWTCounterDisable()       (DWT->CTRL &= ~0x00000001)
#endif
/**
 * @brief  Gets current DWT counter value
 * @param  None
 * @retval DWT counter value
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_DWTCounterGetValue()      (DWT->CYCCNT)
/**
 * @brief  Sets DWT counter value
 * @param  x: Value to be set to DWT counter
 * @retval None
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_DWTCounterSetValue(x)     (DWT->CYCCNT = (x))
/**
 * @brief  Delays for amount of microseconds using DWT counter
 * @param  micros: Number of micro seconds for delay 
 * @retval None
 * @note   DWT Counter HAVE to be initialized first using @ref TM_GENERAL_EnableDWTCounter()
 */
static __INLINE void TM_GENERAL_DWTCounterDelayus(uint32_t micros) {
	uint32_t c = TM_GENERAL_DWTCounterGetValue();
	
	/* Calculate clock cycles */
	micros *= (SystemCoreClock / 1000000);
	micros -= 12;
	
	/* Wait till done */
	while ((TM_GENERAL_DWTCounterGetValue() - c) < micros);
}
/**
 * @brief  Delays for amount of milliseconds using DWT counter
 * @param  millis: Number of micro seconds for delay 
 * @retval None
 * @note   DWT Counter HAVE to be initialized first using @ref TM_GENERAL_EnableDWTCounter()
 */
static __INLINE void TM_GENERAL_DWTCounterDelayms(uint32_t millis) {
	uint32_t c = TM_GENERAL_DWTCounterGetValue();
	
	/* Calculate clock cycles */
	millis *= (SystemCoreClock / 1000);
	millis -= 12;
	
	/* Wait till done */
	while ((TM_GENERAL_DWTCounterGetValue() - c) < millis);
}
/**
 * @brief  Checks if number is odd or even
 * @param  number: Number to check if it is odd or even
 * @retval Is number even status:
 *            - 0: Number is odd
 *            - > 0: Number is even
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_IsNumberEven(number)          ((number & 1) == 0)
/**
 * @brief  Converts float coded number into integer and decimal part
 * @param  *Float_Struct: Pointer to empty @ref TM_GENERAL_Float_t to store result into
 * @param  Number: Float number to convert
 * @param  decimals: Number of decimal places for conversion, maximum 9 decimal places
 * @retval None
 * @note   Example: You have number 15.002 in float format.
 *            - You want to split this to integer and decimal part with 6 decimal places.
 *            - Call @ref TM_GENERAL_ConvertFloat(&Float_Struct, 15.002, 6);
 *            - Result will be: Integer: 15; Decimal: 2000 (0.002 * 10^6)
 */
void TM_GENERAL_ConvertFloat(TM_GENERAL_Float_t* Float_Struct, float Number, uint8_t decimals);
/**
 * @brief  Round float number to nearest number with custom number of decimal places
 * @param  Number: Float number to round
 * @param  decimals: Number of decimal places to round, maximum 9 decimal places
 * @retval Rounded float number
 */
float TM_GENERAL_RoundFloat(float Number, uint8_t decimals);
/**
 * @brief  Checks if number is power of 2
 * @note   It can be used to determine if number has more than just one bit set
 *         If only one bit is set, function will return > 0 because this is power of 2.
 * @param  number: Number to check if it is power of 2
 * @retval Is power of 2 status:
 *            - 0: Number if not power of 2
 *            - > 0: Number is power of 2
 * @note   Defined as macro for faster execution
 */
#define TM_GENERAL_IsNumberPowerOfTwo(number)    (number && !(number & (number - 1)))
/**
 * @brief  Calculates next power of 2 from given number
 * @param  number: Input number to be calculated
 * @retval Number with next power of 2
 *         Example:
 *            - Input number: 450
 *            - Next power of 2 is: 512 = 2^9
 */
uint32_t TM_GENERAL_NextPowerOf2(uint32_t number);
/**
 * @brief  Forces processor to jump to Hard-fault handler
 * @note   Function tries to call function at zero location in memory which causes hard-fault
 * @param  None
 * @retval None
 */
void TM_GENERAL_ForceHardFaultError(void);
/**
 * @brief  System reset callback.
 * @note   Function is called before software reset occurs.
 * @param  None
 * @retval None
 * @note   With __weak parameter to prevent link errors if not defined by user
 */
void TM_GENERAL_SystemResetCallback(void);
void TM_GENERAL_DisableInterrupts(void) {
	/* Disable interrupts */
	__disable_irq();
	
	/* Increase number of disable interrupt function calls */
	InterruptDisabledCount++;
}
uint8_t TM_GENERAL_EnableInterrupts(void) {
	/* Decrease number of disable interrupt function calls */
	if (InterruptDisabledCount) {
		InterruptDisabledCount--;
	}
	
	/* Check if we are ready to enable interrupts */
	if (!InterruptDisabledCount) {
		/* Enable interrupts */
		__enable_irq();
	}
	
	/* Return interrupt enabled status */
	return !InterruptDisabledCount;
}
void TM_GENERAL_SystemReset(void) {
	/* Call user callback function */
	TM_GENERAL_SystemResetCallback();
	
	/* Perform a system software reset */
	NVIC_SystemReset();
}
uint32_t TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_t clock) {
	uint32_t c = 0;

	/* Return clock speed */
	switch (clock) {
		case TM_GENERAL_Clock_HSI:
			c = HSI_VALUE;
			break;
		case TM_GENERAL_Clock_HSE:
			c = HSE_VALUE;
			break;
		case TM_GENERAL_Clock_HCLK:
			c = HAL_RCC_GetHCLKFreq();
			break;
		case TM_GENERAL_Clock_PCLK1:
			c = HAL_RCC_GetPCLK1Freq();
			break;
		case TM_GENERAL_Clock_PCLK2:
			c = HAL_RCC_GetPCLK2Freq();
			break;
		case TM_GENERAL_Clock_SYSCLK:
			c = HAL_RCC_GetSysClockFreq();
			break;
		default:
			break;
	}
	
	/* Return clock */
	return c;
}
TM_GENERAL_ResetSource_t TM_GENERAL_GetResetSource(uint8_t reset_flags) {
	TM_GENERAL_ResetSource_t source = TM_GENERAL_ResetSource_None;

	/* Check bits */
	if (RCC->CSR & RCC_CSR_LPWRRSTF) {
		source = TM_GENERAL_ResetSource_LowPower;
	} else if (RCC->CSR & RCC_CSR_WWDGRSTF) {
		source = TM_GENERAL_ResetSource_WWDG;
#if defined(STM32F4xx)
	} else if (RCC->CSR & RCC_CSR_WDGRSTF) {
#else
	} else if (RCC->CSR & RCC_CSR_IWDGRSTF) {
#endif
		source = TM_GENERAL_ResetSource_IWDG;
	} else if (RCC->CSR & RCC_CSR_SFTRSTF) {
		source = TM_GENERAL_ResetSource_Software;
	} else if (RCC->CSR & RCC_CSR_PORRSTF) {
		source = TM_GENERAL_ResetSource_POR;
	} else if (RCC->CSR & RCC_CSR_BORRSTF) {
		source = TM_GENERAL_ResetSource_BOR;
#if defined(STM32F4xx)
	} else if (RCC->CSR & RCC_CSR_PADRSTF) {
#else
	} else if (RCC->CSR & RCC_CSR_PINRSTF) {
#endif		
		source = TM_GENERAL_ResetSource_PIN;
	}
	
	/* Check for clearing flags */
	if (reset_flags) {
		RCC->CSR = RCC_CSR_RMVF;
	}
	
	/* Return source */
	return source;
}
#if !defined(STM32F0xx)
uint8_t TM_GENERAL_DWTCounterEnable(void) {
	uint32_t c;
	
	/* Set clock speed if not already */
	if (GENERAL_SystemSpeedInMHz == 0) {
		/* Get clock speed in MHz */
		GENERAL_SystemSpeedInMHz = TM_GENERAL_GetClockSpeed(TM_GENERAL_Clock_SYSCLK) / 1000000;
	}
	
    /* Enable TRC */
    CoreDebug->DEMCR &= ~0x01000000;
    CoreDebug->DEMCR |=  0x01000000;
	
    /* Enable counter */
    DWT->CTRL &= ~0x00000001;
    DWT->CTRL |=  0x00000001;
	
    /* Reset counter */
    DWT->CYCCNT = 0;
	
	/* Check if DWT has started */
	c = DWT->CYCCNT;
	
	/* 2 dummys */
	__ASM volatile ("NOP");
	__ASM volatile ("NOP");
	
	/* Return difference, if result is zero, DWT has not started */
	return (DWT->CYCCNT - c);
}
#endif
void TM_GENERAL_ConvertFloat(TM_GENERAL_Float_t* Float_Struct, float Number, uint8_t decimals) {
	/* Check decimals */
	if (decimals > 9) {
		decimals = 9;
	}
	
	/* Get integer part */
	Float_Struct->Integer = (int32_t)Number;
	
	/* Get decimal part */
	if (Number < 0) {
		Float_Struct->Decimal = (int32_t)((float)(Float_Struct->Integer - Number) * x_na_y(10, decimals));
	} else {
		Float_Struct->Decimal = (int32_t)((float)(Number - Float_Struct->Integer) * x_na_y(10, decimals));
	}
}
float TM_GENERAL_RoundFloat(float Number, uint8_t decimals) {
	float x;
		
	/* Check decimals */
	if (decimals > 9) {
		decimals = 9;
	}
	
	x = x_na_y(10, decimals);
	
	/* Make truncating */
	if (Number > 0) {
		return (float)(Number * x + (float)0.5) / x;
	} 
	if (Number < 0) {
		return (float)(Number * x - (float)0.5) / x;
	}
	
	/* Return number */
	return 0;
}
uint32_t TM_GENERAL_NextPowerOf2(uint32_t number) {
	/* Check number */
	if (number <= 1) {
		return 1;
	}
	
	/* Do some bit operations */
	number--;
	number |= number >> 1;
	number |= number >> 2;
	number |= number >> 4;
	number |= number >> 8;
	number |= number >> 16;
	number++;
	
	/* Return calculated number */
	return number;
}
void TM_GENERAL_ForceHardFaultError(void) {
	/* Create hard-fault-function typedef */
	typedef void (*hff)(void);
	hff hf_func = 0;
	
	/* Call function at zero location in memory = HARDFAULT */
	hf_func();
}
__weak void TM_GENERAL_SystemResetCallback(void) {
	/* NOTE: This function should not be modified, when the callback is needed,
            the TM_GENERAL_SystemResetCallback could be implemented in the user file
	*/
}
/**
 * @defgroup BUFF_Macros
 * @brief    Library defines
 * @{
 */
#if defined(USE_HAL_DRIVER)
#include "stm32fxxx_hal.h"
#endif
#define BUFFER_INITIALIZED     0x01 /*!< Buffer initialized flag */
#define BUFFER_MALLOC          0x02 /*!< Buffer uses malloc for memory */
/* Custom allocation and free functions if needed */
#ifndef LIB_ALLOC_FUNC
#define LIB_ALLOC_FUNC         malloc
#endif
#ifndef LIB_FREE_FUNC
#define LIB_FREE_FUNC          free
#endif
/**
 * @brief  Buffer structure
 */
typedef struct _BUFF_t {
	uint16_t Size;           /*!< Size of buffer in units of bytes, DO NOT MOVE OFFSET, 0 */
	uint16_t In;             /*!< Input pointer to save next value, DO NOT MOVE OFFSET, 1 */
	uint16_t Out;            /*!< Output pointer to read next value, DO NOT MOVE OFFSET, 2 */
	uint8_t* Buffer;         /*!< Pointer to buffer data array, DO NOT MOVE OFFSET, 3 */
	uint8_t Flags;           /*!< Flags for buffer, DO NOT MOVE OFFSET, 4 */
	uint8_t StringDelimiter; /*!< Character for string delimiter when reading from buffer as string, DO NOT MOVE OFFSET, 5 */
	void* UserParameters;    /*!< Pointer to user value if needed */
} BUFF_t;
/**
 * @brief  Initializes buffer structure for work
 * @param  *Buffer: Pointer to @ref BUFF_t structure to initialize
 * @param  Size: Size of buffer in units of bytes
 * @param  *BufferPtr: Pointer to array for buffer storage. Its length should be equal to @param Size parameter.
 *           If NULL is passed as parameter, @ref malloc will be used to allocate memory on heap.
 * @retval Buffer initialization status:
 *            - 0: Buffer initialized OK
 *            - > 0: Buffer initialization error. Malloc has failed with allocation
 */
uint8_t BUFF_Init(BUFF_t* Buffer, uint16_t Size, uint8_t* BufferPtr);
uint8_t BUFF_Init(BUFF_t* Buffer, uint16_t Size, uint8_t* BufferPtr) {
	/* Set buffer values to all zeros */
	memset(Buffer, 0, sizeof(BUFF_t));
	
	/* Set default values */
	Buffer->Size = Size;
	Buffer->Buffer = BufferPtr;
	Buffer->StringDelimiter = '\n';
	
	/* Check if malloc should be used */
	if (!Buffer->Buffer) {
		/* Try to allocate */
		Buffer->Buffer = (uint8_t *) LIB_ALLOC_FUNC(Size * sizeof(uint8_t));
		
		/* Check if allocated */
		if (!Buffer->Buffer) {
			/* Reset size */
			Buffer->Size = 0;
			
			/* Return error */
			return 1;
		} else {
			/* Set flag for malloc */
			Buffer->Flags |= BUFFER_MALLOC;
		}
	}
	
	/* We are initialized */
	Buffer->Flags |= BUFFER_INITIALIZED;
	
	/* Initialized OK */
	return 0;
}
/**
 * @brief  Free memory for buffer allocated using @ref malloc
 * @note   This function has sense only if malloc was used for dynamic allocation
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @retval None
 */
void BUFF_Free(BUFF_t* Buffer);
void BUFF_Free(BUFF_t* Buffer) {
	/* Check buffer structure */
	if (Buffer == NULL) {
		return;
	}
	
	/* If malloc was used for allocation */
	if (Buffer->Flags & BUFFER_MALLOC) {
		/* Free memory */
		LIB_FREE_FUNC(Buffer->Buffer);
	}
	
	/* Clear flags */
	Buffer->Flags = 0;
	Buffer->Size = 0;
}
/**
 * @brief  Writes data to buffer
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  *Data: Pointer to data to be written
 * @param  count: Number of elements of type unsigned char to write
 * @retval Number of elements written in buffer 
 */
uint16_t BUFF_Write(BUFF_t* Buffer, const uint8_t* Data, uint16_t count);
uint16_t BUFF_Write(BUFF_t* Buffer, const uint8_t* Data, uint16_t count) {
	uint8_t i = 0;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}

	/* Check input pointer */
	if (Buffer->In >= Buffer->Size) {
		Buffer->In = 0;
	}
	
	/* Go through all elements */
	while (count--) {
		/* Check if buffer full */
		if (
			(Buffer->In == (Buffer->Out - 1)) ||
			(Buffer->Out == 0 && Buffer->In == (Buffer->Size - 1))
		) {
			break;
		}
		
		/* Add to buffer */
		Buffer->Buffer[Buffer->In++] = *Data++;
		
		/* Increase pointers */
		i++;
		
		/* Check input overflow */
		if (Buffer->In >= Buffer->Size) {
			Buffer->In = 0;
		}
	}
	
	/* Return number of elements stored in memory */
	return i;
}
/**
 * @brief  Reads data from buffer
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  *Data: Pointer to data where read values will be stored
 * @param  count: Number of elements of type unsigned char to read
 * @retval Number of elements read from buffer 
 */
uint16_t BUFF_Read(BUFF_t* Buffer, uint8_t* Data, uint16_t count);
uint16_t BUFF_Read(BUFF_t* Buffer, uint8_t* Data, uint16_t count) {
	uint16_t i = 0;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}

	/* Check output pointer */
	if (Buffer->Out >= Buffer->Size) {
		Buffer->Out = 0;
	}
	
	/* Go through all elements */
	while (count--) {
		/* Check if pointers are same = buffer is empty */
		if (Buffer->Out == Buffer->In) {
			break;
		}
		
		/* Save to user buffer */
		*Data++ = Buffer->Buffer[Buffer->Out++];
		
		/* Increase pointers */
		i++;

		/* Check output overflow */
		if (Buffer->Out >= Buffer->Size) {
			Buffer->Out = 0;
		}
	}
	
	/* Return number of elements read from buffer */
	return i;
}
/**
 * @brief  Gets number of free elements in buffer 
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @retval Number of free elements in buffer
 */
uint16_t BUFF_GetFree(BUFF_t* Buffer);
uint16_t BUFF_GetFree(BUFF_t* Buffer) {
	uint32_t size, in, out;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Save values */
	in = Buffer->In;
	out = Buffer->Out;
	
	/* Check if the same */
	if (in == out) {
		size = Buffer->Size;
	}

	/* Check normal mode */
	if (out > in) {
		size = out - in;
	}
	
	/* Check if overflow mode */
	if (in > out) {
		size = Buffer->Size - (in - out);
	}
	
	/* Return free memory */
	return size - 1;
}
/**
 * @brief  Gets number of elements in buffer 
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @retval Number of elements in buffer
 */
uint16_t BUFF_GetFull(BUFF_t* Buffer);
uint16_t BUFF_GetFull(BUFF_t* Buffer) {
	uint32_t in, out, size;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Save values */
	in = Buffer->In;
	out = Buffer->Out;
	
	/* Pointer are same? */
	if (in == out) {
		size = 0;
	}
	
	/* Check pointers and return values */
	/* Buffer is not in overflow mode */
	if (in > out) {
		size = in - out;
	}
	
	/* Buffer is in overflow mode */
	if (out > in) {
		size = Buffer->Size - (out - in);
	}
	
	/* Return number of elements in buffer */
	return size;
}
/**
 * @brief  Resets (clears) buffer pointers
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @retval None
 */
void BUFF_Reset(BUFF_t* Buffer);
void BUFF_Reset(BUFF_t* Buffer) {
	/* Check buffer structure */
	if (Buffer == NULL) {
		return;
	}
	
	/* Reset values */
	Buffer->In = 0;
	Buffer->Out = 0;
}
/**
 * @brief  Checks if specific element value is stored in buffer
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  uint8_t Element: Element to check
 * @retval Status of element:
 *            -  < 0: Element was not found
 *            - >= 0: Element found, location in buffer is returned
 *                   Ex: If value 1 is returned, it means 1 read from buffer and your element will be returned
 */
int16_t BUFF_FindElement(BUFF_t* Buffer, uint8_t Element);
int16_t BUFF_FindElement(BUFF_t* Buffer, uint8_t Element) {
	uint16_t Num, Out, retval = 0;
	
	/* Check buffer structure */
	if (Buffer == NULL) {
		return -1;
	}
	
	/* Create temporary variables */
	Num = BUFF_GetFull(Buffer);
	Out = Buffer->Out;
	
	/* Go through input elements */
	while (Num > 0) {
		/* Check output overflow */
		if (Out >= Buffer->Size) {
			Out = 0;
		}
		
		/* Check for element */
		if ((uint8_t)Buffer->Buffer[Out] == (uint8_t)Element) {
			/* Element found, return position in buffer */
			return retval;
		}
		
		/* Set new variables */
		Out++;
		Num--;
		retval++;
	}
	
	/* Element is not in buffer */
	return -1;
}
/**
 * @brief  Checks if specific data sequence are stored in buffer
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  uint8_t* Data: Array with data sequence
 * @param  Size: Data size in units of bytes
 * @retval Status of sequence:
 *            -  < 0: Sequence was not found
 *            - >= 0: Sequence found, start sequence location in buffer is returned
 */
int16_t BUFF_Find(BUFF_t* Buffer, const uint8_t* Data, uint16_t Size);
int16_t BUFF_Find(BUFF_t* Buffer, const uint8_t* Data, uint16_t Size) {
	uint16_t Num, Out, i, retval = 0;
	uint8_t found = 0;

	/* Check buffer structure and number of elements in buffer */
	if (Buffer == NULL || (Num = BUFF_GetFull(Buffer)) < Size) {
		return -1;
	}

	/* Create temporary variables */
	Out = Buffer->Out;

	/* Go through input elements in buffer */
	while (Num > 0) {
		/* Check output overflow */
		if (Out >= Buffer->Size) {
			Out = 0;
		}

		/* Check if current element in buffer matches first element in data array */
		if ((uint8_t)Buffer->Buffer[Out] == (uint8_t)Data[0]) {
			found = 1;
		}

		/* Set new variables */
		Out++;
		Num--;
		retval++;

		/* We have found first element */
		if (found) {
			/* First character found */
			/* Check others */
			i = 1;
			while (i < Size) {
				/* Check output overflow */
				if (Out >= Buffer->Size) {
					Out = 0;
				}

				/* Check if current character in buffer matches character in string */
				if ((uint8_t)Buffer->Buffer[Out] != (uint8_t)Data[i]) {
					retval += i - 1;
					break;
				}

				/* Set new variables */
				Out++;
				Num--;
				i++;
			}

			/* We have found data sequence in buffer */
			if (i == Size) {
				return retval;
			}
		}
	}

	/* Data sequence is not in buffer */
	return -1;
}
/**
 * @brief  Sets string delimiter character when reading from buffer as string
 * @param  Buffer: Pointer to @ref BUFF_t structure
 * @param  StringDelimIter: Character as string delimiter
 * @retval None
 */
#define BUFF_SetStringDelimiter(Buffer, StrDel)  ((Buffer)->StringDelimiter = (StrDel))
/**
 * @brief  Writes string formatted data to buffer
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  *buff: Pointer to string to write 
 * @retval Number of characters written
 */
uint16_t BUFF_WriteString(BUFF_t* Buffer, const char* buff);
uint16_t BUFF_WriteString(BUFF_t* Buffer, const char* buff) {
	/* Write string to buffer */
	return BUFF_Write(Buffer, (uint8_t *)buff, strlen(buff));
}
/**
 * @brief  Reads from buffer as string
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  *buff: Pointer to buffer where string will be stored
 * @param  buffsize: Buffer size in units of bytes
 * @retval Number of characters in string
 */
uint16_t BUFF_ReadString(BUFF_t* Buffer, char* buff, uint16_t buffsize);
uint16_t BUFF_ReadString(BUFF_t* Buffer, char* buff, uint16_t buffsize) {
	uint16_t i = 0;
	uint8_t ch;
	uint16_t memFree, memFull;
	
	/* Check value buffer */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Get free */
	memFree = BUFF_GetFree(Buffer);
	memFull = BUFF_GetFull(Buffer);
	
	/* Check for any data on USART */
	if (
		memFull == 0 ||                                                   /*!< Buffer empty */
		(
			BUFF_FindElement(Buffer, Buffer->StringDelimiter) < 0 && /*!< String delimiter is not in buffer */
			memFree != 0 &&                                               /*!< Buffer is not full */
			memFull < buffsize                                            /*!< User buffer size is larger than number of elements in buffer */
		)
	) {
		/* Return 0 */
		return 0;
	}
	
	/* If available buffer size is more than 0 characters */
	while (i < (buffsize - 1)) {
		/* We have available data */
		if (BUFF_Read(Buffer, &ch, 1) == 0) {
			break;
		}
		
		/* Save character */
		buff[i] = (char)ch;
		
		/* Check for end of string */
		if ((char)buff[i] == (char)Buffer->StringDelimiter) {
			/* Done */
			break;
		}
		
		/* Increase */
		i++;
	}
	
	/* Add zero to the end of string */
	if (i == (buffsize - 1)) {
		buff[i] = 0;
	} else {
		buff[++i] = 0;
	}

	/* Return number of characters in buffer */
	return i;
}
/**
 * @brief  Checks if character exists in location in buffer
 * @param  *Buffer: Pointer to @ref BUFF_t structure
 * @param  pos: Position in buffer, starting from 0
 * @param  *element: Pointer to save value at desired position to be stored into
 * @retval Check status:
 *            - 0: Buffer is not so long as position desired
 *            - > 0: Position to check was inside buffer data size
 */
int8_t BUFF_CheckElement(BUFF_t* Buffer, uint16_t pos, uint8_t* element);
int8_t BUFF_CheckElement(BUFF_t* Buffer, uint16_t pos, uint8_t* element) {
	uint16_t In, Out, i = 0;
	
	/* Check value buffer */
	if (Buffer == NULL) {
		return 0;
	}
	
	/* Read current values */
	In = Buffer->In;
	Out = Buffer->Out;
	
	/* Set pointers to right location */
	while (i < pos && (In != Out)) {
		/* Increase output pointer */
		Out++;
		i++;
		
		/* Check overflow */
		if (Out >= Buffer->Size) {
			Out = 0;
		}
	}
	
	/* If positions match */
	if (i == pos) {
		/* Save element */
		*element = Buffer->Buffer[Out];
		
		/* Return OK */
		return 1;
	}
	
	/* Return zero */
	return 0;
}
//
// public methods
//
bool TinyGPSPlus::encode(char c)
{
  ++encodedCharCount;

  switch(c)
  {
  case ',': // term terminators
    parity ^= (uint8_t)c;
  case '\r':
  case '\n':
  case '*':
    {
      bool isValidSentence = false;
      if (curTermOffset < sizeof(term))
      {
        term[curTermOffset] = 0;
        isValidSentence = endOfTermHandler();
      }
      ++curTermNumber;
      curTermOffset = 0;
      isChecksumTerm = c == '*';
      return isValidSentence;
    }
    break;

  case '$': // sentence begin
    curTermNumber = curTermOffset = 0;
    parity = 0;
    curSentenceType = GPS_SENTENCE_OTHER;
    isChecksumTerm = false;
    sentenceHasFix = false;
    return false;

  default: // ordinary characters
    if (curTermOffset < sizeof(term) - 1)
      term[curTermOffset++] = c;
    if (!isChecksumTerm)
      parity ^= c;
    return false;
  }

  return false;
}
//
// internal utilities
//
int TinyGPSPlus::fromHex(char a)
{
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}
// static
// Parse a (potentially negative) number with up to 2 decimal digits -xxxx.yy
int32_t TinyGPSPlus::parseDecimal(const char *term)
{
  bool negative = *term == '-';
  if (negative) ++term;
  int32_t ret = 100 * (int32_t)atol(term);
  while (isdigit(*term)) ++term;
  if (*term == '.' && isdigit(term[1]))
  {
    ret += 10 * (term[1] - '0');
    if (isdigit(term[2]))
      ret += term[2] - '0';
  }
  return negative ? -ret : ret;
}
// static
// Parse degrees in that funny NMEA format DDMM.MMMM
void TinyGPSPlus::parseDegrees(const char *term, RawDegrees &deg)
{
  uint32_t leftOfDecimal = (uint32_t)atol(term);
  uint16_t minutes = (uint16_t)(leftOfDecimal % 100);
  uint32_t multiplier = 10000000UL;
  uint32_t tenMillionthsOfMinutes = minutes * multiplier;

  deg.deg = (int16_t)(leftOfDecimal / 100);

  while (isdigit(*term))
    ++term;

  if (*term == '.')
    while (isdigit(*++term))
    {
      multiplier /= 10;
      tenMillionthsOfMinutes += (*term - '0') * multiplier;
    }

  deg.billionths = (5 * tenMillionthsOfMinutes + 1) / 3;
  deg.negative = false;
}
#define COMBINE(sentence_type, term_number) (((unsigned)(sentence_type) << 5) | term_number)
// Processes a just-completed term
// Returns true if new sentence has just passed checksum test and is validated
bool TinyGPSPlus::endOfTermHandler()
{
  // If it's the checksum term, and the checksum checks out, commit
  if (isChecksumTerm)
  {
    byte checksum = 16 * fromHex(term[0]) + fromHex(term[1]);
    if (checksum == parity)
    {
      passedChecksumCount++;
      if (sentenceHasFix)
        ++sentencesWithFixCount;

      switch(curSentenceType)
      {
      case GPS_SENTENCE_GPRMC:
        date.commit();
        time.commit();
        if (sentenceHasFix)
        {
           location.commit();
           speed.commit();
           course.commit();
        }
        break;
      case GPS_SENTENCE_GPGGA:
        time.commit();
        if (sentenceHasFix)
        {
          location.commit();
          altitude.commit();
        }
        satellites.commit();
        hdop.commit();
        break;
      }

      // Commit all custom listeners of this sentence type
      for (TinyGPSCustom *p = customCandidates; p != NULL && strcmp(p->sentenceName, customCandidates->sentenceName) == 0; p = p->next)
         p->commit();
      return true;
    }

    else
    {
      ++failedChecksumCount;
    }

    return false;
  }

  // the first term determines the sentence type
  if (curTermNumber == 0)
  {
    if (!strcmp(term, _GPRMCterm))
      curSentenceType = GPS_SENTENCE_GPRMC;
    else if (!strcmp(term, _GPGGAterm))
      curSentenceType = GPS_SENTENCE_GPGGA;
    else
      curSentenceType = GPS_SENTENCE_OTHER;

    // Any custom candidates of this sentence type?
    for (customCandidates = customElts; customCandidates != NULL && strcmp(customCandidates->sentenceName, term) < 0; customCandidates = customCandidates->next);
    if (customCandidates != NULL && strcmp(customCandidates->sentenceName, term) > 0)
       customCandidates = NULL;

    return false;
  }

  if (curSentenceType != GPS_SENTENCE_OTHER && term[0])
    switch(COMBINE(curSentenceType, curTermNumber))
  {
    case COMBINE(GPS_SENTENCE_GPRMC, 1): // Time in both sentences
    case COMBINE(GPS_SENTENCE_GPGGA, 1):
      time.setTime(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 2): // GPRMC validity
      sentenceHasFix = term[0] == 'A';
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 3): // Latitude
    case COMBINE(GPS_SENTENCE_GPGGA, 2):
      location.setLatitude(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 4): // N/S
    case COMBINE(GPS_SENTENCE_GPGGA, 3):
      location.rawNewLatData.negative = term[0] == 'S';
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 5): // Longitude
    case COMBINE(GPS_SENTENCE_GPGGA, 4):
      location.setLongitude(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 6): // E/W
    case COMBINE(GPS_SENTENCE_GPGGA, 5):
      location.rawNewLngData.negative = term[0] == 'W';
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 7): // Speed (GPRMC)
      speed.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 8): // Course (GPRMC)
      course.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPRMC, 9): // Date (GPRMC)
      date.setDate(term);
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 6): // Fix data (GPGGA)
      sentenceHasFix = term[0] > '0';
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 7): // Satellites used (GPGGA)
      satellites.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 8): // HDOP
      hdop.set(term);
      break;
    case COMBINE(GPS_SENTENCE_GPGGA, 9): // Altitude (GPGGA)
      altitude.set(term);
      break;
  }

  // Set custom values as needed
  for (TinyGPSCustom *p = customCandidates; p != NULL && strcmp(p->sentenceName, customCandidates->sentenceName) == 0 && p->termNumber <= curTermNumber; p = p->next)
    if (p->termNumber == curTermNumber)
         p->set(term);

  return false;
}
/* static */
double TinyGPSPlus::distanceBetween(double lat1, double long1, double lat2, double long2)
{
  // returns distance in meters between two positions, both specified
  // as signed decimal-degrees latitude and longitude. Uses great-circle
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers
  double delta = radians(long1-long2);
  double sdlong = sin(delta);
  double cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double slat1 = sin(lat1);
  double clat1 = cos(lat1);
  double slat2 = sin(lat2);
  double clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = sq(delta);
  delta += sq(clat2 * sdlong);
  delta = sqrt(delta);
  double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2(delta, denom);
  return delta * 6372795;
}
double TinyGPSPlus::courseTo(double lat1, double long1, double lat2, double long2)
{
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  // both specified as signed decimal-degrees latitude and longitude.
  // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
  // Courtesy of Maarten Lamers
  double dlon = radians(long2-long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double a1 = sin(dlon) * cos(lat2);
  double a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  return degrees(a2);
}
const char *TinyGPSPlus::cardinal(double course)
{
  static const char* directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
  int direction = (int)((course + 11.25f) / 22.5f);
  return directions[direction % 16];
}
void TinyGPSLocation::commit()
{
   rawLatData = rawNewLatData;
   rawLngData = rawNewLngData;
   lastCommitTime = millis();
   valid = updated = true;
}
void TinyGPSLocation::setLatitude(const char *term)
{
   TinyGPSPlus::parseDegrees(term, rawNewLatData);
}
void TinyGPSLocation::setLongitude(const char *term)
{
   TinyGPSPlus::parseDegrees(term, rawNewLngData);
}
double TinyGPSLocation::lat()
{
   updated = false;
   double ret = rawLatData.deg + rawLatData.billionths / 1000000000.0;
   return rawLatData.negative ? -ret : ret;
}
double TinyGPSLocation::lng()
{
   updated = false;
   double ret = rawLngData.deg + rawLngData.billionths / 1000000000.0;
   return rawLngData.negative ? -ret : ret;
}
void TinyGPSDate::commit()
{
   date = newDate;
   lastCommitTime = millis();
   valid = updated = true;
}
void TinyGPSTime::commit()
{
   time = newTime;
   lastCommitTime = millis();
   valid = updated = true;
}
void TinyGPSTime::setTime(const char *term)
{
   newTime = (uint32_t)TinyGPSPlus::parseDecimal(term);
}
void TinyGPSDate::setDate(const char *term)
{
   newDate = atol(term);
}
uint16_t TinyGPSDate::year()
{
   updated = false;
   uint16_t year = date % 100;
   return year + 2000;
}
uint8_t TinyGPSDate::month()
{
   updated = false;
   return (date / 100) % 100;
}
uint8_t TinyGPSDate::day()
{
   updated = false;
   return date / 10000;
}
uint8_t TinyGPSTime::hour()
{
   updated = false;
   return time / 1000000;
}
uint8_t TinyGPSTime::minute()
{
   updated = false;
   return (time / 10000) % 100;
}
uint8_t TinyGPSTime::second()
{
   updated = false;
   return (time / 100) % 100;
}
uint8_t TinyGPSTime::centisecond()
{
   updated = false;
   return time % 100;
}
void TinyGPSDecimal::commit()
{
   val = newval;
   lastCommitTime = millis();
   valid = updated = true;
}
void TinyGPSDecimal::set(const char *term)
{
   newval = TinyGPSPlus::parseDecimal(term);
}
void TinyGPSInteger::commit()
{
   val = newval;
   lastCommitTime = millis();
   valid = updated = true;
}
void TinyGPSInteger::set(const char *term)
{
   newval = atol(term);
}
TinyGPSCustom::TinyGPSCustom(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber)
{
   begin(gps, _sentenceName, _termNumber);
}
void TinyGPSCustom::begin(TinyGPSPlus &gps, const char *_sentenceName, int _termNumber)
{
   lastCommitTime = 0;
   updated = valid = false;
   sentenceName = _sentenceName;
   termNumber = _termNumber;
   memset(stagingBuffer, '\0', sizeof(stagingBuffer));
   memset(buffer, '\0', sizeof(buffer));

   // Insert this item into the GPS tree
   gps.insertCustom(this, _sentenceName, _termNumber);
}
void TinyGPSCustom::commit()
{
   strcpy(this->buffer, this->stagingBuffer);
   lastCommitTime = millis();
   valid = updated = true;
}
void TinyGPSCustom::set(const char *term)
{
   strncpy(this->stagingBuffer, term, sizeof(this->stagingBuffer));
}
void TinyGPSPlus::insertCustom(TinyGPSCustom *pElt, const char *sentenceName, int termNumber)
{
   TinyGPSCustom **ppelt;

   for (ppelt = &this->customElts; *ppelt != NULL; ppelt = &(*ppelt)->next)
   {
      int cmp = strcmp(sentenceName, (*ppelt)->sentenceName);
      if (cmp < 0 || (cmp == 0 && termNumber < (*ppelt)->termNumber))
         break;
   }

   pElt->next = *ppelt;
   *ppelt = pElt;
}

/**
 * \brief         Time structure
 */
typedef struct _GSM_Date_t {
    uint8_t Day;                                            /*!< Day in month, from 1 to 31 */
    uint8_t Month;                                          /*!< Month in a year, from 1 to 12 */
    uint16_t Year;                                          /*!< Year with included thousands */
} GSM_Date_t;

/**
 * \brief         Time structure
 */
typedef struct _GSM_Time_t {
    uint8_t Hours;                                          /*!< Hours in format 00-24 */
    uint8_t Minutes;                                        /*!< Minutes */
    uint8_t Seconds;                                        /*!< Seconds */
} GSM_Time_t;

/**
 * \brief         Date and time structure
 */
typedef struct _GSM_DateTime_t {
    GSM_Date_t Date;                                        /*!< Date data */
    GSM_Time_t Time;                                        /*!< Time data */
} GSM_DateTime_t;
/* Check if needle exists in haystack memory */
//gstatic
void* mem_mem(void* haystack, size_t haystacksize, void* needle, size_t needlesize) {
    unsigned char* hptr = (unsigned char *)haystack;
    unsigned char* nptr = (unsigned char *)needle;
    unsigned int i;

    if (needlesize > haystacksize) {                		/* Check sizes */
        return 0;                                   		/* Needle is greater than haystack = nothing in memory */
    }
    if (haystacksize == needlesize) {                		/* Check if same length */
        if (memcmp(hptr, nptr, needlesize) == 0) {
            return hptr;
        }
        return 0;
    }
    haystacksize -= needlesize;                        		/* Set haystack size pointers */
    for (i = 0; i < haystacksize; i++) {            		/* Go through entire memory */
        if (memcmp(&hptr[i], nptr, needlesize) == 0) {      /* Check memory match */
            return &hptr[i];
        }
    }
    return 0;
}

/* Parses and returns number from string */
gstatic
int32_t ParseNumber(const char* ptr, uint8_t* cnt) {
    uint8_t minus = 0, i = 0;
    int32_t sum = 0;
    
    if (*ptr == '-') {                                      /* Check for minus character */
        minus = 1;
        ptr++;
        i++;
    }
    while (CHARISNUM(*ptr)) {                               /* Parse number */
        sum = 10 * sum + CHARTONUM(*ptr);
        ptr++;
        i++;
    }
    if (cnt != NULL) {                                      /* Save number of characters used for number */
        *cnt = i;
    }
    if (minus) {                                            /* Minus detected */
        return 0 - sum;
    }
    return sum;                                             /* Return number */
}

/* Parse float number */
static
float ParseFloatNumber(const char* ptr, uint8_t* cnt) {
    uint8_t i = 0, j = 0;
    float sum = 0.0f;

    sum = (float)ParseNumber(ptr, &i);                      /* Parse number */
    j += i;
    ptr += i;
    if (*ptr == '.') {                                      /* Check decimals */
        float dec;
        dec = (float)ParseNumber(ptr + 1, &i);
        dec /= (float)pow(10, i);
        if (sum >= 0) {
            sum += dec;
        } else {
            sum -= dec;
        }
        j += i + 1;
    }

    if (cnt != NULL) {                                      /* Save number of characters used for number */
        *cnt = j;
    }
    return sum;                                             /* Return number */
}

/* Parses date from string and stores to date structure */
void ParseDATE(gvol GSM_t* GSM, GSM_Date_t* DateStr, const char* str) {
    uint8_t len = 0, i;
    
    DateStr->Year = 2000 + ParseNumber(&str[0], &i);
    len += i + 1;
    DateStr->Month = ParseNumber(&str[len], &i);
    len += i + 1;
    DateStr->Day = ParseNumber(&str[len], &i);
}

/* Parses time from string and stores to time structure */
void ParseTIME(gvol GSM_t* GSM, GSM_Time_t* TimeStr, const char* str) {
    uint8_t len = 0, i;
    
    TimeStr->Hours = ParseNumber(&str[0], &i);
    len += i + 1;
    TimeStr->Minutes = ParseNumber(&str[len], &i);
    len += i + 1;
    TimeStr->Seconds = ParseNumber(&str[len], &i);
}


