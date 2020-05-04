/***************************************************************************//**
* \file cy_pra.c
* \version 1.0
*
* \brief
* Provides the public functions for the API for the PSoC 6 Protection Access
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

#include "cy_pra.h"
#include "cy_pra_cfg.h"
#include "cy_sysint.h"
#include "cy_ipc_drv.h"
#include "cy_gpio.h"
#include "cy_device.h"
#include "cy_flash.h"           /* Cy_Flash_RAMDelay() */

#if defined(CY_DEVICE_SECURE)


void Cy_PRA_RAMDelay(uint32_t microseconds);
uint32_t structInit = 0UL;
cy_stc_pra_system_config_t structCpy = {0};

#define CY_PRA_REG_POLICY_WRITE_ALL   (0x00000000UL)

/* Table to get register/function address based on its index */
cy_stc_pra_reg_policy_t regIndexToAddr[20U] = {{0 ,  CY_PRA_REG_POLICY_WRITE_ALL}};


#if (CY_CPU_CORTEX_M0P) || defined (CY_DOXYGEN)
/*******************************************************************************
* Function Name: Cy_PRA_Init
****************************************************************************//**
*
* Initializes the Protected Register Access driver.
*
*******************************************************************************/
void Cy_PRA_Init(void)
{
    regIndexToAddr[CY_PRA_INDX_SRSS_PWR_LVD_CTL].addr          = (uint32_t) &SRSS_PWR_LVD_CTL;
    regIndexToAddr[CY_PRA_INDX_SRSS_SRSS_INTR].addr            = (uint32_t) &SRSS_SRSS_INTR;
    regIndexToAddr[CY_PRA_INDX_SRSS_SRSS_INTR_SET].addr        = (uint32_t) &SRSS_SRSS_INTR_SET;
    regIndexToAddr[CY_PRA_INDX_SRSS_SRSS_INTR_MASK].addr       = (uint32_t) &SRSS_SRSS_INTR_MASK;
    regIndexToAddr[CY_PRA_INDX_SRSS_SRSS_INTR_CFG].addr        = (uint32_t) &SRSS_SRSS_INTR_CFG;
    regIndexToAddr[CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_1].addr    = (uint32_t) &SRSS_CLK_ROOT_SELECT[1U];
    regIndexToAddr[CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_2].addr    = (uint32_t) &SRSS_CLK_ROOT_SELECT[2U];
    regIndexToAddr[CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_3].addr    = (uint32_t) &SRSS_CLK_ROOT_SELECT[3U];
    regIndexToAddr[CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_4].addr    = (uint32_t) &SRSS_CLK_ROOT_SELECT[4U];
    regIndexToAddr[CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_5].addr    = (CY_SRSS_NUM_HFROOT > 4U) ? (uint32_t) &SRSS_CLK_ROOT_SELECT[5U] : 0U;
    regIndexToAddr[CY_PRA_INDX_SRSS_CLK_ROOT_SELECT_6].addr    = (CY_SRSS_NUM_HFROOT > 5U) ? (uint32_t) &SRSS_CLK_ROOT_SELECT[6U] : 0U;
    regIndexToAddr[CY_PRA_INDX_FLASHC_FLASH_CMD].addr          = (uint32_t) &FLASHC_FLASH_CMD;
    regIndexToAddr[CY_PRA_INDX_CPUSS_CM4_PWR_CTL].addr         = (uint32_t) &CPUSS_CM4_PWR_CTL;
    regIndexToAddr[CY_PRA_INDX_SRSS_PWR_CTL].addr              = (uint32_t) &SRSS_PWR_CTL;
    regIndexToAddr[CY_PRA_INDX_SRSS_PWR_HIBERNATE].addr        = (uint32_t) &SRSS_PWR_HIBERNATE;
    regIndexToAddr[CY_PRA_INDX_SRSS_PWR_HIBERNATE].writeMask   = (uint32_t) ~ (SRSS_PWR_HIBERNATE_TOKEN_Msk |
        SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Msk | SRSS_PWR_HIBERNATE_MASK_HIBPIN_Msk);

    /* Configure the IPC interrupt handler. */
    Cy_IPC_Drv_SetInterruptMask(Cy_IPC_Drv_GetIntrBaseAddr(CY_IPC_INTR_PRA), 0UL, CY_PRA_IPC_NOTIFY_INTR);
    cy_stc_sysint_t intr = {
        .intrSrc = (IRQn_Type)CY_SYSINT_CM0P_MUX4,
        /* TODO: Ask if it is fasible or add additional field into config structure for the CY_IPC_INTR_PRA */
        .cm0pSrc = (cy_en_intr_t)CY_IPC_INTR_NUM_TO_VECT(CY_IPC_INTR_PRA),
        .intrPriority = 0UL
    };
    Cy_SysInt_Init(&intr, &Cy_PRA_Handler);
    NVIC_EnableIRQ(intr.intrSrc);
}


/*******************************************************************************
* Function Name: Cy_PRA_Handler
****************************************************************************//**
*
* The IPC interrupt handler called once there is request from non-secure core.
*
*******************************************************************************/
CY_RAMFUNC_BEGIN
#if !defined (__ICCARM__)
    CY_NOINLINE
#endif
void Cy_PRA_Handler(void)
{
    cy_stc_pra_msg_t msgLocal;
    cy_stc_pra_msg_t* msgRemote;

    /* Process internal command's copy and update original value */
    msgRemote = (cy_stc_pra_msg_t *)Cy_IPC_Drv_ReadDataValue(Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_PRA));

    msgLocal = *msgRemote;
    Cy_PRA_ProcessCmd(&msgLocal);
    *msgRemote = msgLocal;

    /* Clear interrupt logic. Required to detect next interrupt */
    Cy_IPC_Drv_ClearInterrupt(Cy_IPC_Drv_GetIntrBaseAddr(CY_IPC_INTR_PRA), 0u, CY_PRA_IPC_NOTIFY_INTR);

    Cy_IPC_Drv_LockRelease(Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_PRA), CY_IPC_CHAN_PRA);
}
CY_RAMFUNC_END


/*******************************************************************************
* Function Name: Cy_PRA_ProcessCmd
****************************************************************************//**
*
* Process and executes the command received from non-secure core.
*
* \param \ref cy_stc_pra_msg_t
*
* \return Result of the command proccessing see \ref cy_pra_status_t.
*
*******************************************************************************/
CY_RAMFUNC_BEGIN
#if !defined (__ICCARM__)
    CY_NOINLINE
#endif
void Cy_PRA_ProcessCmd(cy_stc_pra_msg_t *message)
{
    if ((CY_PRA_MSG_TYPE_REG32_GET     == message->command) ||
        (CY_PRA_MSG_TYPE_REG32_CLR_SET == message->command) ||
        (CY_PRA_MSG_TYPE_REG32_SET     == message->command))
    {
        /* TODO: check input parameter */
        if (message->index > sizeof(regIndexToAddr)/sizeof(regIndexToAddr[0U]))
        {
            message->status = CY_PRA_STATUS_ACCESS_DENIED;
        }

        /* Some registers do not exist for some families */
        if (regIndexToAddr[message->index].addr == 0U)
        {
            message->status = CY_PRA_STATUS_ACCESS_DENIED;
        }
    }

    switch (message->command)
    {
        case CY_PRA_MSG_TYPE_REG32_CLR_SET:
            if (0U == (message->data2 & regIndexToAddr[message->index].writeMask))
            {
                uint32_t tmp;

                tmp =  (*((const volatile uint32_t *)(regIndexToAddr[message->index].addr)));
                tmp &= (message->data1 | regIndexToAddr[message->index].writeMask);
                tmp |= message->data2;
                *((volatile uint32_t *)(regIndexToAddr[message->index].addr)) = tmp;
                message->status = CY_PRA_STATUS_SUCCESS;
            }
            else
            {
                message->status = CY_PRA_STATUS_ACCESS_DENIED;
            }
            break;

        case CY_PRA_MSG_TYPE_REG32_SET:
            if (0U == (message->data1 & regIndexToAddr[message->index].writeMask))
            {
                CY_SET_REG32(regIndexToAddr[message->index].addr, message->data1);
                 message->status = CY_PRA_STATUS_SUCCESS;
            }
            else
            {
                message->status = CY_PRA_STATUS_ACCESS_DENIED;
            }
            break;

        case CY_PRA_MSG_TYPE_REG32_GET:
            message->data1 = (*((const volatile uint32_t *)(regIndexToAddr[message->index].addr)));
            message->status = CY_PRA_STATUS_SUCCESS;
            break;

        case CY_PRA_MSG_TYPE_CM0_WAKEUP:
            message->status = CY_PRA_STATUS_SUCCESS;
            break;

        case CY_PRA_MSG_TYPE_SYS_CFG_FUNC:
            structCpy = *((cy_stc_pra_system_config_t *)(message->data1));
            message->status = Cy_PRA_SystemConfig(&structCpy);
            if((0UL == structInit) && (CY_PRA_STATUS_SUCCESS == message->status))
            {
                structInit = 1UL;
            }
            break;

        case CY_PRA_MSG_TYPE_SECURE_ONLY:
            switch (message->index)
            {
                case CY_PRA_INDX_PM_HIBERNATE:
                    Cy_PRA_PmHibernate(message->data1);
                    message->status = CY_PRA_STATUS_SUCCESS;
                    break;

                case CY_PRA_INDX_PM_CM4_DP_FLAG_SET:
                    Cy_PRA_PmCm4DpFlagSet();
                    message->status = CY_PRA_STATUS_SUCCESS;
                    break;

                case CY_PRA_INDX_FLASH_RAM_DELAY:
                    Cy_Flash_RAMDelay(message->data1);
                    message->status = CY_PRA_STATUS_SUCCESS;
                    break;

                default:
                    message->status = CY_PRA_STATUS_ACCESS_DENIED;
            }
            break;

        case CY_PRA_MSG_TYPE_FUNC_POLICY:
            if(0UL != structInit)
            {
                switch (message->index)
                {
                    case CY_PRA_INDX_PM_LDO_SET_VOLTAGE:
                        structCpy.powerEnable = true;
                        structCpy.ldoEnable = true;
                        structCpy.ldoVoltage = (cy_en_syspm_ldo_voltage_t)message->data1;
                        message->status = Cy_PRA_SystemConfig(&structCpy);
                        break;

                    case CY_PRA_INDX_PM_LDO_SET_MODE:
                        structCpy.powerEnable = true;
                        structCpy.ldoEnable = true;
                        structCpy.ldoMode = (cy_en_syspm_ldo_mode_t)message->data1;
                        message->status = Cy_PRA_SystemConfig(&structCpy);
                        break;

                    case CY_PRA_INDX_PM_BUCK_ENABLE:
                        structCpy.powerEnable = true;
                        structCpy.ldoEnable = false;
                        structCpy.buckVoltage = (cy_en_syspm_buck_voltage1_t)message->data1;
                        message->status = Cy_PRA_SystemConfig(&structCpy);
                        break;

                    case CY_PRA_CLK_FUNC_ECO_DISBALE:
						structCpy.ecoEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_FLL_DISABLE:
						structCpy.fllEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PLL_DISABLE:
						if((message->data1) == 0)
						{
							structCpy.pll0Enable = false;
						}
						else
						{
							structCpy.pll1Enable = false;
						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_ILO_ENABLE:
						structCpy.iloEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_ILO_DISABLE:
						structCpy.iloEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_ILO_HIBERNATE_ON:
						structCpy.iloHibernateON = message->data1;;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PILO_ENABLE:
						structCpy.piloEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PILO_DISABLE:
						structCpy.piloEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_WCO_ENABLE:
						structCpy.wcoEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_WCO_DISABLE:
						structCpy.wcoEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_WCO_BYPASS:
						structCpy.bypassEnable = message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_HF_ENABLE:
						switch (message->data1)
						{
							case 0:
							structCpy.clkHF0Enable = true;
							break;

							case 1:
							structCpy.clkHF1Enable = true;
							break;

							case 2:
							structCpy.clkHF2Enable = true;
							break;

							case 3:
							structCpy.clkHF3Enable = true;
							break;

							case 4:
							structCpy.clkHF4Enable = true;
							break;

							case 5:
							structCpy.clkHF5Enable = true;
							break;

							default:
							message->status = CY_PRA_STATUS_ACCESS_DENIED;
							break;

						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_HF_DISABLE:
						switch (message->data1)
						{
							case 0:
							structCpy.clkHF0Enable = false;
							break;

							case 1:
							structCpy.clkHF1Enable = false;
							break;

							case 2:
							structCpy.clkHF2Enable = false;
							break;

							case 3:
							structCpy.clkHF3Enable = false;
							break;

							case 4:
							structCpy.clkHF4Enable = false;
							break;

							case 5:
							structCpy.clkHF5Enable = false;
							break;

							default:
							message->status = CY_PRA_STATUS_ACCESS_DENIED;
							break;
						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_HF_SET_SOURCE:
						switch (((cy_stc_pra_clkhfsetsource_t *) message->data1)->clkHf)
						{
							case 0:
							structCpy.hf0Source = ((cy_stc_pra_clkhfsetsource_t *) message->data1)->source;
							break;

							case 1:
							structCpy.hf1Source = ((cy_stc_pra_clkhfsetsource_t *) message->data1)->source;
							break;

							case 2:
							structCpy.hf2Source = ((cy_stc_pra_clkhfsetsource_t *) message->data1)->source;
							break;

							case 3:
							structCpy.hf3Source = ((cy_stc_pra_clkhfsetsource_t *) message->data1)->source;
							break;

							case 4:
							structCpy.hf4Source = ((cy_stc_pra_clkhfsetsource_t *) message->data1)->source;
							break;

							case 5:
							structCpy.hf5Source = ((cy_stc_pra_clkhfsetsource_t *) message->data1)->source;
							break;

							default:
							message->status = CY_PRA_STATUS_ACCESS_DENIED;
							break;
						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_HF_SET_DIVIDER:
						switch (((cy_stc_pra_clkhfsetdivider_t *) message->data1)->clkHf)
						{
							case 0:
							structCpy.hf0Divider = ((cy_stc_pra_clkhfsetdivider_t *) message->data1)->divider;
							break;

							case 1:
							structCpy.hf1Divider = ((cy_stc_pra_clkhfsetdivider_t *) message->data1)->divider;
							break;

							case 2:
							structCpy.hf2Divider = ((cy_stc_pra_clkhfsetdivider_t *) message->data1)->divider;
							break;

							case 3:
							structCpy.hf3Divider = ((cy_stc_pra_clkhfsetdivider_t *) message->data1)->divider;
							break;

							case 4:
							structCpy.hf4Divider = ((cy_stc_pra_clkhfsetdivider_t *) message->data1)->divider;
							break;

							case 5:
							structCpy.hf5Divider = ((cy_stc_pra_clkhfsetdivider_t *) message->data1)->divider;
							break;

							default:
							message->status = CY_PRA_STATUS_ACCESS_DENIED;
							break;
						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_FAST_SET_DIVIDER:
						structCpy.clkFastDiv =  message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PERI_SET_DIVIDER:
						structCpy.clkPeriDiv = message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_LF_SET_SOURCE:
						structCpy.clkLfSource = (cy_en_clklf_in_sources_t)message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_TIMER_SET_SOURCE:
						structCpy.clkTimerSource = (cy_en_clktimer_in_sources_t)message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_TIMER_SET_DIVIDER:
						structCpy.clkTimerDivider = message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_TIMER_ENABLE:
						structCpy.clkTimerEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_TIMER_DISABLE:
						structCpy.clkTimerEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PUMP_SET_SOURCE:
						structCpy.pumpSource = (cy_en_clkpump_in_sources_t)message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PUMP_SET_DIVIDERE:
						structCpy.pumpDivider = (cy_en_clkpump_divide_t)message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PUMP_ENABLE:
						structCpy.clkPumpEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PUMP_DISABLE:
						structCpy.clkPumpEnable = false;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_BAK_SET_SOURCE:
						structCpy.clkBakSource = (cy_en_clkbak_in_sources_t)message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_ECO_CONFIGURE:
						structCpy.ecoFreqHz = ((cy_stc_pra_clk_eco_configure_t *) message->data1)->freq;
						structCpy.ecoLoad = ((cy_stc_pra_clk_eco_configure_t *) message->data1)->csum;
						structCpy.ecoEsr = ((cy_stc_pra_clk_eco_configure_t *) message->data1)->esr;
						structCpy.ecoDriveLevel = ((cy_stc_pra_clk_eco_configure_t *) message->data1)->drive_level;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_ECO_ENABLE:
						structCpy.ecoEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PATH_SET_SOURCE:
						switch (((cy_stc_pra_clkpathsetsource_t *) message->data1)->clk_path)
						{
							case 0:
							structCpy.path0Src = ((cy_stc_pra_clkpathsetsource_t *) message->data1)->source;
							break;

							case 1:
							structCpy.path1Src = ((cy_stc_pra_clkpathsetsource_t *) message->data1)->source;
							break;

							case 2:
							structCpy.path2Src = ((cy_stc_pra_clkpathsetsource_t *) message->data1)->source;
							break;

							case 3:
							structCpy.path3Src = ((cy_stc_pra_clkpathsetsource_t *) message->data1)->source;
							break;

							case 4:
							structCpy.path4Src = ((cy_stc_pra_clkpathsetsource_t *) message->data1)->source;
							break;

							case 5:
							structCpy.path5Src = ((cy_stc_pra_clkpathsetsource_t *) message->data1)->source;
							break;

							default:
							message->status = CY_PRA_STATUS_ACCESS_DENIED;
							break;
						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_FLL_MANCONFIG:
						structCpy.fllMult = ((cy_stc_fll_manual_config_t *) message->data1)->fllMult;
						structCpy.fllRefDiv = ((cy_stc_fll_manual_config_t *) message->data1)->refDiv;
						structCpy.fllCcoRange = ((cy_stc_fll_manual_config_t *) message->data1)->ccoRange;
						structCpy.enableOutputDiv = ((cy_stc_fll_manual_config_t *) message->data1)->enableOutputDiv;
						structCpy.lockTolerance = ((cy_stc_fll_manual_config_t *) message->data1)->lockTolerance;
						structCpy.igain = ((cy_stc_fll_manual_config_t *) message->data1)->igain;
						structCpy.pgain = ((cy_stc_fll_manual_config_t *) message->data1)->pgain;
						structCpy.settlingCount = ((cy_stc_fll_manual_config_t *) message->data1)->settlingCount;
						structCpy.outputMode = ((cy_stc_fll_manual_config_t *) message->data1)->outputMode;
						structCpy.ccoFreq = ((cy_stc_fll_manual_config_t *) message->data1)->cco_Freq;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_FLL_ENABLE:
						structCpy.fllEnable = true;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

                    case CY_PRA_CLK_FUNC_PLL_MANCONFIG:
						if(((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->clkPath == 0)
						{
							structCpy.pll0FeedbackDiv = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->feedbackDiv;
							structCpy.pll0ReferenceDiv = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->referenceDiv;
							structCpy.pll0OutputDiv = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->outputDiv;
							structCpy.pll0LfMode = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->lfMode;
							structCpy.pll0OutputMode = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->outputMode;
						}
						else
						{
							structCpy.pll1FeedbackDiv = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->feedbackDiv;
							structCpy.pll1ReferenceDiv = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->referenceDiv;
							structCpy.pll1OutputDiv = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->outputDiv;
							structCpy.pll1LfMode = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->lfMode;
							structCpy.pll1OutputMode = ((cy_stc_pra_clk_pll_manconfigure_t *) message->data1)->config->outputMode;
						}
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

					case CY_PRA_CLK_FUNC_PLL_ENABLE:
						if((message->data1) == 0)
						{
							structCpy.pll0Enable = true;
						}
						else
						{
							structCpy.pll1Enable = true;
						}

						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

					case CY_PRA_CLK_FUNC_SLOW_SET_DIVIDER:
						structCpy.clkSlowDiv = message->data1;
						message->status = Cy_PRA_SystemConfig(&structCpy);
						break;

					default:
                        message->status = CY_PRA_STATUS_ACCESS_DENIED;
                }
            }
            else
            {
                message->status = CY_PRA_STATUS_ACCESS_DENIED;
            }
            break;

        default:
            message->status = CY_PRA_STATUS_ACCESS_DENIED;
    }

}
CY_RAMFUNC_END

#endif /* (CY_CPU_CORTEX_M0P) */

#if (CY_CPU_CORTEX_M4) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_PRA_SendCmd
****************************************************************************//**
*
* Process and executes the command received from non-secure core.
*
* \param cmd Command to be executed on secure side.
* \param regIndex Index of the function or register depending on command parameter.
* \param clearMask Data send to secure core.
* \param setMask Additional data send to secure core.
*
* \return Status of the command execution. For register read command the read
* value is returned.
*
*******************************************************************************/
CY_RAMFUNC_BEGIN
#if !defined (__ICCARM__)
    CY_NOINLINE
#endif
cy_pra_status_t Cy_PRA_SendCmd(uint16_t cmd, uint16_t regIndex, uint32_t clearMask, uint32_t setMask)
{
    cy_pra_status_t status = CY_PRA_STATUS_INVALID_PARAM;
    CY_ALIGN(4) cy_stc_pra_msg_t ipcMsg;
    IPC_STRUCT_Type *ipcPraBase = Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_PRA);
    uint32_t interruptState;

    /* TODO: Check input parameters */

    ipcMsg.command = cmd;
    ipcMsg.status  = CY_PRA_STATUS_REQUEST_SENT;
    ipcMsg.index   = regIndex;
    ipcMsg.data1   = clearMask;
    ipcMsg.data2   = setMask;

    interruptState = Cy_SysLib_EnterCriticalSection();

    while (CY_IPC_DRV_SUCCESS != Cy_IPC_Drv_SendMsgWord(ipcPraBase, CY_PRA_IPC_NOTIFY_INTR, (uint32_t)&ipcMsg))
    {
        /* Try to acquire the PRA IPC structure and pass the arguments */
    }

    /* Checks whether the IPC structure is not locked */
    while (Cy_IPC_Drv_IsLockAcquired(ipcPraBase))
    {
        /* Polls whether the IPC is released */
    }

    Cy_SysLib_ExitCriticalSection(interruptState);

    /* Cortex-M0+ has updated ipcMsg variable */

    status = (cy_pra_status_t) ipcMsg.status;

    if (CY_PRA_STATUS_ACCESS_DENIED == status)
    {
        CY_HALT();
    }

    if (CY_PRA_MSG_TYPE_REG32_GET == ipcMsg.command)
    {
        status = (cy_pra_status_t)ipcMsg.data1;
    }

    return status;
}
CY_RAMFUNC_END

#endif /* (CY_CPU_CORTEX_M4) */


#if (CY_CPU_CORTEX_M0P) || defined (CY_DOXYGEN)
/*******************************************************************************
* Function Name: Cy_PRA_PmHibernate
****************************************************************************//**
*
* Update SRSS_PWR_HIBERNATE register for Cy_SysPm_SystemEnterHibernate and
* Cy_SysPm_IoUnfreeze functions.
*
*******************************************************************************/
void Cy_PRA_PmHibernate(uint32_t funcProc)
{
    /* The mask to unlock the Hibernate power mode */
    #define HIBERNATE_UNLOCK_VAL                 ((uint32_t) 0x3Au << SRSS_PWR_HIBERNATE_UNLOCK_Pos)

    /* The mask to set the Hibernate power mode */
    #define SET_HIBERNATE_MODE                   ((HIBERNATE_UNLOCK_VAL |\
                                                   SRSS_PWR_HIBERNATE_FREEZE_Msk |\
                                                   SRSS_PWR_HIBERNATE_HIBERNATE_Msk))

    /* The mask to retain the Hibernate power mode status */
    #define HIBERNATE_RETAIN_STATUS_MASK         ((SRSS_PWR_HIBERNATE_TOKEN_Msk |\
                                                   SRSS_PWR_HIBERNATE_MASK_HIBALARM_Msk |\
                                                   SRSS_PWR_HIBERNATE_MASK_HIBWDT_Msk |\
                                                   SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Msk |\
                                                   SRSS_PWR_HIBERNATE_MASK_HIBPIN_Msk))

    /** The mask for the Hibernate wakeup sources */
    #define HIBERNATE_WAKEUP_MASK               ((SRSS_PWR_HIBERNATE_MASK_HIBALARM_Msk |\
                                                  SRSS_PWR_HIBERNATE_MASK_HIBWDT_Msk |\
                                                  SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Msk |\
                                                  SRSS_PWR_HIBERNATE_MASK_HIBPIN_Msk))

    /** The define to update the token to indicate the transition into Hibernate */
    #define HIBERNATE_TOKEN                    ((uint32_t) 0x1BU << SRSS_PWR_HIBERNATE_TOKEN_Pos)

    if(0UL == funcProc)
    {
        /* Preserve the token that will be retained through a wakeup sequence.
         * This could be used by Cy_SysLib_GetResetReason() to differentiate
         * Wakeup from a general reset event.
         * Preserve the wakeup source(s) configuration.
         */
        SRSS_PWR_HIBERNATE = (SRSS_PWR_HIBERNATE & HIBERNATE_WAKEUP_MASK) | HIBERNATE_TOKEN;

        /* Disable overriding by the peripherals the next pin-freeze command */
        SRSS_PWR_HIBERNATE |= SET_HIBERNATE_MODE;

        /* The second write causes freezing of I/O cells to save the I/O-cell state */
        SRSS_PWR_HIBERNATE |= SET_HIBERNATE_MODE;

        /* Third write cause system to enter Hibernate */
        SRSS_PWR_HIBERNATE |= SET_HIBERNATE_MODE;
    }
    else
    {
        /* Preserve the last reset reason and wakeup polarity. Then, unfreeze I/O:
         * write PWR_HIBERNATE.FREEZE=0, .UNLOCK=0x3A, .HIBERANTE=0
         */
        SRSS_PWR_HIBERNATE = (SRSS_PWR_HIBERNATE & HIBERNATE_RETAIN_STATUS_MASK) | HIBERNATE_UNLOCK_VAL;

        /* Lock the Hibernate mode:
        * write PWR_HIBERNATE.HIBERNATE=0, UNLOCK=0x00, HIBERANTE=0
        */
        SRSS_PWR_HIBERNATE &= HIBERNATE_RETAIN_STATUS_MASK;
    }
}


/*******************************************************************************
* Function Name: Cy_PRA_PmCm4DpFlagSet
****************************************************************************//**
*
* Set Deep Sleep Flag for the CM4 core
*
*******************************************************************************/
void Cy_PRA_PmCm4DpFlagSet(void)
{
    uint32_t ddftStructData = 0UL;

    /* Acquire the IPC to prevent changing of the shared resources at the same time */
    while (0U == _FLD2VAL(IPC_STRUCT_ACQUIRE_SUCCESS, REG_IPC_STRUCT_ACQUIRE(CY_IPC_STRUCT_PTR(CY_IPC_CHAN_DDFT))))
    {
        /* Wait until the IPC structure is released by another CPU */
    }

    ddftStructData = REG_IPC_STRUCT_DATA(CY_IPC_STRUCT_PTR(CY_IPC_CHAN_DDFT));

    /* Update CM4 core deep sleep mask */
    ddftStructData |= (0x01UL << 28u);

    /* Update pointer to the latest saved structure */
    REG_IPC_STRUCT_DATA(CY_IPC_STRUCT_PTR(CY_IPC_CHAN_DDFT)) = ddftStructData;

    /* Release the IPC */
    REG_IPC_STRUCT_RELEASE(CY_IPC_STRUCT_PTR(CY_IPC_CHAN_DDFT)) = 0U;

    /* Read the release value to make sure it is set */
    (void) REG_IPC_STRUCT_RELEASE(CY_IPC_STRUCT_PTR(CY_IPC_CHAN_DDFT));
}

#endif /* (CY_CPU_CORTEX_M0P) */

#endif /* defined(CY_DEVICE_SECURE) */


/* [] END OF FILE */
