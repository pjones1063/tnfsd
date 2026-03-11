/* The MIT License
 *
 * Copyright (c) 2010 Dylan Smith
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * The main()
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "datagram.h"
#include "session.h"
#include "directory.h"
#include "errortable.h"
// #include "chroot.h"
#include "log.h"


// New entry point called from the Qt C++ thread
int start_tnfs_server(const char *root_dir, int port)
{
    // Set the root directory provided by the Qt GUI
    if(tnfs_setroot(root_dir) < 0)
    {
        LOG("Invalid root directory: %s\n", root_dir);
        return -1; // Return instead of exit(-1) to prevent killing the Qt app
    }

    if (port < 1 || port > 65535)
    {
        LOG("Invalid port\n");
        return -1;
    }

    const char *version = "24.0522.1 (Qt GUI Edition)";

    LOG("Starting tnfsd version %s on port %d using root directory \"%s\"\n", version, port, root_dir);

    tnfs_init();          /* initialize structures etc. */
    tnfs_init_errtable(); /* initialize error lookup table */
    if (tnfs_sockinit(port) < 0) {
        LOG("Server failed to start due to a network error.\n");
        return -1; // Abort cleanly, returning to Qt
    }
    // NOTE: This function still blocks infinitely. We must fix datagram.c next!
    tnfs_mainloop();      /* run */

    tnfs_shutdown_sockets();

    return 0;
}
