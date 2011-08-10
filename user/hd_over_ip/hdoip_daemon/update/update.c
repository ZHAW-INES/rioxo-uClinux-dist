/*
 * update.c
 *
 *  Created on: 11.01.2011
 *      Author: alda
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hdoipd.h"
#include "altremote.h"
#include "flash.h"
#include "update.h"


#define BLOCK_SIZE          (4096)
#define FLASH_DEV_NAME      "/dev/mtd0"


typedef struct {
    uint8_t     id[4];
    uint32_t    hdr_length;

    uint32_t    version;
    uint64_t    offset;
} __attribute__((__packed__)) t_file_header;

#define HDR_A   (8)
#define HDR_B   (sizeof(t_file_header) - 8)

static int update_hdr(t_file_header *hdr, FILE *f)
{
    int ret;

    // id / length field
    if ((ret = fread(hdr, 1, HDR_A, f)) != HDR_A) { 
        return -1;
    }

    if (memcmp(hdr->id, "INES", 4)) { 
        return -1;
    }

    // rest of header
    if ((ret = fread(&hdr->version, 1, HDR_B, f)) != HDR_B) {
        return -1;
    }

    int len = ((hdr->hdr_length > HDR_B) ? (hdr->hdr_length - HDR_B) : (0));
    uint8_t buf[len];

    // unsupported header
    if (len) {
        if ((ret = fread(buf, 1, len, f)) != len) {
            return -1;
        }
    } 

    report("\n ID       : %s\n hdr_size : %d bytes\n", hdr->id, hdr->hdr_length);
    report(" version  : %08x\n offset   : %016x\n\n", hdr->version, hdr->offset);

    return 0;
}

static int update(uint64_t addr, FILE *f)
{
    char *block;
    struct flash_dev dev;
    int ret;
    size_t rd_size;
    size_t len;
    uint64_t off = addr;

    if (!f) return -1;

    block = malloc(BLOCK_SIZE);
    if(block == NULL) {
        report("Could not allocate buffer\n");
        return -1;
    }

    ret = flash_dev_open(&dev, FLASH_DEV_NAME, O_RDWR);
    if (ret == -1) {
        report("Could not open %s\n", FLASH_DEV_NAME);
        return ret;
    } 

    // disable watchdog
    hoi_drv_wdg_disable();

    // erase flash 
    report("erasing flash...\n");
    ret = flash_dev_eraseall(&dev);
    if(ret == -1) {   
        report("flash erase failed\n");
        return -1;
    }

    // write data until block alignment
    uint64_t base = (addr/BLOCK_SIZE)*BLOCK_SIZE; // block base address
    size_t align = addr - base; // offset to base
    if (align) {
        report("align needed (untested!)\n");
        // read mixed block
        flash_dev_read(&dev, block, BLOCK_SIZE, base);
        len = BLOCK_SIZE - align;
        ret = fread(block + align, 1, len, f);
        if (ret != (ssize_t)len) {
            flash_dev_close(&dev);
            report("align failed\n");
            return ret;
        }
        ret = flash_dev_write_block(&dev, block, len, base);
        off = base + BLOCK_SIZE;
    }

    report("writing configuration to flash...\n");
    rd_size = (size_t) BLOCK_SIZE;
    while (rd_size == BLOCK_SIZE) {
        rd_size = fread(block, 1, BLOCK_SIZE, f);
        if (rd_size <= 0) break;
        ret = flash_dev_write_block(&dev, block, rd_size, off);
        if (ret <= 0) break;
        off += (uint64_t) rd_size;
    }

    flash_dev_close(&dev);
    free(block);
    return 0;
}

int update_flash(char* filename)
{
    int ret = -1;
    int i;
    t_file_header hdr;
    FILE* f = fopen(filename, "r");

    if (f != NULL) {
        ret = update_hdr(&hdr, f);
        if (ret != 0) {
            report("Failed to read header\n");
            return ret;
        }

        ret = update(hdr.offset, f);
        if(ret != 0) {
            report("Remote update failed\n");
            return ret;
        }
        if(altremote_is_loaded()) {
            report("\n\n");
            for(i= 5; i >0 ; i--) {
                fprintf(stdout, "\nTriggering FPGA reconfiguration in %d\n", i);
                fflush(stdout);
                sleep(1);
            }
            altremote_trigger_reconfig();
        } else {
            report("altremote driver is not loaded, could not reset FPGA\n");
        }
        fclose(f);
    } else {
        report("Could not open %s\n", filename);
    }

    return ret;
}

