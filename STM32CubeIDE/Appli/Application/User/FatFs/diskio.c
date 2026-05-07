/**
 ****************************************************************************************************
 * @file        diskio.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2025-01-13
 * @brief       FATFS底层(diskio)驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 * 
 * 实验平台:正点原子 N647开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 * 
 ****************************************************************************************************
 */

#include "ff.h"
#include "diskio.h"
#include "./SD_NAND/sd_nand.h"
#include "./SD_CARD/sd_card.h"

#define SD_CARD 0   /* SD卡,卷标为0 */
#define SD_NAND 1   /* SD NAND,卷标为1 */

/**
 * @brief   获得磁盘状态
 * @param   pdrv : 磁盘编号0~9
 * @retval  执行结果(参见FATFS, DSTATUS的定义)
 */
DSTATUS disk_status(BYTE pdrv)
{
    return RES_OK;
}

/**
 * @brief   初始化磁盘
 * @param   pdrv : 磁盘编号0~9
 * @retval  执行结果(参见FATFS, DSTATUS的定义)
 */
DSTATUS disk_initialize(BYTE pdrv)
{
    uint8_t res = 0;

    switch (pdrv)
    {
        case SD_CARD:
        {
            res = sd_card_init();
            break;
        }
        case SD_NAND:
        {
            res = sd_nand_init();
            break;
        }
        default:
        {
            res = 1;
            break;
        }
    }

    if (res != 0)
    {
        return RES_ERROR;
    }

    return RES_OK;
}

/**
 * @brief   读扇区
 * @param   pdrv   : 磁盘编号0~9
 * @param   buff   : 数据接收缓冲首地址
 * @param   sector : 扇区地址
 * @param   count  : 需要读取的扇区数
 * @retval  执行结果(参见FATFS, DRESULT的定义)
 */
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    uint8_t res = 0;

    switch (pdrv)
    {
        case SD_CARD:
        {
            res = sd_card_read_disk(buff, sector, count);
            break;
        }
        case SD_NAND:
        {
            res = sd_nand_read_disk(buff, sector, count);
            break;
        }
        default:
        {
            res = 1;
            break;
        }
    }

    if (res != 0)
    {
        return RES_ERROR;
    }

    return RES_OK;
}

#if FF_FS_READONLY == 0
/**
 * @brief   写扇区
 * @param   pdrv   : 磁盘编号0~9
 * @param   buff   : 发送数据缓存区首地址
 * @param   sector : 扇区地址
 * @param   count  : 需要写入的扇区数
 * @retval  执行结果(参见FATFS, DRESULT的定义)
 */
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    uint8_t res = 0;

    switch (pdrv)
    {
        case SD_CARD:
        {
            res = sd_card_write_disk((uint8_t *)buff, sector, count);
            break;
        }
        case SD_NAND:
        {
            res = sd_nand_write_disk((uint8_t *)buff, sector, count);
            break;
        }
        default:
        {
            res = 1;
            break;
        }
    }

    if (res != 0)
    {
        return RES_ERROR;
    }

    return RES_OK;
}
#endif

/**
 * @brief   获取其他控制参数
 * @param   pdrv   : 磁盘编号0~9
 * @param   ctrl   : 控制代码
 * @param   buff   : 发送/接收缓冲区指针
 * @retval  执行结果(参见FATFS, DRESULT的定义)
 */
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res = 0;

    switch (pdrv)
    {
        case SD_CARD:
        {
            switch (cmd)
            {
                case CTRL_SYNC:
                {
                    res = RES_OK;
                    break;
                }
                case GET_SECTOR_SIZE:
                {
                    *(DWORD *)buff = 512;
                    res = RES_OK;
                    break;
                }
                case GET_BLOCK_SIZE:
                {
                    *(WORD *)buff = g_sd_card_info_struct.LogBlockSize;
                    res = RES_OK;
                    break;
                }
                case GET_SECTOR_COUNT:
                {
                    *(DWORD *)buff = g_sd_card_info_struct.LogBlockNbr;
                    res = RES_OK;
                    break;
                }
                default:
                {
                    res = RES_PARERR;
                    break;
                }
            }
            break;
        }
        case SD_NAND:
        {
            switch (cmd)
            {
                case CTRL_SYNC:
                {
                    res = RES_OK;
                    break;
                }
                case GET_SECTOR_SIZE:
                {
                    *(DWORD *)buff = 512;
                    res = RES_OK;
                    break;
                }
                case GET_BLOCK_SIZE:
                {
                    *(WORD *)buff = g_sd_nand_info_struct.LogBlockSize;
                    res = RES_OK;
                    break;
                }
                case GET_SECTOR_COUNT:
                {
                    *(DWORD *)buff = g_sd_nand_info_struct.LogBlockNbr - SD_NAND_FONT_BLK_NUM;
                    res = RES_OK;
                    break;
                }
                default:
                {
                    res = RES_PARERR;
                    break;
                }
            }
            break;
        }
        default:
        {
            res = RES_ERROR;
            break;
        }
    }
    
    return res;
}
