/*
 * utility functions for the TM Series boards
 * from National Instruments
 */

#include "spm_types.h"
#include "nilib.h"
#include "register_macro.h"
#include "switch_endian.h"

/* ai section */

/*
 *
 *  Model specific information:
 *
 *  Gain range map
 *      Range     -> gain value
 *
 *  NI 622x: 
 *      +/- 10V   -> 0
 *      +/- 5V    -> 1
 *      +/- 1V    -> 4
 *      +/- 200mV -> 5
 *
 *  NI 625x / NI 628x:
 *      +/- 10V   -> 1
 *      +/- 5V    -> 2
 *      +/- 2V    -> 3
 *      +/- 1V    -> 4
 *      +/- 500mV -> 5
 *      +/- 200mV -> 6
 *      +/- 100mV -> 7
 *
 * -------------------------------
 *
 * Minimum convert period divisor: (20 MHz timebase)
 *
 *  NI 622x        
 *      Single Channel    -> 80
 *      Multiple Channels -> 80
 *
 *  NI 625x        
 *      Single Channel    -> 16
 *      Multiple Channels -> 20
 *
 *  NI 628x         
 *      Single Channel    -> 30
 *      Multiple Channels -> 40
 *
 * Minimum convert delay divisor -> 3
 *
 */

/*
 *  adc_reset ()
 */

void adc_reset (u8 ** memap) {

	volatile int i = 1;

	NIREG (memap,Static_AI_Control,0,write,0);
	NIREG (memap,Static_AI_Control,0,write,1);

	NIBIT (memap,AI_Command_1,write,AI_CONVERT_Pulse,i);
	NIBIT (memap,AI_Command_1,write,AI_CONVERT_Pulse,i);
	NIBIT (memap,AI_Command_1,write,AI_CONVERT_Pulse,i);
	return;
}

/*
 *   ai_arm ()
 */

void ai_arm (u8 ** memap, tBoolean armSI) {

	NIBIT (memap,AI_Command_1,set,AI_SC_Arm,1);
	NIBIT (memap,AI_Command_1,set,AI_SI2_Arm,1);    

	if (armSI) NIBIT (memap,AI_Command_1,set,AI_SI_Arm,1);

	NIREG (memap,AI_Command_1,flush);

	return;
}

/*
 *   ai_clear_configuration_memory ()
 */

void ai_clear_configuration_memory (u8 ** memap) {

	NIREG (memap,Configuration_Memory_Clear,write,1);

	/* Use configuration FIFO */

	NIBIT (memap,AI_Config_FIFO_Bypass,set,AI_Bypass_Config_FIFO,0);
	NIREG (memap,AI_Config_FIFO_Bypass,flush);

	return; 
}

/*
 *   ai_clear_fifo ()
 */

void ai_clear_fifo (u8 ** memap) {

	NIREG (memap,AI_FIFO_Clear,write,1);
	return;
}

/*
 *   ai_configure_channel ()
 */

void ai_configure_channel (u8 ** memap,
                           u16 channel, 
                           u16 gain, 
                           int polarity, 
                           int channelType,
                           tBoolean lastChannel) {

	NIBIT (memap,AI_Config_FIFO_Data,set,AI_Config_Polarity,polarity);
	NIBIT (memap,AI_Config_FIFO_Data,set,AI_Config_Gain,gain);   
	NIBIT (memap,AI_Config_FIFO_Data,set,AI_Config_Channel,channel & 0xF);
	NIBIT (memap,AI_Config_FIFO_Data,set,
	       AI_Config_Bank, (channel & 0x30) >> 4);
	NIBIT (memap,AI_Config_FIFO_Data,set,
	       AI_Config_Channel_Type,channelType);
	NIBIT (memap,AI_Config_FIFO_Data,set,AI_Config_Dither,0);
	NIBIT (memap,AI_Config_FIFO_Data,set,
	       AI_Config_Last_Channel,lastChannel);
	NIREG (memap,AI_Config_FIFO_Data,flush);   
    
	if (lastChannel) {
		NIBIT (memap,AI_Command_1,set,AI_LOCALMUX_CLK_Pulse,kTrue);
		NIREG (memap,AI_Command_1,flush);
	}

	return;
}

/*
 *   ai_convert ()
 */

void ai_convert (u8 ** memap,
		 u32 periodDivisor,
		 u32 delayDivisor,
		 tBoolean externalSampleClock) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,AI_Mode_1,set,AI_CONVERT_Source_Select,
	       _kAI_CONVERT_Source_SelectSI2TC);
	NIBIT (memap,AI_Mode_1,set,AI_CONVERT_Source_Polarity,
	       _kAI_CONVERT_Source_PolarityFalling_Edge);
	NIREG (memap,AI_Mode_1,flush);

	NIBIT (memap,AI_Mode_2,set,AI_SC_Gate_Enable,0);
	NIBIT (memap,AI_Mode_2,set,AI_Start_Stop_Gate_Enable,0);
	NIBIT (memap,AI_Mode_2,set,AI_SI2_Reload_Mode,
	       _kAI_SI2_Reload_ModeAlternate_First_Period_Every_STOP);
	NIREG (memap,AI_Mode_2,flush);

	if (externalSampleClock)
		NIBIT (memap,AI_Mode_3,set,AI_SI2_Source_Select,
		       _kAI_SI2_Source_SelectINTIMEBASE1);
	else
		NIBIT (memap,AI_Mode_3,set,AI_SI2_Source_Select,
		       _kAI_SI2_Source_SelectSame_As_SI);
	NIREG (memap,AI_Mode_3,flush);

	NIREG (memap,AI_SI2_Load_A,write,delayDivisor);
	NIREG (memap,AI_SI2_Load_B,write,periodDivisor);

	NIBIT (memap,AI_Mode_2,set,AI_SI2_Initial_Load_Source,
	       _kAI_SI2_Initial_Load_SourceLoad_A);
	NIREG (memap,AI_Mode_2,flush);

	NIBIT (memap,AI_Command_1,set,AI_SI2_Load,1);
	NIREG (memap,AI_Command_1,flush);

	NIBIT (memap,AI_Mode_2,set,AI_SI2_Initial_Load_Source,
	       _kAI_SI2_Initial_Load_SourceLoad_B);
	NIREG (memap,AI_Mode_2,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 *   ai_disarm ()
 */

void ai_disarm (u8 ** memap) {

	NIBIT (memap,Joint_Reset,write,AI_Reset,1);
	return; 
}

/*
 * ai_environmentalize ()
 */

void ai_environmentalize (u8 ** memap) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,AI_Mode_2,set,AI_External_MUX_Present,
	       _kAI_External_MUX_PresentEvery_Convert);
	NIREG (memap,AI_Mode_2,flush);

	NIBIT (memap,AI_Output_Control,set,AI_EXTMUX_CLK_Output_Select,
	       _kAI_EXTMUX_CLK_Output_SelectGround);
	NIREG (memap,AI_Output_Control,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 *   ai_hardware_gating ()
 */

void ai_hardware_gating (u8 ** memap) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,AI_Mode_3,set,AI_External_Gate_Select,
	       _kAI_External_Gate_SelectDisabled);
	NIREG (memap,AI_Mode_3,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 *   ai_number_of_samples ()
 */

void ai_number_of_samples (u8 ** memap, 
			   u32 postTriggerSamples,
			   u32 preTriggerSamples, 
			   tBoolean continuous) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);    

	NIBIT (memap,AI_Mode_1,set,AI_Continuous,continuous);
	NIREG (memap,AI_Mode_1,flush);

	NIREG (memap,AI_SC_Load_A,write,postTriggerSamples-1);

	if (!preTriggerSamples) {

		NIREG (memap,AI_SC_Load_B,set,0);

		NIBIT (memap,AI_Mode_2,set,AI_SC_Reload_Mode,
		       _kAI_SC_Reload_ModeNo_Change);
		NIBIT (memap,AI_Mode_2,set,AI_SC_Initial_Load_Source,
		       _kAI_SC_Initial_Load_SourceLoad_A);
	} else {

		NIREG (memap,AI_SC_Load_B,set,(preTriggerSamples-1));

		NIBIT (memap,AI_Mode_2,set,AI_SC_Reload_Mode,
		       _kAI_SC_Reload_ModeSwitch);
		NIBIT (memap,AI_Mode_2,set,AI_SC_Initial_Load_Source,
		       _kAI_SC_Initial_Load_SourceLoad_B);
	}
	NIREG (memap,AI_Mode_2,flush);
	NIREG (memap,AI_SC_Load_B,flush);

	NIBIT (memap,AI_Command_1,set,AI_SC_Load,1);
	NIREG (memap,AI_Command_1,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 * ai_personalize ()
 */

/*
 * NI 622x --
 *      tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_High
 *
 * NI 625x --
 *      tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_Low
 *
 * NI 628x --
 *      tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_Low
 *
 */

void ai_personalize (u8 ** memap, int convertOutputSelect) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,AI_Output_Control,set,AI_CONVERT_Output_Select,
	       convertOutputSelect);
	NIBIT (memap,AI_Output_Control,set,AI_SCAN_IN_PROG_Output_Select,
	       _kAI_SCAN_IN_PROG_Output_SelectActive_High);
	NIREG (memap,AI_Output_Control,flush);

	NIBIT (memap,AI_Personal,set,AI_CONVERT_Pulse_Width,
	       _kAI_CONVERT_Pulse_WidthAbout_1_Clock_Period);
	NIREG (memap,AI_Personal,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 *   ai_reset ()
 */

void ai_reset (u8 ** memap) {

	NIBIT (memap,Joint_Reset,write,AI_Reset,1);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,Interrupt_A_Enable,set,AI_SC_TC_Interrupt_Enable,0);
	NIBIT (memap,Interrupt_A_Enable,set,AI_START1_Interrupt_Enable,0);
	NIBIT (memap,Interrupt_A_Enable,set,AI_START2_Interrupt_Enable,0);
	NIBIT (memap,Interrupt_A_Enable,set,AI_START_Interrupt_Enable,0);
	NIBIT (memap,Interrupt_A_Enable,set,AI_STOP_Interrupt_Enable,0);
	NIBIT (memap,Interrupt_A_Enable,set,AI_Error_Interrupt_Enable,0);
	NIBIT (memap,Interrupt_A_Enable,set,AI_FIFO_Interrupt_Enable,0);
	NIREG (memap,Interrupt_A_Enable,flush);

	NIBIT (memap,Interrupt_A_Ack,set,AI_SC_TC_Error_Confirm,1);
	NIBIT (memap,Interrupt_A_Ack,set,AI_SC_TC_Interrupt_Ack,1);
	NIBIT (memap,Interrupt_A_Ack,set,AI_START1_Interrupt_Ack,1);
	NIBIT (memap,Interrupt_A_Ack,set,AI_START2_Interrupt_Ack,1);
	NIBIT (memap,Interrupt_A_Ack,set,AI_START_Interrupt_Ack,1);
	NIBIT (memap,Interrupt_A_Ack,set,AI_STOP_Interrupt_Ack,1);
	NIBIT (memap,Interrupt_A_Ack,set,AI_Error_Interrupt_Ack,1);
	NIREG (memap,Interrupt_A_Ack,flush);

	NIBIT (memap,AI_Mode_1,set,AI_Start_Stop,1);
	NIREG (memap,AI_Mode_1,flush);

	NIREG (memap,AI_Mode_2,write,0);
	NIREG (memap,AI_Mode_3,write,0);
	NIREG (memap,AI_Output_Control,write,0);
	NIREG (memap,AI_Personal,write,0);
	NIREG (memap,AI_START_STOP_Select,write,0);
	NIREG (memap,AI_Trigger_Select,write,0);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);    

	return;

}

/*
 *   ai_sample_start ()
 */

void ai_sample_start (u8 ** memap, 
		      u32 periodDivisor, 
		      u32 delayDivisor, 
		      int source, 
		      int polarity) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,AI_START_STOP_Select,set,AI_START_Select,source);
	NIBIT (memap,AI_START_STOP_Select,set,AI_START_Polarity,polarity);
	NIBIT (memap,AI_START_STOP_Select,set,AI_START_Sync,1);
	NIBIT (memap,AI_START_STOP_Select,set,AI_START_Edge,1);
	NIREG (memap,AI_START_STOP_Select,flush);

	if (source == _kAI_START_SelectSI_TC) {

		NIBIT (memap,AI_Mode_1,set,AI_SI_Source_Polarity,
		       _kAI_SI_Source_PolarityRising_Edge);
		NIBIT (memap,AI_Mode_1,set,AI_SI_Source_Select,
		       _kAI_SI_Source_SelectINTIMEBASE1);
		NIREG (memap,AI_Mode_1,flush);

		NIREG (memap,AI_SI_Load_B,write,delayDivisor-1);
		NIREG (memap,AI_SI_Load_A,write,periodDivisor-1);

		NIBIT (memap,AI_Mode_2,set,AI_SI_Initial_Load_Source,
		       _kAI_SI_Initial_Load_SourceLoad_B);        
		NIREG (memap,AI_Mode_2,flush);

		NIBIT (memap,AI_Command_1,write,AI_SI_Load,1);

		NIBIT (memap,AI_Mode_2,set,AI_SI_Reload_Mode,
		       _kAI_SI_Reload_ModeAlternate_First_Period_Every_SCTC);
		NIBIT (memap,AI_Mode_2,set,AI_SI_Initial_Load_Source,
		       _kAI_SI_Initial_Load_SourceLoad_A);
		NIREG (memap,AI_Mode_2,flush);
	}

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}
/*
 *   ai_sample_stop ()
 */

void ai_sample_stop (u8 ** memap, tBoolean multiChannel) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);    

	NIBIT (memap,AI_START_STOP_Select,set,
	       AI_STOP_Select, _kAI_STOP_SelectIN);
	NIBIT (memap,AI_START_STOP_Select,set,AI_STOP_Polarity,
	       _kAI_STOP_PolarityActive_High);
	NIBIT (memap,AI_START_STOP_Select,set,AI_STOP_Sync,1);
	NIBIT (memap,AI_START_STOP_Select,set,AI_STOP_Edge,multiChannel);
	NIREG (memap,AI_START_STOP_Select,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 *   ai_set_fifo_request_mode ()
 */

void ai_set_fifo_request_mode (u8 ** memap) {

	/* Not_Empty -> Full 28-06-07 m.c. */

	/* NIBIT (memap,AI_Mode_3,set,AI_FIFO_Mode,_kAI_FIFO_ModeFull); */
	NIBIT (memap,AI_Mode_3,set,AI_FIFO_Mode,_kAI_FIFO_ModeNot_Empty);
	return;     
}

/*
 *   ai_start ()
 */

void ai_start (u8 ** memap) {

	NIBIT (memap,AI_Command_2,set,AI_START1_Pulse,1);
	NIREG (memap,AI_Command_2,flush);

	return;
}

/*
 *   ai_start_on_demand ()
 */

void ai_start_on_demand (u8 ** memap) {

	NIBIT (memap,AI_Command_2,set,AI_START_Pulse,1);
	NIREG (memap,AI_Command_2,flush);

	return;
}

/*
 *   ai_trigger ()
 */

void ai_trigger (u8 ** memap,
                 int startSource,
                 int startPolarity,
                 int refSource,
                 int refPolarity) {

	NIBIT (memap,Joint_Reset,set,AI_Configuration_Start,1);
	NIREG (memap,Joint_Reset,flush);

	NIBIT (memap,AI_Mode_1,set,AI_Trigger_Once,1);
	NIREG (memap,AI_Mode_1,flush);

	/* start trigger signal */

	NIBIT (memap,AI_Trigger_Select,set,AI_START1_Select,startSource);
	NIBIT (memap,AI_Trigger_Select,set,AI_START1_Polarity,startPolarity);
	NIBIT (memap,AI_Trigger_Select,set,AI_START1_Edge,1);
	NIBIT (memap,AI_Trigger_Select,set,AI_START1_Sync,1);
	NIREG (memap,AI_Trigger_Select,flush);

	/* reference trigger signal */

	if( refSource == _kAI_START2_SelectPulse)
		NIBIT (memap,AI_Mode_2,set,AI_Pre_Trigger,0);
	else
		NIBIT (memap,AI_Mode_2,set,AI_Pre_Trigger,1);
	NIREG (memap,AI_Mode_2,flush);

	NIBIT (memap,AI_Trigger_Select,set,AI_START2_Select,refSource);
	NIBIT (memap,AI_Trigger_Select,set,AI_START2_Polarity,refPolarity);
	NIBIT (memap,AI_Trigger_Select,set,AI_START2_Edge,1);
	NIBIT (memap,AI_Trigger_Select,set,AI_START2_Sync,1);
	NIREG (memap,AI_Trigger_Select,flush);

	NIBIT (memap,Joint_Reset,set,AI_Configuration_End,1);
	NIREG (memap,Joint_Reset,flush);

	return;
}

/*
 *   analog_trigger_reset ()
 */

void analog_trigger_reset (u8 ** memap) {

	NIBIT (memap,Analog_Trigger_Etc,set,Analog_Trigger_Reset,1);
	NIBIT (memap,Analog_Trigger_Etc,set,Analog_Trigger_Mode,
	       _kAnalog_Trigger_ModeLow_Window);
	NIREG (memap,Analog_Trigger_Etc,flush);

	NIBIT (memap,Analog_Trigger_Control,set,Analog_Trigger_Select,
	       _kAnalog_Trigger_SelectGround);
	NIREG (memap,Analog_Trigger_Control,flush);

	NIREG (memap,Gen_PWM,0,write,0);
	NIREG (memap,Gen_PWM,1,write,0);

	NIBIT (memap,Analog_Trigger_Etc,set,Analog_Trigger_Enable,
	       _kAnalog_Trigger_EnableDisabled);
	NIREG (memap,Analog_Trigger_Etc,flush);

	return;
}

void board_reset(u8 ** memap) {

	/*  The original board_reset() was intended to
	 *  clear the shadow register area; it is unnecessary
	 *  now, as shadow area is cleared at allocation time
	 *  and it is not a good idea to modify it later.
	 */
}

/*
 *   configure_timebase ()
 */

void configure_timebase (u8 ** memap) {

	NIBIT (memap,Clock_and_FOUT,set,Slow_Internal_Timebase,1);
	NIREG (memap,Clock_and_FOUT,flush);

	return;
}

/*
 *   ao section
 */

/*
 *   ao_arm ()
 */

void ao_arm (u8 ** board) {
	NIBIT(board,AO_Mode_3,write,AO_Not_An_UPDATE,1);
	NIBIT(board,AO_Mode_3,write,AO_Not_An_UPDATE,0);

	while (NIBIT(board,Joint_Status_2,read,AO_TMRDACWRs_In_Progress_St)) {
		// Wait
	}

	NIBIT(board,AO_Command_1,set,AO_UI_Arm,1);
	NIBIT(board,AO_Command_1,set,AO_UC_Arm,1);
	NIBIT(board,AO_Command_1,set,AO_BC_Arm,1);
	NIREG(board,AO_Command_1,flush);

	return;
}

/*
 *   ao_channel_select ()
 */

void ao_channel_select (u8 ** board, u32 numberOfChannels) {
	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);

	numberOfChannels--;

	if(!numberOfChannels) {
		//single channel
		NIBIT(board,AO_Mode_1,set,AO_Multiple_Channels,0);
		NIBIT(board,AO_Output_Control,set,AO_Number_Of_Channels,0);
	} else {
		// multiple channels
		NIBIT(board,AO_Mode_1,set,AO_Multiple_Channels,1);
		NIBIT(board,AO_Output_Control,set,
		      AO_Number_Of_Channels,numberOfChannels);
	}

	NIREG(board,AO_Mode_1,flush);
	NIREG(board,AO_Output_Control,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);

	return;     
}

/*
 *    ao_clear_fifo ()
 */

void ao_clear_fifo (u8 ** board) {

	NIREG(board,AO_FIFO_Clear,write,1);
	return; 
}

/*
 *   ao_configure_dac ()
 */

void ao_configure_dac (u8 ** board, 
		       u32 dac, 
		       u32 waveformOrder, 
		       int polarity,
		       int  mode) {

	if (dac > _kAO_Config_BankArraySize-1) return;

	NIBIT(board,AO_Config_Bank,dac,set,
	      AO_DAC_Offset_Select,0);  // aognd offset
	// 10V Internal reference
	NIBIT(board,AO_Config_Bank,dac,set,AO_DAC_Reference_Select,0);
	NIBIT(board,AO_Config_Bank,dac,set,AO_Update_Mode,mode);    
	NIBIT(board,AO_Config_Bank,dac,set,AO_DAC_Polarity,polarity);
	NIREG(board,AO_Config_Bank,dac,flush);    

	NIREG(board,Static_AI_Control,dac+4,
	      write,0); //reference attenuation off

	NIREG(board,AO_Waveform_Order,dac,write,waveformOrder);

	return;
}

/*
 *   ao_count ()
 */

void ao_count (u8 ** board,
	       u32 numberOfSamples,
	       u32 numberOfBuffers,
	       tBoolean continuous) {

	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);   

	int continuousMode; 

	continuousMode = continuous ?
		_kAO_ContinuousIgnore_BC_TC : _kAO_ContinuousStop_On_BC_TC;

	NIBIT(board,AO_Mode_1,set,AO_Continuous,continuousMode);
	NIBIT(board,AO_Mode_1,set,AO_Trigger_Once,continuous? 0:1);
	NIREG(board,AO_Mode_1,flush);

	NIBIT(board,AO_Mode_2,set,AO_BC_Initial_Load_Source,
	      _kAO_BC_Initial_Load_SourceReg_A);
	NIBIT(board,AO_Mode_2,set,AO_UC_Initial_Load_Source,
	      _kAO_UC_Initial_Load_SourceReg_A);
	NIREG(board,AO_Mode_2,flush);

	NIREG(board,AO_BC_Load_A,write,numberOfBuffers - 1);
	NIREG(board,AO_UC_Load_A,write,numberOfSamples - 1);     

	NIBIT(board,AO_Command_1,set,AO_BC_Load,1);
	NIBIT(board,AO_Command_1,set,AO_UC_Load,1);
	NIREG(board,AO_Command_1,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);   

	return;
}

/*
 * ao_disarm ()
 */

void ao_disarm (u8 ** board) {
	NIBIT(board,AO_Command_1,set,AO_Disarm,1);
	NIREG(board,AO_Command_1,flush);

	return; 
}

/*
 *   ao_fifo_mode ()
 */

void ao_fifo_mode (u8 ** board, tBoolean fifoRetransmit) {
	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);

	NIBIT(board,AO_Mode_2,set,AO_FIFO_Retransmit_Enable,fifoRetransmit);
	NIBIT(board,AO_Mode_2,set,AO_FIFO_Mode, _kAO_FIFO_ModeLess_Than_Full);
	NIREG(board,AO_Mode_2,flush);

	NIBIT(board,AO_Personal,set,AO_FIFO_Enable,1);
	NIREG(board,AO_Personal,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);

	return;        
}

/*
 *   ao_personalize ()
 */

void ao_personalize (u8 ** board) {

	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);

	NIBIT(board,AO_Personal,set,AO_UPDATE_Pulse_Timebase,
	      _kAO_UPDATE_Pulse_TimebaseSelect_By_PulseWidth);
	NIBIT(board,AO_Personal,set,AO_UPDATE_Pulse_Width,
	      _kAO_UPDATE_Pulse_WidthAbout_3_TIMEBASE_Periods);
	NIBIT(board,AO_Personal,set,AO_TMRDACWR_Pulse_Width,
	      _kAO_TMRDACWR_Pulse_WidthAbout_2_TIMEBASE_Periods);
	NIBIT(board,AO_Personal,set,AO_Number_Of_DAC_Packages,
	      _kAO_Number_Of_DAC_PackagesSingle_DAC_mode);
	NIREG(board,AO_Personal,flush);

	NIBIT(board,AO_Output_Control,set,AO_UPDATE_Output_Select,
	      _kAO_UPDATE_Output_SelectHigh_Z);
	NIREG(board,AO_Output_Control,flush);

	NIBIT(board,AO_Mode_3,set,AO_Last_Gate_Disable,1);
	NIREG(board,AO_Mode_3,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);

	return;
}

/*
 *   ao_reset ()
 */

void ao_reset (u8 ** board) {

	NIBIT(board,Joint_Reset,set,AO_Reset,1);
	NIREG(board,Joint_Reset,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);

	NIBIT(board,AO_Command_1,set,AO_Disarm,1);
	NIREG(board,AO_Command_1,flush);

	NIBIT(board,Interrupt_B_Enable,set,AO_BC_TC_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_START1_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_UPDATE_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_START_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_STOP_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_Error_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_UC_TC_Interrupt_Enable,0);
	NIBIT(board,Interrupt_B_Enable,set,AO_FIFO_Interrupt_Enable,0);
	NIREG(board,Interrupt_B_Enable,flush);

	NIBIT(board,AO_Personal,set,
	      AO_BC_Source_Select, _kAO_BC_Source_SelectUC_TC);
	NIREG(board,AO_Personal,flush);

	NIBIT(board,Interrupt_B_Ack,set,AO_BC_TC_Trigger_Error_Confirm,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_BC_TC_Error_Confirm,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_UC_TC_Interrupt_Ack,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_BC_TC_Interrupt_Ack,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_START1_Interrupt_Ack,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_UPDATE_Interrupt_Ack,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_START_Interrupt_Ack,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_STOP_Interrupt_Ack,1);
	NIBIT(board,Interrupt_B_Ack,set,AO_Error_Interrupt_Ack,1);
	NIREG(board,Interrupt_B_Ack,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);

	return;
}

/*
 *   ao_reset_waveform_channels ()
 */

void ao_reset_waveform_channels (u8 ** board) {

	u32 i; 

	for (i=0; i <  /*tMSeries_*/ _kAO_Config_BankArraySize; i++) {
		NIBIT(board,AO_Config_Bank,i,write,AO_Update_Mode,
		      _kAO_Update_ModeImmediate);
	}

	for (i=0; i <  /*tMSeries_*/ _kAO_Waveform_OrderArraySize; i++) {
		NIREG(board,AO_Waveform_Order,i,write,0xF);
	}

	return;
}

/*
 *   ao_start ()
 */

void ao_start (u8 ** board) {
	NIBIT(board,AO_Command_2,write,AO_START1_Pulse,1);

	return;
}

/*
 *   ao_stop ()
 */

void ao_stop (u8 ** board) {
	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);

	NIBIT(board,AO_Mode_3,set,AO_Stop_On_BC_TC_Error,0);
	NIBIT(board,AO_Mode_3,set,AO_Stop_On_BC_TC_Trigger_Error,1);
	NIBIT(board,AO_Mode_3,set,AO_Stop_On_Overrun_Error,1);
	NIREG(board,AO_Mode_3,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);

	return; 
}
/*
 *   ao_trigger ()
 */

void ao_trigger (u8 ** board, 
                 int source,
		 int polarity) {
	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);   

	NIBIT(board,AO_Trigger_Select,set,AO_START1_Select,source);
	NIBIT(board,AO_Trigger_Select,set,AO_START1_Polarity,polarity);
	NIBIT(board,AO_Trigger_Select,set,AO_START1_Edge,1);
	NIBIT(board,AO_Trigger_Select,set,AO_START1_Sync,1);
	NIREG(board,AO_Trigger_Select,flush);

	NIBIT(board,AO_Mode_3,set,
	      AO_Trigger_Length, _kAO_Trigger_LengthDA_START1);
	NIREG(board,AO_Mode_3,flush);

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush); 

	return; 
}

/*
 *   ao_update ()
 */

void ao_update (u8 ** board, 
		int source, 
		int polarity, 
                u32 periodDivisor) {
	tBoolean internalUpdate;

	internalUpdate = (source == _kAO_UPDATE_Source_SelectUI_TC);

	NIBIT(board,Joint_Reset,set,AO_Configuration_Start,1);
	NIREG(board,Joint_Reset,flush);

	NIBIT(board,AO_Command_2,set,AO_BC_Gate_Enable,internalUpdate ? 0 : 1);
	NIREG(board,AO_Command_2,flush);

	NIBIT(board,AO_Mode_1,set,AO_UPDATE_Source_Select,source);
	NIBIT(board,AO_Mode_1,set,AO_UPDATE_Source_Polarity,polarity);
	NIREG(board,AO_Mode_1,flush);

	if(internalUpdate) {
		NIREG(board,AO_UI_Load_A,write,1);
		NIBIT(board,AO_Command_1,write,AO_UI_Load,1);
		NIREG(board,AO_UI_Load_A,write, periodDivisor );
	}

	NIBIT(board,Joint_Reset,set,AO_Configuration_End,1);
	NIREG(board,Joint_Reset,flush);

	return;        
}

/*
 *   pll_reset ()
 */

void pll_reset (u8 ** memap) {

	NIBIT (memap,Clock_And_Fout2,set,TB1_Select,_kTB1_SelectSelect_OSC);
	NIBIT (memap,Clock_And_Fout2,set,TB3_Select,_kTB3_SelectSelect_OSC);
	NIREG (memap,Clock_And_Fout2,flush);

	NIBIT (memap,PLL_Control,set,PLL_Enable,kFalse);
	NIREG (memap,PLL_Control,flush);

	return;
}

/*
 *  read eeprom area
 */

void read_from_eeprom (u8 ** memap, u32 offset,
		       u8 *buffer, u32 size) {

	u32 iowcr1;
	u32 iowbsr1;
	u32 iodwbsr; 
	u32 bar1Value;

	u8 * bar0;
	u8 * bar1;

	bar0 = memap[2];
	bar1 = memap[0];

	/* map the eeprom area */

	iodwbsr = read32 (bar0, 0xC0);          /* save register area setup */
	write32 (bar0, 0xC0, 0);                     /* unmap register area */

	iowbsr1 = read32 (bar0, 0xC4);            /* save eeprom area setup */
	bar1Value      = read32 (bar0, 0x314);          /* get bar1 address */

	write32 (bar0, 0xC4, 0x0000008B | bar1Value); /* map eeprom area
							 to bar1 */

	iowcr1 = read32 (bar0, 0xF4); /* save magic numbers for eeprom area */
	write32 (bar0, 0xF4, 0x00000001 | iowcr1);     /* set magic numbers */
	write32 (bar0, 0x30, 0xF);

	/* read the eeprom area from bar1 */

	u32 i;
	for( i = 0; i < size; ++i) {

		buffer[i] = bar1[i + offset];
	}

	/* restore everything as found */

	write32 (bar0, 0xC4, iowbsr1); /* unmap and eprom area and
					  restore setup */
	write32 (bar0, 0xC0, iodwbsr);                /* remap register area */
	write32 (bar0, 0xF4, iowcr1);   /* restore eeprom area magic numbers */
	write32 (bar0, 0x30, 0x00);
}

/*
 *    read calibration info from eeprom
 */

void eeprom_read_MSeries(u8 ** memap, u8 *buffer, u32 size) {

	u32 kStartCalEEPROM = 1024;

	read_from_eeprom(memap, kStartCalEEPROM, buffer, size);
}

/*
 *    read serial number from eeprom
 */

void serial_number_MSeries(u8 ** memap, u32 *serial_number) {

	read_from_eeprom(memap, 4, (u8 *) serial_number, 4);

	/* serial number is returned as big-endian u32 */

	*serial_number = ReadBigEndianU32(*serial_number);
}
