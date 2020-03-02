/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Edward.Wu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _TCPRole_INCLUDE_
#define _TCPRole_INCLUDE_

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <fcntl.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "conf.hpp"

typedef struct DATA_PARAM {
	bool readable;
	bool writable;
};
/**
 * CTCPRole , the base of tcp client
 */
class CTCPRole
{
public :
    CTCPRole();
	virtual ~CTCPRole();

    int         open(int port, int backlog);
    int         open(char *host, int port);
    virtual int close();

    virtual int handler(DATA_PARAM *p);

    int         write(const char *buf, int size);
    int         read(char *buf, int size);

    char       *get_role_name();
    int         set_nonblock();
    int         get_fd();

    bool        is_valid();
protected:
    int         m_fd;
    int         m_port;
    char        m_role_name[256];
    char        m_remote_host[256];
    int         m_remote_port;
    bool        m_valid;

    int         setup();
    int         listen(int port, int backlog);
    int         connect(char *host, int port);

};


#endif
