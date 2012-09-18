
/*
 * access to NI-DAQ registers is through the NIREG/NIBIT functions:
 *
 * NIREG (memap,register,[index],command,[value])
 * NIBIT (memap,register,[index],command,field,[value])
 *
 * memap:    two-pointers array to register memory area and back copy
 * register: string constant with register name
 * index:    (only for indexed registers) register index
 * command:  string constant
 *              set: write value to register back copy
 *              get: read value from back copy
 *              write: write value to register and back copy
 *              read: update back copy from register and read value
 *              flush: write back copy to register
 *              refresh: update back copy from register
 *              initialize: TODO
 * field:    string constant with register bit field
 * value:    (only for write and set) numerical value to be written
 *          
 */

#include "tMSeries.register_names.h"
#include "tMSeries.register_fields.h"
#include "tMSeries.constant.h"

/*
#include "tTIO.register_names.h"
#include "tTIO.register_fields.h"
#include "tTIO.constant.h"
*/

/*
 *
 *  substitution for register operation
 *
 */

#define NIREG(memap,register,...) \
        NI_REG_OP1(register,memap,register ## _ , __VA_ARGS__)

#define NI_REG_OP1(register,memap,parameters, ...) \
        NI_REG_OP2(register,memap,parameters, __VA_ARGS__)

#define NI_REG_OP2\
(register,memap,chip,offset,type,size,mask,alg,index,operation, ...) \
_nireg ## _ ## operation ## _x (register, chip,offset, NI_IND_ALG_ ## alg, \
                               index, type, size, mask, memap, __VA_ARGS__)

/*
 *
 *  substitution for bit field operation
 *
 */

#define NIBIT(memap,register, ...) \
        NI_BIT_OP1(register,memap, register ## _ , __VA_ARGS__)

#define NI_BIT_OP1(register,memap,parameters, ...) \
        NI_BIT_OP2(register,memap,parameters, __VA_ARGS__)

#define NI_BIT_OP2\
(register,memap,chip,offset,type,size,mask,alg,index,operation,field, ...) \
NI_BIT_OP3(register,memap,chip,offset,type,size,mask,alg,index,operation, \
                                     register ## __ ## field, __VA_ARGS__)

#define NI_BIT_OP3\
(register,memap,chip,offset,type,size,mask,alg,index,operation,subfields, ...) \
NI_BIT_OP4(register,memap,chip,offset,type,size,mask,alg,index,operation, \
                                                     subfields, __VA_ARGS__)

#define NI_BIT_OP4\
(register,memap,chip,offset,type,size,mask,alg,index,operation,bit_pos,\
bit_mask,...)\
_nibit ## _ ## operation ## _x (register, chip, offset, NI_IND_ALG_ ## alg, \
index, type, size, mask,memap, bit_pos, bit_mask, __VA_ARGS__)

/*
 *
 *  algorithms for array index computation
 *
 */

#define NI_IND_ALG_0(offset,index) offset
#define NI_IND_ALG_1(offset,index) offset+(index)*4
#define NI_IND_ALG_2(offset,index) offset+(index)*2
#define NI_IND_ALG_3(offset,index) offset+((index)?(index)+0x260:64)

/*
 *
 *  true register access
 *
 */
 
#ifdef __KERNEL__

#define BOARD_REGISTER_R(size,memap,true_offset) \
                               ioread ## size((memap)[0]+true_offset)

#define BOARD_REGISTER_W(size,memap,true_offset,value) \
                     iowrite ## size (value, (memap)[0]+true_offset)

#else

#define BOARD_REGISTER_R(size,memap,true_offset) \
                                    *((u ## size *) ((memap)[0]+true_offset))

#define BOARD_REGISTER_W(size,memap,true_offset,value) \
                          (*((u ## size *) ((memap)[0]+true_offset)) = value)

#endif /* __KERNEL__ */

#define SHADOW_REGISTER(chip,memap,register,index) \
         (((struct shadow_ ## chip ## _registers *) (memap)[1])->register[index])

/*
 *
 *  get - read value from back copy
 *
 */

#define _nireg_get_x(register,chip,offset,alg,index,type,size,mask,memap,...) \
                          SHADOW_REGISTER(chip,memap,register,index)

#define _nibit_get_x\
(register,chip,offset,alg,index,type,size,mask,memap,bit,bit_mask,...) \
  ((SHADOW_REGISTER(chip,memap,register,index) & (u ## size) ~bit_mask) >> bit)

/*
 *
 *  set - store value into back copy
 *
 */

#define _nireg_set_x\
(register,chip,offset,alg,index,type,size,mask,memap,value) \
                     (SHADOW_REGISTER(chip,memap,register,index) = value)

#define _nibit_set_x\
(register,chip,offset,alg,index,type,size,mask,memap,bit,bit_mask,value) \
	(SHADOW_REGISTER(chip,memap,register,index) = \
        (SHADOW_REGISTER(chip,memap,register,index) & (u ## size) bit_mask) | \
                   (((u ## size) value << bit) & (u ## size) ~bit_mask))

/*
 *  read - read from back copy after updating from memap
 */

#define _nireg_read_x\
(register,chip,offset,alg,index,type,size,mask,memap,...) \
        _nireg_read_type ## _ ## type \
                          (register,chip,offset,alg,index,size,mask,memap,...)

#define _nireg_read_type_read\
(register,chip,offset,alg,index,size,mask,memap,...) \
        (SHADOW_REGISTER(chip,memap,register,index) = \
                          BOARD_REGISTER_R(size,memap,alg(offset,index)))
 
#define _nireg_read_type_write\
(register,chip,offset,alg,index,size,mask,memap,...) \
                    SHADOW_REGISTER(chip,memap,register,index)


#define _nibit_read_x\
(register,chip,offset,alg,index,type,size,mask,memap,bit,bit_mask,...) \
        _nibit_read_type ## _ ## type \
            (register,chip,offset,alg,index,size,mask,memap,bit,bit_mask,...)

#define _nibit_read_type_read\
(register,chip,offset,alg,index,size,mask,memap,bit,bit_mask,...) \
            (((SHADOW_REGISTER(chip,memap,register,index) = \
              BOARD_REGISTER_R(size,memap,alg(offset,index))) \
              & (u ## size) ~bit_mask) >> bit)

#define _nibit_read_type_write\
(register,chip,offset,alg,index,size,mask,memap,bit,bit_mask,...) \
  ((SHADOW_REGISTER(chip,memap,register,index) & (u ## size) ~bit_mask) >> bit)

/*
 *
 *  write - store value into back copy and write to memap
 *
 */

#define _nireg_write_x\
(register,chip,offset,alg,index,type,size,mask,memap,value) \
        _nireg_write_type ## _ ## type \
                      (register,chip,offset,alg,index,size,mask,memap,value)

#define _nireg_write_type_write\
(register,chip,offset,alg,index,size,mask,memap,value) ( \
            SHADOW_REGISTER(chip,memap,register,index) = value, \
            BOARD_REGISTER_W(size,memap,alg(offset,index), \
            SHADOW_REGISTER(chip,memap,register,index)), \
            SHADOW_REGISTER(chip,memap,register,index) &= (u ## size)mask )

#define _nireg_write_type_read\
(register,chip,offset,alg,index,size,mask,memap,value) \
                         (SHADOW_REGISTER(chip,memap,register,index) = value)

#define _nibit_write_x\
(register,chip,offset,alg,index,type,size,mask,memap,bit,bit_mask,value) \
   _nibit_write_type ## _ ## type \
           (register,chip,offset,alg,index,size,mask,memap,bit,bit_mask,value)

#define _nibit_write_type_write\
(register,chip,offset,alg,index,size,mask,memap, \
                                             bit,bit_mask,value) \
        (SHADOW_REGISTER(chip,memap,register,index) = \
        (SHADOW_REGISTER(chip,memap,register,index) & (u ## size) bit_mask) | \
        (((u ## size) value << bit) & (u ## size) ~bit_mask), \
        BOARD_REGISTER_W(size,memap,alg(offset,index), \
        SHADOW_REGISTER(chip,memap,register,index)), \
        SHADOW_REGISTER(chip,memap,register,index) &= (u ## size)mask )

#define _nibit_write_type_read\
(register,chip,offset,alg,index,size,mask,memap,bit,bit_mask,value) \
        (SHADOW_REGISTER(chip,memap,register,index) = \
        (SHADOW_REGISTER(chip,memap,register,index) & (u ## size) bit_mask) | \
        (((u ## size) value << bit) & (u ## size) ~bit_mask))

/*
 *
 *  flush - write value from back copy to memap
 *
 */

#define _nireg_flush_x\
(register,chip,offset,alg,index,type,size,mask,memap,...) \
        _nireg_flush_type ## _ ## type\
                        (register,chip,offset,alg,index,size,mask,memap,...) 

#define _nireg_flush_type_write\
(register,chip,offset,alg,index,size,mask,memap,...) \
        (BOARD_REGISTER_W(size,memap,alg(offset,index), \
          SHADOW_REGISTER(chip,memap,register,index)), \
        SHADOW_REGISTER(chip,memap,register,index) &= (u ## size)mask)

/*
 *
 *  initialize
 *
 */

#define _nireg_initialize_x\
(register,chip,offset,alg,index,type,size,mask,memap,...)   /* TODO */

/*
 *
 *  refresh - update back copy from memap if register is "read"
 *
 */

#define _nireg_refresh_x\
(register,chip,offset,alg,index,type,size,mask,memap,...) \
        _nireg_refresh_type ## _ ## type \
                          (register,chip,offset,alg,index,size,mask,memap,...)

#define _nireg_refresh_type_read\
(register,chip,offset,alg,index,size,mask,memap,...) \
        (SHADOW_REGISTER(chip,memap,register,index) = \
         BOARD_REGISTER_R(size,memap,alg(offset,index)))

#define _nireg_refresh_type_write\
(register,chip,offset,alg,index,size,mask,memap,...) \
        (SHADOW_REGISTER(chip,memap,register,index) = \
         BOARD_REGISTER_R(size,memap,alg(offset,index)))

