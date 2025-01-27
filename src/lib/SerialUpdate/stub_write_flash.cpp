/*
 * SPDX-FileCopyrightText: 2016 Cesanta Software Limited
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * SPDX-FileContributor: 2016-2022 Espressif Systems (Shanghai) CO LTD
 */

#if defined(PLATFORM_ESP32) && defined(TARGET_RX)

#include "stub_write_flash.h"
#include "rom/miniz.h"
#include "slip.h"
#include "soc_support.h"
#include "stub_flasher.h"
#include "targets.h"

#include <Update.h>

/* local flashing state

   This is wrapped in a structure because gcc 4.8
   generates significantly more code for ESP32
   if they are static variables (literal pool, I think!)
*/
static struct
{
    /* set by flash_begin, cleared by flash_end */
    bool in_flash_mode;
    /* number of output bytes remaining to write */
    uint32_t remaining;
    /* last error generated by a data packet */
    esp_command_error last_error;

    uint8_t *last_buf;
    uint32_t last_length;
    uint32_t last_end;
    MD5Builder md5;

    /* inflator state for deflate write */
    tinfl_decompressor inflator;
    /* number of compressed bytes remaining to read */
    uint32_t remaining_compressed;
} fs;

bool is_in_flash_mode(void)
{
    return fs.in_flash_mode;
}

esp_command_error get_flash_error(void)
{
    return fs.last_error;
}

esp_command_error handle_flash_get_md5sum(uint32_t addr, uint32_t len)
{
    /* ESP32 ROM sends as hex, but stub just send raw bytes - esptool.py can handle either. */
    fs.md5.add(fs.last_buf, len - fs.last_end);
    fs.md5.calculate();
    uint8_t md5[16];
    fs.md5.getBytes(md5);
    SLIP_send_frame_data_buf(md5, sizeof(md5));
    return ESP_UPDATE_OK;
}

esp_command_error handle_flash_begin(uint32_t total_size, uint32_t offset)
{
    fs.in_flash_mode = true;
    fs.remaining = total_size;
    if (total_size != 0)
    {
        Update.begin(total_size);
        fs.last_buf = static_cast<uint8_t *>(malloc(32768));
        fs.md5.begin();
    }
    fs.last_length = 0;
    fs.last_end = 0;
    return ESP_UPDATE_OK;
}

esp_command_error handle_flash_deflated_begin(uint32_t uncompressed_size, uint32_t compressed_size, uint32_t offset)
{
    esp_command_error err = handle_flash_begin(uncompressed_size, offset);
    tinfl_init(&fs.inflator);
    fs.remaining_compressed = compressed_size;
    return err;
}

void handle_flash_data(uint8_t *data_buf, uint32_t length)
{
    if (fs.last_length)
    {
        fs.md5.add(fs.last_buf, fs.last_length);
        fs.last_end += fs.last_length;
    }
    if (length > fs.remaining)
    {
        /* Trim the final block, as it may have padding beyond
            the length we are writing */
        length = fs.remaining;
    }
    memcpy(fs.last_buf, data_buf, length);
    fs.last_length = length;
    fs.remaining -= length;
    Update.write(data_buf, length);

    fs.last_error = ESP_UPDATE_OK;
}

void handle_flash_deflated_data(uint8_t *data_buf, uint32_t length)
{
    static uint8_t *out_buf = nullptr;
    static uint8_t *next_out = nullptr;
    int status = TINFL_STATUS_NEEDS_MORE_INPUT;

    const size_t out_size = 32768;
    if (out_buf == nullptr)
    {
        out_buf = static_cast<uint8_t *>(malloc(out_size));
        next_out = out_buf;
    }
    while (length > 0 && fs.remaining > 0 && status > TINFL_STATUS_DONE)
    {
        size_t in_bytes = length;                                /* input remaining */
        size_t out_bytes = out_buf + out_size - next_out; /* output space remaining */
        int flags = TINFL_FLAG_PARSE_ZLIB_HEADER;
        if (fs.remaining_compressed > length)
        {
            flags |= TINFL_FLAG_HAS_MORE_INPUT;
        }

        status = tinfl_decompress(&fs.inflator, data_buf, &in_bytes, out_buf, next_out, &out_bytes, flags);

        fs.remaining_compressed -= in_bytes;
        length -= in_bytes;
        data_buf += in_bytes;

        next_out += out_bytes;
        size_t bytes_in_out_buf = next_out - out_buf;
        if (status == TINFL_STATUS_DONE || bytes_in_out_buf == out_size)
        {
            // Output buffer full, or done
            handle_flash_data(out_buf, bytes_in_out_buf);
            next_out = out_buf;
        }
    } // while

    if (status < TINFL_STATUS_DONE)
    {
        /* error won't get sent back to esptool.py until next block is sent */
        fs.last_error = ESP_INFLATE_ERROR;
    }

    if (status == TINFL_STATUS_DONE && fs.remaining > 0)
    {
        fs.last_error = ESP_NOT_ENOUGH_DATA;
    }
    if (status != TINFL_STATUS_DONE && fs.remaining == 0)
    {
        fs.last_error = ESP_TOO_MUCH_DATA;
    }
}

esp_command_error handle_flash_end(void)
{
    if (!fs.in_flash_mode)
    {
        return ESP_NOT_IN_FLASH_MODE;
    }

    if (fs.remaining > 0)
    {
        return ESP_NOT_ENOUGH_DATA;
    }

    fs.in_flash_mode = false;

    if (!Update.end(true))
    {
        switch (Update.getError())
        {
        case UPDATE_ERROR_SPACE:
            return ESP_TOO_MUCH_DATA;
        case UPDATE_ERROR_SIZE:
            return ESP_NOT_ENOUGH_DATA;
        case UPDATE_ERROR_MD5:
            return ESP_BAD_DATA_CHECKSUM;
        default:
            return ESP_INVALID_COMMAND;
        }
    }
    return fs.last_error;
}
#endif