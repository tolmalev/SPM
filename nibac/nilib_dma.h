
/* dma constants */

#define DMA_8_BIT  1
#define DMA_16_BIT 2
#define DMA_32_BIT 3

#define DMA_NORMAL        0
#define DMA_RING          1
#define DMA_LINKCHAIN     2
#define DMA_LINKCHAINRING 3
#define DMA_NONE          4

#define DMA_IN  0
#define DMA_OUT 1

#define DMA_UNKNOWN    0
#define DMA_IDLE       1
#define DMA_CONFIGURED 2
#define DMA_STARTED    3
#define DMA_STOPPED    4


#define DMA_NO_ERROR            0
#define DMA_BUFFER_OVERFLOW     1
#define DMA_BUFFER_UNDERFLOW    2
#define DMA_DATA_NOT_AVAILABLE  3
#define DMA_SPACE_NOT_AVAILABLE 4
#define DMA_WRONG_STATE         5
#define DMA_INVALID_CHANNEL     6
#define DMA_INVALID_INPUT       7

struct linear_dma_buffer {
  //        void * bus;
  //        void * memory;
  //        u32 size;           /* buffer size / bytes */
        unsigned long physical_address;
        void * virtual_address;
        u32 index;
};

struct dma_channel {
        u8  * memap[2];
        int channel;
        u32 read_idx;
        u32 write_idx;
        int direction;
        int drq;
        int transfer_width;   /* data size 8, 16 or 32 bit */
        int mode;
        int size;   /* dma buffer size in bytes */
        int state;
        int fd;
        int status;
        void * physical_address;
        struct linear_dma_buffer buffer;
};

/* dma functions */

int dma_config (struct dma_channel *, int, int, int, int, int);
int dma_start (struct dma_channel *);
int dma_read (struct dma_channel *, u32, u8 *, u32 *);
int dma_stop (struct dma_channel *);
int dma_reset (struct dma_channel *);
void dma_configure (struct dma_channel *);
u32 dma_get_bytes_in_buffer (struct dma_channel *);

int allocate_dma_buffer (struct dma_channel *);
void dma_buffer_read (struct dma_channel *, int, u8 *);

/*
dma_write
*/
