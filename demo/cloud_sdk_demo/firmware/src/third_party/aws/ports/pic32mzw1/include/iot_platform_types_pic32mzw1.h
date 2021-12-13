/*
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iot_platform_types_pic32mzw1.h
 * @brief Template definitions of platform layer types.
 */

#ifndef IOT_PLATFORM_TYPES_PIC32MZW1_H_
#define IOT_PLATFORM_TYPES_PIC32MZW1_H_

#include "timers.h"

#define IOT_THREAD_DEFAULT_PRIORITY      0
#define IOT_THREAD_DEFAULT_STACK_SIZE configMINIMAL_STACK_SIZE

typedef struct {
    uint16_t  u16Year;
    uint8_t   u8Month;
    uint8_t   u8Day;
    uint8_t   u8Hour;
    uint8_t   u8Minute;
    uint8_t   u8Second;
    uint8_t   __PAD8__;
} tstrSystemTime;


/**
 * @brief Set this to the target system's mutex type.
 */
typedef struct iot_mutex_internal
{
    SemaphoreHandle_t xMutex; /**< FreeRTOS mutex. */
    BaseType_t isRecursive;     /**< Type; used for indicating if this is reentrant or normal. */
} iot_mutex_internal_t;

typedef iot_mutex_internal_t  _IotSystemMutex_t;

/**
 * @brief Set this to the target system's semaphore type.
 */
typedef SemaphoreHandle_t  _IotSystemSemaphore_t;

/**
 * @brief Holds information about an active timer.
 */
typedef struct timerInfo
{
    TimerHandle_t timer;                /**< @brief Underlying timer. */
    void ( * threadRoutine )( void * ); /**< @brief Thread function to run on timer expiration. */
    void * pArgument;                   /**< @brief First argument to threadRoutine. */
    TickType_t xTimerPeriod;            /**< Period of this timer. */
} timerInfo_t;

/**
 * @brief Set this to the target system's timer type.
 */
typedef timerInfo_t   _IotSystemTimer_t;

/**
 * @brief Thread routine function.
 *
 * @param[in] void * The argument passed to the @ref
 * platform_threads_function_createdetachedthread. For application use.
 */
typedef void ( * IotThreadRoutine_t )( _IotSystemTimer_t xTimer);

/**
 * @cond DOXYGEN_IGNORE
 * Doxygen should ignore this section.
 *
 * Forward declarations of the network types.
 */
struct IotNetworkServerInfo;
struct IotNetworkCredentials;
struct _networkConnection;
/** @endcond */

/**
 * @brief The format for remote server host and port on this system.
 */
typedef struct IotNetworkServerInfo * _IotNetworkServerInfo_t;

/**
 * @brief The format for network credentials on this system.
 */
typedef struct IotNetworkCredentials * _IotNetworkCredentials_t;

/**
 * @brief The handle of a network connection on this system.
 */
typedef struct _networkConnection * _IotNetworkConnection_t;

#endif /* ifndef IOT_PLATFORM_TYPES_PIC32MZW1_H_ */
