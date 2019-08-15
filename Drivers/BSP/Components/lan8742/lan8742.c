/** 
  ******************************************************************************
  * @file    lan8742.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    08-March-2017
  * @brief   This file provides a set of functions needed to manage the LAN742
  *          PHY devices.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */  

/* Includes ------------------------------------------------------------------*/
#include "lan8742.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */ 
  
/** @defgroup LAN8742 LAN8742
  * @{
  */   
  
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup LAN8742_Private_Defines LAN8742 Private Defines
  * @{
  */
#define LAN8742_SW_RESET_TO    ((uint32_t)500U)
#define LAN8742_INIT_TO        ((uint32_t)1000U)
#define LAN8742_MAX_DEV_ADDR   ((uint32_t)31U)
/**
  * @}
  */
 
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @defgroup LAN8742_Private_Functions LAN8742 Private Functions
  * @{
  */
    
/**
  * @brief  Register IO functions to component object
  * @param  pObj: device object  of LAN8742_Object_t. 
  * @param  ioctx: holds device IO functions.  
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_ERROR if missing mandatory function
  */
int32_t  LAN8742_RegisterBusIO(lan8742_Object_t *pObj, lan8742_IOCtx_t *ioctx)
{
  if(!pObj || !ioctx->ReadReg || !ioctx->WriteReg || !ioctx->GetTick)
  {
    return LAN8742_STATUS_ERROR;
  }
  
  pObj->IO.Init = ioctx->Init;
  pObj->IO.DeInit = ioctx->DeInit;
  pObj->IO.ReadReg = ioctx->ReadReg;
  pObj->IO.WriteReg = ioctx->WriteReg;
  pObj->IO.GetTick = ioctx->GetTick;
  
  return LAN8742_STATUS_OK;
}

/**
  * @brief  Initialize the lan8742 and configure the needed hardware resources
  * @param  pObj: device object LAN8742_Object_t. 
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_ADDRESS_ERROR if cannot find device address
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  *         LAN8742_STATUS_RESET_TIMEOUT if cannot perform a software reset
  */
 int32_t LAN8742_Init(lan8742_Object_t *pObj)
 {
   uint32_t tickstart = 0, regvalue = 0, addr = 0;
   int32_t status = LAN8742_STATUS_OK;
   
   if(pObj->Is_Initialized == 0)
   {
     if(pObj->IO.Init != 0)
     {
       /* GPIO and Clocks initialization */
       pObj->IO.Init();
     }
   
     /* for later check */
     pObj->DevAddr = LAN8742_MAX_DEV_ADDR + 1;
   
     /* Get the device address from special mode register */  
     for(addr = 0; addr <= LAN8742_MAX_DEV_ADDR; addr ++)
     {
       if(pObj->IO.ReadReg(addr, LAN8742_PHYI1R, &regvalue) < 0)
       { 
         status = LAN8742_STATUS_READ_ERROR;
         /* Can't read from this device address 
            continue with next address */
         continue;
       }
     
       if((regvalue & 0xff) == 0x22)
       {
         pObj->DevAddr = addr;
         status = LAN8742_STATUS_OK;
         break;
       }
     }
   
     if(pObj->DevAddr > LAN8742_MAX_DEV_ADDR)
     {
       status = LAN8742_STATUS_ADDRESS_ERROR;
     }
#if 0
     /* if device address is matched */
     if(status == LAN8742_STATUS_OK)
     {
       /* set a software reset  */
       if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, LAN8742_BCR_SOFT_RESET) >= 0)
       { 
         /* get software reset status */
         if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &regvalue) >= 0)
         { 
           tickstart = pObj->IO.GetTick();
           
           /* wait until software reset is done or timeout occured  */
           while(regvalue & LAN8742_BCR_SOFT_RESET)
           {
             if((pObj->IO.GetTick() - tickstart) <= LAN8742_SW_RESET_TO)
             {
               if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &regvalue) < 0)
               { 
                 status = LAN8742_STATUS_READ_ERROR;
                 break;
               }
             }
             else
             {
               status = LAN8742_STATUS_RESET_TIMEOUT;
             }
           } 
         }
         else
         {
           status = LAN8742_STATUS_READ_ERROR;
         }
       }
       else
       {
         status = LAN8742_STATUS_WRITE_ERROR;
       }
     }
#endif
   }
      
   if(status == LAN8742_STATUS_OK)
   {
     tickstart =  pObj->IO.GetTick();
     
     /* Wait for 2s to perform initialization */
     while((pObj->IO.GetTick() - tickstart) <= LAN8742_INIT_TO)
     {
     }
     pObj->Is_Initialized = 1;
   }
   
   return status;
 }

/**
  * @brief  De-Initialize the lan8742 and it's hardware resources
  * @param  pObj: device object LAN8742_Object_t. 
  * @retval None
  */
int32_t LAN8742_DeInit(lan8742_Object_t *pObj)
{
  if(pObj->Is_Initialized)
  {
    if(pObj->IO.DeInit != 0)
    {
      if(pObj->IO.DeInit() < 0)
      {
        return LAN8742_STATUS_ERROR;
      }
    }
  
    pObj->Is_Initialized = 0;  
  }
  
  return LAN8742_STATUS_OK;
}
#if 0
/**
  * @brief  Disable the LAN8742 power down mode.
  * @param  pObj: device object LAN8742_Object_t.  
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_DisablePowerDownMode(lan8742_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &readval) >= 0)
  {
    readval &= ~LAN8742_BCR_POWER_DOWN;
  
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, readval) < 0)
    {
      status =  LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
   
  return status;
}

/**
  * @brief  Enable the LAN8742 power down mode.
  * @param  pObj: device object LAN8742_Object_t.  
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_EnablePowerDownMode(lan8742_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &readval) >= 0)
  {
    readval |= LAN8742_BCR_POWER_DOWN;
  
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, readval) < 0)
    {
      status =  LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
   
  return status;
}

/**
  * @brief  Start the auto negotiation process.
  * @param  pObj: device object LAN8742_Object_t.  
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_StartAutoNego(lan8742_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &readval) >= 0)
  {
    readval |= LAN8742_BCR_AUTONEGO_EN;
  
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, readval) < 0)
    {
      status =  LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
   
  return status;
}
#endif
/**
  * @brief  Get the link state of LAN8742 device.
  * @param  pObj: Pointer to device object. 
  * @param  pLinkState: Pointer to link state
  * @retval LAN8742_STATUS_LINK_DOWN  if link is down
  *         LAN8742_STATUS_AUTONEGO_NOTDONE if Auto nego not completed 
  *         LAN8742_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
  *         LAN8742_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
  *         LAN8742_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
  *         LAN8742_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD       
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_GetLinkState(lan8742_Object_t *pObj)
{
  uint32_t readval = 0;
  
  /* Read Status register  */
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BSR, &readval) < 0)
  {
    return LAN8742_STATUS_READ_ERROR;
  }
  
  /* Read Status register again */
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BSR, &readval) < 0)
  {
    return LAN8742_STATUS_READ_ERROR;
  }
  
  if((readval & LAN8742_BSR_LINK_STATUS) == 0)
  {
    /* Return Link Down status */
    return LAN8742_STATUS_LINK_DOWN;    
  }
  
  /* Check Auto negotiaition */
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &readval) < 0)
  {
    return LAN8742_STATUS_READ_ERROR;
  }
  
  if((readval & LAN8742_BCR_AUTONEGO_EN) != LAN8742_BCR_AUTONEGO_EN)
  {
    if(((readval & LAN8742_BCR_SPEED_SELECT) == LAN8742_BCR_SPEED_SELECT) && ((readval & LAN8742_BCR_DUPLEX_MODE) == LAN8742_BCR_DUPLEX_MODE)) 
    {
      return LAN8742_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & LAN8742_BCR_SPEED_SELECT) == LAN8742_BCR_SPEED_SELECT)
    {
      return LAN8742_STATUS_100MBITS_HALFDUPLEX;
    }        
    else if ((readval & LAN8742_BCR_DUPLEX_MODE) == LAN8742_BCR_DUPLEX_MODE)
    {
      return LAN8742_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return LAN8742_STATUS_10MBITS_HALFDUPLEX;
    }  		
  }
  else /* Auto Nego enabled */
  {
    if(pObj->IO.ReadReg(pObj->DevAddr, 1, &readval) < 0)
    {
      return LAN8742_STATUS_READ_ERROR;
    }
    
    /* Check if auto nego not done */
    if((readval & 0x0020) == 0)
    {
      return LAN8742_STATUS_AUTONEGO_NOTDONE;
    }
    if(pObj->IO.ReadReg(pObj->DevAddr, 0x1e, &readval) < 0)
    {
      return LAN8742_STATUS_READ_ERROR;
    }
    if((readval & 0x0007) == 0x6)
    {
      return LAN8742_STATUS_100MBITS_FULLDUPLEX;
    }
    else if ((readval & 0x0007) == 0x2)
    {
      return LAN8742_STATUS_100MBITS_HALFDUPLEX;
    }
    else if ((readval & 0x0007) == 0x5)
    {
      return LAN8742_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
      return LAN8742_STATUS_10MBITS_HALFDUPLEX;
    }				
  }
}
#if 0
/**
  * @brief  Set the link state of LAN8742 device.
  * @param  pObj: Pointer to device object. 
  * @param  pLinkState: link state can be one of the following
  *         LAN8742_STATUS_100MBITS_FULLDUPLEX if 100Mb/s FD
  *         LAN8742_STATUS_100MBITS_HALFDUPLEX if 100Mb/s HD
  *         LAN8742_STATUS_10MBITS_FULLDUPLEX  if 10Mb/s FD
  *         LAN8742_STATUS_10MBITS_HALFDUPLEX  if 10Mb/s HD   
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_ERROR  if parameter error  
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_SetLinkState(lan8742_Object_t *pObj, uint32_t LinkState)
{
  uint32_t bcrvalue = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &bcrvalue) >= 0)
  {
    /* Disable link config (Auto nego, speed and duplex) */
    bcrvalue &= ~(LAN8742_BCR_AUTONEGO_EN | LAN8742_BCR_SPEED_SELECT | LAN8742_BCR_DUPLEX_MODE);
    
    if(LinkState == LAN8742_STATUS_100MBITS_FULLDUPLEX)
    {
      bcrvalue |= (LAN8742_BCR_SPEED_SELECT | LAN8742_BCR_DUPLEX_MODE);
    }
    else if (LinkState == LAN8742_STATUS_100MBITS_HALFDUPLEX)
    {
      bcrvalue |= LAN8742_BCR_SPEED_SELECT;
    }
    else if (LinkState == LAN8742_STATUS_10MBITS_FULLDUPLEX)
    {
      bcrvalue |= LAN8742_BCR_DUPLEX_MODE;
    }
    else
    {
      /* Wrong link status parameter */
      status = LAN8742_STATUS_ERROR;
    }	
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
  
  if(status == LAN8742_STATUS_OK)
  {
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, bcrvalue) < 0)
    {
      status = LAN8742_STATUS_WRITE_ERROR;
    }
  }
  
  return status;
}

/**
  * @brief  Enable loopback mode.
  * @param  pObj: Pointer to device object. 
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_EnableLoopbackMode(lan8742_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &readval) >= 0)
  {
    readval |= LAN8742_BCR_LOOPBACK;
    
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, readval) < 0)
    {
      status = LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
  
  return status;
}

/**
  * @brief  Disable loopback mode.
  * @param  pObj: Pointer to device object. 
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_DisableLoopbackMode(lan8742_Object_t *pObj)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_BCR, &readval) >= 0)
  {
    readval &= ~LAN8742_BCR_LOOPBACK;
  
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_BCR, readval) < 0)
    {
      status =  LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
   
  return status;
}

/**
  * @brief  Enable IT source.
  * @param  pObj: Pointer to device object. 
  * @param  Interrupt: IT source to be enabled
  *         should be a value or a combination of the following:
  *         LAN8742_WOL_IT                     
  *         LAN8742_ENERGYON_IT                
  *         LAN8742_AUTONEGO_COMPLETE_IT       
  *         LAN8742_REMOTE_FAULT_IT            
  *         LAN8742_LINK_DOWN_IT               
  *         LAN8742_AUTONEGO_LP_ACK_IT         
  *         LAN8742_PARALLEL_DETECTION_FAULT_IT
  *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_EnableIT(lan8742_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_IMR, &readval) >= 0)
  {
    readval |= Interrupt;
  
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_IMR, readval) < 0)
    {
      status =  LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
   
  return status;
}

/**
  * @brief  Disable IT source.
  * @param  pObj: Pointer to device object. 
  * @param  Interrupt: IT source to be disabled
  *         should be a value or a combination of the following:
  *         LAN8742_WOL_IT                     
  *         LAN8742_ENERGYON_IT                
  *         LAN8742_AUTONEGO_COMPLETE_IT       
  *         LAN8742_REMOTE_FAULT_IT            
  *         LAN8742_LINK_DOWN_IT               
  *         LAN8742_AUTONEGO_LP_ACK_IT         
  *         LAN8742_PARALLEL_DETECTION_FAULT_IT
  *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  *         LAN8742_STATUS_WRITE_ERROR if connot write to register
  */
int32_t LAN8742_DisableIT(lan8742_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_IMR, &readval) >= 0)
  {
    readval &= ~Interrupt;
  
    /* Apply configuration */
    if(pObj->IO.WriteReg(pObj->DevAddr, LAN8742_IMR, readval) < 0)
    {
      status = LAN8742_STATUS_WRITE_ERROR;
    }
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
   
  return status;
}

/**
  * @brief  Clear IT flag.
  * @param  pObj: Pointer to device object. 
  * @param  Interrupt: IT flag to be cleared
  *         should be a value or a combination of the following:
  *         LAN8742_WOL_IT                     
  *         LAN8742_ENERGYON_IT                
  *         LAN8742_AUTONEGO_COMPLETE_IT       
  *         LAN8742_REMOTE_FAULT_IT            
  *         LAN8742_LINK_DOWN_IT               
  *         LAN8742_AUTONEGO_LP_ACK_IT         
  *         LAN8742_PARALLEL_DETECTION_FAULT_IT
  *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT
  * @retval LAN8742_STATUS_OK  if OK
  *         LAN8742_STATUS_READ_ERROR if connot read register
  */
int32_t  LAN8742_ClearIT(lan8742_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = LAN8742_STATUS_OK;  
  
  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_ISFR, &readval) < 0)
  {
    status =  LAN8742_STATUS_READ_ERROR;
  }
  
  return status;
}

/**
  * @brief  Get IT Flag status.
  * @param  pObj: Pointer to device object. 
  * @param  Interrupt: IT Flag to be checked, 
  *         should be a value or a combination of the following:
  *         LAN8742_WOL_IT                     
  *         LAN8742_ENERGYON_IT                
  *         LAN8742_AUTONEGO_COMPLETE_IT       
  *         LAN8742_REMOTE_FAULT_IT            
  *         LAN8742_LINK_DOWN_IT               
  *         LAN8742_AUTONEGO_LP_ACK_IT         
  *         LAN8742_PARALLEL_DETECTION_FAULT_IT
  *         LAN8742_AUTONEGO_PAGE_RECEIVED_IT  
  * @retval 1 IT flag is SET
  *         0 IT flag is RESET
  *         LAN8742_STATUS_READ_ERROR if connot read register
  */
int32_t LAN8742_GetITStatus(lan8742_Object_t *pObj, uint32_t Interrupt)
{
  uint32_t readval = 0;
  int32_t status = 0;

  if(pObj->IO.ReadReg(pObj->DevAddr, LAN8742_ISFR, &readval) >= 0)
  {
    status = ((readval & Interrupt) == Interrupt);
  }
  else
  {
    status = LAN8742_STATUS_READ_ERROR;
  }
	
  return status;
}
#endif
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */      
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
