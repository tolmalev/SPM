/*
 * register names
 */

#define ChannelOperation_  I , 0x0   , write , 32 , 0x7fffff20 , 0 ,0
#define ChannelControl_    I , 0x4   , write , 32 , 0x3cfcffff , 0 ,0
#define ChannelStatus_     I , 0x3c  ,  read , 32 , .no..mask. , 0 ,0
#define BaseCount_         I , 0x2c  , write , 32 , 0xffffffff , 0 ,0
#define TransferCount_     I , 0x8   , write , 32 , 0xffffffff , 0 ,0
#define FifoCount_         I , 0x40  ,  read , 32 , .no..mask. , 0 ,0
#define MemoryConfig_      I , 0xc   , write , 32 , 0xffffffff , 0 ,0
#define DeviceConfig_      I , 0x14  , write , 32 , 0xffffffff , 0 ,0
#define BaseAddress_       I , 0x28  , write , 32 , 0xffffffff , 0 ,0
#define MemoryAddress_     I , 0x10  , write , 32 , 0xffffffff , 0 ,0
#define DeviceAddress_     I , 0x18  , write , 32 , 0xffffffff , 0 ,0

/* Shadow registers */

struct shadow_I_registers {
 u32 ChannelOperation[1];
 u32 ChannelControl[1];
 u32 ChannelStatus[1];
 u32 BaseCount[1];
 u32 TransferCount[1];
 u32 FifoCount[1];
 u32 MemoryConfig[1];
 u32 DeviceConfig[1];
 u32 BaseAddress[1];
 u32 MemoryAddress[1];
 u32 DeviceAddress[1];
};

/*
 * register fields
 */

#define ChannelOperation__Start           0x0   ,  0xfffffffe
#define ChannelOperation__Cont            0x1   ,  0xfffffffd
#define ChannelOperation__Stop            0x2   ,  0xfffffffb
#define ChannelOperation__Abort           0x3   ,  0xfffffff7
#define ChannelOperation__FReset          0x4   ,  0xffffffef
#define ChannelOperation__ClrRB           0x6   ,  0xffffffbf
#define ChannelOperation__ClrDone         0x7   ,  0xffffff7f
#define ChannelOperation__DmaReset        0x1f  ,  0x7fffffff
#define ChannelControl__XMode             0x0   ,  0xfffffff8
#define ChannelControl__Dir               0x3   ,  0xfffffff7
#define ChannelControl__BurstEnable       0xe   ,  0xffffbfff
#define ChannelControl__ClrContinueIE     0x10  ,  0xfffeffff
#define ChannelControl__SetContinueIE     0x11  ,  0xfffdffff
#define ChannelControl__ClrDoneIE         0x18  ,  0xfeffffff
#define ChannelControl__SetDoneIE         0x19  ,  0xfdffffff
#define ChannelControl__ClrDmaIE          0x1e  ,  0xbfffffff
#define ChannelControl__SetDmaIE          0x1f  ,  0x7fffffff
#define ChannelStatus__DeviceErr          0x0   ,  0xfffffffc
#define ChannelStatus__MemoryErr          0x2   ,  0xfffffff3
#define ChannelStatus__TransferErr        0x9   ,  0xfffffdff
#define ChannelStatus__OperationErr       0xa   ,  0xfffff3ff
#define ChannelStatus__StoppedStatus      0xc   ,  0xffffefff
#define ChannelStatus__SoftwareAbort      0xe   ,  0xffffbfff
#define ChannelStatus__Error              0xf   ,  0xffff7fff
#define ChannelStatus__ContinueStatus     0x11  ,  0xfffdffff
#define ChannelStatus__DmaDone            0x19  ,  0xfdffffff
#define ChannelStatus__Interrupting       0x1f  ,  0x7fffffff
#define BaseCount__Value                  0x0   ,  0x0       
#define TransferCount__Value              0x0   ,  0x0       
#define FifoCount__FifoCR                 0x0   ,  0xffffff00
#define FifoCount__EmptyCR                0x10  ,  0xff00ffff
#define MemoryConfig__PortSize            0x8   ,  0xfffffcff
#define DeviceConfig__PortSize            0x8   ,  0xfffffcff
#define DeviceConfig__ReqSource           0x10  ,  0xfff8ffff
#define BaseAddress__Value                0x0   ,  0x0       
#define MemoryAddress__Value              0x0   ,  0x0       
#define DeviceAddress__Value              0x0   ,  0x0       

/*
 * constants
 */

#define _kStart 1
#define _kCont 2
#define _kStop 3
#define _kAbort 4
#define _kFReset 5
#define _kClrRB 7
#define _kClrDone 8
#define _kDmaReset 10
         
#define _kXMode 1
#define _kDir 2
#define _kBurstEnable 4
#define _kClrContinueIE 6
#define _kSetContinueIE 7
#define _kClrDoneIE 9
#define _kSetDoneIE 10
#define _kClrDmaIE 12
#define _kSetDmaIE 13
         
#define _kDeviceErr 1
#define _kMemoryErr 2
#define _kTransferErr 4
#define _kOperationErr 5
#define _kStoppedStatus 6
#define _kSoftwareAbort 8
#define _kError 9
#define _kContinueStatus 11
#define _kDmaDone 13
#define _kInterrupting 15
         
#define _kFifoCR 1
#define _kEmptyCR 3
         
#define _kPortSize 2
         
#define _kPortSize 2
#define _kReqSource 4
         
#define _kChannelOperation 0
#define _kChannelControl 1
#define _kChannelStatus 2
#define _kBaseCount 3
#define _kTransferCount 4
#define _kFifoCount 5
#define _kMemoryConfig 6
#define _kDeviceConfig 7
#define _kBaseAddress 8
#define _kMemoryAddress 9
#define _kDeviceAddress 10
#define _kLastRegister 10

#define _kDirIn 0
#define _kDirOut 1
