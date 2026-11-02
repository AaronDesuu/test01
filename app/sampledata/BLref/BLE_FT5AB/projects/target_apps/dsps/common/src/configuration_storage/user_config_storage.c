/**
 ****************************************************************************************
 *
 * @file user_config_storage.c
 *
 * @brief Application configuration data storage API.
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
 
 /**
 ****************************************************************************************
 * @addtogroup CONFIG_STORAGE
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "user_config_storage.h"
#include "spi.h"
#include "spi_flash.h"
#include "user_periph_setup.h"

#ifdef CFG_CONFIG_STORAGE 

user_config_struct_header_t user_config_struct_header __SECTION_ZERO("retention_mem_area0");
#ifdef USE_CONFIG_STORAGE_CRC
extern uint32_t crc32(uint32_t crc, const void *buf, size_t size);
#endif
uint8_t current_config_struct_index __SECTION_ZERO("retention_mem_area0");
uint8_t current_config_struct_id __SECTION_ZERO("retention_mem_area0");
uint32_t prod_head_config_offset_1 __SECTION_ZERO("retention_mem_area0");
uint32_t prod_head_config_offset_2 __SECTION_ZERO("retention_mem_area0");

/**
 ****************************************************************************************
 * @brief Reads data from external memory
 * @param[in]   rd_data_ptr     Address to store the data
 * @param[in]   address         Offset in the external memory
 * @param[in]   size            Size of data
 * @return negative error code if failed, else 0
 ****************************************************************************************
 */
static int user_config_storage_read_ext_mem( uint8_t *rd_data_ptr, uint32_t address, uint32_t size )
{
    int ret = -1;
    uint32_t actual_size;

    ret = spi_flash_read_data( (uint8_t*)rd_data_ptr, (uint32_t)address,(uint32_t)size, &actual_size);
    return ret;
}

enum user_config_storage_error_codes user_conf_storage_init(user_config_elem_t *conf_table, uint8_t conf_table_num_of_elmts, char *version, uint8_t *version_length)
{
    user_config_struct_header_t conf_header;
    uint8_t index = 0x3;
    uint32_t gpio_map = SPI_FLASH_GPIO_MAP;
    uint32_t conf_data_pos;
    int ret;
    config_product_header_t product_header;

    user_spi_flash_init(gpio_map);

    ret = user_config_storage_read_ext_mem( (uint8_t*)&product_header, (uint32_t)PRODUCT_HEADER_POSITION, (uint32_t)sizeof(config_product_header_t) );
    if(ret < 0)
        return USER_CONFIG_STORAGE_PRODUCT_HEADER_FAILURE;
    prod_head_config_offset_1 = product_header.config_offset1;
    prod_head_config_offset_2 = product_header.config_offset2;

    ret = user_config_storage_read_ext_mem( (uint8_t*)&user_config_struct_header, product_header.config_offset1, (uint32_t)sizeof(user_config_struct_header_t) );
    if(ret < 0)
        return USER_CONFIG_STORAGE_CFG_HEADER_FAILURE;
    ret = user_config_storage_read_ext_mem( (uint8_t*)&conf_header, product_header.config_offset2, (uint32_t)sizeof(user_config_struct_header_t) );
    if(ret < 0)
        return USER_CONFIG_STORAGE_CFG_HEADER_FAILURE;

    if (user_config_struct_header.signature[0] != CONFIGURATION_HEADER_SIGNATURE1 || user_config_struct_header.signature[1] != CONFIGURATION_HEADER_SIGNATURE2 || user_config_struct_header.valid != STATUS_VALID_CONFIG)
        index &= ~1;
    if (conf_header.signature[0] != CONFIGURATION_HEADER_SIGNATURE1 || conf_header.signature[1] != CONFIGURATION_HEADER_SIGNATURE2 || conf_header.valid != STATUS_VALID_CONFIG)
        index &= ~2;

    if (!index)
        return USER_CONFIG_STORAGE_CFG_HEADER_FAILURE;
    else if (index == 0x1)
        conf_data_pos = product_header.config_offset1 + (uint32_t)sizeof(user_config_struct_header_t);
    else if (index == 0x2)
        conf_data_pos = product_header.config_offset2 + (uint32_t)sizeof(user_config_struct_header_t);
    else if (index == 0x3)
    {
        if (user_config_struct_header.struct_id - conf_header.struct_id >= 0)
        {
            index = 0x1;
            conf_data_pos = product_header.config_offset1 + (uint32_t)sizeof(user_config_struct_header_t);
        }
        else
        {
            index = 0x2;
            conf_data_pos = product_header.config_offset2 + (uint32_t)sizeof(user_config_struct_header_t);
        }
    }

    current_config_struct_index = index;
    if (index == 2)
        memcpy((uint8_t*)&user_config_struct_header, (uint8_t*)&conf_header, (unsigned long)sizeof(user_config_struct_header_t));
    current_config_struct_id = user_config_struct_header.struct_id;
    memcpy(version, user_config_struct_header.version, 16);
    *version_length = sizeof(user_config_struct_header.version);

    ret = (int)user_config_read_configuration_struct(conf_data_pos, conf_table, conf_table_num_of_elmts);
    user_spi_flash_release();
    if (ret != USER_CONFIG_STORAGE_SUCCESS)
        return (enum user_config_storage_error_codes)ret;

    return USER_CONFIG_STORAGE_SUCCESS;
}

enum user_config_storage_error_codes user_config_read_configuration_struct(uint32_t conf_data_pos,
                                                                           user_config_elem_t *conf_table,
                                                                           uint8_t conf_table_num_of_elmts)
{
    int i, j;
    uint8_t* p;
    uint8_t* mem_data_buff;
    uint16_t elemid;
#ifdef USE_CONFIG_STORAGE_CRC
    uint32_t crc = 0;
#endif
    int ret = 0;

    mem_data_buff = (uint8_t*) ke_malloc(user_config_struct_header.data_size, KE_MEM_NON_RETENTION);
    ret = user_config_storage_read_ext_mem(mem_data_buff, conf_data_pos, user_config_struct_header.data_size);
    if (ret < 0)
        return USER_CONFIG_STORAGE_READ_FAILURE;

    p = mem_data_buff;
#ifdef USE_CONFIG_STORAGE_CRC
    crc = crc32(0, mem_data_buff, (size_t)user_config_struct_header.data_size);
    if (crc != user_config_struct_header.crc)
        return USER_CONFIG_STORAGE_CRC_ERR;
#endif
    for (i = 0; i < user_config_struct_header.num_of_items; i++)
    {
        elemid = p[0] | ((uint16_t)p[1] << 8);
        for (j = 0; j < conf_table_num_of_elmts; j++)
        {
            if (elemid == conf_table[j].elem_id)
                break;
        }
        if (j == conf_table_num_of_elmts)
        {
            p += 3 + p[2];
            continue;
        }

        conf_table[j].cur_size = p[2];
        memset(conf_table[j].data, 0, conf_table[j].max_size);
        memcpy(conf_table[j].data, &p[3], p[2]);
        p += 3 + p[2];
    }
    ke_free(mem_data_buff);
    return USER_CONFIG_STORAGE_SUCCESS;
}

enum user_config_storage_error_codes user_config_storage_save_configuration(user_configuration_struct_tag* config_data)
{
    uint32_t gpio_map = SPI_FLASH_GPIO_MAP;
    uint16_t data_size = 0;
    int ret, i;
    uint32_t config_offset;

    user_spi_flash_init(gpio_map);

    if (current_config_struct_index == 1)
    {
        config_offset = prod_head_config_offset_2;
    }
    else
    {
        config_offset = prod_head_config_offset_1;
    }
    for (i = 0; i < config_data->params_num; i++)
        data_size += (config_data->params[i].cur_size + 3);

    ret = (int)user_config_write_configuration_struct(config_data, config_offset, (uint32_t)sizeof(user_config_struct_header_t) + (uint32_t)data_size);
    user_spi_flash_release();
    if (ret != USER_CONFIG_STORAGE_SUCCESS)
        return (enum user_config_storage_error_codes)ret;

    current_config_struct_index++;
    if (current_config_struct_index == 3) current_config_struct_index = 1;
    current_config_struct_id++;

    return USER_CONFIG_STORAGE_SUCCESS;
}

enum user_config_storage_error_codes user_config_write_configuration_struct(user_configuration_struct_tag* config_data,
                                                                            uint32_t config_offset, uint32_t size)
{
    uint8_t *mem_data_buff;
    int ret;
    uint8_t *p;
    user_config_struct_header_t *header;
    int i;
#ifdef USE_CONFIG_STORAGE_CRC
    uint32_t crc = 0;
#endif

    mem_data_buff = (uint8_t*) ke_malloc(size , KE_MEM_NON_RETENTION);
    ret = user_config_storage_read_ext_mem( (uint8_t*)mem_data_buff, config_offset, size);
    if (ret < 0)
        return USER_CONFIG_STORAGE_READ_FAILURE;

    header = (user_config_struct_header_t *)mem_data_buff;
    p = &mem_data_buff[sizeof(user_config_struct_header_t)];

    for (i = 0 ; i < config_data->params_num; i++)
    {
        p[0] = (uint8_t)config_data->params[i].elem_id;
        p[1] = (uint8_t)(config_data->params[i].elem_id>>8);
        p[2] = config_data->params[i].cur_size;
        memcpy(&p[3], (uint8_t*)config_data->params[i].data, p[2]);
        p += (3 + p[2]);
    }
    header->signature[0] = CONFIGURATION_HEADER_SIGNATURE1;
    header->signature[1] = CONFIGURATION_HEADER_SIGNATURE2;
    header->valid = STATUS_INVALID_CONFIG;
    header->struct_id = current_config_struct_id + 1;
    memcpy(header->version, spss_conf_struct_version, 16);
    header->data_size = size - (uint32_t)sizeof(user_config_struct_header_t);
    header->num_of_items = config_data->params_num;
    header->encr = 0;

#ifdef USE_CONFIG_STORAGE_CRC
    crc = crc32(0, &mem_data_buff[sizeof(user_config_struct_header_t)], (size_t)header->data_size);
    header->crc = crc;
#endif

    ret = user_flash_write_data((uint8_t*)mem_data_buff, config_offset, size);
    if (ret < 0 )
    {
        ke_free(mem_data_buff);
        return USER_CONFIG_STORAGE_WRITE_FAILURE;
    }
    header->valid = STATUS_VALID_CONFIG;
    ret = user_flash_write_data(&header->valid, config_offset+2, 1);
    ke_free(mem_data_buff);
    if (ret < 0 )
        return USER_CONFIG_STORAGE_WRITE_FAILURE;

    return USER_CONFIG_STORAGE_SUCCESS;
}

#endif //CFG_CONFIG_STORAGE 

/// @} CONFIG_STORAGE
