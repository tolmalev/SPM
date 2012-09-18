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
      software without specific prior written permission.
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

#define AI_Command_1_                M , 0x110 , write , 16 , 0xffff0000 , 0 ,0
#define AI_Command_2_                M , 0x108 , write , 16 , 0xffff3c00 , 0 ,0
#define AI_Mode_2_                   M , 0x11a , write , 16 , 0xffffffff , 0 ,0
#define AI_Mode_3_                   M , 0x1ae , write , 16 , 0xffffffff , 0 ,0
#define AI_Personal_                 M , 0x19a , write , 16 , 0xffffffff , 0 ,0
#define AI_Output_Control_           M , 0x178 , write , 16 , 0xffffffff , 0 ,0
#define AI_SC_Load_A_                M , 0x124 , write , 32 , 0xffffffff , 0 ,0
#define AI_SC_Load_B_                M , 0x128 , write , 32 , 0xffffffff , 0 ,0
#define AI_SC_Save_                  M , 0x184 ,  read , 32 , .no..mask. , 0 ,0
#define AI_SI_Load_A_                M , 0x11c , write , 32 , 0x00000000 , 0 ,0
#define AI_SI_Load_B_                M , 0x120 , write , 32 , 0xffffffff , 0 ,0
#define AI_SI_Save_                  M , 0x180 ,  read , 32 , .no..mask. , 0 ,0
#define AI_SI2_Load_A_               M , 0x12c , write , 32 , 0xffffffff , 0 ,0
#define AI_SI2_Load_B_               M , 0x130 , write , 32 , 0xffffffff , 0 ,0
#define AI_START_STOP_Select_        M , 0x17c , write , 16 , 0xffffffff , 0 ,0
#define AI_Status_1_                 M , 0x104 ,  read , 16 , .no..mask. , 0 ,0
#define Analog_Trigger_Etc_          M , 0x17a , write , 16 , 0xffffffdf , 0 ,0
#define AI_Trigger_Select_           M , 0x17e , write , 16 , 0xffffffff , 0 ,0
#define AO_BC_Load_A_                M , 0x158 , write , 32 , 0xffffffff , 0 ,0
#define AO_BC_Load_B_                M , 0x15c , write , 32 , 0xffffffff , 0 ,0
#define AO_BC_Save_                  M , 0x124 ,  read , 32 , .no..mask. , 0 ,0
#define AO_Command_2_                M , 0x10a , write , 16 , 0xffff3c0e , 0 ,0
#define AO_Command_1_                M , 0x112 , write , 16 , 0xffff181e , 0 ,0
#define AO_Mode_1_                   M , 0x14c , write , 16 , 0xffffffff , 0 ,0
#define AO_Mode_2_                   M , 0x14e , write , 16 , 0xffffffff , 0 ,0
#define AO_Output_Control_           M , 0x1ac , write , 16 , 0xffffffff , 0 ,0
#define AO_Mode_3_                   M , 0x18c , write , 16 , 0xffffffff , 0 ,0
#define AO_Personal_                 M , 0x19c , write , 16 , 0xffffffff , 0 ,0
#define AO_START_Select_             M , 0x184 , write , 16 , 0xffffffff , 0 ,0
#define AO_Status_2_                 M , 0x10c ,  read , 16 , .no..mask. , 0 ,0
#define AO_Status_1_                 M , 0x106 ,  read , 16 , .no..mask. , 0 ,0
#define AO_Trigger_Select_           M , 0x186 , write , 16 , 0xffffffff , 0 ,0
#define AO_UC_Load_A_                M , 0x160 , write , 32 , 0xffffffff , 0 ,0
#define AO_UC_Load_B_                M , 0x164 , write , 32 , 0xffffffff , 0 ,0
#define AO_UC_Save_                  M , 0x128 ,  read , 32 , .no..mask. , 0 ,0
#define AO_UI_Load_A_                M , 0x150 , write , 32 , 0xffffffff , 0 ,0
#define AO_UI_Load_B_                M , 0x154 , write , 32 , 0xffffffff , 0 ,0
#define AO_UI_Save_                  M , 0x120 ,  read , 32 , .no..mask. , 0 ,0
#define Clock_and_FOUT_              M , 0x170 , write , 16 , 0xffffffff , 0 ,0
#define Interrupt_A_Ack_             M , 0x104 , write , 16 , 0xffff001f , 0 ,0
#define Interrupt_A_Enable_          M , 0x192 , write , 16 , 0xffffffff , 0 ,0
#define Interrupt_B_Ack_             M , 0x106 , write , 16 , 0xffff0061 , 0 ,0
#define Interrupt_Control_           M , 0x176 , write , 16 , 0xffffffff , 0 ,0
#define Interrupt_B_Enable_          M , 0x196 , write , 16 , 0xffffffff , 0 ,0
#define IO_Bidirection_Pin_          M , 0x172 , write , 16 , 0xffffffff , 0 ,0
#define Joint_Reset_                 M , 0x190 , write , 16 , 0xfffffcc0 , 0 ,0
#define Joint_Status_2_              M , 0x13a ,  read , 16 , .no..mask. , 0 ,0
#define Joint_Status_1_              M , 0x136 ,  read , 16 , .no..mask. , 0 ,0
#define RTSI_Shared_MUX_             M , 0x1a2 , write , 16 , 0xffffffff , 0 ,0
#define RTSI_Trig_A_Output_          M , 0x19e , write , 16 , 0xffffffff , 0 ,0
#define RTSI_Trig_Direction_         M , 0x174 , write , 16 , 0xffffffff , 0 ,0
#define RTSI_Trig_B_Output_          M , 0x1a0 , write , 16 , 0xffffffff , 0 ,0
#define G0_DMA_Config_               M , 0x1b8 , write , 16 , 0xfffffff7 , 0 ,0
#define G1_DMA_Config_               M , 0x1ba , write , 16 , 0xfffffff7 , 0 ,0
#define G0_DMA_Status_               M , 0x1b8 ,  read , 16 , .no..mask. , 0 ,0
#define G1_DMA_Status_               M , 0x1ba ,  read , 16 , .no..mask. , 0 ,0
#define CDIO_DMA_Select_             M , 0x7   , write ,  8 , 0xffffffff , 0 ,0
#define SCXI_Control_                M , 0x13  , write ,  8 , 0xffffffbf , 0 ,0
#define AI_Mode_1_                   M , 0x118 , write , 16 , 0xffffffff , 0 ,0
#define AI_DIV_Load_A_               M , 0x180 , write , 16 , 0xffffffff , 0 ,0
#define SCXI_Serial_Data_In_         M , 0x9   ,  read ,  8 , .no..mask. , 0 ,0
#define SCXI_Serial_Data_Out_        M , 0x11  , write ,  8 , 0xffffffff , 0 ,0
#define SCXI_Status_                 M , 0x7   ,  read ,  8 , .no..mask. , 0 ,0
#define SCXI_Output_Enable_          M , 0x15  , write ,  8 , 0xffffffff , 0 ,0
#define SCXI_DIO_Enable_             M , 0x21c , write ,  8 , 0xffffffff , 0 ,0
#define Static_Digital_Output_       M , 0x24  , write , 32 , 0xffffffff , 0 ,0
#define Static_Digital_Input_        M , 0x24  ,  read , 32 , .no..mask. , 0 ,0
#define DIO_Direction_               M , 0x28  , write , 32 , 0xffffffff , 0 ,0
#define AO_Serial_Interrupt_Enable_  M , 0xa0  , write ,  8 , 0xffffffff , 0 ,0
#define AO_Serial_Interrupt_Ack_     M , 0xa1  , write ,  8 , 0xfffffff0 , 0 ,0
#define AO_Serial_Interrupt_Status_  M , 0xa1  ,  read ,  8 , .no..mask. , 0 ,0
#define Interrupt_C_Enable_          M , 0x88  , write ,  8 , 0xffffffff , 0 ,0
#define Interrupt_C_Status_          M , 0x88  ,  read ,  8 , .no..mask. , 0 ,0
#define Analog_Trigger_Control_      M , 0x8c  , write ,  8 , 0xffffffff , 0 ,0
#define AI_FIFO_Data_                M , 0x1c  ,  read , 32 , .no..mask. , 0 ,0
#define AI_FIFO_Clear_               M , 0x1a6 , write , 16 , 0xfffffffe , 0 ,0
#define AI_AO_Select_                M , 0x9   , write ,  8 , 0xffffffff , 0 ,0
#define AI_Config_FIFO_Data_         M , 0x5e  , write , 16 , 0xffffffff , 0 ,0
#define Configuration_Memory_Clear_  M , 0x1a4 , write , 16 , 0xfffffffe , 0 ,0
#define AO_FIFO_Data_                M , 0xa4  , write , 32 , 0x00000000 , 0 ,0
#define AO_FIFO_Clear_               M , 0x1a8 , write , 16 , 0xfffffffe , 0 ,0
#define G0_G1_Select_                M , 0xb   , write ,  8 , 0xffffffff , 0 ,0
#define Misc_Command_                M , 0xf   , write ,  8 , 0xffffffff , 0 ,0
#define AO_Calibration_              M , 0xa3  , write ,  8 , 0xffffffff , 0 ,0
#define PFI_Output_Select_1_         M , 0x1d0 , write , 16 , 0xffffffff , 0 ,0
#define PFI_Output_Select_2_         M , 0x1d2 , write , 16 , 0xffffffff , 0 ,0
#define PFI_Output_Select_3_         M , 0x1d4 , write , 16 , 0xffffffff , 0 ,0
#define PFI_Output_Select_4_         M , 0x1d6 , write , 16 , 0xffffffff , 0 ,0
#define PFI_Output_Select_5_         M , 0x1d8 , write , 16 , 0xffffffff , 0 ,0
#define PFI_Output_Select_6_         M , 0x1da , write , 16 , 0xffffffff , 0 ,0
#define PFI_DI_                      M , 0x1dc ,  read , 16 , .no..mask. , 0 ,0
#define PFI_DO_                      M , 0x1de , write , 16 , 0xffffffff , 0 ,0
#define PFI_Filter_                  M , 0xb0  , write , 32 , 0xffffffff , 0 ,0
#define RTSI_Filter_                 M , 0xb4  , write , 32 , 0xffffffff , 0 ,0
#define Clock_And_Fout2_             M , 0x1c4 , write , 16 , 0xffffffff , 0 ,0
#define PLL_Control_                 M , 0x1c6 , write , 16 , 0xffffffff , 0 ,0
#define PLL_Status_                  M , 0x1c8 ,  read , 16 , .no..mask. , 0 ,0
#define AI_Config_FIFO_Bypass_       M , 0x218 , write , 32 , 0xffffffff , 0 ,0
#define CDI_FIFO_Data_               M , 0x220 ,  read , 32 , .no..mask. , 0 ,0
#define CDO_FIFO_Data_               M , 0x220 , write , 32 , 0x00000000 , 0 ,0
#define CDIO_Status_                 M , 0x224 ,  read , 32 , .no..mask. , 0 ,0
#define CDIO_Command_                M , 0x224 , write , 32 , 0xffe40000 , 0 ,0
#define CDI_Mode_                    M , 0x228 , write , 32 , 0xffffffff , 0 ,0
#define CDO_Mode_                    M , 0x22c , write , 32 , 0xffffffff , 0 ,0
#define CDI_Mask_Enable_             M , 0x230 , write , 32 , 0xffffffff , 0 ,0
#define CDO_Mask_Enable_             M , 0x234 , write , 32 , 0xffffffff , 0 ,0
#define SCXI_Legacy_Compatibility_   M , 0xbc  , write ,  8 , 0xffffffff , 0 ,0
#define DIO_Reserved_0_              M , 0x240 , write , 32 , 0xffffffff , 0 ,0
#define DIO_Reserved_1_              M , 0x244 , write , 32 , 0xffffffff , 0 ,0
#define DIO_Reserved_2_              M , 0x240 ,  read , 32 , .no..mask. , 0 ,0
#define DIO_Reserved_3_              M , 0x244 ,  read , 32 , .no..mask. , 0 ,0
#define DIO_Reserved_4_              M , 0x248 , write , 32 , 0xffffffff , 0 ,0
#define DIO_Reserved_5_              M , 0x24c , write , 32 , 0xffffffff , 0 ,0
#define DIO_Reserved_6_              M , 0x250 , write , 32 , 0xffffffff , 0 ,0
#define DIO_Reserved_7_              M , 0x254 , write , 32 , 0xffffffff , 0 ,0
#define LB_Reserved_0_               M , 0x204 , write , 16 , 0xffffffff , 0 ,0
#define LB_Reserved_1_               M , 0x200 , write , 32 , 0xffffffff , 0 ,0
#define AO_Waveform_Order_           M , 0xc2  , write ,  8 , 0xffffffff , 1  
#define AO_Config_Bank_              M , 0xc3  , write ,  8 , 0xffffffff , 1  
#define DAC_Direct_Data_             M , 0xc0  , write , 32 , 0x00000000 , 1  
#define Cal_PWM_                     M , 0x40  , write , 32 , 0xffffffff , 0 ,0
#define Gen_PWM_                     M , 0x44  , write , 16 , 0xffffffff , 2  
#define Static_AI_Control_           M , 0x0   , write ,  8 , 0xffffffff , 3  

/* Shadow registers */

struct shadow_M_registers {
 u32 AI_SC_Load_A[1];
 u32 AI_SC_Load_B[1];
 u32 AI_SC_Save[1];
 u32 AI_SI_Load_A[1];
 u32 AI_SI_Load_B[1];
 u32 AI_SI_Save[1];
 u32 AI_SI2_Load_A[1];
 u32 AI_SI2_Load_B[1];
 u32 AO_BC_Load_A[1];
 u32 AO_BC_Load_B[1];
 u32 AO_BC_Save[1];
 u32 AO_UC_Load_A[1];
 u32 AO_UC_Load_B[1];
 u32 AO_UC_Save[1];
 u32 AO_UI_Load_A[1];
 u32 AO_UI_Load_B[1];
 u32 AO_UI_Save[1];
 u32 Static_Digital_Output[1];
 u32 Static_Digital_Input[1];
 u32 DIO_Direction[1];
 u32 AI_FIFO_Data[1];
 u32 AO_FIFO_Data[1];
 u32 PFI_Filter[1];
 u32 RTSI_Filter[1];
 u32 AI_Config_FIFO_Bypass[1];
 u32 CDI_FIFO_Data[1];
 u32 CDO_FIFO_Data[1];
 u32 CDIO_Status[1];
 u32 CDIO_Command[1];
 u32 CDI_Mode[1];
 u32 CDO_Mode[1];
 u32 CDI_Mask_Enable[1];
 u32 CDO_Mask_Enable[1];
 u32 DIO_Reserved_0[1];
 u32 DIO_Reserved_1[1];
 u32 DIO_Reserved_2[1];
 u32 DIO_Reserved_3[1];
 u32 DIO_Reserved_4[1];
 u32 DIO_Reserved_5[1];
 u32 DIO_Reserved_6[1];
 u32 DIO_Reserved_7[1];
 u32 LB_Reserved_1[1];
 u32 DAC_Direct_Data[16];
 u32 Cal_PWM[1];
 u16 AI_Command_1[1];
 u16 AI_Command_2[1];
 u16 AI_Mode_2[1];
 u16 AI_Mode_3[1];
 u16 AI_Personal[1];
 u16 AI_Output_Control[1];
 u16 AI_START_STOP_Select[1];
 u16 AI_Status_1[1];
 u16 Analog_Trigger_Etc[1];
 u16 AI_Trigger_Select[1];
 u16 AO_Command_2[1];
 u16 AO_Command_1[1];
 u16 AO_Mode_1[1];
 u16 AO_Mode_2[1];
 u16 AO_Output_Control[1];
 u16 AO_Mode_3[1];
 u16 AO_Personal[1];
 u16 AO_START_Select[1];
 u16 AO_Status_2[1];
 u16 AO_Status_1[1];
 u16 AO_Trigger_Select[1];
 u16 Clock_and_FOUT[1];
 u16 Interrupt_A_Ack[1];
 u16 Interrupt_A_Enable[1];
 u16 Interrupt_B_Ack[1];
 u16 Interrupt_Control[1];
 u16 Interrupt_B_Enable[1];
 u16 IO_Bidirection_Pin[1];
 u16 Joint_Reset[1];
 u16 Joint_Status_2[1];
 u16 Joint_Status_1[1];
 u16 RTSI_Shared_MUX[1];
 u16 RTSI_Trig_A_Output[1];
 u16 RTSI_Trig_Direction[1];
 u16 RTSI_Trig_B_Output[1];
 u16 G0_DMA_Config[1];
 u16 G1_DMA_Config[1];
 u16 G0_DMA_Status[1];
 u16 G1_DMA_Status[1];
 u16 AI_Mode_1[1];
 u16 AI_DIV_Load_A[1];
 u16 AI_FIFO_Clear[1];
 u16 AI_Config_FIFO_Data[1];
 u16 Configuration_Memory_Clear[1];
 u16 AO_FIFO_Clear[1];
 u16 PFI_Output_Select_1[1];
 u16 PFI_Output_Select_2[1];
 u16 PFI_Output_Select_3[1];
 u16 PFI_Output_Select_4[1];
 u16 PFI_Output_Select_5[1];
 u16 PFI_Output_Select_6[1];
 u16 PFI_DI[1];
 u16 PFI_DO[1];
 u16 Clock_And_Fout2[1];
 u16 PLL_Control[1];
 u16 PLL_Status[1];
 u16 LB_Reserved_0[1];
 u16 Gen_PWM[7];
  u8 CDIO_DMA_Select[1];
  u8 SCXI_Control[1];
  u8 SCXI_Serial_Data_In[1];
  u8 SCXI_Serial_Data_Out[1];
  u8 SCXI_Status[1];
  u8 SCXI_Output_Enable[1];
  u8 SCXI_DIO_Enable[1];
  u8 AO_Serial_Interrupt_Enable[1];
  u8 AO_Serial_Interrupt_Ack[1];
  u8 AO_Serial_Interrupt_Status[1];
  u8 Interrupt_C_Enable[1];
  u8 Interrupt_C_Status[1];
  u8 Analog_Trigger_Control[1];
  u8 AI_AO_Select[1];
  u8 G0_G1_Select[1];
  u8 Misc_Command[1];
  u8 AO_Calibration[1];
  u8 SCXI_Legacy_Compatibility[1];
  u8 AO_Waveform_Order[16];
  u8 AO_Config_Bank[4];
  u8 Static_AI_Control[8];
};
