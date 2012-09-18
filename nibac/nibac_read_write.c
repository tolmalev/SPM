/*
 *
 *    nibac_read - read a data buffer
 *
 */

ssize_t nibac_read  (struct file * filp,
                     char __user * buf,
                     size_t byte_count,
                     loff_t * offset) {

	return 0;      /* always return EOF */

}

/*
 *
 *    nibac_write - write function
 *
 */

ssize_t nibac_write (struct file * filp,
		     const char __user * user_buf,
		     size_t count,
		     loff_t * offset) {

	return count;    /* always return success */
}
