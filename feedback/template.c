/*
 * Copyright (C) 2008-2009 Marcello Carla'
 *
 * This program is free software; you can use, modify and redistribute
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (www.fsf.org).
 *
 * Address of the author:
 * Department of Physics, University of Florence
 * Via G. Sansone 1
 * Sesto F.no (Firenze) I 50019 - Italy
 * e-mail: carla@fi.infn.it
 */

/*
 *   This file contains informations and a template for the function to
 *   be executed as a part of the interrupt service routine to implement
 *   the user feedback algorithm.
 *
 *   The function is called by spm_irq_handler(). A data structures is
 *   passed as argument, as declared in the function prototype:
 *
 *      void feedback_code (struct irq_parameters * irq)
 *   
 *   The 'struct irq_parameters * irq' structure is defined in
 *   'include/spm_dev_data.h' and contains all data connected in any
 *   way with the interrupt machinery.
 *
 *   Here is a description of the fields of some interest for the user
 *   algorithm. All fields in the 'irq' structure that are not declared
 *   explicitely to be writable for the user are
 *
 *                      *** ABSOLUTELY READ ONLY ***
 *
 *   If the user code modifies anyone of these fields, a system crash
 *   will be the most likely end of the scan experiment.
 *
 *   irq->action
 *              a flag that can have two values: 'Idle' or 'Go_Process'.
 *              In the 'Idle' state no scan is currently performed.
 *              Neverthless, the feedback loop for the Z axis must be kept
 *              active and efficient or very likely the probe tip will
 *              crash onto the sample surface.
 *
 *   irq->cadence_usec
 *              this field contains the rate in usec of the interrupt request
 *              generation; the feedback code may need it to build its own
 *              timescale; rely only on this value to measure the time  in
 *              the interrupt code; there can be a significant random skew
 *              between the time counted this way and the content of
 *              data->irq_time (see below).
 *
 *   irq->samples_per_point
 *		how many data vectors the ADC puts into the event buffer
 *              for each event (point).
 *
 *   irq->sample_adc
 *              how many ADC conversions the ADC performs at the end of
 *              every cadence_usec period.
 *   
 *   irq->sample_dac
 *              how many values can be stored in the DAC vector for each
 *              event.
 *
 *   irq->adc
 *              pointer to the ADC data vector of the current event.
 *
 *   irq->dac
 *              pointer to the DAC data vector of the current event.
 *
 *   irq->data
 *              pointer to the 'event_head' structure, described below,
 *              of the current event. This structure, closely packed with
 *              the DAC and ADC data vectors, is the 'event' data block
 *              that will be passed to user space. 
 *
 *                             *** PLEASE, NOTE ***
 *
 *              Only the previous fields are guaranteed to be always
 *              meaningful; all other fields of the structure are meaningful
 *              only in the Go_Process state and are guaranteed to be
 *              absolutely *** meaningless *** in the Idle state.
 *
 *              The 'pay_load' area may even not be mapped in memory. A
 *              good feedback routine will completely ignore the following
 *              fields as long as irq->action is 'Idle'.
 *
 *   irq->points_per_line
 *   irq->lines_per_frame
 *              constants that define the size of the frame scan; they
 *              come directly from the user space request.
 * 
 *   irq->points
 *   irq->lines
 *              these are the counters of the frame scan progress.
 *              They count downward:
 *
 *                  line==lines_per_frame    on first line in the frame
 *                  line==1                  on last line in the frame
 *
 *                  points==points_per_line  on first point in the line
 *                  points==1                on last point in the line
 *
 *   irq->sync
 *              a flag that is set to 1 exclusively when the feedback function
 *              is called to process the first point of the first line of a
 *              new frame, otherwise it is set to 0.
 *
 *   irq->request
 *              a flag that the user code can safely set to 'Go_Idle' and ask
 *              for the immediate end of the frame scan.
 *              All data collected up to this point will be delivered to the
 *              user space process, thereafter an EOF will be declared.
 *
 *   irq->pay_load
 *   irq->pay_load_size
 *              'pay_load' is a pointer to a free area, 'pay_load_size'
 *              bytes wide, where private data can be passed from the
 *              user program that is requesting the scan (see par. 6.2
 *              in README).
 *              
 *                      * * * B I G   W A R N I N G * * *
 *              
 *              When the scan process is not active, I mean when 
 *              irq->action == Idle, 'pay_load' will point to some not
 *              mapped memory. The feedback code must be absolutely careful
 *              not to try to dereference pay_load while in the Idle state.
 *
 *   There are several other fields in the irq structure, that are of no
 *   direct use to the user feedback code, but are needed, e.g., by the
 *   EVENT macros defined below.
 *   They are described in include/spm_dev_data.h.
 *
 *
 *   The 'struct event_head * data' structure is part of the data block
 *   that will be passed to the user space program with the ADC and DAC
 *   data. It is defined in 'include/spm_dev_data.h'.
 *   Its fields are:
 *
 *   irq->data->irq_time
 *              a structure with two int32 values, sec and nsec; it contains
 *              the time the interrupt handler function has been entered.
 *
 *   irq->data->n_adc
 *              the number of used ADC channels.
 *
 *   irq->data->n_dac
 *              the lenght of the DAC data vector, where the user code
 *              can write the results of the feedback algorithm.
 *
 *   irq->data->samples
 *              the number of ADC readings in each event.
 *              The ADC data vector contains n_adc*samples values.
 *
 *   irq->data->adc_time
 *              the time (in tens of nsec) it took to read all data from
 *              the ADC's (for diagnostics and performance test purpose).
 *
 *   irq->data->service_time
 *              the total time (in usec) used to complete the interrupt
 *              service procedure (for diagnostics and test purpose).
 *              
 *   irq->data->byte[0/1]
 *              on exit of the interrupt service routine, the eight bit
 *              value the user code puts into byte[0] will be written to
 *              eight digital output lines on the master board, the bits
 *              inbyte[1] to the slave board. These lines can be used to
 *              control on/off operations in the apparatus, e.g. to switch
 *              on/off a motor or a relay.
 *
 *   irq->data->r_adc
 *              this is a diagnostic flag. It contains the number of
 *              values read from the ADC's and stored into the irq->adc[]
 *              vector. It should be n_adc*samples; it can be a different
 *              value if something is going wrong with the DAQ board. It
 *              should be very unlikely, but it may mean that one or two
 *              interrupt requests have not been serviced in due time and
 *              data have piled-up into the ADC's fifo. The user feedback
 *              code should handle thise condition, either recovering the
 *              anomaly or declaring the failure of the experiment through
 *              the irq->request flag described above. In any case, the
 *              irq->adc[] vector contains the oldest ADC's value, while
 *              the most recent readings are discarded.
 */

/*
 *   In the next lines it is possible to declare any required parameters,
 *   that will be made accessible through the /sys filesystem. These
 *   parameters will appear as files in the /sys/module/spm_fc/parameters/
 *   directory, hence they will also appear in the 'gamp' window for the
 *   spm_fc module. It will be possible to change their value at wish. But,
 *   please, keep in mind that the new values will be seen by the feedback
 *   code soon, at the occurrence of the very next interrupt, i.e. in a few
 *   usec. So, be careful.
 *
 *   The following two lines define a counter of the processed events
 */

static int counter = 0;
module_param (counter, int, S_IRUGO | S_IWUSR);

/*
 *   MACRO'S
 *
 *   While in the Go_Process state, there are macros (defined in spm_bac.h)
 *   to retrieve the previous n-th event in the buffer. The buffer is always
 *   guaranteed to contain the current line and at least a full previous
 *   line. It is to you to ask for an existing n-th event. An illegal request
 *   here may crash your system. In the Idle state, the macros always returns
 *   the current event.
 *
 *   EVENT(n) pointer to the 'struct events_head' data structure of the
 *            previous n-th event.
 *
 *   EVENT_DAC(n) pointer to the DAC data vector of previous n-th event.
 *   EVENT_ADC(n) pointer to the ADC data vector of previous n-th event.
 *
 *   EVENT_SAMPLE(n,m) pointer to the ADC data vector of sample m of
 *                     previous n-th event.
 *
 *   EVENT(0) is the current event, n < 0 is converted always to the
 *   current event. 
 *
 *
 *   Examples:
 *
 *   ADC 'i' in event 'j' can be read with:
 *
 *              val = EVENT_ADC(j)[i];
 *
 *   ADC 'i' in sample 'j' of event 'k' can be obtained with:
 *
 *              val = EVENT_SAMPLE(k,j)[i];
 *
 *   The DAC 3 and byte 0 fields of the current event, to be sent to the
 *   DAQ board outputs, can be filled with:
 *
 *              EVENT_DAC(0)[3] = something;
 *              EVENT(0)->byte[0] = a_value;
 *
 *   or, more directly, with:
 *
 *              irq->dac[3] = something;
 *              irq->data->byte[0] = a_value;
 *
 *   The two pointers irq->adc and irq->dac are the most direct (hence,
 *   efficient) way to handle adc and dac vectors of current event. The
 *   EVENT macros make the current event to appear also as element zero
 *   in the same vector of previous events.
 */

/*
 *   A dummy feedback code follows, to be replaced with the actual code.
 *   For test purposes, the dummy code writes to the DAC's the reading
 *   from the ADC's.
 *
 *   A final note. Remember: feedback_code() will be a function of the
 *   kernel and will be executed in interrupt context. 
 *   So:
 *      no floating point;
 *      no libc (but most of the resources of libc are supplied by
 *          equivalent routines in the kernel);
 *      be fast: other parts of the system are waiting for the CPU;
 *      never sleep: if your code sleeps for a while, your system will
 *          sleep forever;
 *      no exception: an arithmetic exception, like divide by zero,
 *          will freeze the kernel.
 */

void feedback_code (struct irq_parameters * irq) {

        int i;

        /* copy available ADCs to available DACs */

        for ( i=0 ; i < irq->sample_dac && i < irq->sample_adc ; i++ ) {
               irq->dac[i] = irq->adc[i];
        }

        /* copy the previous value of ADC 0 to last DAC */

        if (i) irq->dac[i-1] = EVENT_ADC(1)[0];

        /* put some value for the digital output lines */

	irq->data->byte[0] = 17;
	irq->data->byte[1] = 18;

        /* increment the event counter, readable in tha gamp window */

	counter++;
}
