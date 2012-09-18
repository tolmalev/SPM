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

#define G0_Command__G0_Arm                                                 0x0  ,  0xfffe
#define G0_Command__G0_Save_Trace                                          0x1  ,  0xfffd
#define G0_Command__G0_Load                                                0x2  ,  0xfffb
#define G0_Command__G0_Disarm                                              0x4  ,  0xffef
#define G0_Command__G0_Up_Down                                             0x5  ,  0xff9f
#define G0_Command__G0_Write_Switch                                        0x7  ,  0xff7f
#define G0_Command__G0_Synchronized_Gate                                   0x8  ,  0xfeff
#define G0_Command__G0_Little_Big_Endian                                   0x9  ,  0xfdff
#define G0_Command__G0_Bank_Switch_Start                                   0xa  ,  0xfbff
#define G0_Command__G0_Bank_Switch_Mode                                    0xb  ,  0xf7ff
#define G0_Command__G0_Bank_Switch_Enable                                  0xc  ,  0xefff
#define G0_Command__G1_Arm_Copy                                            0xd  ,  0xdfff
#define G0_Command__G1_Save_Trace_Copy                                     0xe  ,  0xbfff
#define G0_Command__G1_Disarm_Copy                                         0xf  ,  0x7fff
#define G0_Counting_Mode__G0_Encoder_Counting_Mode                         0x0  ,  0xfff8
#define G0_Counting_Mode__G0_Index_Enable                                  0x4  ,  0xffef
#define G0_Counting_Mode__G0_Index_Phase                                   0x5  ,  0xff9f
#define G0_Counting_Mode__G0_HW_Arm_Enable                                 0x7  ,  0xff7f
#define G0_Counting_Mode__G0_HW_Arm_Select                                 0x8  ,  0xf8ff
#define G0_Counting_Mode__G0_Prescale                                      0xc  ,  0xefff
#define G0_Counting_Mode__G0_Alternate_Synchronization                     0xd  ,  0xdfff
#define G0_Counting_Mode__G0_Prescale_By_2                                 0xe  ,  0xbfff
#define G0_MSeries_Counting_Mode__G0_MSeries_Encoder_Counting_Mode         0x0  ,  0xfff8
#define G0_MSeries_Counting_Mode__G0_MSeries_Index_Enable                  0x4  ,  0xffef
#define G0_MSeries_Counting_Mode__G0_MSeries_Index_Phase                   0x5  ,  0xff9f
#define G0_MSeries_Counting_Mode__G0_MSeries_HW_Arm_Enable                 0x7  ,  0xff7f
#define G0_MSeries_Counting_Mode__G0_MSeries_HW_Arm_Select                 0x8  ,  0xe0ff
#define G0_MSeries_Counting_Mode__G0_MSeries_Prescale                      0xd  ,  0xdfff
#define G0_MSeries_Counting_Mode__G0_MSeries_Alternate_Synchronization     0xe  ,  0xbfff
#define G0_MSeries_Counting_Mode__G0_MSeries_Prescale_By_2                 0xf  ,  0x7fff
#define G0_DMA_Control__G0_DMA_Enable                                      0x0  ,  0xfffe
#define G0_DMA_Control__G0_DMA_Output_Enable                               0x1  ,  0xfffd
#define G0_DMA_Control__G0_DMA_Int_Enable                                  0x2  ,  0xfffb
#define G0_Input_Select__G0_Read_Acknowledges_Irq                          0x0  ,  0xfffe
#define G0_Input_Select__G0_Write_Acknowledges_Irq                         0x1  ,  0xfffd
#define G0_Input_Select__G0_Source_Select                                  0x2  ,  0xff83
#define G0_Input_Select__G0_Gate_Select                                    0x7  ,  0xf07f
#define G0_Input_Select__G0_Gate_Select_Load_Source                        0xc  ,  0xefff
#define G0_Input_Select__G0_OR_Gate                                        0xd  ,  0xdfff
#define G0_Input_Select__G0_Output_Polarity                                0xe  ,  0xbfff
#define G0_Input_Select__G0_Source_Polarity                                0xf  ,  0x7fff
#define G0_Mode__G0_Gating_Mode                                            0x0  ,  0xfffc
#define G0_Mode__G0_Gate_On_Both_Edges                                     0x2  ,  0xfffb
#define G0_Mode__G0_Trigger_Mode_For_Edge_Gate                             0x3  ,  0xffe7
#define G0_Mode__G0_Stop_Mode                                              0x5  ,  0xff9f
#define G0_Mode__G0_Load_Source_Select                                     0x7  ,  0xff7f
#define G0_Mode__G0_Output_Mode                                            0x8  ,  0xfcff
#define G0_Mode__G0_Counting_Once                                          0xa  ,  0xf3ff
#define G0_Mode__G0_Loading_On_TC                                          0xc  ,  0xefff
#define G0_Mode__G0_Gate_Polarity                                          0xd  ,  0xdfff
#define G0_Mode__G0_Loading_On_Gate                                        0xe  ,  0xbfff
#define G0_Mode__G0_Reload_Source_Switching                                0xf  ,  0x7fff
#define G0_Second_Gate__G0_Second_Gate_Gating_Mode                         0x0  ,  0xfffe
#define G0_Second_Gate__G0_Second_Gate_Select                              0x7  ,  0xf07f
#define G0_Second_Gate__G0_Second_Gate_Polarity                            0xd  ,  0xdfff
#define G0_Second_Gate__G0_MSeries_Second_Gate_SubSelect                   0xe  ,  0xbfff
#define G0_Second_Gate__G0_MSeries_Source_SubSelect                        0xf  ,  0x7fff
#define G0_MSeries_ABZ__G0_Z_Select                                        0x0  ,  0xffe0
#define G0_MSeries_ABZ__G0_B_Select                                        0x5  ,  0xfc1f
#define G0_MSeries_ABZ__G0_A_Select                                        0xa  ,  0x83ff
#define G01_Joint_Reset__G0_Reset                                          0x2  ,  0xfffb
#define G01_Joint_Reset__G1_Reset                                          0x3  ,  0xfff7
#define G1_Command__G1_Arm                                                 0x0  ,  0xfffe
#define G1_Command__G1_Save_Trace                                          0x1  ,  0xfffd
#define G1_Command__G1_Load                                                0x2  ,  0xfffb
#define G1_Command__G1_Disarm                                              0x4  ,  0xffef
#define G1_Command__G1_Up_Down                                             0x5  ,  0xff9f
#define G1_Command__G1_Write_Switch                                        0x7  ,  0xff7f
#define G1_Command__G1_Synchronized_Gate                                   0x8  ,  0xfeff
#define G1_Command__G1_Little_Big_Endian                                   0x9  ,  0xfdff
#define G1_Command__G1_Bank_Switch_Start                                   0xa  ,  0xfbff
#define G1_Command__G1_Bank_Switch_Mode                                    0xb  ,  0xf7ff
#define G1_Command__G1_Bank_Switch_Enable                                  0xc  ,  0xefff
#define G1_Command__G0_Arm_Copy                                            0xd  ,  0xdfff
#define G1_Command__G0_Save_Trace_Copy                                     0xe  ,  0xbfff
#define G1_Command__G0_Disarm_Copy                                         0xf  ,  0x7fff
#define G1_Counting_Mode__G1_Encoder_Counting_Mode                         0x0  ,  0xfff8
#define G1_Counting_Mode__G1_Index_Enable                                  0x4  ,  0xffef
#define G1_Counting_Mode__G1_Index_Phase                                   0x5  ,  0xff9f
#define G1_Counting_Mode__G1_HW_Arm_Enable                                 0x7  ,  0xff7f
#define G1_Counting_Mode__G1_HW_Arm_Select                                 0x8  ,  0xf8ff
#define G1_Counting_Mode__G1_Prescale                                      0xc  ,  0xefff
#define G1_Counting_Mode__G1_Alternate_Synchronization                     0xd  ,  0xdfff
#define G1_Counting_Mode__G1_Prescale_By_2                                 0xe  ,  0xbfff
#define G1_MSeries_Counting_Mode__G1_MSeries_Encoder_Counting_Mode         0x0  ,  0xfff8
#define G1_MSeries_Counting_Mode__G1_MSeries_Index_Enable                  0x4  ,  0xffef
#define G1_MSeries_Counting_Mode__G1_MSeries_Index_Phase                   0x5  ,  0xff9f
#define G1_MSeries_Counting_Mode__G1_MSeries_HW_Arm_Enable                 0x7  ,  0xff7f
#define G1_MSeries_Counting_Mode__G1_MSeries_HW_Arm_Select                 0x8  ,  0xe0ff
#define G1_MSeries_Counting_Mode__G1_MSeries_Prescale                      0xd  ,  0xdfff
#define G1_MSeries_Counting_Mode__G1_MSeries_Alternate_Synchronization     0xe  ,  0xbfff
#define G1_MSeries_Counting_Mode__G1_MSeries_Prescale_By_2                 0xf  ,  0x7fff
#define G1_DMA_Control__G1_DMA_Enable                                      0x0  ,  0xfffe
#define G1_DMA_Control__G1_DMA_Output_Enable                               0x1  ,  0xfffd
#define G1_DMA_Control__G1_DMA_Int_Enable                                  0x2  ,  0xfffb
#define G1_Input_Select__G1_Read_Acknowledges_Irq                          0x0  ,  0xfffe
#define G1_Input_Select__G1_Write_Acknowledges_Irq                         0x1  ,  0xfffd
#define G1_Input_Select__G1_Source_Select                                  0x2  ,  0xff83
#define G1_Input_Select__G1_Gate_Select                                    0x7  ,  0xf07f
#define G1_Input_Select__G1_Gate_Select_Load_Source                        0xc  ,  0xefff
#define G1_Input_Select__G1_OR_Gate                                        0xd  ,  0xdfff
#define G1_Input_Select__G1_Output_Polarity                                0xe  ,  0xbfff
#define G1_Input_Select__G1_Source_Polarity                                0xf  ,  0x7fff
#define G1_Mode__G1_Gating_Mode                                            0x0  ,  0xfffc
#define G1_Mode__G1_Gate_On_Both_Edges                                     0x2  ,  0xfffb
#define G1_Mode__G1_Trigger_Mode_For_Edge_Gate                             0x3  ,  0xffe7
#define G1_Mode__G1_Stop_Mode                                              0x5  ,  0xff9f
#define G1_Mode__G1_Load_Source_Select                                     0x7  ,  0xff7f
#define G1_Mode__G1_Output_Mode                                            0x8  ,  0xfcff
#define G1_Mode__G1_Counting_Once                                          0xa  ,  0xf3ff
#define G1_Mode__G1_Loading_On_TC                                          0xc  ,  0xefff
#define G1_Mode__G1_Gate_Polarity                                          0xd  ,  0xdfff
#define G1_Mode__G1_Loading_On_Gate                                        0xe  ,  0xbfff
#define G1_Mode__G1_Reload_Source_Switching                                0xf  ,  0x7fff
#define G1_Second_Gate__G1_Second_Gate_Gating_Mode                         0x0  ,  0xfffe
#define G1_Second_Gate__G1_Second_Gate_Select                              0x7  ,  0xf07f
#define G1_Second_Gate__G1_Second_Gate_Polarity                            0xd  ,  0xdfff
#define G1_Second_Gate__G1_MSeries_Second_Gate_SubSelect                   0xe  ,  0xbfff
#define G1_Second_Gate__G1_MSeries_Source_SubSelect                        0xf  ,  0x7fff
#define G1_MSeries_ABZ__G1_Z_Select                                        0x0  ,  0xffe0
#define G1_MSeries_ABZ__G1_B_Select                                        0x5  ,  0xfc1f
#define G1_MSeries_ABZ__G1_A_Select                                        0xa  ,  0x83ff
#define Interrupt_G0_Ack__G0_Gate_Error_Confirm                            0x5  ,  0xffdf
#define Interrupt_G0_Ack__G0_TC_Error_Confirm                              0x6  ,  0xffbf
#define Interrupt_G0_Ack__G0_TC_Interrupt_Ack                              0xe  ,  0xbfff
#define Interrupt_G0_Ack__G0_Gate_Interrupt_Ack                            0xf  ,  0x7fff
#define Interrupt_G0_Enable__G0_TC_Interrupt_Enable                        0x6  ,  0xffbf
#define Interrupt_G0_Enable__G0_Gate_Interrupt_Enable                      0x8  ,  0xfeff
#define Interrupt_G1_Ack__G1_Gate_Error_Confirm                            0x1  ,  0xfffd
#define Interrupt_G1_Ack__G1_TC_Error_Confirm                              0x2  ,  0xfffb
#define Interrupt_G1_Ack__G1_TC_Interrupt_Ack                              0xe  ,  0xbfff
#define Interrupt_G1_Ack__G1_Gate_Interrupt_Ack                            0xf  ,  0x7fff
#define Interrupt_G1_Enable__G1_TC_Interrupt_Enable                        0x9  ,  0xfdff
#define Interrupt_G1_Enable__G1_Gate_Interrupt_Enable                      0xa  ,  0xfbff
#define G0_DMA_Status__G0_DMA_Read_Value                                   0xd  ,  0xdfff
#define G0_DMA_Status__G0_DMA_Error_St                                     0xe  ,  0xbfff
#define G0_DMA_Status__G0_DRQ_St                                           0xf  ,  0x7fff
#define G0_HW_Save__G0_HW_Save_Value                                       0x0  ,  0x0   
#define G0_HW_Save_High__G0_HW_Save_High_Value                             0x0  ,  0x0   
#define G0_HW_Save_Low__G0_HW_Save_Low_Value                               0x0  ,  0x0   
#define G0_Save__G0_Save_Value                                             0x0  ,  0x0   
#define G0_Save_High__G0_Save_High_Value                                   0x0  ,  0x0   
#define G0_Save_Low__G0_Save_Low_Value                                     0x0  ,  0x0   
#define G0_Status_1__G0_Gate_Interrupt_St                                  0x2  ,  0xfffb
#define G0_Status_1__G0_TC_St                                              0x3  ,  0xfff7
#define G0_Status_1__G0_Interrupt_St                                       0xf  ,  0x7fff
#define G01_Joint_Status_1__G0_Bank_St                                     0x0  ,  0xfffe
#define G01_Joint_Status_1__G1_Bank_St                                     0x1  ,  0xfffd
#define G01_Joint_Status_1__G0_Gate_St                                     0x2  ,  0xfffb
#define G01_Joint_Status_1__G1_Gate_St                                     0x3  ,  0xfff7
#define G01_Joint_Status_1__G01_DIO_Serial_IO_In_Progress_St               0xc  ,  0xefff
#define G01_Joint_Status_2__G0_Output_St                                   0x0  ,  0xfffe
#define G01_Joint_Status_2__G1_Output_St                                   0x1  ,  0xfffd
#define G01_Joint_Status_2__G0_HW_Save_St                                  0xc  ,  0xefff
#define G01_Joint_Status_2__G1_HW_Save_St                                  0xd  ,  0xdfff
#define G01_Joint_Status_2__G0_Permanent_Stale_Data_St                     0xe  ,  0xbfff
#define G01_Joint_Status_2__G1_Permanent_Stale_Data_St                     0xf  ,  0x7fff
#define G01_Status__G0_Save_St                                             0x0  ,  0xfffe
#define G01_Status__G1_Save_St                                             0x1  ,  0xfffd
#define G01_Status__G0_Counting_St                                         0x2  ,  0xfffb
#define G01_Status__G1_Counting_St                                         0x3  ,  0xfff7
#define G01_Status__G0_Next_Load_Source_St                                 0x4  ,  0xffef
#define G01_Status__G1_Next_Load_Source_St                                 0x5  ,  0xffdf
#define G01_Status__G0_Stale_Data_St                                       0x6  ,  0xffbf
#define G01_Status__G1_Stale_Data_St                                       0x7  ,  0xff7f
#define G01_Status__G0_Armed_St                                            0x8  ,  0xfeff
#define G01_Status__G1_Armed_St                                            0x9  ,  0xfdff
#define G01_Status__G0_No_Load_Between_Gates_St                            0xa  ,  0xfbff
#define G01_Status__G1_No_Load_Between_Gates_St                            0xb  ,  0xf7ff
#define G01_Status__G0_TC_Error_St                                         0xc  ,  0xefff
#define G01_Status__G1_TC_Error_St                                         0xd  ,  0xdfff
#define G01_Status__G0_Gate_Error_St                                       0xe  ,  0xbfff
#define G01_Status__G1_Gate_Error_St                                       0xf  ,  0x7fff
#define G1_DMA_Status__G1_DMA_Read_Value                                   0xd  ,  0xdfff
#define G1_DMA_Status__G1_DMA_Error_St                                     0xe  ,  0xbfff
#define G1_DMA_Status__G1_DRQ_St                                           0xf  ,  0x7fff
#define G1_HW_Save__G1_HW_Save_Value                                       0x0  ,  0x0   
#define G1_HW_Save_High__G1_HW_Save_High_Value                             0x0  ,  0x0   
#define G1_HW_Save_Low__G1_HW_Save_Low_Value                               0x0  ,  0x0   
#define G1_Save__G1_Save_Value                                             0x0  ,  0x0   
#define G1_Save_High__G1_Save_High_Value                                   0x0  ,  0x0   
#define G1_Save_Low__G1_Save_Low_Value                                     0x0  ,  0x0   
#define G1_Status_1__G1_Gate_Interrupt_St                                  0x2  ,  0xfffb
#define G1_Status_1__G1_TC_St                                              0x3  ,  0xfff7
#define G1_Status_1__G1_Interrupt_St                                       0xf  ,  0x7fff
