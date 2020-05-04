/***************************************************************************//**
* \file cy_pra.h
* \version 1.0
*
* Provides the API declarations of the PSoC 6 Protection Access
* Driver.
*
********************************************************************************
* \copyright
* Copyright 2020 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/


/**
* \addtogroup group_pra
* \{
* The Protected Register Access (PRA) driver used to provice access to the
* protected registers to non-secure application on PSoC64 devices.
*
* \section group_pra_more_information More Information
* See the device technical reference manual (TRM).
*
* \section group_pra_MISRA MISRA-C Compliance
* The PRA driver does not have any specific deviations.
*
* \section group_pra_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_pra_macros                Macros
* \defgroup group_pra_functions             Functions
* \defgroup group_pra_enums                 Enumerated Types
* \defgroup group_pra_data_structures_cfg   Data Structures
*/

#if !defined(CY_PRA_H)
#define CY_PRA_H

#include <stdint.h>
#include <stdbool.h>
#include "cy_systick.h"
#include "cy_ble_clk.h"
#include "cy_device_headers.h"

#if defined(CY_DEVICE_SECURE)

#ifdef __cplusplus
extern "C" {
#endif

/***************************************
*        Constants
***************************************/

/** \cond INTERNAL */

#define CY_PRA_MSG_TYPE_REG32_GET        (1U)
#define CY_PRA_MSG_TYPE_REG32_CLR_SET    (2U)
#define CY_PRA_MSG_TYPE_REG32_SET        (3U)
#define CY_PRA_MSG_TYPE_CM0_WAKEUP       (4U)
#define CY_PRA_MSG_TYPE_SYS_CFG_FUNC     (5U)
#define CY_PRA_MSG_TYPE_SECURE_ONLY      (6U)
#define CY_PRA_MSG_TYPE_FUNC_POLICY      (7U)


/* IPC */
#define CY_PRA_IPC_NOTIFY_INTR          (0x1UL << CY_IPC_INTR_PRA)

/* Registers Index */
#define CY_PRA_INDX_SRSS_PWR_LVD_CTL            (0U)
#define CY_PRA_INDX_SRSS_SRSS_INTR              (1U)
#define CY_PRA_INDX_SRSS_SRSS_INTR_SET          (2U)
#define CY_PRA_INDX_SRSS_SRSS_INTR_MASK         (3U)
#define CY_PRA_INDX_SRSS_SRSS_INTR_CFG          (4U)
#define CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_1      (5U)
/* Do not change index below as it is used in flash loaders */
#define CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_2      (6U)
#define CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_3      (7U)
#define CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_4      (8U)
#define CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_5      (9U)
#define CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_6      (10U)
#define CY_PRA_INDX_FLASHC_FLASH_CMD            (11U)
#define CY_PRA_INDX_CPUSS_CM4_PWR_CTL           (12U)
#define CY_PRA_INDX_SRSS_PWR_CTL                (13U)
#define CY_PRA_INDX_SRSS_PWR_HIBERNATE          (14U)

/* Functions Index */
#define CY_PRA_INDX_INIT_CYCFG_DEVICE       	(0U)
#define CY_PRA_INDX_PM_HIBERNATE            	(1U)
#define CY_PRA_INDX_PM_CM4_DP_FLAG_SET      	(2U)
#define CY_PRA_INDX_FLASH_RAM_DELAY         	(3U)
#define CY_PRA_INDX_PM_LDO_SET_VOLTAGE      	(4U)
#define CY_PRA_INDX_PM_LDO_SET_MODE         	(5U)
#define CY_PRA_INDX_PM_BUCK_ENABLE          	(6U)
#define CY_PRA_CLK_FUNC_ECO_DISBALE				(7U)
#define CY_PRA_CLK_FUNC_FLL_DISABLE				(8U)
#define CY_PRA_CLK_FUNC_PLL_DISABLE				(9U)
#define CY_PRA_CLK_FUNC_ILO_ENABLE				(10U)
#define CY_PRA_CLK_FUNC_ILO_DISABLE				(11U)
#define CY_PRA_CLK_FUNC_ILO_HIBERNATE_ON		(12U)
#define CY_PRA_CLK_FUNC_PILO_ENABLE				(13U)
#define CY_PRA_CLK_FUNC_PILO_DISABLE			(14U)
#define CY_PRA_CLK_FUNC_PILO_SET_TRIM			(15U)
#define CY_PRA_CLK_FUNC_WCO_ENABLE				(16U)
#define CY_PRA_CLK_FUNC_WCO_DISABLE				(17U)
#define CY_PRA_CLK_FUNC_WCO_BYPASS				(18U)
#define CY_PRA_CLK_FUNC_HF_ENABLE				(19U)
#define CY_PRA_CLK_FUNC_HF_DISABLE				(20U)
#define CY_PRA_CLK_FUNC_HF_SET_SOURCE			(21U)
#define CY_PRA_CLK_FUNC_HF_SET_DIVIDER			(22U)
#define CY_PRA_CLK_FUNC_FAST_SET_DIVIDER		(23U)
#define CY_PRA_CLK_FUNC_PERI_SET_DIVIDER		(24U)
#define CY_PRA_CLK_FUNC_LF_SET_SOURCE			(25U)
#define CY_PRA_CLK_FUNC_TIMER_SET_SOURCE		(26U)
#define CY_PRA_CLK_FUNC_TIMER_SET_DIVIDER		(27U)
#define CY_PRA_CLK_FUNC_TIMER_ENABLE			(28U)
#define CY_PRA_CLK_FUNC_TIMER_DISABLE			(29U)
#define CY_PRA_CLK_FUNC_PUMP_SET_SOURCE			(30U)
#define CY_PRA_CLK_FUNC_PUMP_SET_DIVIDERE		(31U)
#define CY_PRA_CLK_FUNC_PUMP_ENABLE				(32U)
#define CY_PRA_CLK_FUNC_PUMP_DISABLE			(33U)
#define CY_PRA_CLK_FUNC_BAK_SET_SOURCE			(34U)
#define CY_PRA_CLK_FUNC_ECO_CONFIGURE			(35U)
#define CY_PRA_CLK_FUNC_ECO_ENABLE				(36U)
#define CY_PRA_CLK_FUNC_PATH_SET_SOURCE			(37U)
#define CY_PRA_CLK_FUNC_FLL_MANCONFIG			(38U)
#define CY_PRA_CLK_FUNC_FLL_ENABLE 				(39U)
#define CY_PRA_CLK_FUNC_PLL_MANCONFIG			(40U)
#define CY_PRA_CLK_FUNC_PLL_ENABLE				(41U)
#define CY_PRA_CLK_FUNC_SLOW_SET_DIVIDER		(42U)


/* Status */
typedef uint32_t cy_pra_status_t;
#define CY_PRA_STATUS_SUCCESS                           (0UL)
#define CY_PRA_STATUS_ACCESS_DENIED                     (0xFFFFFFFFUL)
#define CY_PRA_STATUS_INVALID_PARAM                     (0xFFFFFFFEUL)
#define CY_PRA_STATUS_ERROR_PROCESSING                  (0xFFFFFFFDUL)
#define CY_PRA_STATUS_REQUEST_SENT                      (0xFFFFFFFCUL)
/* Reserve 0xFFFFFFFDUL - 0xFFFFFFF0UL*/

#define CY_PRA_STATUS_INVALID_PARAM_ECO                 (0xFFFFFFEFUL)
#define CY_PRA_STATUS_INVALID_PARAM_EXTCLK              (0xFFFFFFEEUL)
#define CY_PRA_STATUS_INVALID_PARAM_ALTHF               (0xFFFFFFEDUL)
#define CY_PRA_STATUS_INVALID_PARAM_ILO                 (0xFFFFFFECUL)
#define CY_PRA_STATUS_INVALID_PARAM_PILO                (0xFFFFFFEBUL)
#define CY_PRA_STATUS_INVALID_PARAM_WCO                 (0xFFFFFFEAUL)
/* Reserve for other source clocks 0xFFFFFFE9UL - 0xFFFFFFE0UL */
#define CY_PRA_STATUS_INVALID_PARAM_PATHMUX0            (0xFFFFFFDFUL)
#define CY_PRA_STATUS_INVALID_PARAM_PATHMUX1            (0xFFFFFFDEUL)
#define CY_PRA_STATUS_INVALID_PARAM_PATHMUX2            (0xFFFFFFDDUL)
#define CY_PRA_STATUS_INVALID_PARAM_PATHMUX3            (0xFFFFFFDCUL)
#define CY_PRA_STATUS_INVALID_PARAM_PATHMUX4            (0xFFFFFFDBUL)
#define CY_PRA_STATUS_INVALID_PARAM_PATHMUX5            (0xFFFFFFDAUL)
/* Reserve for other path-mux 0xFFFFFFD9UL - 0xFFFFFFD0UL */
#define CY_PRA_STATUS_INVALID_PARAM_FLL0                (0xFFFFFFCFUL)
/* Reserve for other FLLs 0xFFFFFFCEUL - 0xFFFFFFC0UL */
#define CY_PRA_STATUS_INVALID_PARAM_PLL0                (0xFFFFFFBFUL)
#define CY_PRA_STATUS_INVALID_PARAM_PLL1                (0xFFFFFFBEUL)
/* Reserve for other PLLs 0xFFFFFFBDUL - 0xFFFFFFB0UL */
#define CY_PRA_STATUS_INVALID_PARAM_CLKLF               (0xFFFFFFAFUL)
/* Reserve for other clocks 0xFFFFFFAEUL - 0xFFFFFFA0UL */
#define CY_PRA_STATUS_INVALID_PARAM_CLKHF0              (0xFFFFFF9FUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKHF1              (0xFFFFFF9EUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKHF2              (0xFFFFFF9DUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKHF3              (0xFFFFFF9CUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKHF4              (0xFFFFFF9BUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKHF5              (0xFFFFFF9AUL)
/* Reserve for other HF clocks 0xFFFFFF99UL - 0xFFFFFF90UL */
#define CY_PRA_STATUS_INVALID_PARAM_CLKPUMP             (0xFFFFFF8FUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKBAK              (0xFFFFFF8EUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKFAST             (0xFFFFFF8DUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKPERI             (0xFFFFFF8CUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKSLOW             (0xFFFFFF8BUL)
#define CY_PRA_STATUS_INVALID_PARAM_SYSTICK             (0xFFFFFF8AUL)
#define CY_PRA_STATUS_INVALID_PARAM_CLKTIMER            (0xFFFFFF89UL)
/* Reserve for other HF clocks 0xFFFFFF88UL - 0xFFFFFF80UL */

#define CY_PRA_STATUS_ERROR_PROCESSING_PWR              (0xFFFFFF6FUL)
/* Reserve 0xFFFFFF6EUL - 0xFFFFFF60UL*/
#define CY_PRA_STATUS_ERROR_PROCESSING_ECO              (0xFFFFFF5FUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_EXTCLK           (0xFFFFFF5EUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_ALTHF            (0xFFFFFF5DUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_ILO              (0xFFFFFF5CUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PILO             (0xFFFFFF5BUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_WCO              (0xFFFFFF5AUL)
/* Reserve for other source clocks 0xFFFFF59UL - 0xFFFFFF50UL */
#define CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX0         (0xFFFFFF4FUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX1         (0xFFFFFF4EUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX2         (0xFFFFFF4DUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX3         (0xFFFFFF4CUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX4         (0xFFFFFF4BUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX5         (0xFFFFFF4AUL)
/* Reserve for other path-mux 0xFFFFFF49UL - 0xFFFFFF40UL */
#define CY_PRA_STATUS_ERROR_PROCESSING_FLL0             (0xFFFFFF3FUL)
/* Reserve for other FLLs 0xFFFFFF3EUL - 0xFFFFFF30UL */
#define CY_PRA_STATUS_ERROR_PROCESSING_PLL0             (0xFFFFFF2FUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_PLL1             (0xFFFFFF2EUL)
/* Reserve for other PLLs 0xFFFFFF2DUL - 0xFFFFFF20UL */
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKLF            (0xFFFFFF1FUL)
/* Reserve for other clocks 0xFFFFFF1EUL - 0xFFFFFF10UL */
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0           (0xFFFFFF0FUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKHF1           (0xFFFFFF0EUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKHF2           (0xFFFFFF0DUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKHF3           (0xFFFFFF0CUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKHF4           (0xFFFFFF0BUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKHF5           (0xFFFFFF0AUL)
/* Reserve for other HF clocks 0xFFFFFF09UL - 0xFFFFFF00UL */
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKPUMP          (0xFFFFFEFFUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKBAK           (0xFFFFFEFEUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKFAST          (0xFFFFFEFDUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKPERI          (0xFFFFFEFCUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKSLOW          (0xFFFFFEFBUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_SYSTICK          (0xFFFFFEFAUL)
#define CY_PRA_STATUS_ERROR_PROCESSING_CLKTIMER         (0xFFFFFEF9UL)
/** \endcond */


/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/**
* \addtogroup group_pra_enums
* \{
*/

/** PRA register access */
typedef struct
{
    uint32_t addr;           /**< Register address */
    uint32_t writeMask;      /**< Write mask */
} cy_stc_pra_reg_policy_t;


/** Message used for communication */
typedef struct
{
    uint16_t command;       /**< Message Type */
    uint16_t index;         /**< Register index */
    uint32_t status;        /**< Status */
    uint32_t data1;         /**< First data word */
    uint32_t data2;         /**< Second data word */
} cy_stc_pra_msg_t;
/** \} group_pra_enums */


/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_pra_functions
* \{
*/

#if (CY_CPU_CORTEX_M0P) || defined (CY_DOXYGEN)
    void Cy_PRA_Init(void);
    void Cy_PRA_Handler(void);
    void Cy_PRA_ProcessCmd(cy_stc_pra_msg_t *message);

    /** \cond INTERNAL */
    void Cy_PRA_PmCm4DpFlagSet(void);
    void Cy_PRA_PmHibernate(uint32_t funcProc);
    void Cy_PRA_OpenSrssMain2(void);
    void Cy_PRA_CloseSrssMain2(void);
    /** \endcond */

#endif /* (CY_CPU_CORTEX_M0P) */

#if (CY_CPU_CORTEX_M4) || defined (CY_DOXYGEN)
    cy_pra_status_t Cy_PRA_SendCmd(uint16_t cmd, uint16_t regIndex, uint32_t clearMask, uint32_t setMask);

    /** \} group_pra_functions */


    /**
    * \addtogroup group_pra_macros
    * \{
    */

    /*******************************************************************************
    * Macro Name: CY_PRA_REG32_CLR_SET
    ****************************************************************************//**
    *
    * Provides get-clear-modify-write operations with a name field and value and
    * writes a resulting value to the 32-bit register.
    *
    *******************************************************************************/
    #define CY_PRA_REG32_CLR_SET(regIndex, field, value)  \
        (void)Cy_PRA_SendCmd(CY_PRA_MSG_TYPE_REG32_CLR_SET, \
                              (regIndex), \
                              ((uint32_t)(~(field ## _Msk))), \
                              (_VAL2FLD(field, \
                              (value))))


    /*******************************************************************************
    * Macro Name: CY_PRA_REG32_SET(regIndex, value)
    ****************************************************************************//**
    *
    * Writes the 32-bit value to the specified register.
    *
    * \param regIndex The register address index.
    *
    * \param value The value to write.
    *
    *******************************************************************************/
    #define CY_PRA_REG32_SET(regIndex, value)  \
        (void)Cy_PRA_SendCmd(CY_PRA_MSG_TYPE_REG32_SET, (regIndex), (value), 0UL)


    /*******************************************************************************
    * Macro Name: CY_PRA_REG32_GET(regIndex)
    ****************************************************************************//**
    *
    * Reads the 32-bit value from the specified register.
    *
    * \param regIndex The register address index.
    *
    * \return The read value.
    *
    *******************************************************************************/
    #define CY_PRA_REG32_GET(regIndex)  \
        (uint32_t) Cy_PRA_SendCmd(CY_PRA_MSG_TYPE_REG32_GET, (regIndex), 0UL, 0UL)


    /*******************************************************************************
    * Macro Name: CY_PRA_CM0_WAKEUP(regIndex)
    ****************************************************************************//**
    *
    * A simple request to wake up Cortex-M0+ core.
    *
    *******************************************************************************/
    #define CY_PRA_CM0_WAKEUP(regIndex)  \
        (void)Cy_PRA_SendCmd(CY_PRA_MSG_TYPE_CM0_WAKEUP, (regIndex), 0UL, 0UL)


    /*******************************************************************************
    * Macro Name: CY_PRA_FUNCTION_CALL_RETURN_PARAM(msgType, funcIndex, param)
    ****************************************************************************//**
    *
    * Calls the specified function with the provided parameter and return the
    * execution status.
    *
    * \param msgType Function type.
    *
    * \param funcIndex Function reference.
    *
    * \param param Pointer to the function parameter.
    *
    * \return Function execution status.
    *
    *******************************************************************************/
    #define CY_PRA_FUNCTION_CALL_RETURN_PARAM(msgType, funcIndex, param)  \
        Cy_PRA_SendCmd((msgType), (funcIndex), (uint32_t)(param), 0UL)


    /*******************************************************************************
    * Macro Name: CY_PRA_FUNCTION_CALL_RETURN_VOID(msgType, funcIndex)
    ****************************************************************************//**
    *
    * Calls the specified function without parameter and return void.
    *
    * \param msgType Function type.
    *
    * \param funcIndex Function reference.
    *
    * \return Function execution status.
    *
    *******************************************************************************/
    #define CY_PRA_FUNCTION_CALL_RETURN_VOID(msgType, funcIndex)  \
        Cy_PRA_SendCmd((msgType), (funcIndex), 0UL, 0UL)


    /*******************************************************************************
    * Macro Name: CY_PRA_FUNCTION_CALL_RETURN_VOID(msgType, funcIndex, param)
    ****************************************************************************//**
    *
    * Calls the specified function with the provided parameter and return void.
    *
    * \param msgType Function type.
    *
    * \param funcIndex Function reference.
    *
    * \param param Pointer to the function parameter.
    *
    *******************************************************************************/
    #define CY_PRA_FUNCTION_CALL_VOID_PARAM(msgType, funcIndex, param)  \
        (void)Cy_PRA_SendCmd((msgType), (funcIndex), (uint32_t)(param), 0UL)


    /*******************************************************************************
    * Macro Name: CY_PRA_FUNCTION_CALL_VOID_VOID(msgType, funcIndex)
    ****************************************************************************//**
    *
    * Calls the specified function without parameter and return void.
    *
    * \param msgType Function type.
    *
    * \param funcIndex Function reference.
    *
    *******************************************************************************/
    #define CY_PRA_FUNCTION_CALL_VOID_VOID(msgType, funcIndex)  \
        (void)Cy_PRA_SendCmd((msgType), (funcIndex), 0UL, 0UL)

    /** \} group_pra_macros */

#endif /* (CY_CPU_CORTEX_M4) */

#ifdef __cplusplus
}
#endif

#endif /* defined(CY_DEVICE_SECURE) */

#endif /* #if !defined(CY_PRA_H) */

/** \} group_pra */

/* [] END OF FILE */
