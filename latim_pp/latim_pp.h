
#define HERE NAME, __FUNCTION__

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define DOUBLE_LOG(format,...) \
{ \
        char buf[80]; \
        snprintf (buf, 80, format, __VA_ARGS__); \
        printk (KERN_ALERT "%s", buf); \
        if (current->signal->tty) { \
                current->signal->tty->driver->write ( \
                        current->signal->tty, buf, strlen(buf)); \
                current->signal->tty->driver->write ( \
                        current->signal->tty, "\r", 1); \
         } \
}
#else
#define DOUBLE_LOG(format,...) \
{ \
        char buf[80]; \
        snprintf (buf, 80, format, __VA_ARGS__); \
        printk (KERN_ALERT "%s", buf); \
        if (current->signal->tty) { \
                current->signal->tty->driver->ops->write ( \
                        current->signal->tty, buf, strlen(buf)); \
                current->signal->tty->driver->ops->write ( \
                        current->signal->tty, "\r", 1); \
        } \
}
#endif
