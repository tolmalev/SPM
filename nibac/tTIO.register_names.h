/*
 *
 * The following tables are derived from National Instruments examples
 * under their Copyright:
 *
 * Copyright (c) 2006, National Instruments Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of National Instruments Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define G0_AutoIncrement_          T , 0x88  , write , 16 , 0xffffffff , 0 ,0
#define G0_Command_                T , 0xc   , write , 16 , 0xffff5be2 , 0 ,0
#define G0_Counting_Mode_          T , 0xb0  , write , 16 , 0xffffffff , 0 ,0
#define G0_MSeries_Counting_Mode_  T , 0xb0  , write , 16 , 0xffffffff , 0 ,0
#define G0_DMA_Control_            T , 0xb8  , write , 16 , 0xffffffff , 0 ,0
#define G0_Input_Select_           T , 0x48  , write , 16 , 0xffffffff , 0 ,0
#define G0_Load_A_                 T , 0x38  , write , 32 , 0xffffffff , 0 ,0
#define G0_Load_B_                 T , 0x3c  , write , 32 , 0xffffffff , 0 ,0
#define G0_Mode_                   T , 0x34  , write , 16 , 0xffffffff , 0 ,0
#define G0_Second_Gate_            T , 0xb4  , write , 16 , 0xffffffff , 0 ,0
#define G0_MSeries_ABZ_            T , 0xc0  , write , 16 , 0xffffffff , 0 ,0
#define G01_Joint_Reset_           T , 0x90  , write , 16 , 0xfffffff3 , 0 ,0
#define G1_AutoIncrement_          T , 0x8a  , write , 16 , 0xffffffff , 0 ,0
#define G1_Command_                T , 0xe   , write , 16 , 0xffff5bea , 0 ,0
#define G1_Counting_Mode_          T , 0xb2  , write , 16 , 0xffffffff , 0 ,0
#define G1_MSeries_Counting_Mode_  T , 0xb2  , write , 16 , 0xffffffff , 0 ,0
#define G1_DMA_Control_            T , 0xba  , write , 16 , 0xffffffff , 0 ,0
#define G1_Input_Select_           T , 0x4a  , write , 16 , 0xffffffff , 0 ,0
#define G1_Load_A_                 T , 0x40  , write , 32 , 0xffffffff , 0 ,0
#define G1_Load_B_                 T , 0x44  , write , 32 , 0xffffffff , 0 ,0
#define G1_Mode_                   T , 0x36  , write , 16 , 0xffffffff , 0 ,0
#define G1_Second_Gate_            T , 0xb6  , write , 16 , 0xffffffff , 0 ,0
#define G1_MSeries_ABZ_            T , 0xc2  , write , 16 , 0xffffffff , 0 ,0
#define Interrupt_G0_Ack_          T , 0x4   , write , 16 , 0xffff3f9f , 0 ,0
#define Interrupt_G0_Enable_       T , 0x92  , write , 16 , 0xffffffff , 0 ,0
#define Interrupt_G1_Ack_          T , 0x6   , write , 16 , 0xffff3ff9 , 0 ,0
#define Interrupt_G1_Enable_       T , 0x96  , write , 16 , 0xffffffff , 0 ,0
#define G0_DMA_Status_             T , 0xb8  ,  read , 16 , .no..mask. , 0 ,0
#define G0_HW_Save_                T , 0x10  ,  read , 32 , .no..mask. , 0 ,0
#define G0_HW_Save_High_           T , 0x10  ,  read , 16 , .no..mask. , 0 ,0
#define G0_HW_Save_Low_            T , 0x12  ,  read , 16 , .no..mask. , 0 ,0
#define G0_Save_                   T , 0x18  ,  read , 32 , .no..mask. , 0 ,0
#define G0_Save_High_              T , 0x18  ,  read , 16 , .no..mask. , 0 ,0
#define G0_Save_Low_               T , 0x1a  ,  read , 16 , .no..mask. , 0 ,0
#define G0_Status_1_               T , 0x4   ,  read , 16 , .no..mask. , 0 ,0
#define G01_Joint_Status_1_        T , 0x36  ,  read , 16 , .no..mask. , 0 ,0
#define G01_Joint_Status_2_        T , 0x3a  ,  read , 16 , .no..mask. , 0 ,0
#define G01_Status_                T , 0x8   ,  read , 16 , .no..mask. , 0 ,0
#define G1_DMA_Status_             T , 0xba  ,  read , 16 , .no..mask. , 0 ,0
#define G1_HW_Save_                T , 0x14  ,  read , 32 , .no..mask. , 0 ,0
#define G1_HW_Save_High_           T , 0x14  ,  read , 16 , .no..mask. , 0 ,0
#define G1_HW_Save_Low_            T , 0x16  ,  read , 16 , .no..mask. , 0 ,0
#define G1_Save_                   T , 0x1c  ,  read , 32 , .no..mask. , 0 ,0
#define G1_Save_High_              T , 0x1c  ,  read , 16 , .no..mask. , 0 ,0
#define G1_Save_Low_               T , 0x1e  ,  read , 16 , .no..mask. , 0 ,0
#define G1_Status_1_               T , 0x6   ,  read , 16 , .no..mask. , 0 ,0

/* Shadow registers */

struct shadow_T_registers {
 u32 G0_Load_A[1];
 u32 G0_Load_B[1];
 u32 G1_Load_A[1];
 u32 G1_Load_B[1];
 u32 G0_HW_Save[1];
 u32 G0_Save[1];
 u32 G1_HW_Save[1];
 u32 G1_Save[1];
 u16 G0_AutoIncrement[1];
 u16 G0_Command[1];
 u16 G0_Counting_Mode[1];
 u16 G0_MSeries_Counting_Mode[1];
 u16 G0_DMA_Control[1];
 u16 G0_Input_Select[1];
 u16 G0_Mode[1];
 u16 G0_Second_Gate[1];
 u16 G0_MSeries_ABZ[1];
 u16 G01_Joint_Reset[1];
 u16 G1_AutoIncrement[1];
 u16 G1_Command[1];
 u16 G1_Counting_Mode[1];
 u16 G1_MSeries_Counting_Mode[1];
 u16 G1_DMA_Control[1];
 u16 G1_Input_Select[1];
 u16 G1_Mode[1];
 u16 G1_Second_Gate[1];
 u16 G1_MSeries_ABZ[1];
 u16 Interrupt_G0_Ack[1];
 u16 Interrupt_G0_Enable[1];
 u16 Interrupt_G1_Ack[1];
 u16 Interrupt_G1_Enable[1];
 u16 G0_DMA_Status[1];
 u16 G0_HW_Save_High[1];
 u16 G0_HW_Save_Low[1];
 u16 G0_Save_High[1];
 u16 G0_Save_Low[1];
 u16 G0_Status_1[1];
 u16 G01_Joint_Status_1[1];
 u16 G01_Joint_Status_2[1];
 u16 G01_Status[1];
 u16 G1_DMA_Status[1];
 u16 G1_HW_Save_High[1];
 u16 G1_HW_Save_Low[1];
 u16 G1_Save_High[1];
 u16 G1_Save_Low[1];
 u16 G1_Status_1[1];
};
