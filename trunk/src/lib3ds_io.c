/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.kyprianidis.com>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: io.c,v 1.9 2007/06/20 17:04:08 jeh Exp $
 */
#include "lib3ds_impl.h"


/*!
 * \defgroup io Binary Input/Ouput Abstraction Layer
 */

typedef union {
    Lib3dsDword dword_value;
    float float_value;
} Lib3dsDwordFloat;


Lib3dsIo*
lib3ds_io_new(void *self, Lib3dsIoSeekFunc seek_func, Lib3dsIoTellFunc tell_func, 
              Lib3dsIoReadFunc read_func, Lib3dsIoWriteFunc write_func,
              Lib3dsIoLogFunc log_func) {
    Lib3dsIo *io = calloc(sizeof(Lib3dsIo), 1);
    ASSERT(io);
    if (!io) {
        return 0;
    }

    io->self = self;
    io->seek_func = seek_func;
    io->tell_func = tell_func;
    io->read_func = read_func;
    io->write_func = write_func;
    io->log_func = log_func;

    return io;
}


void
lib3ds_io_free(Lib3dsIo *io) {
    ASSERT(io);
    if (!io) {
        return;
    }

    if (io->tmp_mem) {
        free(io->tmp_mem);
    }
    if (io->tmp_node) {
        lib3ds_node_free(io->tmp_node);
    }

    free(io);
}


long
lib3ds_io_seek(Lib3dsIo *io, long offset, Lib3dsIoSeek origin) {
    ASSERT(io);
    if (!io || !io->seek_func) {
        return 0;
    }
    return (*io->seek_func)(io->self, offset, origin);
}


long
lib3ds_io_tell(Lib3dsIo *io) {
    ASSERT(io);
    if (!io || !io->tell_func) {
        return 0;
    }
    return (*io->tell_func)(io->self);
}


size_t
lib3ds_io_read(Lib3dsIo *io, void *buffer, size_t size) {
    ASSERT(io);
    if (!io || !io->read_func) {
        return 0;
    }
    return (*io->read_func)(io->self, buffer, size);
}


size_t
lib3ds_io_write(Lib3dsIo *io, const void *buffer, size_t size) {
    ASSERT(io);
    if (!io || !io->write_func) {
        return 0;
    }
    return (*io->write_func)(io->self, buffer, size);
}


static void 
lib3ds_io_log_str(Lib3dsIo *io, Lib3dsLogLevel level, const char *str) {
    char msg[1024];
    int i;
    for (i=0; i<2*io->log_indent; ++i) {
        msg[i] = ' ';
    }
    msg[io->log_indent] = 0;
    strcat(msg, str);
    (*io->log_func)(io->self, level, msg);
}


void 
lib3ds_io_log(Lib3dsIo *io, Lib3dsLogLevel level, const char *format, ...) {
    ASSERT(io);
    if (!io || !io->log_func) {
        return;
    }
    {
        va_list args;
        char str[1024];
        va_start(args, format);
        /* FIXME: */ vsprintf(str, format, args); 
        lib3ds_io_log_str(io, level, str);
    }
}


void 
lib3ds_io_fatal_error(Lib3dsIo *io, const char *format, ...) {
    ASSERT(io);
    if (!io || !io->log_func) {
        return;
    }
    {
        va_list args;
        char str[1024];
        va_start(args, format);
        /* FIXME: */ vsprintf(str, format, args); 
        lib3ds_io_log_str(io, LIB3DS_LOG_ERROR, str);
    }
    longjmp(io->jmpbuf, 1);
}


void 
lib3ds_io_read_error(Lib3dsIo *io) {
    lib3ds_io_fatal_error(io, "Reading from input stream failed.");
}


void 
lib3ds_io_write_error(Lib3dsIo *io) {
    lib3ds_io_fatal_error(io, "Writing to output stream failed.");
}


/*!
 * \ingroup io
 *
 * Read a byte from a file stream.
 */
Lib3dsByte
lib3ds_io_read_byte(Lib3dsIo *io) {
    Lib3dsByte b;

    ASSERT(io);
    lib3ds_io_read(io, &b, 1);
    return(b);
}


/**
 * Read a word from a file stream in little endian format.
 */
Lib3dsWord
lib3ds_io_read_word(Lib3dsIo *io) {
    Lib3dsByte b[2];
    Lib3dsWord w;

    ASSERT(io);
    lib3ds_io_read(io, b, 2);
    w = ((Lib3dsWord)b[1] << 8) |
        ((Lib3dsWord)b[0]);
    return(w);
}


/*!
 * \ingroup io
 *
 * Read a dword from file a stream in little endian format.
 */
Lib3dsDword
lib3ds_io_read_dword(Lib3dsIo *io) {
    Lib3dsByte b[4];
    Lib3dsDword d;

    ASSERT(io);
    lib3ds_io_read(io, b, 4);
    d = ((Lib3dsDword)b[3] << 24) |
        ((Lib3dsDword)b[2] << 16) |
        ((Lib3dsDword)b[1] << 8) |
        ((Lib3dsDword)b[0]);
    return(d);
}


/*!
 * \ingroup io
 *
 * Read a signed byte from a file stream.
 */
Lib3dsIntb
lib3ds_io_read_intb(Lib3dsIo *io) {
    Lib3dsIntb b;

    ASSERT(io);
    lib3ds_io_read(io, &b, 1);
    return(b);
}


/*!
 * \ingroup io
 *
 * Read a signed word from a file stream in little endian format.
 */
Lib3dsIntw
lib3ds_io_read_intw(Lib3dsIo *io) {
    Lib3dsByte b[2];
    Lib3dsWord w;

    ASSERT(io);
    lib3ds_io_read(io, b, 2);
    w = ((Lib3dsWord)b[1] << 8) |
        ((Lib3dsWord)b[0]);
    return((Lib3dsIntw)w);
}


/*!
 * \ingroup io
 *
 * Read a signed dword a from file stream in little endian format.
 */
Lib3dsIntd
lib3ds_io_read_intd(Lib3dsIo *io) {
    Lib3dsByte b[4];
    Lib3dsDword d;

    ASSERT(io);
    lib3ds_io_read(io, b, 4);
    d = ((Lib3dsDword)b[3] << 24) |
        ((Lib3dsDword)b[2] << 16) |
        ((Lib3dsDword)b[1] << 8) |
        ((Lib3dsDword)b[0]);
    return((Lib3dsIntd)d);
}


/*!
 * \ingroup io
 *
 * Read a float from a file stream in little endian format.
 */
float
lib3ds_io_read_float(Lib3dsIo *io) {
    Lib3dsByte b[4];
    Lib3dsDwordFloat d;

    ASSERT(io);
    lib3ds_io_read(io, b, 4);
    d.dword_value = ((Lib3dsDword)b[3] << 24) |
                    ((Lib3dsDword)b[2] << 16) |
                    ((Lib3dsDword)b[1] << 8) |
                    ((Lib3dsDword)b[0]);
    return d.float_value;
}


/*!
 * \ingroup io
 *
 * Read a vector from a file stream in little endian format.
 *
 * \param io IO input handle.
 * \param v  The vector to store the data.
 */
void
lib3ds_io_read_vector(Lib3dsIo *io, Lib3dsVector v) {
    ASSERT(io);
    v[0] = lib3ds_io_read_float(io);
    v[1] = lib3ds_io_read_float(io);
    v[2] = lib3ds_io_read_float(io);
}


/*!
 * \ingroup io
 */
void
lib3ds_io_read_rgb(Lib3dsIo *io, Lib3dsRgb rgb) {
    ASSERT(io);
    rgb[0] = lib3ds_io_read_float(io);
    rgb[1] = lib3ds_io_read_float(io);
    rgb[2] = lib3ds_io_read_float(io);
}


/*!
 * \ingroup io
 *
 * Read a zero-terminated string from a file stream.
 *
 * \param io      IO input handle.
 * \param s       The buffer to store the read string.
 * \param buflen  Buffer length.
 *
 * \return        True on success, False otherwise.
 */
void
lib3ds_io_read_string(Lib3dsIo *io, char *s, int buflen) {
    char c;
    int k = 0;

    ASSERT(io);
    for (;;) {
        if (lib3ds_io_read(io, &c, 1) != 1) {
            lib3ds_io_read_error(io);
        }
        *s++ = c;
        if (!c) {
            break;
        }
        ++k;
        if (k >= buflen) {
            lib3ds_io_fatal_error(io, "Invalid string in input stream.");
        }
    }
}


/*!
 * \ingroup io
 *
 * Writes a byte into a file stream.
 */
void
lib3ds_io_write_byte(Lib3dsIo *io, Lib3dsByte b) {
    ASSERT(io);
    if (lib3ds_io_write(io, &b, 1) != 1) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a word into a little endian file stream.
 */
void
lib3ds_io_write_word(Lib3dsIo *io, Lib3dsWord w) {
    Lib3dsByte b[2];

    ASSERT(io);
    b[1] = ((Lib3dsWord)w & 0xFF00) >> 8;
    b[0] = ((Lib3dsWord)w & 0x00FF);
    if (lib3ds_io_write(io, b, 2) != 2) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a dword into a little endian file stream.
 */
void
lib3ds_io_write_dword(Lib3dsIo *io, Lib3dsDword d) {
    Lib3dsByte b[4];

    ASSERT(io);
    b[3] = (Lib3dsByte)(((Lib3dsDword)d & 0xFF000000) >> 24);
    b[2] = (Lib3dsByte)(((Lib3dsDword)d & 0x00FF0000) >> 16);
    b[1] = (Lib3dsByte)(((Lib3dsDword)d & 0x0000FF00) >> 8);
    b[0] = (Lib3dsByte)(((Lib3dsDword)d & 0x000000FF));
    if (lib3ds_io_write(io, b, 4) != 4) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a signed byte in a file stream.
 */
void
lib3ds_io_write_intb(Lib3dsIo *io, Lib3dsIntb b) {
    ASSERT(io);
    if (lib3ds_io_write(io, &b, 1) != 1) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a signed word into a little endian file stream.
 */
void
lib3ds_io_write_intw(Lib3dsIo *io, Lib3dsIntw w) {
    Lib3dsByte b[2];

    ASSERT(io);
    b[1] = ((Lib3dsWord)w & 0xFF00) >> 8;
    b[0] = ((Lib3dsWord)w & 0x00FF);
    if (lib3ds_io_write(io, b, 2) != 2) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a signed dword into a little endian file stream.
 */
void
lib3ds_io_write_intd(Lib3dsIo *io, Lib3dsIntd d) {
    Lib3dsByte b[4];

    ASSERT(io);
    b[3] = (Lib3dsByte)(((Lib3dsDword)d & 0xFF000000) >> 24);
    b[2] = (Lib3dsByte)(((Lib3dsDword)d & 0x00FF0000) >> 16);
    b[1] = (Lib3dsByte)(((Lib3dsDword)d & 0x0000FF00) >> 8);
    b[0] = (Lib3dsByte)(((Lib3dsDword)d & 0x000000FF));
    if (lib3ds_io_write(io, b, 4) != 4) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a float into a little endian file stream.
 */
void
lib3ds_io_write_float(Lib3dsIo *io, float l) {
    Lib3dsByte b[4];
    Lib3dsDwordFloat d;

    ASSERT(io);
    d.float_value = l;
    b[3] = (Lib3dsByte)(((Lib3dsDword)d.dword_value & 0xFF000000) >> 24);
    b[2] = (Lib3dsByte)(((Lib3dsDword)d.dword_value & 0x00FF0000) >> 16);
    b[1] = (Lib3dsByte)(((Lib3dsDword)d.dword_value & 0x0000FF00) >> 8);
    b[0] = (Lib3dsByte)(((Lib3dsDword)d.dword_value & 0x000000FF));
    if (lib3ds_io_write(io, b, 4) != 4) {
        lib3ds_io_write_error(io);
    }
}


/*!
 * \ingroup io
 *
 * Writes a vector into a file stream in little endian format.
 */
void
lib3ds_io_write_vector(Lib3dsIo *io, Lib3dsVector v) {
    int i;
    for (i = 0; i < 3; ++i) {
        lib3ds_io_write_float(io, v[i]);
    }
}


/*!
 * \ingroup io
 */
void
lib3ds_io_write_rgb(Lib3dsIo *io, Lib3dsRgb rgb) {
    int i;
    for (i = 0; i < 3; ++i) {
        lib3ds_io_write_float(io, rgb[i]);
    }
}


/*!
 * \ingroup io
 *
 * Writes a zero-terminated string into a file stream.
 */
void
lib3ds_io_write_string(Lib3dsIo *io, const char *s) {
    size_t len;
    assert(io && s);
    len = strlen(s);
    if (lib3ds_io_write(io, s, len + 1) != len +1) {
        lib3ds_io_write_error(io);
    }
}
