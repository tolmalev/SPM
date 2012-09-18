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

         
#define _kG0_Arm 1
#define _kG0_Save_Trace 2
#define _kG0_Load 3
#define _kG0_Disarm 5
#define _kG0_Up_Down 6
#define _kG0_Write_Switch 7
#define _kG0_Synchronized_Gate 8
#define _kG0_Little_Big_Endian 9
#define _kG0_Bank_Switch_Start 10
#define _kG0_Bank_Switch_Mode 11
#define _kG0_Bank_Switch_Enable 12
#define _kG1_Arm_Copy 13
#define _kG1_Save_Trace_Copy 14
#define _kG1_Disarm_Copy 15
#define _kG0_Up_DownSoftware_Down 0
#define _kG0_Up_DownSoftware_Up 1
#define _kG0_Up_DownHardware 2
#define _kG0_Up_DownHardware_Gate 3
#define _kG0_Write_SwitchAlways_Load_A 0
#define _kG0_Write_SwitchInactive_Load 1
#define _kG0_Synchronized_GateDisabled 0
#define _kG0_Synchronized_GateEnabled 1
#define _kG0_Little_Big_EndianLow_Register 0
#define _kG0_Little_Big_EndianHigh_Register 1
#define _kG0_Bank_Switch_ModeGate 0
#define _kG0_Bank_Switch_ModeSoftware 1
#define _kG0_Bank_Switch_EnableBank_X 0
#define _kG0_Bank_Switch_EnableBank_Y 1
         
#define _kG0_Encoder_Counting_Mode 1
#define _kG0_Index_Enable 3
#define _kG0_Index_Phase 4
#define _kG0_HW_Arm_Enable 5
#define _kG0_HW_Arm_Select 6
#define _kG0_Prescale 8
#define _kG0_Alternate_Synchronization 9
#define _kG0_Prescale_By_2 10
         
#define _kG0_MSeries_Encoder_Counting_Mode 1
#define _kG0_MSeries_Index_Enable 3
#define _kG0_MSeries_Index_Phase 4
#define _kG0_MSeries_HW_Arm_Enable 5
#define _kG0_MSeries_HW_Arm_Select 6
#define _kG0_MSeries_Prescale 7
#define _kG0_MSeries_Alternate_Synchronization 8
#define _kG0_MSeries_Prescale_By_2 9
         
#define _kG0_DMA_Enable 1
#define _kG0_DMA_Output_Enable 2
#define _kG0_DMA_Int_Enable 3
#define _kG0_DMA_EnableRead 0
#define _kG0_DMA_EnableWrite 1
         
#define _kG0_Read_Acknowledges_Irq 1
#define _kG0_Write_Acknowledges_Irq 2
#define _kG0_Source_Select 3
#define _kG0_Gate_Select 4
#define _kG0_Gate_Select_Load_Source 5
#define _kG0_OR_Gate 6
#define _kG0_Output_Polarity 7
#define _kG0_Source_Polarity 8
         
#define _kG0_Gating_Mode 1
#define _kG0_Gate_On_Both_Edges 2
#define _kG0_Trigger_Mode_For_Edge_Gate 3
#define _kG0_Stop_Mode 4
#define _kG0_Load_Source_Select 5
#define _kG0_Output_Mode 6
#define _kG0_Counting_Once 7
#define _kG0_Loading_On_TC 8
#define _kG0_Gate_Polarity 9
#define _kG0_Loading_On_Gate 10
#define _kG0_Reload_Source_Switching 11
#define _kG0_Gating_ModeGating_Disabled 0
#define _kG0_Gating_ModeLevel_Gating 1
#define _kG0_Gating_ModeEdge_Gating_Active_High 2
#define _kG0_Gating_ModeEdge_Gating_Active_Low 3
#define _kG0_Gate_On_Both_EdgesBoth_Edges_Disabled 0
#define _kG0_Gate_On_Both_EdgesBoth_Edges_Enabled 1
#define _kG0_Trigger_Mode_For_Edge_GateFirst_Starts_Next_Stops 0
#define _kG0_Trigger_Mode_For_Edge_GateFirst_Starts_Next_Starts 1
#define _kG0_Trigger_Mode_For_Edge_GateGate_Starts_TC_Stops 2
#define _kG0_Trigger_Mode_For_Edge_GateGate_Does_Not_Stop 3
#define _kG0_Stop_ModeStop_On_Gate 0
#define _kG0_Stop_ModeStop_On_Gate_Or_First_TC 1
#define _kG0_Stop_ModeStop_On_Gate_Or_Second_TC 2
#define _kG0_Stop_ModeStop_Mode_Reserved 3
#define _kG0_Load_Source_SelectLoad_A 0
#define _kG0_Load_Source_SelectLoad_B 1
#define _kG0_Output_ModeReserved 0
#define _kG0_Output_ModePulse 1
#define _kG0_Output_ModeToggle 2
#define _kG0_Output_ModeToggle_On_TC_Or_Gate 3
#define _kG0_Counting_OnceNo_HW_Disarm 0
#define _kG0_Counting_OnceDisarm_On_TC 1
#define _kG0_Counting_OnceDisarm_On_Gate 2
#define _kG0_Counting_OnceDisarm_On_Gate_Or_TC 3
#define _kG0_Loading_On_TCRollover_On_TC 0
#define _kG0_Loading_On_TCReload_On_TC 1
#define _kG0_Loading_On_GateNo_Reload 0
#define _kG0_Loading_On_GateReload_On_Stop_Gate 1
#define _kG0_Reload_Source_SwitchingUse_Same 0
#define _kG0_Reload_Source_SwitchingAlternate 1
         
#define _kG0_Second_Gate_Gating_Mode 1
#define _kG0_Second_Gate_Select 3
#define _kG0_Second_Gate_Polarity 5
#define _kG0_MSeries_Second_Gate_SubSelect 6
#define _kG0_MSeries_Source_SubSelect 7
         
#define _kG0_Z_Select 1
#define _kG0_B_Select 2
#define _kG0_A_Select 3
         
#define _kG0_Reset 2
#define _kG1_Reset 3
         
#define _kG1_Arm 1
#define _kG1_Save_Trace 2
#define _kG1_Load 3
#define _kG1_Disarm 5
#define _kG1_Up_Down 6
#define _kG1_Write_Switch 7
#define _kG1_Synchronized_Gate 8
#define _kG1_Little_Big_Endian 9
#define _kG1_Bank_Switch_Start 10
#define _kG1_Bank_Switch_Mode 11
#define _kG1_Bank_Switch_Enable 12
#define _kG0_Arm_Copy 13
#define _kG0_Save_Trace_Copy 14
#define _kG0_Disarm_Copy 15
#define _kG1_Up_DownSoftware_Down 0
#define _kG1_Up_DownSoftware_Up 1
#define _kG1_Up_DownHardware 2
#define _kG1_Up_DownHardware_Gate 3
#define _kG1_Write_SwitchAlways_Load_A 0
#define _kG1_Write_SwitchInactive_Load 1
#define _kG1_Synchronized_GateDisabled 0
#define _kG1_Synchronized_GateEnabled 1
#define _kG1_Little_Big_EndianLow_Register 0
#define _kG1_Little_Big_EndianHigh_Register 1
#define _kG1_Bank_Switch_ModeGate 0
#define _kG1_Bank_Switch_ModeSoftware 1
#define _kG1_Bank_Switch_EnableBank_X 0
#define _kG1_Bank_Switch_EnableBank_Y 1
         
#define _kG1_Encoder_Counting_Mode 1
#define _kG1_Index_Enable 3
#define _kG1_Index_Phase 4
#define _kG1_HW_Arm_Enable 5
#define _kG1_HW_Arm_Select 6
#define _kG1_Prescale 8
#define _kG1_Alternate_Synchronization 9
#define _kG1_Prescale_By_2 10
         
#define _kG1_MSeries_Encoder_Counting_Mode 1
#define _kG1_MSeries_Index_Enable 3
#define _kG1_MSeries_Index_Phase 4
#define _kG1_MSeries_HW_Arm_Enable 5
#define _kG1_MSeries_HW_Arm_Select 6
#define _kG1_MSeries_Prescale 7
#define _kG1_MSeries_Alternate_Synchronization 8
#define _kG1_MSeries_Prescale_By_2 9
         
#define _kG1_DMA_Enable 1
#define _kG1_DMA_Output_Enable 2
#define _kG1_DMA_Int_Enable 3
#define _kG1_DMA_EnableRead 0
#define _kG1_DMA_EnableWrite 1
         
#define _kG1_Read_Acknowledges_Irq 1
#define _kG1_Write_Acknowledges_Irq 2
#define _kG1_Source_Select 3
#define _kG1_Gate_Select 4
#define _kG1_Gate_Select_Load_Source 5
#define _kG1_OR_Gate 6
#define _kG1_Output_Polarity 7
#define _kG1_Source_Polarity 8
         
#define _kG1_Gating_Mode 1
#define _kG1_Gate_On_Both_Edges 2
#define _kG1_Trigger_Mode_For_Edge_Gate 3
#define _kG1_Stop_Mode 4
#define _kG1_Load_Source_Select 5
#define _kG1_Output_Mode 6
#define _kG1_Counting_Once 7
#define _kG1_Loading_On_TC 8
#define _kG1_Gate_Polarity 9
#define _kG1_Loading_On_Gate 10
#define _kG1_Reload_Source_Switching 11
#define _kG1_Gating_ModeGating_Disabled 0
#define _kG1_Gating_ModeLevel_Gating 1
#define _kG1_Gating_ModeEdge_Gating_Active_High 2
#define _kG1_Gating_ModeEdge_Gating_Active_Low 3
#define _kG1_Gate_On_Both_EdgesBoth_Edges_Disabled 0
#define _kG1_Gate_On_Both_EdgesBoth_Edges_Enabled 1
#define _kG1_Trigger_Mode_For_Edge_GateFirst_Starts_Next_Stops 0
#define _kG1_Trigger_Mode_For_Edge_GateFirst_Starts_Next_Starts 1
#define _kG1_Trigger_Mode_For_Edge_GateGate_Starts_TC_Stops 2
#define _kG1_Trigger_Mode_For_Edge_GateGate_Does_Not_Stop 3
#define _kG1_Stop_ModeStop_On_Gate 0
#define _kG1_Stop_ModeStop_On_Gate_Or_First_TC 1
#define _kG1_Stop_ModeStop_On_Gate_Or_Second_TC 2
#define _kG1_Stop_ModeStop_Mode_Reserved 3
#define _kG1_Load_Source_SelectLoad_A 0
#define _kG1_Load_Source_SelectLoad_B 1
#define _kG1_Output_ModeReserved 0
#define _kG1_Output_ModePulse 1
#define _kG1_Output_ModeToggle 2
#define _kG1_Output_ModeToggle_On_TC_Or_Gate 3
#define _kG1_Counting_OnceNo_HW_Disarm 0
#define _kG1_Counting_OnceDisarm_On_TC 1
#define _kG1_Counting_OnceDisarm_On_Gate 2
#define _kG1_Counting_OnceDisarm_On_Gate_Or_TC 3
#define _kG1_Loading_On_TCRollover_On_TC 0
#define _kG1_Loading_On_TCReload_On_TC 1
#define _kG1_Loading_On_GateNo_Reload 0
#define _kG1_Loading_On_GateReload_On_Stop_Gate 1
#define _kG1_Reload_Source_SwitchingUse_Same 0
#define _kG1_Reload_Source_SwitchingAlternate 1
         
#define _kG1_Second_Gate_Gating_Mode 1
#define _kG1_Second_Gate_Select 3
#define _kG1_Second_Gate_Polarity 5
#define _kG1_MSeries_Second_Gate_SubSelect 6
#define _kG1_MSeries_Source_SubSelect 7
         
#define _kG1_Z_Select 1
#define _kG1_B_Select 2
#define _kG1_A_Select 3
         
#define _kG0_Gate_Error_Confirm 2
#define _kG0_TC_Error_Confirm 3
#define _kG0_TC_Interrupt_Ack 5
#define _kG0_Gate_Interrupt_Ack 6
         
#define _kG0_TC_Interrupt_Enable 2
#define _kG0_Gate_Interrupt_Enable 4
         
#define _kG1_Gate_Error_Confirm 2
#define _kG1_TC_Error_Confirm 3
#define _kG1_TC_Interrupt_Ack 5
#define _kG1_Gate_Interrupt_Ack 6
         
#define _kG1_TC_Interrupt_Enable 2
#define _kG1_Gate_Interrupt_Enable 3
         
#define _kG0_DMA_Read_Value 2
#define _kG0_DMA_Error_St 3
#define _kG0_DRQ_St 4
         
#define _kG0_HW_Save_Value 1
         
#define _kG0_HW_Save_High_Value 1
         
#define _kG0_HW_Save_Low_Value 1
         
#define _kG0_Save_Value 1
         
#define _kG0_Save_High_Value 1
         
#define _kG0_Save_Low_Value 1
         
#define _kG0_Gate_Interrupt_St 2
#define _kG0_TC_St 3
#define _kG0_Interrupt_St 5
         
#define _kG0_Bank_St 1
#define _kG1_Bank_St 2
#define _kG0_Gate_St 3
#define _kG1_Gate_St 4
#define _kG01_DIO_Serial_IO_In_Progress_St 6
         
#define _kG0_Output_St 1
#define _kG1_Output_St 2
#define _kG0_HW_Save_St 4
#define _kG1_HW_Save_St 5
#define _kG0_Permanent_Stale_Data_St 6
#define _kG1_Permanent_Stale_Data_St 7
         
#define _kG0_Save_St 1
#define _kG1_Save_St 2
#define _kG0_Counting_St 3
#define _kG1_Counting_St 4
#define _kG0_Next_Load_Source_St 5
#define _kG1_Next_Load_Source_St 6
#define _kG0_Stale_Data_St 7
#define _kG1_Stale_Data_St 8
#define _kG0_Armed_St 9
#define _kG1_Armed_St 10
#define _kG0_No_Load_Between_Gates_St 11
#define _kG1_No_Load_Between_Gates_St 12
#define _kG0_TC_Error_St 13
#define _kG1_TC_Error_St 14
#define _kG0_Gate_Error_St 15
#define _kG1_Gate_Error_St 16
         
#define _kG1_DMA_Read_Value 2
#define _kG1_DMA_Error_St 3
#define _kG1_DRQ_St 4
         
#define _kG1_HW_Save_Value 1
         
#define _kG1_HW_Save_High_Value 1
         
#define _kG1_HW_Save_Low_Value 1
         
#define _kG1_Save_Value 1
         
#define _kG1_Save_High_Value 1
         
#define _kG1_Save_Low_Value 1
         
#define _kG1_Gate_Interrupt_St 2
#define _kG1_TC_St 3
#define _kG1_Interrupt_St 5
#define _kG0_AutoIncrement 0
#define _kG0_Command 1
#define _kG0_Counting_Mode 2
#define _kG0_MSeries_Counting_Mode 3
#define _kG0_DMA_Control 4
#define _kG0_Input_Select 5
#define _kG0_Load_A 6
#define _kG0_Load_B 7
#define _kG0_Mode 8
#define _kG0_Second_Gate 9
#define _kG0_MSeries_ABZ 10
#define _kG01_Joint_Reset 11
#define _kG1_AutoIncrement 12
#define _kG1_Command 13
#define _kG1_Counting_Mode 14
#define _kG1_MSeries_Counting_Mode 15
#define _kG1_DMA_Control 16
#define _kG1_Input_Select 17
#define _kG1_Load_A 18
#define _kG1_Load_B 19
#define _kG1_Mode 20
#define _kG1_Second_Gate 21
#define _kG1_MSeries_ABZ 22
#define _kInterrupt_G0_Ack 23
#define _kInterrupt_G0_Enable 24
#define _kInterrupt_G1_Ack 25
#define _kInterrupt_G1_Enable 26
#define _kG0_DMA_Status 27
#define _kG0_HW_Save 28
#define _kG0_HW_Save_High 29
#define _kG0_HW_Save_Low 30
#define _kG0_Save 31
#define _kG0_Save_High 32
#define _kG0_Save_Low 33
#define _kG0_Status_1 34
#define _kG01_Joint_Status_1 35
#define _kG01_Joint_Status_2 36
#define _kG01_Status 37
#define _kG1_DMA_Status 38
#define _kG1_HW_Save 39
#define _kG1_HW_Save_High 40
#define _kG1_HW_Save_Low 41
#define _kG1_Save 42
#define _kG1_Save_High 43
#define _kG1_Save_Low 44
#define _kG1_Status_1 45
#define _kMaxRegister 45
