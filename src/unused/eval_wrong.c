/* fd.c -- file descriptor manipulations ($Revision: 1.2 $) */

#include "es.h"


/* mvfd -- duplicate a file descriptor and close the old one */
extern void mvfd(int old_fd, int new_fd)
{   if (old_fd != new_fd)
    {   int duplicated_fd = dup2(old_fd, new_fd);
        if (duplicated_fd == -1)
            fail("es:mvfd", "dup2: %s", esstrerror(errno));
        assert(duplicated_fd == new_fd);
        close(old_fd);
    }
}


/*
 * Deferred file descriptor operations
 * 
 * We maintain a stack of deferred file descriptor operations if in the parent shell.
 * If already in a forked process, operations are done immediately.
 * The deferred operations are performed later during closefds() call.
 */

typedef struct {
    int real_fd;
    int user_fd;
} Defer;

static Defer *defer_table;
static int defer_count = 0, defer_capacity = 0;

/* dodeferred -- perform a deferred file descriptor operation
 * real_fd == -1 means close user_fd
 * otherwise duplicate real_fd to user_fd and close real_fd
 */
static void dodeferred(int real_fd, int user_fd)
{   assert(user_fd >= 0);
    releasefd(user_fd);

    if (real_fd == -1)
        close(user_fd);
    else
    {   assert(real_fd >= 0);
        mvfd(real_fd, user_fd);
    }
}

/* pushdefer -- register a deferred fd operation
 * Arguments:
 *   in_parent: Boolean indicating if we're in parent shell
 *   real_fd: actual file descriptor (-1 for close)
 *   user_fd: user-visible file descriptor
 * Returns: ticket number for later undefer, or UNREGISTERED if immediate
 */
static int pushdefer(Boolean in_parent, int real_fd, int user_fd)
{   if (in_parent)
    {   Defer *deferred_op;
        if (defer_count >= defer_capacity)
        {   int i;
            for (i = 0; i < defer_count; i++)
                unregisterfd(&defer_table[i].real_fd);
            defer_capacity += 10;
            defer_table = erealloc(defer_table, defer_capacity * sizeof(Defer));
            for (i = 0; i < defer_count; i++)
                registerfd(&defer_table[i].real_fd, TRUE);
        }
        deferred_op = &defer_table[defer_count++];
        deferred_op->real_fd = real_fd;
        deferred_op->user_fd = user_fd;
        registerfd(&deferred_op->real_fd, TRUE);
        return defer_count - 1;
    }
    else
    {   dodeferred(real_fd, user_fd);
        return UNREGISTERED;
    }
}

/* defer_mvfd -- defer duplicating an fd (or immediately do it if in child) */
extern int defer_mvfd(Boolean in_parent, int old_fd, int new_fd)
{   assert(old_fd >= 0);
    assert(new_fd >= 0);
    return pushdefer(in_parent, old_fd, new_fd);
}

/* defer_close -- defer closing an fd (or immediately close if in child) */
extern int defer_close(Boolean in_parent, int fd)
{   assert(fd >= 0);
    return pushdefer(in_parent, -1, fd);
}

/* undefer -- undo a deferred operation by ticket number
 * Closes and unregisters the real fd if appropriate
 */
extern void undefer(int ticket)
{   if (ticket != UNREGISTERED)
    {   Defer *deferred_op;
        assert(ticket >= 0);
        assert(defer_count > 0);
        deferred_op = &defer_table[--defer_count];
        assert(ticket == defer_count);
        unregisterfd(&deferred_op->real_fd);
        if (deferred_op->real_fd != -1)
            close(deferred_op->real_fd);
    }
}

/* fdmap -- translate user fd to real fd accounting for deferred operations
 * Returns -1 if user fd maps to closed fd
 */
extern int fdmap(int user_fd)
{   int index = defer_count;
    while (--index >= 0)
    {   Defer *deferred_op = &defer_table[index];
        if (user_fd == deferred_op->user_fd)
        {   user_fd = deferred_op->real_fd;
            if (user_fd == -1)
                return -1;
        }
    }
    return user_fd;
}

/* remapfds -- apply all deferred fd operations to the current fd table
 * Called to finalize deferred operations
 */
static void remapfds(void)
{   Defer *defer, *defer_end;

    if (defer_table == NULL)
        return;
    for (defer = defer_table, defer_end = &defer_table[defer_count]; defer < defer_end; defer++)
    {   unregisterfd(&defer->real_fd);
        dodeferred(defer->real_fd, defer->user_fd);
    }
    defer_count = 0;
}


/*
 * Registered file descriptor list
 * This list tracks pointers to file descriptors owned by the shell.
 * This helps avoid fd collisions and ensures proper cleanup on fork.
 */

typedef struct {
    int *fd_ptr;
    Boolean close_on_fork;
} Reserve;

static Reserve *reserved_fds = NULL;
static int reserved_count = 0, reserved_capacity = 0;

/* registerfd -- reserve a file descriptor for shell use
 * Arguments:
 *   fd_ptr: pointer to file descriptor variable
 *   close_on_fork: whether to close fd during fork
 */
extern void registerfd(int *fd_ptr, Boolean close_on_fork)
{   
#if ASSERTIONS
    int i;
    for (i = 0; i < reserved_count; i++)
        assert(fd_ptr != reserved_fds[i].fd_ptr);
#endif

    if (reserved_count    >= reserved_capacity)
    {   reserved_capacity += 10;
        reserved_fds       = erealloc(reserved_fds, reserved_capacity * sizeof(Reserve));
    }

    reserved_fds[reserved_count].fd_ptr = fd_ptr;
    reserved_fds[reserved_count].close_on_fork = close_on_fork;
    reserved_count++;
}

/* unregisterfd -- release a reserved file descriptor
 */
extern void unregisterfd(int *fd_ptr)
{   int i;
    assert(reserved_fds != NULL);
    assert(reserved_count > 0);
    for (i = 0; i < reserved_count; i++)
        if (reserved_fds[i].fd_ptr == fd_ptr)
        {   reserved_fds[i] = reserved_fds[--reserved_count];
            return;
        }
    panic("%x not on file descriptor reserved list", fd_ptr);
}

/* closefds -- close reserved file descriptors during fork
 */
extern void closefds(void)
{   int i;
    remapfds();
    for (i = 0; i < reserved_count; i++)
    {   Reserve *res = &reserved_fds[i];
        if (res->close_on_fork)
        {   int fd = *res->fd_ptr;
            if (fd >= 3)
                close(fd);
            *res->fd_ptr = -1;
        }
    }
}

/* releasefd -- release a specific file descriptor for reuse
 * Duplicates fd to a safe number and closes original fd
 */
extern void releasefd(int fd)
{   int i;
    assert(fd >= 0);
    for (i = 0; i < reserved_count; i++)
    {   int *fd_ptr = reserved_fds[i].fd_ptr;
        int fd_val = *fd_ptr;
        if (fd_val == fd)
        {   *fd_ptr = dup(fd_val);
            if (*fd_ptr == -1)
            {   assert(errno != EBADF);
                fail("es:releasefd", "%s", esstrerror(errno));
            }
            close(fd_val);
        }
    }
}

/* isdeferred -- check if fd is on deferred fd list
 */
static Boolean isdeferred(int fd)
{   Defer *defer, *defer_end = &defer_table[defer_count];
    for (defer = defer_table; defer < defer_end; defer++)
        if (defer->user_fd == fd)
            return TRUE;
    return FALSE;
}

/* newfd -- get a free file descriptor >= 3, avoiding deferred descriptors
 */
extern int newfd(void)
{   int i;
    for (i = 3;; i++)
    {   if (!isdeferred(i))
        {   int fd = dup(i);
		 
            if (fd == -1)
            {   if (errno != EBADF)
                    fail("$&newfd", "newfd: %s", esstrerror(errno));
                return i;
            }
				
            else if (isdeferred(fd))
            {   int new_fd = newfd();
                close(fd);
                return new_fd;
            }
				
            else
            {   close(fd);
                return fd;
            }
		 
        }
    }
}
