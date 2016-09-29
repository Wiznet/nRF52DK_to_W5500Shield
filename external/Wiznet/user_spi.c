 /* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic
 * Semiconductor ASA.Terms and conditions of usage are described in detail
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#include "user_spi.h"
#include <string.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "bsp.h"


static SPIConfig_t spi_config_table[2];
static NRF_SPI_Type *spi_base[2] = {NRF_SPI0, NRF_SPI1};
static NRF_SPI_Type *SPI;

void spi0_master_init()
{
    SPIConfig_t spi_info = {.Config.Fields.BitOrder = SPI_BITORDER_MSB_LSB,
                        .Config.Fields.Mode     = SPI_MODE3,
                        .Frequency              = SPI_FREQ_8MBPS,
                        .Pin_SCK                = SPIM0_SCK_PIN,
                        .Pin_MOSI               = SPIM0_MOSI_PIN,
                        .Pin_MISO               = SPIM0_MISO_PIN,
                        .Pin_CSN                = SPIM0_SS_PIN};	
    spi_master_init(SPI0,&spi_info);
}

uint32_t* spi_master_init(SPIModuleNumber spi_num, SPIConfig_t *spi_config)
{
    if(spi_num > 1)
    {
        return 0;
    }
    memcpy(&spi_config_table[spi_num], spi_config, sizeof(SPIConfig_t));

    /* Configure GPIO pins used for pselsck, pselmosi, pselmiso and pselss for SPI0 */
    nrf_gpio_cfg_output(spi_config->Pin_SCK);
    nrf_gpio_cfg_output(spi_config->Pin_MOSI);
    nrf_gpio_cfg_input(spi_config->Pin_MISO, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(spi_config->Pin_CSN);

    /* Configure pins, frequency and mode */
    spi_base[spi_num]->PSELSCK  = spi_config->Pin_SCK;
    spi_base[spi_num]->PSELMOSI = spi_config->Pin_MOSI;
    spi_base[spi_num]->PSELMISO = spi_config->Pin_MISO;
    nrf_gpio_pin_set(spi_config->Pin_CSN); /* disable Set slave select (inactive high) */

    spi_base[spi_num]->FREQUENCY = (uint32_t)spi_config->Frequency << 24;

    spi_base[spi_num]->CONFIG = spi_config->Config.SPI_Cfg;

    spi_base[spi_num]->EVENTS_READY = 0;
    /* Enable */
    spi_base[spi_num]->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

    return (uint32_t *)spi_base[spi_num];
}

bool spi_master_tx_rx(SPIModuleNumber spi_num, uint16_t transfer_size, const uint8_t *tx_data, uint8_t *rx_data)
{
    volatile uint32_t *SPI_DATA_READY;
    uint32_t tmp;
    if(tx_data == 0 || rx_data == 0)
    {
        return false;
    }

    SPI = spi_base[spi_num];
    SPI_DATA_READY = &SPI->EVENTS_READY;
    /* enable slave (slave select active low) */
    nrf_gpio_pin_clear(spi_config_table[spi_num].Pin_CSN);

    *SPI_DATA_READY = 0;

    SPI->TXD = (uint32_t)*tx_data++;
    tmp = (uint32_t)*tx_data++;
    while(--transfer_size)
    {
        SPI->TXD =  tmp;
        tmp = (uint32_t)*tx_data++;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while (*SPI_DATA_READY == 0);

        /* clear the event to be ready to receive next messages */
        *SPI_DATA_READY = 0;

        *rx_data++ = SPI->RXD;
    }

    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (*SPI_DATA_READY == 0);

    *rx_data = SPI->RXD;

    /* disable slave (slave select active low) */
    nrf_gpio_pin_set(spi_config_table[spi_num].Pin_CSN);

    return true;
}

bool spi_master_tx(SPIModuleNumber spi_num, uint16_t transfer_size, const uint8_t *tx_data)
{
    volatile uint32_t dummyread;

    if(tx_data == 0)
    {
        return false;
    }

    SPI = spi_base[spi_num];

    /* enable slave (slave select active low) */
    //nrf_gpio_pin_clear(spi_config_table[spi_num].Pin_CSN);

    SPI->EVENTS_READY = 0;

    SPI->TXD = (uint32_t)*tx_data++;

    while(--transfer_size)
    {
        SPI->TXD =  (uint32_t)*tx_data++;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while (SPI->EVENTS_READY == 0);

        /* clear the event to be ready to receive next messages */
        SPI->EVENTS_READY = 0;

        dummyread = SPI->RXD;
    }

    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (SPI->EVENTS_READY == 0);

    dummyread = SPI->RXD;

    /* disable slave (slave select active low) */
    //nrf_gpio_pin_set(spi_config_table[spi_num].Pin_CSN);

    return true;
}

bool spi_master_rx(SPIModuleNumber spi_num, uint16_t transfer_size, uint8_t *rx_data)
{
    if(rx_data == 0)
    {
        return false;
    }

    SPI = spi_base[spi_num];

    /* enable slave (slave select active low) */
    //nrf_gpio_pin_clear(spi_config_table[spi_num].Pin_CSN);

    SPI->EVENTS_READY = 0;

    SPI->TXD = 0;

    while(--transfer_size)
    {
        SPI->TXD = 0;

        /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
        while (SPI->EVENTS_READY == 0);

        /* clear the event to be ready to receive next messages */
        SPI->EVENTS_READY = 0;

        *rx_data++ = SPI->RXD;
    }

    /* Wait for the transaction complete or timeout (about 10ms - 20 ms) */
    while (SPI->EVENTS_READY == 0);

    *rx_data = SPI->RXD;

    /* disable slave (slave select active low) */
    //nrf_gpio_pin_set(spi_config_table[spi_num].Pin_CSN);

    return true;
}
