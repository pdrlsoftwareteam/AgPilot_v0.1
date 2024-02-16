/*
 * APDirectFlashAccess.cpp
 *
 *  Created on: 02-Nov-2019
 *      Author: owner
 */

#include "APDirectFlashAccess.h"
#include "../AP_HAL_ChibiOS/hwdef/common/stm32_util.h"
#include "../AP_HAL_ChibiOS/hwdef/common/flash.h"

AP_Direct_Flash_Access *AP_Direct_Flash_Access::m_AP_Direct_Flash_Access = 0;

AP_Direct_Flash_Access::AP_Direct_Flash_Access() {
	// TODO Auto-generated constructor stub

}

AP_Direct_Flash_Access* AP_Direct_Flash_Access::getInstance()
{
	if(m_AP_Direct_Flash_Access == 0)
		m_AP_Direct_Flash_Access = new AP_Direct_Flash_Access();
	return m_AP_Direct_Flash_Access;
}

void AP_Direct_Flash_Access::storeInflash(uint8_t* dataBuf, uint16_t dataSize, uint32_t add, bool continueWr)
{

	access_semaphore.take(1);
#if defined(STM32F767xx)
	volatile uint8_t writeData = 0;
	FLASH_Unlock();
	//	FLASH_OB_Unlock();
	for(int i = 0; i < dataSize ; i++)
	{
		writeData = dataBuf[i];
		FLASH_Status status = FLASH_COMPLETE;
		volatile uint32_t addr = (uint32_t)add+i;
		//FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

		for(int j = 0 ; j < 3 ; j++)
		{
			while ( (status=FLASH_ProgramByte((uint32_t)addr, writeData)) != FLASH_COMPLETE)
			{
				#ifdef STM32F767xx
				FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				#endif
				#ifdef STM32H743xx
				FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_WRPERR  | FLASH_FLAG_PGSERR);
				#endif
			}
			if( *(uint8_t *)(addr) == writeData)
				break;
		}
		//HAL_Delay(1);
	}
	FLASH_Lock();
#elif defined(STM32H743xx)
	union aligned4ubData{
		uint8_t u8data[32];
		uint32_t u32data[8];
	};
	union aligned4ubData byalignedData = {0};

	//	uint32_t u32data[1];
	for(int i = 0; i < dataSize ; i++)
	{
		byalignedData.u8data[i%32] = dataBuf[i];
		//		u32data[0] = dataBuf[i];
		if( (((i%32) == 0) && (i != 0))  || ((i+1) >= dataSize) )
		{
			stm32_flash_write((uint32_t)add,&byalignedData.u32data,32);
			memset(&byalignedData,0,sizeof(byalignedData));
			add = add+32;
		}
	}
#else
	FLASH_Unlock();
	union aligned4ubData{
		uint8_t u8data[4];
		uint32_t u32Data;
	};
	union aligned4ubData byalignedData;

	//	uint32_t u32data[1];
	for(int i = 0; i < dataSize ; i++)
	{
		byalignedData.u8data[i%4] = dataBuf[i];
		//		u32data[0] = dataBuf[i];
		if( ((i%4) == 3) || ((i+1) == dataSize) )
			stm32_flash_write((uint32_t)add+i,&byalignedData.u32Data,sizeof(uint32_t));
	}
	FLASH_Lock();
#endif
	access_semaphore.give();
}

void AP_Direct_Flash_Access::readFromInflash(uint8_t* dataBuf, uint16_t dataSize, uint32_t add)
{
	access_semaphore.take(1);
#if 1
	memcpy(dataBuf,(void *)add,dataSize);
#else
	//	uint32_t u32data;
	for(int i = 0 ; i < dataSize ; i ++)
	{
		dataBuf[i] = *(uint8_t *)(add+i);
	}
#endif
	access_semaphore.give();
}
