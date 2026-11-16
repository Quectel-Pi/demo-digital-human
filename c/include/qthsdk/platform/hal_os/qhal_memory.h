/**
 * @file qhal_memory.h
 * @version 0.0.1
 * @date 2023.03.02
 * @brief Memory allocation HAL abstraction.
 * @details The memory APIs provide a unified heap interface for the SDK.
 *
 * @par API Goals
 * - `Qhal_mem_malloc()`
 * - `Qhal_mem_free()`
 * - `Qhal_mem_realloc()`
 * - `Qhal_mem_getFreeHeapSize()`
 *
 * @par Adaptation Notes
 * - In most cases, directly mapping these APIs to the platform heap manager is
 *   sufficient.
 * - If the platform heap is small, keeping the free-heap query is recommended
 *   for debugging and capacity evaluation.
 *
 *
 * @note All APIs in this file are optional for platform adaptation.
 * @copyright Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 */

#ifndef __QHAL_MEMORY_H__
#define __QHAL_MEMORY_H__

#include "qhal_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Reallocate a memory block.
     * @param[in] ptr Previously allocated pointer, or NULL.
     * @param[in] size New size in bytes.
     * @return New memory pointer on success, NULL on failure.
     * @note Exact behavior for `ptr == NULL` or `size == 0` follows the platform allocator.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     * 
     * @par Platform Adaptation Example
     * @code{.c}
     * void *Qhal_mem_realloc(void *ptr, quint32_t size)
     * {
     *     return chip_realloc(ptr, size);
     * }
     * @endcode
     */
    void *Qhal_mem_realloc(void *ptr, quint32_t size);

    /**
     * @brief Allocate memory.
     * @param[in] size Requested size in bytes.
     * @return Allocated pointer on success, NULL on failure.
     * @note Memory content is unspecified unless the platform allocator guarantees otherwise.
     * @note Directly mapping this API to the platform heap allocator is usually sufficient.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     * 
     * @par Platform Adaptation Example
     * @code{.c}
     * void *Qhal_mem_malloc(quint32_t size)
     * {
     *     return chip_malloc(size);
     * }
     * @endcode
     */
    void *Qhal_mem_malloc(quint32_t size);

    /**
     * @brief Free allocated memory.
     * @param[in] ptr Memory pointer returned by the HAL allocator.
     * @note Behavior for a NULL pointer depends on the platform allocator implementation.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     * 
     * @par Platform Adaptation Example
     * @code{.c}
     * void Qhal_mem_free(void *ptr)
     * {
     *     chip_free(ptr);
     * }
     * @endcode
     */
    void Qhal_mem_free(void *ptr);

    /**
     * @brief Get the current free heap size.
     * @return Free heap size in bytes.
     * @note This value is a runtime snapshot and may be approximate on some platforms.
     * @note When the platform heap is small, keeping this interface available is strongly recommended.
     * @note LTE_AT(required)
     * @note LTE_Open(required)
     * @note WIFI_HEX(required)
     * @note WIFI_Open(required)
     * @note BLE_HEX(required)
     * @note BLE_Open(required)
     * 
     * @par Platform Adaptation Example
     * @code{.c}
     * quint32_t Qhal_mem_getFreeHeapSize(void)
     * {
     *     return chip_get_free_heap_size();
     * }
     * @endcode
     */
    quint32_t Qhal_mem_getFreeHeapSize(void);

#ifdef __cplusplus
}
#endif

#endif /* __QHAL_MEMORY_H__ */
