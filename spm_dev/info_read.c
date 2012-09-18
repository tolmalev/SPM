/*
 *    info_read - read the current feedback loop parameters
 *
 *    A 'small' request of exactly sizeof(struct info_request) bytes
 *    will be filled with such a structure (binary data for machines);
 *    a 'large' request (256 bytes or more) will be filled with
 *    equivalent ascii data for humans. Other requests are discarded
 *    with -EINVAL.
 *
 *    'struct info_request' is defined in 'include/spm_dev_data.h'
 */

#define INFO_FORMAT \
"                  %8s %8s\n"\
"cadence_usec      %8d %8d\n"\
"lines_per_frame   %8d %8d\n"\
"points_per_line   %8d %8d\n"\
"samples_per_point %8d %8d\n"\
"sample_adc        %8d %8d\n"\
"sample_dac        %8d %8d\n"\
"slot_size         %8d %8d\n"\
"buffer_size       %8d %8d\n"

ssize_t info_read  (struct file * filp,
                    char __user * buf,
                    size_t byte_count,
                    loff_t * offset) {

	int n, leng, not_copied;
	char info[512];
	struct info_request info_bin;

	if (*offset) return 0;

	/* request for binary data structure */

	if (byte_count == sizeof(info_bin)) {

                info_bin.now.lines_per_frame = irq.lines_per_frame;
                info_bin.now.points_per_line = irq.points_per_line;
                info_bin.now.cadence_usec = irq.cadence_usec;
                info_bin.now.slot_size = irq.slot_size;
                info_bin.now.buffer_size = irq.buffer_size;
                info_bin.now.samples_per_point = irq.samples_per_point;
                info_bin.now.sample_adc = irq.sample_adc;
                info_bin.now.sample_dac = irq.sample_dac;

                info_bin.new.lines_per_frame = lines_per_frame;
                info_bin.new.points_per_line = points_per_line;
                info_bin.new.cadence_usec = cadence_usec;
                info_bin.new.slot_size = slot;
                info_bin.new.buffer_size = buffer_size;
                info_bin.new.samples_per_point = samples_per_point;
                info_bin.new.sample_adc = sample_adc;
                info_bin.new.sample_dac = sample_dac;

                not_copied = copy_to_user(buf, &info_bin, sizeof(info_bin));
                return byte_count - not_copied;

	} else if (byte_count >= 256) {

	/* send data in ascii form  */

                n = snprintf (info,sizeof(info), INFO_FORMAT,
                              "now", "new",
                              irq.cadence_usec, cadence_usec,
                              irq.lines_per_frame, lines_per_frame,
                              irq.points_per_line, points_per_line,
                              irq.samples_per_point, samples_per_point,
                              irq.sample_adc, sample_adc,
                              irq.sample_dac, sample_dac,
                              irq.slot_size, slot,
                              irq.buffer_size, buffer_size);

                leng = strlen(info);
                n = byte_count > leng ? leng : byte_count;

                not_copied = copy_to_user(buf, info, n);
                *offset = n;

                return n;
        } else {
                return -EINVAL;
        }
}
