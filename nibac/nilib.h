/*
 * nilib.h
 */
 
/*
 *    LittleEndian=1 or BigEndian=1 defines
 *    the CPU endianness
 */
 
#define LittleEndian 1

#if LittleEndian

#define ReadLittleEndianU32(x) (x)
#define ReadLittleEndianU16(x) (x)

#define ReadBigEndianU32(x) (SwitchEndianU32(x))
#define ReadBigEndianU16(x) (SwitchEndianU16(x))

#elif BigEndian

#define ReadLittleEndianU32(x) (SwitchEndianU32(x))
#define ReadLittleEndianU16(x) (SwitchEndianU16(x))

#define ReadBigEndianU32(x) (x)
#define ReadBigEndianU16(x) (x)
#else
#error target endianness not specified
#endif

u8 ** acquire_board (char *, int, int *);
void dump_memory (u8 *, u32);

void board_reset (u8 **);
void ai_reset (u8 **);
void ai_personalize (u8 **, int);
void ai_clear_fifo (u8 **);
void ai_disarm (u8 **);
void ai_clear_configuration_memory (u8 **);
void ai_configure_channel (u8 **, u16, u16, int, int, tBoolean);
void ai_set_fifo_request_mode (u8 **);
void ai_environmentalize (u8 **);
void ai_hardware_gating (u8 **);
void ai_trigger (u8 **, int, int, int, int);
void ai_sample_stop (u8 **, tBoolean);
void ai_number_of_samples (u8 **, u32, u32, tBoolean);
void ai_sample_start (u8 **, u32, u32, int,int);
void ai_convert (u8 **, u32, u32, tBoolean);
void ai_arm (u8 **, tBoolean);
void ai_start (u8 **);

void configure_timebase (u8 **);
void pll_reset (u8 **);
void analog_trigger_reset (u8 **);
void ao_reset (u8 **);
void ao_personalize (u8 **);
void ao_reset_waveform_channels (u8 **);
void ao_clear_fifo (u8 **);
void ao_configure_dac (u8 **, u32, u32, int, int);
void ao_channel_select (u8 **, u32);
void ao_trigger (u8 **, int, int);
void ao_count (u8 **, u32, u32, tBoolean);
void ao_update (u8 **, int, int, u32);
void ao_fifo_mode (u8 **, tBoolean);
void ao_stop (u8 **);
void ao_arm (u8 **);
void ao_start (u8 **);
void ao_disarm (u8 **);

void read_from_eeprom (u8 **, u32, u8 *, u32);
void eeprom_read_MSeries (u8 **, u8 *, u32);

/*
 *
 * aiGetScalingCoefficients --
 *
 * modeIdx
 *    0 -> default
 *
 * intervalIdx
 *    0 -> +/- 10V
 *    1 -> +/- 5V
 *    2 -> +/- 2V
 *    3 -> +/- 1V
 *    4 -> +/- 500mV
 *    5 -> +/- 200mV
 *    6 -> +/- 100mV
 *
 * channel
 *    ignored - all channels use the same ADC
 *
 *
 *
 * aoGetScalingCoefficients --
 *
 * modeIdx
 *    ignored - AO does not use the mode constants
 *
 * intervalIdx
 *    0 -> 10V reference
 *    1 -> 5V reference
 *    2 -> 2V reference
 *    3 -> 1V reference
 *
 * channel
 *   dac number
 */

struct scale_table {
	u32 order;
	f32 c[4]; 
};

void ai_get_scaling_coefficients (u8 *, u32, u32, u32, struct scale_table *);
void ai_polynomial_scaler (i32 *, f32 *, struct scale_table *);
void ao_get_scaling_coefficients (u8 *, u32, u32, u32, struct scale_table *);
void ao_linear_scaler (i32 *, f32 *, struct scale_table *);

/* adc_elacs takes a float (volt) and returns an integer (channel number);
   dac_elacs takes an integer (channel number) and returns a float (volt) */

int adc_elacs (float , struct scale_table *);
float dac_elacs (int , struct scale_table *);
