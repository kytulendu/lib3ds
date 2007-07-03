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
 * $Id: viewport.c,v 1.11 2007/06/20 17:04:09 jeh Exp $
 */
#include "lib3ds_impl.h"


/*!
 * \defgroup viewport Viewport and default view settings
 */


/*!
* \ingroup viewport
*/
void
lib3ds_viewport_read(Lib3dsViewport *viewport, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;

    lib3ds_chunk_read_start(&c, 0, io);
    switch (c.chunk) {
        case LIB3DS_VIEWPORT_LAYOUT: {
            int cur = 0;
            viewport->layout.style = lib3ds_io_read_word(io);
            viewport->layout.active = lib3ds_io_read_intw(io);
            lib3ds_io_read_intw(io);
            viewport->layout.swap = lib3ds_io_read_intw(io);
            lib3ds_io_read_intw(io);
            viewport->layout.swap_prior = lib3ds_io_read_intw(io);
            viewport->layout.swap_view = lib3ds_io_read_intw(io);
            lib3ds_chunk_read_tell(&c, io);
            while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
                switch (chunk) {
                    case LIB3DS_VIEWPORT_SIZE: {
                        viewport->layout.position[0] = lib3ds_io_read_word(io);
                        viewport->layout.position[1] = lib3ds_io_read_word(io);
                        viewport->layout.size[0] = lib3ds_io_read_word(io);
                        viewport->layout.size[1] = lib3ds_io_read_word(io);
                        break;
                    }

                    case LIB3DS_VIEWPORT_DATA_3: {
                        if (cur < LIB3DS_LAYOUT_MAX_VIEWS) {
                            lib3ds_io_read_intw(io);
                            viewport->layout.views[cur].axis_lock = lib3ds_io_read_word(io);
                            viewport->layout.views[cur].position[0] = lib3ds_io_read_intw(io);
                            viewport->layout.views[cur].position[1] = lib3ds_io_read_intw(io);
                            viewport->layout.views[cur].size[0] = lib3ds_io_read_intw(io);
                            viewport->layout.views[cur].size[1] = lib3ds_io_read_intw(io);
                            viewport->layout.views[cur].type = lib3ds_io_read_word(io);
                            viewport->layout.views[cur].zoom = lib3ds_io_read_float(io);
                            lib3ds_io_read_vector(io, viewport->layout.views[cur].center);
                            viewport->layout.views[cur].horiz_angle = lib3ds_io_read_float(io);
                            viewport->layout.views[cur].vert_angle = lib3ds_io_read_float(io);
                            lib3ds_io_read(io, viewport->layout.views[cur].camera, 11);
                            ++cur;
                        }
                        break;
                    }

                    case LIB3DS_VIEWPORT_DATA:
                        /* 3DS R2 & R3 chunk unsupported */
                        break;

                    default:
                        lib3ds_chunk_unknown(chunk, io);
                }
            }
            break;
        }

        case LIB3DS_DEFAULT_VIEW: {
            memset(&viewport->default_view, 0, sizeof(Lib3dsDefaultView));
            while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
                switch (chunk) {
                    case LIB3DS_VIEW_TOP: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_TOP;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_BOTTOM: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_BOTTOM;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_LEFT: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_LEFT;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_RIGHT: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_RIGHT;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_FRONT: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_FRONT;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_BACK: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_BACK;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_USER: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_USER;
                        lib3ds_io_read_vector(io, viewport->default_view.position);
                        viewport->default_view.width = lib3ds_io_read_float(io);
                        viewport->default_view.horiz_angle = lib3ds_io_read_float(io);
                        viewport->default_view.vert_angle = lib3ds_io_read_float(io);
                        viewport->default_view.roll_angle = lib3ds_io_read_float(io);
                        break;
                    }

                    case LIB3DS_VIEW_CAMERA: {
                        viewport->default_view.type = LIB3DS_VIEW_TYPE_CAMERA;
                        lib3ds_io_read(io, viewport->default_view.camera, 11);
                        break;
                    }

                    default:
                        lib3ds_chunk_unknown(chunk, io);
                }
            }
            break;
        }
    }

    lib3ds_chunk_read_end(&c, io);
}


/*!
* \ingroup viewport
*/
void
lib3ds_viewport_write(Lib3dsViewport *viewport, Lib3dsIo *io) {
    if (viewport->layout.nviews) {
        Lib3dsChunk c;
        unsigned i;

        c.chunk = LIB3DS_VIEWPORT_LAYOUT;
        lib3ds_chunk_write_start(&c, io);

        lib3ds_io_write_word(io, viewport->layout.style);
        lib3ds_io_write_intw(io, viewport->layout.active);
        lib3ds_io_write_intw(io, 0);
        lib3ds_io_write_intw(io, viewport->layout.swap);
        lib3ds_io_write_intw(io, 0);
        lib3ds_io_write_intw(io, viewport->layout.swap_prior);
        lib3ds_io_write_intw(io, viewport->layout.swap_view);

        {
            Lib3dsChunk c;
            c.chunk = LIB3DS_VIEWPORT_SIZE;
            c.size = 14;
            lib3ds_chunk_write(&c, io);
            lib3ds_io_write_intw(io, viewport->layout.position[0]);
            lib3ds_io_write_intw(io, viewport->layout.position[1]);
            lib3ds_io_write_intw(io, viewport->layout.size[0]);
            lib3ds_io_write_intw(io, viewport->layout.size[1]);
        }

        for (i = 0; i < viewport->layout.nviews; ++i) {
            Lib3dsChunk c;
            c.chunk = LIB3DS_VIEWPORT_DATA_3;
            c.size = 55;
            lib3ds_chunk_write(&c, io);

            lib3ds_io_write_intw(io, 0);
            lib3ds_io_write_word(io, viewport->layout.views[i].axis_lock);
            lib3ds_io_write_intw(io, viewport->layout.views[i].position[0]);
            lib3ds_io_write_intw(io, viewport->layout.views[i].position[1]);
            lib3ds_io_write_intw(io, viewport->layout.views[i].size[0]);
            lib3ds_io_write_intw(io, viewport->layout.views[i].size[1]);
            lib3ds_io_write_word(io, viewport->layout.views[i].type);
            lib3ds_io_write_float(io, viewport->layout.views[i].zoom);
            lib3ds_io_write_vector(io, viewport->layout.views[i].center);
            lib3ds_io_write_float(io, viewport->layout.views[i].horiz_angle);
            lib3ds_io_write_float(io, viewport->layout.views[i].vert_angle);
            lib3ds_io_write(io, viewport->layout.views[i].camera, 11);
        }

        lib3ds_chunk_write_end(&c, io);
    }

    if (viewport->default_view.type) {
        Lib3dsChunk c;

        c.chunk = LIB3DS_DEFAULT_VIEW;
        lib3ds_chunk_write_start(&c, io);

        switch (viewport->default_view.type) {
            case LIB3DS_VIEW_TYPE_TOP: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_TOP;
                c.size = 22;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                break;
            }

            case LIB3DS_VIEW_TYPE_BOTTOM: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_BOTTOM;
                c.size = 22;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                break;
            }

            case LIB3DS_VIEW_TYPE_LEFT: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_LEFT;
                c.size = 22;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                break;
            }

            case LIB3DS_VIEW_TYPE_RIGHT: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_RIGHT;
                c.size = 22;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                break;
            }

            case LIB3DS_VIEW_TYPE_FRONT: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_FRONT;
                c.size = 22;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                break;
            }

            case LIB3DS_VIEW_TYPE_BACK: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_BACK;
                c.size = 22;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                break;
            }

            case LIB3DS_VIEW_TYPE_USER: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_USER;
                c.size = 34;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_vector(io, viewport->default_view.position);
                lib3ds_io_write_float(io, viewport->default_view.width);
                lib3ds_io_write_float(io, viewport->default_view.horiz_angle);
                lib3ds_io_write_float(io, viewport->default_view.vert_angle);
                lib3ds_io_write_float(io, viewport->default_view.roll_angle);
                break;
            }

            case LIB3DS_VIEW_TYPE_CAMERA: {
                Lib3dsChunk c;
                c.chunk = LIB3DS_VIEW_CAMERA;
                c.size = 17;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write(io, viewport->default_view.camera, 11);
                break;
            }
        }

        lib3ds_chunk_write_end(&c, io);
    }
}

