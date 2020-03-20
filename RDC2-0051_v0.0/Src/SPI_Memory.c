/*
********************************************************************************
* COPYRIGHT(c) ��� ���� � ��ϻ, 2018
* 
* ����������� ����������� ��������������� �� �������� ���� ����� (as is).
* ��� ��������������� �������� ������ �����������.
********************************************************************************
*/



#include "SPI_Memory.h"



uint8_t Memory_Init()
{
//  SPI_GPIO->AFR[0] &= ~((0xF << (4 * SPI_MEM_NSS_PIN)) | (0xF << (4 * SPI_SCK_PIN)) | \
//                        (0xF << (4 * SPI_MISO_PIN)) | (0xF << (4 * SPI_MOSI_PIN)));
  
  SPI_MEM_NSS_GPIO->MODER |= (1 << (2 * SPI_MEM_NSS_PIN));
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
  
  SPI_GPIO->MODER |= (2 << (2 * SPI_SCK_PIN)) | \
                     (2 << (2 * SPI_MISO_PIN)) | (2 << (2 * SPI_MOSI_PIN));
  
  SPI_GPIO->OSPEEDR |= (1 << (2 * SPI_SCK_PIN)) | (1 << (2 * SPI_MISO_PIN)) | (1 << (2 * SPI_MOSI_PIN));
  
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  
  SPI_PORT->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSI | SPI_CR1_SSM;
  SPI_PORT->CR2 = SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
  SPI_PORT->CR1 |= SPI_CR1_SPE;
  
  return Memory_IsPresent();
}
//------------------------------------------------------------------------------
//�������� ������ � ������ ��������� � �������� ������� �������� ������
void Memory_Write(uint16_t BytesNum, uint32_t StartAddress, uint8_t *DataArray)
{
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);  
 
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  
  SPI_Send(MEMORY_WRITE_TO_ARRAY);
  SPI_Send(StartAddress >> 16);
  SPI_Send(StartAddress >> 8);
  SPI_Send(StartAddress);
  
  while(BytesNum)
  {    
    SPI_Send(*DataArray);
    *DataArray++;
    BytesNum--;
  }
  
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
}
//------------------------------------------------------------------------------
void Memory_Read(uint16_t BytesNum, uint32_t StartAddress, uint8_t *DataArray)
{
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  
  SPI_Send(MEMORY_READ_FROM_ARRAY);
  SPI_Send(StartAddress >> 16);
  SPI_Send(StartAddress >> 8);
  SPI_Send(StartAddress);
  
  do
  {    
    *DataArray = SPI_Send(0x00);
    *DataArray++;
    BytesNum--;
  }
  while(BytesNum);
  
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
}
//------------------------------------------------------------------------------
void Memory_EraseBlock(uint32_t BlockAddress, uint8_t EraseCmd)
{
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
  
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  SPI_Send(EraseCmd);
  SPI_Send(BlockAddress >> 16);
  SPI_Send(BlockAddress >> 8);
  SPI_Send(BlockAddress);
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
}
//------------------------------------------------------------------------------
void Memory_Erase()
{
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
  
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  SPI_Send(MEMORY_CHIP_ERASE);
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
}
//------------------------------------------------------------------------------
void Memory_ReadStatus(uint8_t *Status)
{
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  
  SPI_Send(MEMORY_READ_STATUS);
  *Status = SPI_Send(0x00);
  
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);
}
//------------------------------------------------------------------------------
void Memory_WhileNotReady()
{
  uint8_t EepromStatus = MEMORY_STATUS_READY;
  while((EepromStatus & MEMORY_STATUS_READY) == MEMORY_STATUS_READY)
    Memory_ReadStatus(&EepromStatus);
}
//------------------------------------------------------------------------------
uint8_t Memory_IsPresent()
{
  uint8_t InitStatus;
  uint8_t Status;
  
  Memory_ReadStatus(&InitStatus);
  
  SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);  
  
  Memory_ReadStatus(&Status);
  
  if (Status != InitStatus)
  {
    Status = MEMORY_PRESENT;
    
    SPI_MEM_NSS_GPIO->ODR &= ~(1 << SPI_MEM_NSS_PIN);
    SPI_Send(MEMORY_WRITE_DISABLE);
    SPI_MEM_NSS_GPIO->ODR |= (1 << SPI_MEM_NSS_PIN);  
  }
  
  else
    Status = 0;
  
  return Status;
}
//------------------------------------------------------------------------------
uint8_t SPI_Send(uint8_t DataToSend)
{
  while(!(SPI_PORT->SR & SPI_SR_TXE));
  SPI1_DR_8BIT = (uint8_t)DataToSend;
  while(!(SPI_PORT->SR & SPI_SR_RXNE));
  return SPI1_DR_8BIT;
}



