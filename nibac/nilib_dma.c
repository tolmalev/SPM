
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "spm_types.h"
#include "nilib_dma.h"
#include "tMite.h"
#include "register_macro.h"

#define NAM "nilib_dma"


/* talk with the Mite chip and set up for DMA work */

void dma_configure (struct dma_channel *dma) {

        unsigned long start_address;

        NIBIT (dma->memap, ChannelControl, set, BurstEnable, 1);
	NIBIT (dma->memap, ChannelControl, set, Dir, 
	       ((dma->direction) == _kDirIn ? 1 : 0));
	       
	NIBIT (dma->memap, ChannelControl, set, SetDmaIE, 1);
	NIBIT (dma->memap, ChannelControl, set, SetContinueIE, 1);

	NIREG (dma->memap, MemoryConfig, set, 0x00E00400);
	NIBIT (dma->memap, MemoryConfig, set, PortSize, dma->transfer_width);

	NIREG (dma->memap, DeviceConfig, set, 0x00000440);
	NIBIT (dma->memap, DeviceConfig, set, PortSize, dma->transfer_width);
	NIBIT (dma->memap, DeviceConfig, set, ReqSource, (4 + dma->drq));

        start_address = (unsigned long) dma->physical_address;
	printf ("dma_configure: physical_address %lx %ld\n",
		start_address, start_address);

        if ( dma->mode == DMA_NORMAL) {

                NIBIT (dma->memap, ChannelControl, set, XMode, 0);

                NIREG (dma->memap, MemoryAddress, write, start_address);
                NIREG (dma->memap, TransferCount, write, dma->size);

                NIREG (dma->memap, BaseAddress, write, start_address);
                NIREG (dma->memap, BaseCount, write, dma->size);


        } else if ( dma->mode == DMA_RING) {

                NIBIT (dma->memap, ChannelControl, set, XMode, 2);

                NIREG (dma->memap, MemoryAddress, write, start_address);
                NIREG (dma->memap, TransferCount, write, dma->size);

                NIREG (dma->memap, BaseAddress, write, start_address);
                NIREG (dma->memap, BaseCount, write, dma->size);
        } else {
                printf ("dma_configure: unexpected mode: %d\n", dma->mode);
                exit (-1);
        }

        NIREG (dma->memap, ChannelControl, flush);
        NIREG (dma->memap, MemoryConfig, flush);
        NIREG (dma->memap, DeviceConfig, flush);

        NIREG (dma->memap, DeviceAddress, write, 0);

        dma->buffer.index = 0;
        dma->read_idx  = 0;
        dma->write_idx = 0;

        dma->state = DMA_CONFIGURED;
        return;
}

#if 0

#define NIRLP_IOCTL_ALLOCATE_DMA_BUFFER 0xc004bb02
#define NIRLP_IOCTL_FREE_DMA_BUFFER     0x4004bb03

int dma_config (struct dma_channel *dma,
                int source,
                int mode,
                int direction,
                int size,
                int width) {
                
        printf ("dma_config: enter\n");

        if (dma->state != DMA_IDLE) return DMA_WRONG_STATE;

        /* save configuration */

        dma->mode = mode;
        dma->direction = direction;
        dma->drq = source;
        dma->size = size;  /* dma buffer size in bytes */
        dma->transfer_width = width;  /* data size: 8,16,32 bit */

        /* allocate DMA buffer */

        if (allocate_dma_buffer (dma) < 0)
                return DMA_SPACE_NOT_AVAILABLE;

        dma_configure (dma);
        dma->state = DMA_CONFIGURED;
        return DMA_NO_ERROR;
}

int dma_start (struct dma_channel *dma) {

        if (dma->state == DMA_STOPPED) dma_configure (dma);

        if (dma->state != DMA_CONFIGURED) return DMA_WRONG_STATE;

        NIBIT (dma->memap, ChannelOperation, write, Start, 1);

        dma->state = DMA_STARTED;

        return DMA_NO_ERROR;
}

int dma_read (struct dma_channel *dma,
              u32 requested_bytes,
              u8 *user_buffer,
              u32 *bytes_left) {

        u32 bytes_in_buffer;

        if (dma->state == DMA_IDLE || dma->state == DMA_UNKNOWN)
                return DMA_WRONG_STATE;

        bytes_in_buffer = dma_get_bytes_in_buffer (dma);

        if (bytes_in_buffer > dma->size)
                return DMA_BUFFER_OVERFLOW;

        if (requested_bytes == 0 || user_buffer == 0) {
                *bytes_left = bytes_in_buffer;
                return DMA_NO_ERROR;
        }

        if (requested_bytes > bytes_in_buffer)
                return DMA_DATA_NOT_AVAILABLE;

        dma_buffer_read (dma, requested_bytes, user_buffer);

        bytes_in_buffer = dma_get_bytes_in_buffer(dma);

        if (bytes_in_buffer > dma->size)
                return DMA_BUFFER_OVERFLOW;

        dma->read_idx += requested_bytes;
        bytes_in_buffer = dma_get_bytes_in_buffer(dma);

        *bytes_left = bytes_in_buffer;

        return DMA_NO_ERROR;
}

int dma_stop (struct dma_channel *dma) {

        NIBIT (dma->memap, ChannelOperation, write, Stop, 1);

        dma->state = DMA_STOPPED;
        return DMA_NO_ERROR;
}

int dma_reset (struct dma_channel *dma) {

        if (dma->state == DMA_STARTED) dma_stop (dma);

        NIBIT (dma->memap, ChannelOperation, write, DmaReset, 1);

        NIREG (dma->memap, ChannelOperation, write, 0);
        NIREG (dma->memap, ChannelControl, write, 0);

        NIREG (dma->memap, BaseCount, write, 0);
        NIREG (dma->memap, TransferCount, write, 0);

        NIREG (dma->memap, MemoryConfig, write, 0);
        NIREG (dma->memap, DeviceConfig, write, 0);

        NIREG (dma->memap, BaseAddress, write, 0);
        NIREG (dma->memap, MemoryAddress, write, 0);
        NIREG (dma->memap, DeviceAddress, write, 0);

        /* clear something in dma->buffer? */

        dma->read_idx = 0;
        dma->write_idx = 0;
        dma->transfer_width = DMA_16_BIT;
        dma->state = DMA_IDLE;

        return DMA_NO_ERROR;
}

/* talk with the Mite chip and set up for DMA work */

void dma_configure (struct dma_channel *dma) {

        u32 start_address;

        NIBIT (dma->memap, ChannelControl, set, BurstEnable, 1);
	NIBIT (dma->memap, ChannelControl, set, Dir,
	       ((dma->direction) == _kDirIn ? 1 : 0));

	NIREG (dma->memap, MemoryConfig, set, 0x00E00400);
	NIBIT (dma->memap, MemoryConfig, set, PortSize, dma->transfer_width);

	NIREG (dma->memap, DeviceConfig, set, 0x00000440);
	NIBIT (dma->memap, DeviceConfig, set, PortSize, dma->transfer_width);
	NIBIT (dma->memap, DeviceConfig, set, ReqSource, (4 + dma->drq));

        if ( dma->mode == DMA_NORMAL) {

                start_address = dma->buffer.physical_address;

                NIBIT (dma->memap, ChannelControl, set, XMode, 0);

                NIREG (dma->memap, MemoryAddress, write, start_address);
                NIREG (dma->memap, TransferCount, write, dma->size);

                NIREG (dma->memap, BaseAddress, write, start_address);
                NIREG (dma->memap, BaseCount, write, dma->size);


        } else if ( dma->mode == DMA_RING) {

                start_address = dma->buffer.physical_address;

                NIBIT (dma->memap, ChannelControl, set, XMode, 2);

                NIREG (dma->memap, MemoryAddress, write, start_address);
                NIREG (dma->memap, TransferCount, write, dma->size);

                NIREG (dma->memap, BaseAddress, write, start_address);
                NIREG (dma->memap, BaseCount, write, dma->size);
        } else {
        
                printf ("dma_configure: unexpected mode: %d\n", dma->mode);
                exit (-1);
        }

        NIREG (dma->memap, ChannelControl, flush);
        NIREG (dma->memap, MemoryConfig, flush);
        NIREG (dma->memap, DeviceConfig, flush);

        NIREG (dma->memap, DeviceAddress, write, 0);

        dma->buffer.index = 0;
        dma->read_idx  = 0;
        dma->write_idx = 0;

        dma->state = DMA_CONFIGURED;
        return;
}

int allocate_dma_buffer (struct dma_channel *dma) {

    unsigned long physical_address;
    void * virtual_address;

    physical_address = (unsigned long) dma->size;

    if (ioctl (dma->fd, NIRLP_IOCTL_ALLOCATE_DMA_BUFFER, &physical_address) <0)
            return -1;

    virtual_address  = mmap (0, dma->size, PROT_READ|PROT_WRITE, MAP_SHARED,
                             dma->fd, physical_address);
    if (virtual_address == MAP_FAILED) {
        ioctl (dma->fd, NIRLP_IOCTL_FREE_DMA_BUFFER, &physical_address);
        return -1;
    }

    dma->buffer.physical_address = physical_address;
    dma->buffer.virtual_address = virtual_address;
    
    printf ("allocate_dma_buffer: physical %lx   virtual %p   size %d\n",
    	 physical_address, virtual_address, dma->size);

    return 0;
}

u32 dma_get_bytes_in_buffer (struct dma_channel *dma) {

        /*   1. Update MITE's location  */

        if (dma->direction == DMA_IN) {

                dma->write_idx  = NIREG (dma->memap,DeviceAddress,refresh);
                dma->write_idx -= NIBIT (dma->memap,FifoCount,read,FifoCR);

        } else {
                dma->read_idx = NIREG (dma->memap,DeviceAddress,refresh);
        }

        /*  2. Calculate difference between read and write indexes
            checking for rollovers  */

        u32 bytesInBuffer = 0;

        if (dma->write_idx < dma->read_idx) {
                bytesInBuffer = 0xffffffff - ( dma->read_idx - dma->write_idx);
                ++bytesInBuffer;
        } else {
                bytesInBuffer = dma->write_idx - dma->read_idx;
        }
//        printf ("dma_get_bytes_in_buffer: bytes %d\n", bytesInBuffer);
        
        return bytesInBuffer;
}

void dma_buffer_read (struct dma_channel *dma,
                      int requested_bytes,
                      u8 *buffer) {

        u32 current, end, offset;
        u8 * address;

        current = dma->buffer.index;
        end = (current + requested_bytes) % dma->size;
        address = dma->buffer.virtual_address;
        offset = 0;

        /* u8 *buffer is a circular buffer;
           copy upper section first, if any */

        if (end <= current) {
                offset = dma->size - current;
                memcpy (buffer, address + current, offset);
                buffer = buffer + offset;
                current = 0;
        }

        memcpy (buffer, address + current, (end - current));
        dma->buffer.index = end;
}

void dma_dump_status (struct dma_channel *dma) {

        printf ("dma_dump_status:ChannelOperation %x\n",
                NIREG (dma->memap,ChannelOperation,refresh));
        printf ("dma_dump_status:ChannelControl   %x\n",
                NIREG (dma->memap,ChannelControl,  refresh));
        printf ("dma_dump_status:ChannelStatus    %x\n",
                NIREG (dma->memap,ChannelStatus,   refresh));
        printf ("dma_dump_status:BaseCount        %x\n", 
                NIREG (dma->memap,BaseCount,       refresh));
        printf ("dma_dump_status:TransferCount    %x\n", 
                NIREG (dma->memap,TransferCount,   refresh));
        printf ("dma_dump_status:FifoCount        %x\n", 
                NIREG (dma->memap,FifoCount,       refresh));
        printf ("dma_dump_status:MemoryConfig     %x\n", 
                NIREG (dma->memap,MemoryConfig,    refresh));
        printf ("dma_dump_status:DeviceConfig     %x\n", 
                NIREG (dma->memap,DeviceConfig,    refresh));
        printf ("dma_dump_status:BaseAddress      %x\n", 
                NIREG (dma->memap,BaseAddress,     refresh));
        printf ("dma_dump_status:MemoryAddress    %x\n", 
                NIREG (dma->memap,MemoryAddress,   refresh));
        printf ("dma_dump_status:DeviceAddress    %x\n", N
                IREG (dma->memap,DeviceAddress,   refresh));

}

#endif
