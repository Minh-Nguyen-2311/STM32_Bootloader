#include "flash_if.h"
#include "main.h"
#include "stm32f4xx_hal.h"

static FLASH_EraseInitTypeDef EraseInitStruct;


static uint32_t GetSector(uint32_t Address) {
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;  
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;
  }

  return sector;
}


void Flash_If_Init() {	
	/* Unlock the program memory */
	HAL_FLASH_Unlock();
	
	/* Clear all flags */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR|  FLASH_FLAG_WRPERR | FLASH_FLAG_BSY);
}

void Flash_If_Erase(uint32_t start_addr, uint32_t end_addr)  {
	uint32_t sectorError = 0;
 
	uint32_t firstSector = GetSector(start_addr);
	uint32_t numOfSector = GetSector(end_addr) - firstSector + 1;	
	/* Unlock the Flash to enable the flash control register access *************/ 
  HAL_FLASH_Unlock();
	
	/* Get flash status */ 
	if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) {
	}
	
	/* Fill EreaseInit structure */
	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = firstSector;
	EraseInitStruct.NbSectors = numOfSector;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &sectorError) != HAL_OK) {
		Error_Handler();
	}
	
	HAL_FLASH_Lock();

}

uint32_t Flash_If_Program_Word(uint32_t start_addr, uint32_t end_addr, uint32_t *data, uint32_t len) {
	uint32_t write_addr = 0;
	uint32_t i = 0;
	
	HAL_FLASH_Unlock();
	write_addr = start_addr;
	
	while(i < len) {
		if (HAL_FLASH_Program(TYPEPROGRAM_WORD, write_addr, *(data + i)) == HAL_OK) {
			if (*((uint32_t *)write_addr) != *(uint32_t *)(data + i)) {
				return 2;
			}
			write_addr += 4;		
			i++;
		}
		else {
			Error_Handler();
			return 1;
		}
	}
	HAL_FLASH_Lock();
	return 0;
}

void Flash_If_EraseProgram_Word(uint32_t start_addr, uint32_t end_addr, uint32_t *data, uint32_t len) {
	/* Erase flash with start -> end address */
	Flash_If_Erase(start_addr, end_addr);
    Flash_If_Program_Word(start_addr, end_addr, data, len);
}

 
uint32_t Flash_If_Program_SingleWord (uint32_t address, uint32_t data) {
	HAL_FLASH_Unlock();
	
	if (HAL_FLASH_Program(TYPEPROGRAM_WORD, address, data) == HAL_OK) {
		if (data != *(__IO uint32_t*)address) {
		}
	}
	else {
		return -1;
	}

	HAL_FLASH_Lock();
	return 0;
}

uint32_t Flash_If_ReadWord(uint32_t address) {
	return (*(__IO uint32_t *) address);
}
