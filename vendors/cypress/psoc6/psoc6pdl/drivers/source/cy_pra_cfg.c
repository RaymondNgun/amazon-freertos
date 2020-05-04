/***************************************************************************//**
* \file cy_pra_cfg.c
* \version 1.0
*
* \brief
* Provides the public functions for the API for the PSoC 6 Protection Access
* System Configuration Driver.
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
#include "cy_pra_cfg.h"
#include "cy_gpio.h"
#include "cy_device.h"
#include "cy_gpio.h"

#if defined(CY_DEVICE_SECURE)

#if (CY_CPU_CORTEX_M0P)
    #include "cy_prot.h"
#endif /* (CY_CPU_CORTEX_M0P) */

#if (CY_CPU_CORTEX_M0P) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_PRA_IloInit
****************************************************************************//**
*
* Initializes ILO
*
* \param hibernateEnable
* true = ILO stays on during hibernate or across XRES/BOD. \n
* false = ILO turns off for hibernate or XRES/BOD.
*
*******************************************************************************/
__STATIC_INLINE void Cy_PRA_IloInit(bool hibernateEnable)
{
    /* The WDT is unlocked in the default startup code */
    Cy_SysClk_IloEnable();
    Cy_SysClk_IloHibernateOn(hibernateEnable);
}


/*******************************************************************************
* Function Name: Cy_PRA_ClkPumpInit
****************************************************************************//**
*
* Initializes PUMP Clock
*
* \param source \ref cy_en_clkpump_in_sources_t
* \param divider \ref cy_en_clkpump_divide_t
*
*******************************************************************************/
__STATIC_INLINE void Cy_PRA_ClkPumpInit(cy_en_clkpump_in_sources_t source, cy_en_clkpump_divide_t divider)
{
    Cy_SysClk_ClkPumpDisable();
    Cy_SysClk_ClkPumpSetSource(source);
    Cy_SysClk_ClkPumpSetDivider(divider);
    Cy_SysClk_ClkPumpEnable();
}


/*******************************************************************************
* Function Name: Cy_PRA_ClkTimerInit
****************************************************************************//**
*
* Initializes Timer Clock
*
* \param source \ref cy_en_clktimer_in_sources_t
* \param divider Divider value; valid range is 0 to 255. Divides the selected
* source (\ref Cy_SysClk_ClkTimerSetSource) by the (value + 1).
*
*******************************************************************************/
__STATIC_INLINE void Cy_PRA_ClkTimerInit(cy_en_clktimer_in_sources_t source, uint8_t divider)
{
    Cy_SysClk_ClkTimerDisable();
    Cy_SysClk_ClkTimerSetSource(source);
    Cy_SysClk_ClkTimerSetDivider(divider);
    Cy_SysClk_ClkTimerEnable();
}


/*******************************************************************************
* Function Name: Cy_PRA_PllInit
****************************************************************************//**
*
* Initializes Phase Locked Loop
*
* \param clkPath Selects which PLL to configure. 1 is the first PLL; 0 is invalid.
* \param pllConfig \ref cy_stc_pll_manual_config_t
*
* \return
* CY_PRA_STATUS_SUCCESS PLL init is success
* CY_PRA_STATUS_ERROR_PROCESSING PLL init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_PllInit(uint32_t clkPath, const cy_stc_pll_manual_config_t *pllConfig)
{
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_PllManualConfigure(clkPath, pllConfig))
    {
        return CY_PRA_STATUS_ERROR_PROCESSING;
    }
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_PllEnable(clkPath, 10000u))
    {
        return CY_PRA_STATUS_ERROR_PROCESSING;
    }

    return CY_PRA_STATUS_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_PRA_ClkHfInit
****************************************************************************//**
*
* Initializes High Frequency Clock
*
* \param clkHf selects which clkHf mux to configure.
* \param hfClkPath \ref cy_en_clkhf_in_sources_t
* \param divider \ref cy_en_clkhf_dividers_t
*
* \return
* CY_PRA_STATUS_SUCCESS CLK_HF init is success
* CY_PRA_STATUS_ERROR_PROCESSING CLK_HF init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_ClkHfInit( uint32_t clkHf, cy_en_clkhf_in_sources_t hfClkPath, cy_en_clkhf_dividers_t divider)
{
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_ClkHfSetSource(clkHf, hfClkPath))
        return CY_PRA_STATUS_ERROR_PROCESSING;
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_ClkHfSetDivider(clkHf, divider))
        return CY_PRA_STATUS_ERROR_PROCESSING;
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_ClkHfEnable(clkHf))
        return CY_PRA_STATUS_ERROR_PROCESSING;

    return CY_PRA_STATUS_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_PRA_FllInit
****************************************************************************//**
*
* Initializes Frequency Locked Loop
*
* \param devConfig
*
* \return
* CY_PRA_STATUS_SUCCESS FLL init is success
* CY_PRA_STATUS_ERROR_PROCESSING_FLL0 FLL init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_FllInit(const cy_stc_pra_system_config_t *devConfig)
{
    const cy_stc_fll_manual_config_t fllconfig =
    {
        .ccoRange = devConfig->fllCcoRange,
        .cco_Freq = devConfig->ccoFreq,
        .enableOutputDiv = devConfig->enableOutputDiv,
        .fllMult = devConfig->fllMult,
        .igain = devConfig->igain,
        .lockTolerance = devConfig->lockTolerance,
        .outputMode = devConfig->outputMode,
        .pgain = devConfig->pgain,
        .refDiv = devConfig->fllRefDiv,
        .settlingCount = devConfig->settlingCount,
    };

    if (CY_SYSCLK_SUCCESS != Cy_SysClk_FllManualConfigure(&fllconfig))
    {
        return CY_PRA_STATUS_ERROR_PROCESSING_FLL0;
    }
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_FllEnable(200000UL))
    {
        return CY_PRA_STATUS_ERROR_PROCESSING_FLL0;
    }

    return CY_PRA_STATUS_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_PRA_ExtClkInit
****************************************************************************//**
*
* Initializes External Clock Source
*
* \param devConfig
*
* \return
* CY_PRA_STATUS_SUCCESS EXT_CLK init is success
* CY_PRA_STATUS_ERROR_PROCESSING_EXTCLK EXT_CLK init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_ExtClkInit( const cy_stc_pra_system_config_t *devConfig )
{
    cy_pra_status_t retStatus;
    if ( (devConfig->extClkFreqHz >= 1000000) && (devConfig->extClkFreqHz <= 100000000) ) /* Frequency range [1-100MHz] */
    {
	    (void)Cy_GPIO_Pin_FastInit(devConfig->extClkPort, devConfig->extClkPinNum, CY_GPIO_DM_HIGHZ, 0UL, devConfig->extClkHsiom);
    	Cy_SysClk_ExtClkSetFrequency(devConfig->extClkFreqHz);
        retStatus = CY_PRA_STATUS_SUCCESS;
    }
    else
        retStatus = CY_PRA_STATUS_ERROR_PROCESSING_EXTCLK;

    return retStatus;
}


/*******************************************************************************
* Function Name: Cy_PRA_EcoInit
****************************************************************************//**
*
* Initializes External Crystal Oscillator
*
* \param devConfig
*
*******************************************************************************/
#if 0
TODO: Need to make decision on the clock
__STATIC_INLINE cy_pra_status_t Cy_PRA_EcoInit(const cy_stc_pra_system_config_t *devConfig)
{
    /* legal range of ECO frequecy is [4.0000-35.0000] */
    /* ECO cLoad range [0 - 100000] */
    /* ECO ESR range [0 - 100000] */
    /* ECO Drive Level range [0-1000] */
    if (((devConfig->ecoFreqHz >= 4000000) && (devConfig->ecoFreqHz <= 35000000)) &&
        (devConfig->ecoLoad <= 100000) &&
        (devConfig->ecoEsr <= 100000) &&
        (devConfig->ecoDriveLevel <= 1000))
    {
        (void)Cy_GPIO_Pin_FastInit(devConfig->ecoInPort, devConfig->ecoInPinNum, CY_GPIO_DM_ANALOG, 0UL, HSIOM_SEL_GPIO);
        (void)Cy_GPIO_Pin_FastInit(devConfig->ecoOutPort, devConfig->ecoOutPinNum, CY_GPIO_DM_ANALOG, 0UL, HSIOM_SEL_GPIO);
        if (CY_SYSCLK_BAD_PARAM == Cy_SysClk_EcoConfigure(devConfig->ecoFreqHz, devConfig->ecoLoad, devConfig->ecoEsr, devConfig->ecoDriveLevel))
        {
            return CY_PRA_STATUS_ERROR_PROCESSING_ECO;
        }
        if (CY_SYSCLK_TIMEOUT == Cy_SysClk_EcoEnable(3000UL))
        {
            return CY_PRA_STATUS_ERROR_PROCESSING_ECO;
        }
    } /* End If */

    return CY_PRA_STATUS_SUCCESS;
}
#endif


/*******************************************************************************
* Function Name: Cy_PRA_PiloInit
****************************************************************************//**
*
* Initializes PILO
*
*******************************************************************************/
__STATIC_INLINE void Cy_PRA_PiloInit()
{
    Cy_SysClk_PiloEnable();
}

#if defined(CY_IP_MXBLESS)
/*******************************************************************************
* Function Name: Cy_PRA_AltHfInit
****************************************************************************//**
*
* Initializes Alternative High-Frequency Clock
*
* \param devConfig
*
* \return
* CY_PRA_STATUS_SUCCESS AltHF init is success
* CY_PRA_STATUS_ERROR_PROCESSING_ALTHF AltHF init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_AltHfInit(const cy_stc_pra_system_config_t *devConfig)
{
    cy_en_ble_eco_status_t status;
    status = Cy_BLE_EcoConfigure((cy_en_ble_eco_freq_t)devConfig->altHFfreq, (cy_en_ble_eco_sys_clk_div_t)devConfig->altHFsysClkDiv, devConfig->altHFcLoad, devConfig->altHFxtalStartUpTime, (cy_en_ble_eco_voltage_reg_t)devConfig->altHFvoltageReg);
    if ((CY_BLE_ECO_SUCCESS != status) && (CY_BLE_ECO_ALREADY_STARTED !=status))
    {
        return CY_PRA_STATUS_ERROR_PROCESSING_ALTHF;
    }

    return CY_PRA_STATUS_SUCCESS;
}
#endif /* CY_IP_MXBLESS */


/*******************************************************************************
* Function Name: Cy_PRA_ClkLfInit
****************************************************************************//**
*
* Initializes Low-Frequency Clock
*
* \param clkLfSource \ref cy_en_clklf_in_sources_t
*
* \return
* CY_PRA_STATUS_SUCCESS LF init is success
* CY_PRA_STATUS_ERROR_PROCESSING_CLKLF LF init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_ClkLfInit(cy_en_clklf_in_sources_t clkLfSource)
{
    cy_pra_status_t retStatus;
    if (clkLfSource <= CY_SYSCLK_CLKLF_IN_PILO)
    {
        /* The WDT is unlocked in the default startup code */
        Cy_SysClk_ClkLfSetSource(clkLfSource);
        retStatus = CY_PRA_STATUS_SUCCESS;
    }
    else
        retStatus = CY_PRA_STATUS_ERROR_PROCESSING_CLKLF;

    return retStatus;
}


/*******************************************************************************
* Function Name: Cy_PRA_WcoInit
****************************************************************************//**
*
* Initializes Watch Crystal Oscillator
*
* \param devConfig
*
* \return
* CY_PRA_STATUS_SUCCESS WCO init is success
* CY_PRA_STATUS_ERROR_PROCESSING_WCO WCO init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_WcoInit(const cy_stc_pra_system_config_t *devConfig)
{
    (void)Cy_GPIO_Pin_FastInit(devConfig->wcoInPort , devConfig->wcoInPinNum, 0x00U, 0x00U, HSIOM_SEL_GPIO);
    (void)Cy_GPIO_Pin_FastInit(devConfig->wcoOutPort , devConfig->wcoOutPinNum, 0x00U, 0x00U, HSIOM_SEL_GPIO);

    if (devConfig->bypassEnable)
    {
        Cy_SysClk_WcoBypass(CY_SYSCLK_WCO_BYPASSED);
    }
    if (CY_SYSCLK_SUCCESS != Cy_SysClk_WcoEnable(1000000UL))
    {
        return CY_PRA_STATUS_ERROR_PROCESSING_WCO;
    }

    return CY_PRA_STATUS_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_PRA_PowerInit
****************************************************************************//**
*
* Initializes Power
*
* \param devConfig
*
* \return
* CY_PRA_STATUS_SUCCESS Power init is success
* CY_PRA_STATUS_ERROR_PROCESSING_PWR Power init failed
*
*******************************************************************************/
__STATIC_INLINE cy_pra_status_t Cy_PRA_PowerInit(const cy_stc_pra_system_config_t *devConfig)
{
     /* Reset the Backup domain on POR, XRES, BOD only if Backup domain is supplied by VDDD */
    if (devConfig->vBackupVDDDEnable)
    {
        if (devConfig->iloEnable)
        {
            if (0u == Cy_SysLib_GetResetReason() /* POR, XRES, or BOD */)
            {
                if (CY_SYSLIB_SUCCESS != Cy_SysLib_ResetBackupDomain())
                    return CY_PRA_STATUS_ERROR_PROCESSING_PWR;
                if (CY_SYSCLK_SUCCESS != Cy_SysClk_IloDisable())
                    return CY_PRA_STATUS_ERROR_PROCESSING_ILO;
                Cy_PRA_IloInit(devConfig->iloHibernateON);
            }
        }
    }

    if (devConfig->ldoEnable)
    {
        /* LDO valid voltage */
        if ((devConfig->ldoVoltage == CY_SYSPM_LDO_VOLTAGE_0_9V) ||
            (devConfig->ldoVoltage == CY_SYSPM_LDO_VOLTAGE_1_1V))
            {
                if (CY_SYSPM_SUCCESS != Cy_SysPm_LdoSetVoltage(devConfig->ldoVoltage))
                    return CY_PRA_STATUS_ERROR_PROCESSING_PWR;
                if (CY_SYSPM_SUCCESS != Cy_SysPm_LdoSetMode(devConfig->ldoMode))
                    return CY_PRA_STATUS_ERROR_PROCESSING_PWR;
            }
    }
    else
    {
        if ((devConfig->buckVoltage == CY_SYSPM_BUCK_OUT1_VOLTAGE_0_9V) ||
            (devConfig->buckVoltage == CY_SYSPM_BUCK_OUT1_VOLTAGE_1_1V))
            {
                if (CY_SYSPM_SUCCESS != Cy_SysPm_BuckEnable(devConfig->buckVoltage))
                    return CY_PRA_STATUS_ERROR_PROCESSING_PWR;
            }
    }

    /* Configure PMIC */
    Cy_SysPm_UnlockPmic();
    if (devConfig->pmicEnable)
    {
        Cy_SysPm_PmicEnableOutput();
    }
    else
    {
        Cy_SysPm_PmicDisableOutput();
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_GetInputPathMuxFrq
****************************************************************************//**
*
* Get Source clock frequency for the PATH MUX
*
* \param pathMuxSrc Source clock for the PATH_MUX
* \param devConfig System Configuration Parameter
*
* \return
* Returns source frequency of path mux
*
*******************************************************************************/
static uint32_t Cy_PRA_GetInputPathMuxFrq(cy_en_clkpath_in_sources_t pathMuxSrc, const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t srcFreq = 0xFFFFFFFE; /* Hz */

    CY_ASSERT_L1(devConfig != NULL);

    switch (pathMuxSrc)
    {
        case CY_SYSCLK_CLKPATH_IN_IMO:
        {
            srcFreq = 8000000; /* IMO Freq = 8 MHz */
        }
        break;
        case CY_SYSCLK_CLKPATH_IN_EXT:
        {
            srcFreq = devConfig->extClkFreqHz;
        }
        break;
        case CY_SYSCLK_CLKPATH_IN_ECO:
        {
            srcFreq = 0; /* ECO is disabled for secure device */
        }
        break;
        case CY_SYSCLK_CLKPATH_IN_ALTHF:
        {
            srcFreq = devConfig->altHFfreq;
        }
        break;
        case CY_SYSCLK_CLKPATH_IN_ILO:
        {
            srcFreq = 32000; /* ILO Freq = 32 KHz */
        }
        break;
        case CY_SYSCLK_CLKPATH_IN_WCO:
        {
            srcFreq = 0; /* WCO is disabled for Secure device */
        }
        break;
        case CY_SYSCLK_CLKPATH_IN_PILO:
        {
            srcFreq = 32768; /* PILO Freq = 32.768 KHz */
        }
        break;
        default:
            srcFreq = 0xFFFFFFFE;
    } /* End Switch */

    return srcFreq;
}

/*******************************************************************************
* Function Name: Cy_PRA_GetInputSourceFreq
****************************************************************************//**
*
* Get Source clock frequency for the Clock Path. This function is called from HF
* level
*
* \param clkPath Clock Path
* \param devConfig System Configuration Parameter
*
* \return
* Returns source frequency of clock path
*
*******************************************************************************/
static uint32_t Cy_PRA_GetInputSourceFreq(uint32_t clkPath, const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t srcFreq = 0xFFFFFFFE;

    CY_ASSERT_L1(devConfig != NULL);

    switch (clkPath)
    {
        case CY_SYSCLK_CLKHF_IN_CLKPATH0: /* CLK_PATH0 */
        {
            if (devConfig->path0Enable)
            {
                /* check for FLL */
                if (devConfig->fllEnable)
                    srcFreq = devConfig->fllOutFreq;
                else
                    srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path0Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH1: /* CLK_PATH1 */
        {
            if (devConfig->path1Enable)
            {
                /* check for PLL0 */
                if (devConfig->pll0Enable)
                    srcFreq = devConfig->pll0OutFreq;
                else
                    srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path1Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH2: /* CLK_PATH2 */
        {
            if (devConfig->path2Enable)
            {
                /* check for PLL1 */
                if (devConfig->pll1Enable)
                    srcFreq = devConfig->pll1OutFreq;
                else
                    srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path2Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH3: /* CLK_PATH3 */
        {
            if (devConfig->path3Enable)
            {
                srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path3Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH4: /* CLK_PATH4 */
        {
            if (devConfig->path4Enable)
            {
                srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path4Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH5: /* CLK_PATH5 */
        {
            if (devConfig->path5Enable)
            {
                srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path5Src, devConfig);
            }
        }
        break;
        default:
            srcFreq = 0xFFFFFFFE;
    }

    return srcFreq;
}


/*******************************************************************************
* Function Name: Cy_PRA_GetClkLfFreq
****************************************************************************//**
*
* Get Low-Frequency Clock (CLK_LF)
*
* \param devConfig System Configuration Parameter
*
* \return CLK_LF frequency
*
*******************************************************************************/
static uint32_t Cy_PRA_GetClkLfFreq(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq = 0xFFFFFFFE;

    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->clkLFEnable)
    {
        switch (devConfig->clkLfSource)
        {
            case CY_SYSCLK_CLKLF_IN_ILO:
            {
                if (devConfig->iloEnable)
                {
                    freq = 32000;
                }
            }
            break;
            case CY_SYSCLK_CLKLF_IN_WCO:
            {
                if (devConfig->wcoEnable)
                {
                    freq = 32768;
                }
            }
            break;
            case CY_SYSCLK_CLKLF_IN_PILO:
            {
                if (devConfig->piloEnable)
                {
                    freq = 32768;
                }
            }
            break;
            default:
                freq = 0xFFFFFFFE;
        } /* End Switch */
    }

    return freq;
}

/*******************************************************************************
* Function Name: Cy_PRA_GetClkBakFreq
****************************************************************************//**
*
* Get BAK Clock (CLK_BAK) frequency
*
* \param devConfig System Configuration Parameter
*
* \return CLK_LF frequency
*
*******************************************************************************/
static uint32_t Cy_PRA_GetClkBakFreq(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq = 0xFFFFFFFE;

    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->clkBakEnable)
    {
        switch (devConfig->clkBakSource)
        {
            case CY_SYSCLK_BAK_IN_WCO:
            {
                if (devConfig->wcoEnable)
                {
                    freq = 32768;
                }
            }
            break;
            case CY_SYSCLK_BAK_IN_CLKLF:
            {
                if (devConfig->clkLFEnable)
                {
                    freq = Cy_PRA_GetClkLfFreq(devConfig);
                }
            }
            break;
            default:
                freq = 0xFFFFFFFE;
        } /* End Switch */
    }

    return freq;
}


/*******************************************************************************
* Function Name: Cy_PRA_GetClkTimerFreq
****************************************************************************//**
*
* Get CLK_TIMER output frequency
*
* \param devConfig System Configuration Parameter
*
* \return CLK_TIME output frequency
*
*******************************************************************************/
static uint32_t Cy_PRA_GetClkTimerFreq(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq = 0xFFFFFFFE;
    uint8_t srcDiv = 1;
    cy_pra_status_t status;

    CY_ASSERT_L1(devConfig != NULL);

    /* source clock must be enabled */
    if (devConfig->clkTimerEnable)
    {
        switch (devConfig->clkTimerSource)
        {
            case CY_SYSCLK_CLKTIMER_IN_IMO:
            {
                /* IMO is always on */
                freq = 8000000; /* 8 MHz */
                srcDiv = 1;
            }
            break;
            case CY_SYSCLK_CLKTIMER_IN_HF0_NODIV:
            case CY_SYSCLK_CLKTIMER_IN_HF0_DIV2:
            case CY_SYSCLK_CLKTIMER_IN_HF0_DIV4:
            case CY_SYSCLK_CLKTIMER_IN_HF0_DIV8:
            {
                if ( devConfig->clkHF0Enable )
                {
                    if (devConfig->clkTimerSource == CY_SYSCLK_CLKTIMER_IN_HF0_DIV2)
                        srcDiv = 2;
                    if (devConfig->clkTimerSource == CY_SYSCLK_CLKTIMER_IN_HF0_DIV4)
                        srcDiv = 4;
                    if (devConfig->clkTimerSource == CY_SYSCLK_CLKTIMER_IN_HF0_DIV8)
                        srcDiv = 8;
                    freq = devConfig->hf0OutFreqMHz*1000000;
                }
                else
                {
                    status = CY_PRA_STATUS_INVALID_PARAM_CLKTIMER;
                }
            }
            break;
            default:
            {
                freq = 0xFFFFFFFE;
                srcDiv = 1;
                status = CY_PRA_STATUS_INVALID_PARAM_CLKTIMER;
            }
        }

        if (status != CY_PRA_STATUS_INVALID_PARAM_CLKTIMER)
            freq = freq / (devConfig->clkTimerDivider+1) / srcDiv;
    }

    return freq;
}

/*******************************************************************************
* Function Name: Cy_PRA_GetInputSourceClock
****************************************************************************//**
*
* Get Source clock for the Clock Path
*
* \param clkPath Clock Path
* \param devConfig System Configuration Parameter
*
* \return
* Returns source clock
*
*******************************************************************************/
static cy_en_clkpath_in_sources_t Cy_PRA_GetInputSourceClock(uint32_t clkPath, const cy_stc_pra_system_config_t *devConfig, cy_pra_status_t *status)
{
    cy_en_clkpath_in_sources_t srcClock = CY_SYSCLK_CLKPATH_IN_IMO;

    CY_ASSERT_L1(clkPath < CY_SRSS_NUM_CLKPATH);
    CY_ASSERT_L1(devConfig != NULL);

    *status = CY_PRA_STATUS_INVALID_PARAM;

    switch (clkPath)
    {
        case CY_SYSCLK_CLKHF_IN_CLKPATH0: /* CLK_PATH0 */
        {
            if (devConfig->path0Enable)
            {
                srcClock = devConfig->path0Src;
                *status = CY_PRA_STATUS_SUCCESS;
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH1: /* CLK_PATH1 */
        {
            if (devConfig->path1Enable)
            {
                srcClock = devConfig->path1Src;
                *status = CY_PRA_STATUS_SUCCESS;
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH2: /* CLK_PATH2 */
        {
            if (devConfig->path2Enable)
            {
                srcClock = devConfig->path2Src;
                *status = CY_PRA_STATUS_SUCCESS;
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH3: /* CLK_PATH3 */
        {
            if (devConfig->path3Enable)
            {
                srcClock = devConfig->path3Src;
                *status = CY_PRA_STATUS_SUCCESS;
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH4: /* CLK_PATH4 */
        {
            if (devConfig->path4Enable)
            {
                srcClock = devConfig->path4Src;
                *status = CY_PRA_STATUS_SUCCESS;
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH5: /* CLK_PATH5 */
        {
            if (devConfig->path5Enable)
            {
                srcClock = devConfig->path5Src;
                *status = CY_PRA_STATUS_SUCCESS;
            }
        }
        break;
        default:
        {
            /* Return CY_PRA_STATUS_INVALID_PARAM */
        }
    }

    return srcClock;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateECO
****************************************************************************//**
*
* Validate ECO
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_ECO for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateECO(const cy_stc_pra_system_config_t *devConfig)
{
    cy_pra_status_t retStatus = CY_PRA_STATUS_SUCCESS;

    CY_ASSERT_L1(devConfig != NULL);

    /* Parameter validation will be implemented in DRIVERS-2751 */
    if (devConfig->ecoEnable)
    {
        /* ECO can't be source for HF0. This check will be performed in HF0 validation  */
        retStatus = CY_PRA_STATUS_SUCCESS;
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateEXTClk
****************************************************************************//**
*
* Validate External Clock Source
*
* \param devConfig System Configuration Parameter
* \param usingULP ULP mode is enabled or disabled
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_EXTCLK for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateEXTClk(const cy_stc_pra_system_config_t *devConfig, bool usingULP)
{
    uint32_t maxFreq;

    CY_ASSERT_L1(devConfig != NULL);

    /* For ULP mode, Fextclk_max = 50 MHz. For LP mode, Fextclk_max = 100 MHz or Fcpu_max (if Fcpu_max < 100 MHz) */
    if (devConfig->extClkEnable)
    {
        if (usingULP)
        {
            if (devConfig->extClkFreqHz > 50000000)
            {
                return CY_PRA_STATUS_INVALID_PARAM_EXTCLK;
            }
        }
        else
        {
            (CY_HF_CLK_MAX_FREQ > 100000000) ? (maxFreq = 100000000) : (maxFreq = CY_HF_CLK_MAX_FREQ);
            if (devConfig->extClkFreqHz > maxFreq)
            {
                return CY_PRA_STATUS_INVALID_PARAM_EXTCLK;
            }
        } /*usingULP*/

        /* GPIO port can't be NULL */
        if ((devConfig->extClkPort == NULL) || (devConfig->extClkHsiom != HSIOM_SEL_ACT_4))
        {
            return CY_PRA_STATUS_INVALID_PARAM_EXTCLK;
        }
    }

    return CY_PRA_STATUS_SUCCESS;
}

#if defined(CY_IP_MXBLESS)

/*******************************************************************************
* Function Name: Cy_PRA_ValidateAltHf
****************************************************************************//**
*
* Validate Alternative High-Frequency Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_ALTHF for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateAltHf(const cy_stc_pra_system_config_t *devConfig)
{
    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->clkAltHfEnable)
    {
        uint32_t startupTime;
        /* Validate Frequency */
        if ((devConfig->altHFfreq < 2000000) || (devConfig->altHFfreq > 32000000))
            return CY_PRA_STATUS_INVALID_PARAM_ALTHF;
        /* Startup Time */
        startupTime = devConfig->altHFxtalStartUpTime*31.25;
        if ((startupTime < 400) || (startupTime > 4593.75))
            return CY_PRA_STATUS_INVALID_PARAM_ALTHF;
        /* Load Cap Range min="7.5" max="26.325" */
        if ((devConfig->altHFcLoad < 7.5) || (devConfig->altHFcLoad > 26.325))
            return CY_PRA_STATUS_INVALID_PARAM_ALTHF;
        /* Validate clock divider */
        if (devConfig->altHFsysClkDiv > CY_BLE_SYS_ECO_CLK_DIV_8)
            return CY_PRA_STATUS_INVALID_PARAM_ALTHF;
    }
    return CY_PRA_STATUS_SUCCESS;
}
#endif /* CY_IP_MXBLESS */

/*******************************************************************************
* Function Name: Cy_PRA_ValidateFLL
****************************************************************************//**
*
* Validate Frequency Locked Loop (FLL)
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_FLL for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateFLL(const cy_stc_pra_system_config_t *devConfig, bool usingULP)
{
    uint32_t srcFreq;

    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->fllEnable)
    {
        /* FLL is always sourced from PATH_MUX0 */
        /* If FLL is sourced from ECO, WCO, ALTHF, EXTCLK, ILO, PILO clocks, then FLL output can't source
         * to HF0. This check is performed at HF0 validation */
        if (devConfig->path0Enable)
        {
            /* Source clock for FLL valid range is 1 kHz - 100 MHz */
            srcFreq = Cy_PRA_GetInputPathMuxFrq(devConfig->path0Src, devConfig);
            if ((srcFreq < 1000) || (srcFreq > 100000000))
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;

            /* For ULP mode, out frequency should not > 50 MHz */
            /* For LP mode, out frequency is <= CY_HF_CLK_MAX_FREQ */
            if (usingULP)
            {
                if ((devConfig->fllOutFreq == 0) || (devConfig->fllOutFreq > 50000000))
                {
                    return CY_PRA_STATUS_INVALID_PARAM_FLL0;
                }
            }
            else
            {
                if ((devConfig->fllOutFreq == 0) || (devConfig->fllOutFreq > CY_HF_CLK_MAX_FREQ))
                {
                    return CY_PRA_STATUS_INVALID_PARAM_FLL0;
                }
            } /*usingULP*/

            /* Validate multiplier min="1" max="262143" */
            if ((devConfig->fllMult < 1) || (devConfig->fllMult > 262143))
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            /* Validate reference min="1" max="8191" */
            if ((devConfig->fllRefDiv < 1) || (devConfig->fllRefDiv > 8191))
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            /* ccoRange */
            if (devConfig->fllCcoRange > CY_SYSCLK_FLL_CCO_RANGE4)
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            /* lockTolerance min="0" max="511" */
            if (devConfig->lockTolerance > 511)
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            if (devConfig->igain > (SRSS_CLK_FLL_CONFIG3_FLL_LF_IGAIN_Msk >> SRSS_CLK_FLL_CONFIG3_FLL_LF_IGAIN_Pos) )
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            if (devConfig->pgain > (SRSS_CLK_FLL_CONFIG3_FLL_LF_PGAIN_Msk >> SRSS_CLK_FLL_CONFIG3_FLL_LF_PGAIN_Pos))
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            if (devConfig->settlingCount > (SRSS_CLK_FLL_CONFIG3_SETTLING_COUNT_Msk >> SRSS_CLK_FLL_CONFIG3_SETTLING_COUNT_Pos))
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;
            if (devConfig->ccoFreq > (SRSS_CLK_FLL_CONFIG4_CCO_FREQ_Msk >> SRSS_CLK_FLL_CONFIG4_CCO_FREQ_Pos))
                return CY_PRA_STATUS_INVALID_PARAM_FLL0;

            return CY_PRA_STATUS_SUCCESS;
        }
        else
            return CY_PRA_STATUS_INVALID_PARAM_FLL0;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidatePLL
****************************************************************************//**
*
* Validate Phase Locked Loop (PLL)
*
* \param devConfig System Configuration Parameter
* \param pllEnable PLL is enabled or disabled
* \param pathEnable Source path-mux is enabled or disabled for PLL
* \param pathSrc Clock source of path-mux
* \param outFreq PLL output frequency
* \param usingULP Power mode
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidatePLL(const cy_stc_pra_system_config_t *devConfig,
                                            bool pllEnable,
                                            bool pathEnable,
                                            cy_en_clkpath_in_sources_t pathSrc,
                                            uint32_t outFreq,
                                            bool usingULP)
{
    uint32_t srcFreq;

    CY_ASSERT_L1(devConfig != NULL);

    /* If PLL is sourced from ECO, WCO, ALTHF, EXTCLK, ILO, PILO clocks, then PLL output can't source
     * to HF0. This check is performed at HF0 validation */
    if (pllEnable)
    {
        if (pathEnable)
        {
            /* Source clock for PLL has valid range of 4 MHz - 64 MHz */
            srcFreq = Cy_PRA_GetInputPathMuxFrq(pathSrc, devConfig);
            if ((srcFreq < 4000000) || (srcFreq > 64000000))
                return CY_PRA_STATUS_INVALID_PARAM;
            /* For ULP mode, out frequency should not > 50 MHz */
            /* For LP mode, out frequency should not > CY_HF_CLK_MAX_FREQ */
            if (usingULP)
            {
                if ((outFreq == 0) || (outFreq > 50000000))
                {
                    return CY_PRA_STATUS_INVALID_PARAM;
                }
            }
            else
            {
                if ((outFreq == 0) || (outFreq > CY_HF_CLK_MAX_FREQ))
                {
                    return CY_PRA_STATUS_INVALID_PARAM;
                }
            } /*usingULP*/
        }
        else
            return CY_PRA_STATUS_INVALID_PARAM;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateAllPLL
****************************************************************************//**
*
* Validate All Phase Locked Loop (PLL)
*
* \param devConfig System Configuration Parameter
* \param usingULP Power mode
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_PLL0 for bad parameter for PLL0
* CY_PRA_STATUS_INVALID_PARAM_PLL1 for bad parameter for PLL1
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateAllPLL(const cy_stc_pra_system_config_t *devConfig, bool usingULP)
{
    cy_pra_status_t retStatus;

    CY_ASSERT_L1(devConfig != NULL);

    /* PLL0 is always sourced from PATH_MUX1 */
    retStatus = Cy_PRA_ValidatePLL(devConfig, devConfig->pll0Enable, devConfig->path1Enable, devConfig->path1Src, devConfig->pll0OutFreq, usingULP);
    if (CY_PRA_STATUS_SUCCESS != retStatus)
        return CY_PRA_STATUS_INVALID_PARAM_PLL0;
    /* PLL1 is always sourced from PATH_MUX2 */
    retStatus = Cy_PRA_ValidatePLL(devConfig, devConfig->pll1Enable, devConfig->path2Enable, devConfig->path2Src, devConfig->pll1OutFreq, usingULP);
    if (CY_PRA_STATUS_SUCCESS != retStatus)
        return CY_PRA_STATUS_INVALID_PARAM_PLL1;

    return retStatus;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkLf
****************************************************************************//**
*
* Validate Low-Frequency Clock (CLK_LF)
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKLF for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkLf(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t retStatus = CY_PRA_STATUS_SUCCESS;
    uint32_t freq = 0xFFFFFFFE;

    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->clkLFEnable)
    {
        switch (devConfig->clkLfSource)
        {
            case CY_SYSCLK_CLKLF_IN_ILO:
            {
                if (devConfig->iloEnable)
                {
                    freq = 32000;
                    retStatus = CY_PRA_STATUS_SUCCESS;
                }
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKLF;
            }
            break;
            case CY_SYSCLK_CLKLF_IN_WCO:
            {
                if (devConfig->wcoEnable)
                {
                    freq = 32768;
                    retStatus = CY_PRA_STATUS_SUCCESS;
                }
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKLF;
            }
            break;
            case CY_SYSCLK_CLKLF_IN_PILO:
            {
                if (devConfig->piloEnable)
                {
                    freq = 32768;
                    retStatus = CY_PRA_STATUS_SUCCESS;
                }
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKLF;
            }
            break;
            default:
                retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKLF;
        } /* End of Switch */
    }

    if (retStatus != CY_PRA_STATUS_SUCCESS)
        return retStatus;

    /* output frequency = input frequency [range min="0" max="100000"] */
    if (freq > 100000)
        retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKLF;

    return retStatus;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkPathMux
****************************************************************************//**
*
* Return error if specified path source is disabled.
*
* \param pathMuxSrc Source clock for the PATH_MUX
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkPathMux(cy_en_clkpath_in_sources_t pathSrc, const cy_stc_pra_system_config_t *devConfig)
{
    cy_pra_status_t status;

    /* Check Source clock is enabled */
    switch (pathSrc)
    {
        case CY_SYSCLK_CLKPATH_IN_IMO:
            status = CY_PRA_STATUS_SUCCESS;
        break;

        case CY_SYSCLK_CLKPATH_IN_EXT:
            status = devConfig->extClkEnable ? CY_PRA_STATUS_SUCCESS : CY_PRA_STATUS_INVALID_PARAM;
        break;

        case CY_SYSCLK_CLKPATH_IN_ECO:
            status = devConfig->ecoEnable ? CY_PRA_STATUS_SUCCESS : CY_PRA_STATUS_INVALID_PARAM;
        break;

        case CY_SYSCLK_CLKPATH_IN_ALTHF:
            status = devConfig->clkAltHfEnable ? CY_PRA_STATUS_SUCCESS : CY_PRA_STATUS_INVALID_PARAM;
        break;

        case CY_SYSCLK_CLKPATH_IN_ILO:
            status = devConfig->iloEnable ? CY_PRA_STATUS_SUCCESS : CY_PRA_STATUS_INVALID_PARAM;
        break;

        case CY_SYSCLK_CLKPATH_IN_WCO:
            status = devConfig->wcoEnable ? CY_PRA_STATUS_SUCCESS : CY_PRA_STATUS_INVALID_PARAM;
        break;

        case CY_SYSCLK_CLKPATH_IN_PILO:
            status = devConfig->piloEnable ? CY_PRA_STATUS_SUCCESS : CY_PRA_STATUS_INVALID_PARAM;
        break;

        default:
            status = CY_PRA_STATUS_INVALID_PARAM;
        break;
    }

    return status;
}


/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkPath
****************************************************************************//**
*
* Validate Clock Path
*
* \param clkPath Clock path
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkPath(uint32_t clkPath, const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t retStatus = CY_PRA_STATUS_INVALID_PARAM;

    CY_ASSERT_L1(devConfig != NULL);

    switch (clkPath)
    {
        case CY_SYSCLK_CLKHF_IN_CLKPATH0: /* CLK_PATH0 */
        {
            if (devConfig->path0Enable)
            {
                retStatus = Cy_PRA_ValidateClkPathMux(devConfig->path0Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH1: /* CLK_PATH1 */
        {
            if (devConfig->path1Enable)
            {
                retStatus = Cy_PRA_ValidateClkPathMux(devConfig->path1Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH2: /* CLK_PATH2 */
        {
            if (devConfig->path2Enable)
            {
                retStatus = Cy_PRA_ValidateClkPathMux(devConfig->path2Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH3: /* CLK_PATH3 */
        {
            if (devConfig->path3Enable)
            {
                retStatus = Cy_PRA_ValidateClkPathMux(devConfig->path3Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH4: /* CLK_PATH4 */
        {
            if (devConfig->path4Enable)
            {
                retStatus = Cy_PRA_ValidateClkPathMux(devConfig->path4Src, devConfig);
            }
        }
        break;
        case CY_SYSCLK_CLKHF_IN_CLKPATH5: /* CLK_PATH5 */
        {
            if (devConfig->path5Enable)
            {
                retStatus = Cy_PRA_ValidateClkPathMux(devConfig->path5Src, devConfig);
            }
        }
        break;
        default:
            retStatus = CY_PRA_STATUS_INVALID_PARAM;
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateAllClkPathMux
****************************************************************************//**
*
* Validate All PATH MUXes
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_PATHMUX0 for bad parameter for path-mux0
* CY_PRA_STATUS_INVALID_PARAM_PATHMUX1 for bad parameter for path-mux1
* CY_PRA_STATUS_INVALID_PARAM_PATHMUX2 for bad parameter for path-mux2
* CY_PRA_STATUS_INVALID_PARAM_PATHMUX3 for bad parameter for path-mux3
* CY_PRA_STATUS_INVALID_PARAM_PATHMUX4 for bad parameter for path-mux4
* CY_PRA_STATUS_INVALID_PARAM_PATHMUX5 for bad parameter for path-mux5
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateAllClkPathMux(const cy_stc_pra_system_config_t *devConfig)
{
    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->path0Enable) /* path_mux0 is enabled */
    {
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPathMux(devConfig->path0Src, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_PATHMUX0;
    }
    if (devConfig->path1Enable) /* path_mux1 is enabled */
    {
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPathMux(devConfig->path1Src, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_PATHMUX1;
    }
    if (devConfig->path2Enable) /* path_mux2 is enabled */
    {
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPathMux(devConfig->path2Src, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_PATHMUX2;
    }
    if (devConfig->path3Enable) /* path_mux3 is enabled */
    {
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPathMux(devConfig->path3Src, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_PATHMUX3;
    }
    if (devConfig->path4Enable) /* path_mux4 is enabled */
    {
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPathMux(devConfig->path4Src, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_PATHMUX4;
    }
    if (devConfig->path5Enable) /* path_mux5 is enabled */
    {
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPathMux(devConfig->path5Src, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_PATHMUX5;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkHfFreqDiv
****************************************************************************//**
*
* Validate High Frequency Clock's output frequency and divider
*
* \param outFreq Output frequency
* \param divider Frequency divider
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkHfFreqDiv(uint32_t outFreqMHz, cy_en_clkhf_dividers_t divider)
{
    /* min="0" max="400000000" */
    if (outFreqMHz > 400)
        return CY_PRA_STATUS_INVALID_PARAM;
    if (divider > CY_SYSCLK_CLKHF_DIVIDE_BY_8)
        return CY_PRA_STATUS_INVALID_PARAM;

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkHFs
****************************************************************************//**
*
* Validate All High Frequency Clocks
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKHF0 for bad parameter for HF0
* CY_PRA_STATUS_INVALID_PARAM_CLKHF1 for bad parameter for HF1
* CY_PRA_STATUS_INVALID_PARAM_CLKHF2 for bad parameter for HF2
* CY_PRA_STATUS_INVALID_PARAM_CLKHF3 for bad parameter for HF3
* CY_PRA_STATUS_INVALID_PARAM_CLKHF4 for bad parameter for HF4
* CY_PRA_STATUS_INVALID_PARAM_CLKHF5 for bad parameter for HF5
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkHFs(const cy_stc_pra_system_config_t *devConfig)
{
    cy_en_clkpath_in_sources_t clkSource;
    cy_pra_status_t status;
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    /* Validate HF0 */
    if (devConfig->clkHF0Enable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->hf0Source, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF0;

        /* ECO, WCO, ALTHF, EXTCLK, ILO, PILO cannot act as source to HF0  */
        clkSource = Cy_PRA_GetInputSourceClock(devConfig->hf0Source, devConfig, &status);
        if ((clkSource != CY_SYSCLK_CLKPATH_IN_IMO) || (status != CY_PRA_STATUS_SUCCESS))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF0;

        /* HF0: input source can not be slower than legal min 200 kHz */
        freq = Cy_PRA_GetInputSourceFreq(devConfig->hf0Source, devConfig);
        if ((freq < 200000) || (freq == 0xFFFFFFFE))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF0;

        /* Validate Output frequency and Dvivider */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHfFreqDiv(devConfig->hf0OutFreqMHz, devConfig->hf0Divider))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF0;
    }
    else
    {
        /* This can't be disabled */
        return CY_PRA_STATUS_INVALID_PARAM_CLKHF0;
    }

    /* Validate HF1 */
    if (devConfig->clkHF1Enable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->hf1Source, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
        /* Validate Output frequency and Dvivider */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHfFreqDiv(devConfig->hf1OutFreqMHz, devConfig->hf1Divider))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
    }

    /* Validate HF2 */
    if (devConfig->clkHF2Enable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->hf2Source, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF2;

        /* Validate Output frequency and Dvivider */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHfFreqDiv(devConfig->hf2OutFreqMHz, devConfig->hf2Divider))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF2;
    }

    /* Validate HF3 */
    if (devConfig->clkHF3Enable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->hf3Source, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF3;

        /* Validate Output frequency and Dvivider */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHfFreqDiv(devConfig->hf3OutFreqMHz, devConfig->hf3Divider))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF3;
    }

    /* Validate HF4 */
    if (devConfig->clkHF4Enable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->hf4Source, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF4;

        /* Validate Output frequency and Dvivider */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHfFreqDiv(devConfig->hf4OutFreqMHz, devConfig->hf4Divider))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF4;
    }

    /* Validate HF5 */
    if (devConfig->clkHF5Enable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->hf5Source, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF5;

        /* Validate Output frequency and Dvivider */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHfFreqDiv(devConfig->hf5OutFreqMHz, devConfig->hf5Divider))
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF5;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkPump
****************************************************************************//**
*
* Validate PUMP Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKPUMP for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkPump(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    /* Validate PUMP */
    if (devConfig->clkPumpEnable)
    {
        /* input source clock should be enabled */
        if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPath(devConfig->pumpSource, devConfig))
            return CY_PRA_STATUS_INVALID_PARAM_CLKPUMP;

        /* Validate Divider */
        if (devConfig->pumpDivider > CY_SYSCLK_PUMP_DIV_16)
            return CY_PRA_STATUS_INVALID_PARAM_CLKPUMP;

        /* Output frequency range min="0" max="400000000" */
        freq = Cy_PRA_GetInputSourceFreq(devConfig->pumpSource,devConfig);
        freq = freq / (1 << devConfig->pumpDivider); /* Calculate Output frequency */
        if (freq > 400000000)
            return CY_PRA_STATUS_INVALID_PARAM_CLKPUMP;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkBak
****************************************************************************//**
*
* Validate Backup Domain Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKBAK for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkBak(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t retStatus = CY_PRA_STATUS_SUCCESS;
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    if (devConfig->clkBakEnable)
    {
        /* Validate CLK_BAK source */
        switch (devConfig->clkBakSource)
        {
            case CY_SYSCLK_BAK_IN_WCO:
            {
                if (devConfig->wcoEnable)
                    retStatus = CY_PRA_STATUS_SUCCESS;
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKBAK;
            }
            break;
            case CY_SYSCLK_BAK_IN_CLKLF:
            {
                if (devConfig->clkLFEnable)
                    retStatus = CY_PRA_STATUS_SUCCESS;
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKBAK;
            }
            break;
            default:
                retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKBAK;
        }

        if (CY_PRA_STATUS_SUCCESS != retStatus)
            return retStatus;

        /* Validate Output Frequency min="0" max="100000" */
        /* There is no divider for CLK_BAK. So output frequency = input frequency */
        freq = Cy_PRA_GetClkBakFreq(devConfig);
        if (freq > 100000)
            retStatus = CY_PRA_STATUS_INVALID_PARAM_CLKBAK;
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkFast
****************************************************************************//**
*
* Validate Fast Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKFAST for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkFast(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    /* source clock (HF0) must be enabled */
    if (devConfig->clkFastEnable)
    {
        if (!(devConfig->clkHF0Enable))
        {
            return CY_PRA_STATUS_INVALID_PARAM_CLKFAST;
        }

        /* Validate frequency range. min="0" max="400000000" */
        freq = devConfig->hf0OutFreqMHz*1000000; /* input frequency */
        freq = freq / (devConfig->clkFastDiv+1); /* Calculate Output frequency */
        if (freq > 400000000)
            return CY_PRA_STATUS_INVALID_PARAM_CLKFAST;

        /* Validate divider min="1" max="256". User has to pass actual divider-1 */
        /* No need to validate divider because max value input can not be more that 255 */
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkPeri
****************************************************************************//**
*
* Validate Peripheral Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKPERI for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkPeri(const cy_stc_pra_system_config_t *devConfig, bool usingULP)
{
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    /* source clock (HF0) must be enabled */
    if (devConfig->clkPeriEnable)
    {
        if (!(devConfig->clkHF0Enable))
        {
            return CY_PRA_STATUS_INVALID_PARAM_CLKPERI;
        }

        freq = devConfig->hf0OutFreqMHz*1000000; /* input frequency */
        if (freq > CY_HF_CLK_MAX_FREQ)
            return CY_PRA_STATUS_INVALID_PARAM_CLKPERI;

        freq = freq / (devConfig->clkPeriDiv+1); /* Calculate Output frequency */

        /* Maximum of 25 MHz when the ULP mode is used and 100 MHz for LP mode */
        if (usingULP)
        {
            if (freq > 25000000)
                return CY_PRA_STATUS_INVALID_PARAM_CLKPERI;
        }
        else
        {
            if (freq > 100000000)
                return CY_PRA_STATUS_INVALID_PARAM_CLKPERI;
        }

        /* Validate divider min="1" max="256". User has to pass actual divider-1 */
        /* No need to validate divider because max value input can not be more that 255 */
    }
    else
    {
        /* check if this clock can't be disabled */
        return CY_PRA_STATUS_INVALID_PARAM_CLKPERI;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkTimer
****************************************************************************//**
*
* Validate Timer Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkTimer(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq;
    uint8_t srcDiv;

    CY_ASSERT_L1(devConfig != NULL);

    /* source clock must be enabled */
    if (devConfig->clkTimerEnable)
    {
        switch (devConfig->clkTimerSource)
        {
            case CY_SYSCLK_CLKTIMER_IN_IMO:
            {
                /* IMO is always on */
                freq = 8000000; /* 8 MHz */
                srcDiv = 1;
            }
            break;
            case CY_SYSCLK_CLKTIMER_IN_HF0_NODIV:
            case CY_SYSCLK_CLKTIMER_IN_HF0_DIV2:
            case CY_SYSCLK_CLKTIMER_IN_HF0_DIV4:
            case CY_SYSCLK_CLKTIMER_IN_HF0_DIV8:
            {
                if ( devConfig->clkHF0Enable )
                {
                    if (devConfig->clkTimerSource == CY_SYSCLK_CLKTIMER_IN_HF0_DIV2)
                        srcDiv = 2;
                    if (devConfig->clkTimerSource == CY_SYSCLK_CLKTIMER_IN_HF0_DIV4)
                        srcDiv = 4;
                    if (devConfig->clkTimerSource == CY_SYSCLK_CLKTIMER_IN_HF0_DIV8)
                        srcDiv = 8;
                    freq = devConfig->hf0OutFreqMHz*1000000;
                }
                else
                {
                    return CY_PRA_STATUS_INVALID_PARAM_CLKTIMER;
                }
            }
            break;
            default:
                return CY_PRA_STATUS_INVALID_PARAM_CLKTIMER;
        }

        freq = freq / (devConfig->clkTimerDivider+1) / srcDiv;
        /* Output frequency range min="0" max="400000000" */
        if (freq > 400000000)
            return CY_PRA_STATUS_INVALID_PARAM_CLKTIMER;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkSlow
****************************************************************************//**
*
* Validate Slow Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM_CLKPERI for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkSlow(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    /* source clock must be enabled */
    if (devConfig->clkSlowEnable)
    {
        if (!(devConfig->clkPeriEnable))
        {
            return CY_PRA_STATUS_INVALID_PARAM_CLKSLOW;
        }

        /* outFreq = (sourceFreq / divider) range is min="0" max="400000000" */
        freq = devConfig->hf0OutFreqMHz*1000000; /* input frequency */
        if (freq > CY_HF_CLK_MAX_FREQ)
            return CY_PRA_STATUS_INVALID_PARAM_CLKPERI;

        freq = freq / (devConfig->clkPeriDiv+1); /* Calculate Output frequency for PERI and Input Freq for SLOW_CLK */

        freq = freq / (devConfig->clkSlowDiv+1); /* Output CLK_SLOW frequency */

        if (freq > 400000000)
            return CY_PRA_STATUS_INVALID_PARAM_CLKSLOW;
    }
    else
    {
        /* check if this clock is always on */
        return CY_PRA_STATUS_INVALID_PARAM_CLKSLOW;
    }

    return CY_PRA_STATUS_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateClkAltSysTick
****************************************************************************//**
*
* Validate Alt Sys Tick Clock
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
static cy_pra_status_t Cy_PRA_ValidateClkAltSysTick(const cy_stc_pra_system_config_t *devConfig)
{
    uint32_t retStatus = CY_PRA_STATUS_SUCCESS;
    uint32_t freq;

    CY_ASSERT_L1(devConfig != NULL);

    /* source clock must be enabled */
    if (devConfig->clkAltSysTickEnable)
    {
        switch (devConfig->clkSrcAltSysTick)
        {
            case CY_SYSTICK_CLOCK_SOURCE_CLK_LF:
            {
                if (devConfig->clkLFEnable)
                {
                    freq = Cy_PRA_GetClkLfFreq(devConfig);
                    retStatus = CY_PRA_STATUS_SUCCESS;
                }
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_SYSTICK;;
            }
            break;
            case CY_SYSTICK_CLOCK_SOURCE_CLK_IMO:
            {
                freq = 8000000;
                retStatus = CY_PRA_STATUS_SUCCESS;

            }
            break;
            case CY_SYSTICK_CLOCK_SOURCE_CLK_ECO:
            {
                /* ECO is disabled for secure device */
                retStatus = CY_PRA_STATUS_INVALID_PARAM_SYSTICK;
            }
            break;
            case CY_SYSTICK_CLOCK_SOURCE_CLK_TIMER:
            {
                if (devConfig->clkTimerEnable)
                {
                    freq = Cy_PRA_GetClkTimerFreq(devConfig);
                    retStatus = CY_PRA_STATUS_SUCCESS;
                }
                else
                    retStatus = CY_PRA_STATUS_INVALID_PARAM_SYSTICK;
            }
            break;
            default:
                retStatus = CY_PRA_STATUS_INVALID_PARAM_SYSTICK;
        } /* switch */

        if (retStatus != CY_PRA_STATUS_SUCCESS)
            return retStatus;

        /* output frequency range min="0" max="400000000" */
        /* There is no divider for Timer so, output frequency = input frequency */
        if (freq > 400000000)
            retStatus = CY_PRA_STATUS_INVALID_PARAM_SYSTICK;
    }

    return retStatus;
}

/*******************************************************************************
* Function Name: Cy_PRA_ValidateSystemConfig
****************************************************************************//**
*
* Validate System Configuration
*
* \param devConfig System Configuration Parameter
*
* \return
* CY_PRA_STATUS_SUCCESS for valid input configuration
* CY_PRA_STATUS_INVALID_PARAM for bad parameter
*
*******************************************************************************/
cy_pra_status_t Cy_PRA_ValidateSystemConfig(const cy_stc_pra_system_config_t *devConfig)
{

    bool usingULP = false;
    cy_pra_status_t retStatus;

    if (devConfig == NULL )
    {
        return CY_PRA_STATUS_INVALID_PARAM;
    }

    /* Validate Power */
    if (devConfig->powerEnable)
    {
        /* validate power */
        /* ULP mode is not supported */
        if (((devConfig->ldoEnable) && (devConfig->ldoVoltage == CY_SYSPM_LDO_VOLTAGE_ULP)) ||
            (devConfig->buckVoltage == CY_SYSPM_BUCK_OUT1_VOLTAGE_ULP))
            {
                usingULP = true;
            }
    }

    /* Validate IMO */
    /* IMO must be enabled for proper chip operation. So user option is not given for IMO.
     * The output frequency of IMO is fixed to 8MHz.
     */

     /* Validate ECO */
     retStatus = Cy_PRA_ValidateECO(devConfig);
    if (CY_PRA_STATUS_SUCCESS != retStatus)
        return retStatus;

    /* Validate EXTCLK */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateEXTClk(devConfig, usingULP))
        return CY_PRA_STATUS_INVALID_PARAM_EXTCLK;

    /* Validate ALTHF (BLE ECO) */
#if defined(CY_IP_MXBLESS)
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateAltHf(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM_ALTHF;
#endif

    /* Validate ILO */
    /* ILO frequency fixed to 32KHz */

    /* Validate PILO */
    /* PILO frequency is fixed to 32.768KHz */

    /* Validate WCO */
    /* WCO frequency is fixed to 32.768KHz */

    /* Validate Path Mux */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateAllClkPathMux(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate FLL */
    /* For ULP mode, Ffll_max = 50 MHz. For LP mode, Ffll_max = 100 MHz or Fcpu_max (if Fcpu_max < 100 MHz) */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateFLL(devConfig, usingULP))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate PLLs */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateAllPLL(devConfig, usingULP))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_LF */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkLf(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_HF */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkHFs(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_PUMP */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPump(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_BAK */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkBak(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_FAST */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkFast(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_PERI */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkPeri(devConfig, usingULP))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_TIMER */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkTimer(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_SLOW */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkSlow(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

    /* Validate CLK_ALT_SYS_TICK */
    if (CY_PRA_STATUS_SUCCESS != Cy_PRA_ValidateClkAltSysTick(devConfig))
        return CY_PRA_STATUS_INVALID_PARAM;

     return CY_PRA_STATUS_SUCCESS;
}

#endif /* (CY_CPU_CORTEX_M0P) */


/*******************************************************************************
* Function Name: Cy_PRA_SystemConfig
****************************************************************************//**
*
* Initializes and Configure Device
*
* \param devConfig
*
* \return
* Status of the function processing.
*
*******************************************************************************/
cy_pra_status_t Cy_PRA_SystemConfig(const cy_stc_pra_system_config_t *devConfig)
{
    cy_pra_status_t status = CY_PRA_STATUS_SUCCESS;
#if (CY_CPU_CORTEX_M4)
    (void)devConfig;
#else
    /* Validate input parameters */
    status = Cy_PRA_ValidateSystemConfig(devConfig);
    if (CY_PRA_STATUS_SUCCESS != status)
        return status;

    /* Set worst case memory wait states (! ultra low power, 150 MHz), will update at the end */
    Cy_SysLib_SetWaitStates(false, 150UL);
    if (devConfig->powerEnable)
    {
        status = Cy_PRA_PowerInit(devConfig);
        if (CY_PRA_STATUS_SUCCESS != status)
            return status;
    }

    /* Reset the core clock path to default and disable all the FLLs/PLLs */
    status = Cy_SysClk_ClkHfSetDivider(0U, CY_SYSCLK_CLKHF_NO_DIVIDE);
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0;

    Cy_SysClk_ClkFastSetDivider(0U);
    Cy_SysClk_ClkPeriSetDivider(1U);
    Cy_SysClk_ClkSlowSetDivider(0U);
    for (uint32_t pll = CY_SRSS_NUM_PLL; pll > 0UL; --pll) /* PLL 1 is the first PLL. 0 is invalid. */
    {
        status = Cy_SysClk_PllDisable(pll);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PLL0;
    }
    status = Cy_SysClk_ClkPathSetSource(CY_SYSCLK_CLKHF_IN_CLKPATH1, CY_SYSCLK_CLKPATH_IN_IMO);
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX1;

    if ((CY_SYSCLK_CLKHF_IN_CLKPATH0 == Cy_SysClk_ClkHfGetSource(0UL)) &&
        (CY_SYSCLK_CLKPATH_IN_WCO == Cy_SysClk_ClkPathGetSource(CY_SYSCLK_CLKHF_IN_CLKPATH0)))
    {
        status = Cy_SysClk_ClkHfSetSource(0U, CY_SYSCLK_CLKHF_IN_CLKPATH1);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0;
    }

    status = Cy_SysClk_FllDisable();
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_FLL0;

    status = Cy_SysClk_ClkPathSetSource(CY_SYSCLK_CLKHF_IN_CLKPATH0, CY_SYSCLK_CLKPATH_IN_IMO);
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX0;

    status = Cy_SysClk_ClkHfSetSource(0UL, CY_SYSCLK_CLKHF_IN_CLKPATH0);
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0;

#ifdef CY_IP_MXBLESS // device specific flag
    (void)Cy_BLE_EcoReset();
#endif

    /* Enable all source clocks */
    if (devConfig->piloEnable)
    {
        Cy_PRA_PiloInit();
    }

    if (devConfig->wcoEnable)
    {
        status = Cy_PRA_WcoInit(devConfig);
        if (CY_PRA_STATUS_SUCCESS != status)
            return status;
    }

    if (devConfig->clkLFEnable)
    {
        status = Cy_PRA_ClkLfInit(devConfig->clkLfSource);
        if (CY_PRA_STATUS_SUCCESS != status)
            return status;
    }
#if defined(CY_IP_MXBLESS)
    if (devConfig->clkAltHfEnable)
    {
        status = Cy_PRA_AltHfInit(devConfig);
        if (CY_PRA_STATUS_SUCCESS != status)
            return status;
    }
#endif /* CY_IP_MXBLESS */
    if (devConfig->ecoEnable)
    {
        /* ECO is disabled for secure device */
        return CY_PRA_STATUS_ERROR_PROCESSING_ECO;
    }

    if (devConfig->extClkEnable)
    {
        status = Cy_PRA_ExtClkInit(devConfig);
        if (CY_PRA_STATUS_SUCCESS != status)
            return status;
    }

    if (devConfig->clkFastEnable)
    {
        Cy_SysClk_ClkFastSetDivider(devConfig->clkFastDiv);
    }

    if (devConfig->clkPeriEnable)
    {
        Cy_SysClk_ClkPeriSetDivider(devConfig->clkPeriDiv);
    }

    if (devConfig->clkSlowEnable)
    {
        Cy_SysClk_ClkSlowSetDivider(devConfig->clkSlowDiv);
    }

    if ((devConfig->path0Src == CY_SYSCLK_CLKPATH_IN_WCO) &&
        (devConfig->hf0Source == CY_SYSCLK_CLKHF_IN_CLKPATH0))
    {
        /* Configure HFCLK0 to temporarily run from IMO to initialize other clocks */
        status = Cy_SysClk_ClkPathSetSource(1UL, CY_SYSCLK_CLKPATH_IN_IMO);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX1;
        status = Cy_SysClk_ClkHfSetSource(0UL, CY_SYSCLK_CLKHF_IN_CLKPATH1);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0;
    }
    else
    {
        status = Cy_SysClk_ClkPathSetSource(1U, devConfig->path1Src);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX1;
    }

    /* Configure Path Clocks */

    if (devConfig->path0Enable)
    {
        status = Cy_SysClk_ClkPathSetSource(0U, devConfig->path0Src);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX0;
    }

    if (devConfig->path2Enable)
    {
        status = Cy_SysClk_ClkPathSetSource(2U, devConfig->path2Src);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX2;
    }

    if (devConfig->path3Enable)
    {
        status = Cy_SysClk_ClkPathSetSource(3U, devConfig->path3Src);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX3;
    }

    if (devConfig->path4Enable)
    {
        status = Cy_SysClk_ClkPathSetSource(4U, devConfig->path4Src);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX4;
    }

    if (devConfig->path5Enable)
    {
        status = Cy_SysClk_ClkPathSetSource(5U, devConfig->path5Src);
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX5;
    }

    /* Configure and enable FLL */
    if (devConfig->fllEnable)
    {
        status = Cy_PRA_FllInit(devConfig);
        if (CY_PRA_STATUS_SUCCESS != status)
            return status;
    }

    /* ClkHf0Init */
    status = Cy_SysClk_ClkHfSetSource(0U, devConfig->hf0Source);
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0;
    status = Cy_SysClk_ClkHfSetDivider(0U, devConfig->hf0Divider);
    if (CY_SYSCLK_SUCCESS != status)
        return CY_PRA_STATUS_ERROR_PROCESSING_CLKHF0;

    if ((devConfig->path0Src == CY_SYSCLK_CLKPATH_IN_WCO) && (devConfig->hf0Source == CY_SYSCLK_CLKHF_IN_CLKPATH0))
    {
        if (devConfig->path1Enable)
        {
            status = Cy_SysClk_ClkPathSetSource(1U, devConfig->path1Src);
            if (CY_SYSCLK_SUCCESS != status)
                return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX1;
        }
        else
        {
            return CY_PRA_STATUS_ERROR_PROCESSING_PATHMUX1;
        }

    }

    /* Configure and enable PLLs */
    if (devConfig->pll0Enable)
    {
        const cy_stc_pll_manual_config_t pll0Config =
        {
            .feedbackDiv = devConfig->pll0FeedbackDiv,
            .referenceDiv = devConfig->pll0ReferenceDiv,
            .outputDiv = devConfig->pll0OutputDiv,
            .lfMode = devConfig->pll0LfMode,
            .outputMode = devConfig->pll0OutputMode,
        };

        status = Cy_PRA_PllInit(1U, &pll0Config);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PLL0;
    }

    if (devConfig->pll1Enable)
    {
        const cy_stc_pll_manual_config_t pll1Config =
        {
            .feedbackDiv = devConfig->pll1FeedbackDiv,
            .referenceDiv = devConfig->pll1ReferenceDiv,
            .outputDiv = devConfig->pll1OutputDiv,
            .lfMode = devConfig->pll1LfMode,
            .outputMode = devConfig->pll1OutputMode,
        };

        status = Cy_PRA_PllInit(2U, &pll1Config);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_PLL1;
    }

    /* Configure HF clocks */
    if (devConfig->clkHF1Enable)
    {
        status = Cy_PRA_ClkHfInit(1U, devConfig->hf1Source, devConfig->hf1Divider);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
    }
    if (devConfig->clkHF2Enable)
    {
        status = Cy_PRA_ClkHfInit(2U, devConfig->hf2Source, devConfig->hf2Divider);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
    }
    if (devConfig->clkHF3Enable)
    {
        status = Cy_PRA_ClkHfInit(3U, devConfig->hf3Source, devConfig->hf3Divider);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
    }
    if (devConfig->clkHF4Enable)
    {
        status = Cy_PRA_ClkHfInit(4U, devConfig->hf4Source, devConfig->hf4Divider);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
    }
    if (devConfig->clkHF5Enable)
    {
        status = Cy_PRA_ClkHfInit(5U, devConfig->hf5Source, devConfig->hf5Divider);
        if (CY_PRA_STATUS_SUCCESS != status)
            return CY_PRA_STATUS_INVALID_PARAM_CLKHF1;
    }

    /* Configure miscellaneous clocks */
    if (devConfig->clkTimerEnable)
    {
        Cy_PRA_ClkTimerInit(devConfig->clkTimerSource, devConfig->clkTimerDivider);
    }

    if (devConfig->clkAltSysTickEnable)
    {
        Cy_SysTick_SetClockSource(devConfig->clkSrcAltSysTick);
    }

    if (devConfig->clkPumpEnable)
    {
        Cy_PRA_ClkPumpInit(devConfig->pumpSource, devConfig->pumpDivider);
    }

    if (devConfig->clkBakEnable)
    {
        Cy_SysClk_ClkBakSetSource(devConfig->clkBakSource);
    }

    /* Configure default enabled clocks */
    if (devConfig->iloEnable)
    {
        Cy_PRA_IloInit(devConfig->iloHibernateON);
    }
    else
    {
        status = Cy_SysClk_IloDisable();
        if (CY_SYSCLK_SUCCESS != status)
            return CY_PRA_STATUS_ERROR_PROCESSING_ILO;
        Cy_SysClk_IloHibernateOn(false);
    }

    /* SYSCLK MFO INIT */
    /* SYSCLK MF INIT */

    /* Set accurate flash wait states */
    if ((devConfig->powerEnable) && (devConfig->clkHF0Enable))
    {
        Cy_SysLib_SetWaitStates(devConfig->ulpEnable != 0, devConfig->hf0OutFreqMHz);
    }

    /* Update System Core Clock values for correct Cy_SysLib_Delay functioning */
    SystemCoreClockUpdate();

#endif /* (CY_CPU_CORTEX_M4) */

    return (status);
}


#if (CY_CPU_CORTEX_M0P) || defined (CY_DOXYGEN)

    #if !defined(CY_DEVICE_PSOC6ABLE2)
        #define CY_PRA_ALL_PC_MASK      (CY_PROT_PCMASK1 + CY_PROT_PCMASK2 + \
                                        CY_PROT_PCMASK3 + CY_PROT_PCMASK4 + \
                                        CY_PROT_PCMASK5 + CY_PROT_PCMASK6 + \
                                        CY_PROT_PCMASK7)

        #define CY_PRA_SECURE_PC_MASK   (CY_PROT_PCMASK1 + CY_PROT_PCMASK2 + \
                                        CY_PROT_PCMASK3 + CY_PROT_PCMASK4)
    #endif /* !defined(CY_DEVICE_PSOC6ABLE2) */


/*******************************************************************************
* Function Name: Cy_PRA_CloseSrssMain2
****************************************************************************//**
*
* Restricts access to the SRSS_MAIN2 region that includes following register:
* SRSS_TST_DDFT_FAST_CTL_REG, SRSS_TST_DDFT_FAST_CTL_REG.
*
*******************************************************************************/
void Cy_PRA_CloseSrssMain2(void)
{
    #if defined(CY_DEVICE_PSOC6ABLE2)
        /* TODO */
    #else
        Cy_Prot_ConfigPpuFixedSlaveAtt(PERI_MS_PPU_FX_SRSS_MAIN2,
                                    CY_PRA_SECURE_PC_MASK,
                                    CY_PROT_PERM_RW,
                                    CY_PROT_PERM_RW,
                                    false);

        Cy_Prot_ConfigPpuFixedSlaveAtt(PERI_MS_PPU_FX_SRSS_MAIN2,
                                    CY_PRA_ALL_PC_MASK ^ CY_PRA_SECURE_PC_MASK,
                                    CY_PROT_PERM_DISABLED,
                                    CY_PROT_PERM_DISABLED,
                                    false);
    #endif /* defined(CY_DEVICE_PSOC6ABLE2) */
}


/*******************************************************************************
* Function Name: Cy_PRA_OpenSrssMain2
****************************************************************************//**
*
* Restores access to the SRSS_MAIN2 region that was restrcited by
* \ref Cy_PRA_CloseSrssMain2.
*
*******************************************************************************/
void Cy_PRA_OpenSrssMain2(void)
{
    #if defined(CY_DEVICE_PSOC6ABLE2)
        /* TODO */
    #else
        Cy_Prot_ConfigPpuFixedSlaveAtt(PERI_MS_PPU_FX_SRSS_MAIN2,
                                    CY_PRA_ALL_PC_MASK,
                                    CY_PROT_PERM_RW,
                                    CY_PROT_PERM_RW,
                                    false);
    #endif /* defined(CY_DEVICE_PSOC6ABLE2) */
}

#endif /* (CY_CPU_CORTEX_M0P) */


#endif /* defined(CY_DEVICE_SECURE) */


/* [] END OF FILE */
