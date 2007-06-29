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
 * $Id: file.c,v 1.34 2007/06/20 17:04:08 jeh Exp $
 */
#include "lib3ds_impl.h"


/*!
 * \defgroup file Files
 */


static Lib3dsBool
fileio_error_func(void *self) {
    FILE *f = (FILE*)self;
    return(ferror(f) != 0);
}


static long
fileio_seek_func(void *self, long offset, Lib3dsIoSeek origin) {
    FILE *f = (FILE*)self;
    int o;
    switch (origin) {
        case LIB3DS_SEEK_SET:
            o = SEEK_SET;
            break;

        case LIB3DS_SEEK_CUR:
            o = SEEK_CUR;
            break;

        case LIB3DS_SEEK_END:
            o = SEEK_END;
            break;

        default:
            ASSERT(0);
            return(0);
    }
    return (fseek(f, offset, o));
}


static long
fileio_tell_func(void *self) {
    FILE *f = (FILE*)self;
    return(ftell(f));
}


static size_t
fileio_read_func(void *self, void *buffer, size_t size) {
    FILE *f = (FILE*)self;
    return(fread(buffer, 1, size, f));
}


static size_t
fileio_write_func(void *self, const void *buffer, size_t size) {
    FILE *f = (FILE*)self;
    return(fwrite(buffer, 1, size, f));
}


/*!
 * Loads a .3DS file from disk into memory.
 *
 * \param filename  The filename of the .3DS file
 *
 * \return   A pointer to the Lib3dsFile structure containing the
 *           data of the .3DS file.
 *           If the .3DS file can not be loaded NULL is returned.
 *
 * \note     To free the returned structure use lib3ds_free.
 *
 * \see lib3ds_file_save
 * \see lib3ds_file_new
 * \see lib3ds_file_free
 *
 * \ingroup file
 */
Lib3dsFile*
lib3ds_file_load(const char *filename) {
    FILE *f;
    Lib3dsFile *file;
    Lib3dsIo *io;

    f = fopen(filename, "rb");
    if (!f) {
        return(0);
    }
    file = lib3ds_file_new();
    if (!file) {
        fclose(f);
        return(0);
    }

    io = lib3ds_io_new(
             f,
             fileio_error_func,
             fileio_seek_func,
             fileio_tell_func,
             fileio_read_func,
             fileio_write_func
         );
    if (!io) {
        lib3ds_file_free(file);
        fclose(f);
        return(0);
    }

    if (!lib3ds_file_read(file, io)) {
        free(file);
        lib3ds_io_free(io);
        fclose(f);
        return(0);
    }

    lib3ds_io_free(io);
    fclose(f);
    return(file);
}


/*!
 * Saves a .3DS file from memory to disk.
 *
 * \param file      A pointer to a Lib3dsFile structure containing the
 *                  the data that should be stored.
 * \param filename  The filename of the .3DS file to store the data in.
 *
 * \return          TRUE on success, FALSE otherwise.
 *
 * \see lib3ds_file_load
 *
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_save(Lib3dsFile *file, const char *filename) {
    FILE *f;
    Lib3dsIo *io;
    Lib3dsBool result;

    f = fopen(filename, "wb");
    if (!f) {
        return(FALSE);
    }
    io = lib3ds_io_new(
             f,
             fileio_error_func,
             fileio_seek_func,
             fileio_tell_func,
             fileio_read_func,
             fileio_write_func
         );
    if (!io) {
        fclose(f);
        return FALSE;
    }

    result = lib3ds_file_write(file, io);

    fclose(f);

    lib3ds_io_free(io);
    return(result);
}


/*!
 * Creates and returns a new, empty Lib3dsFile object.
 *
 * \return A pointer to the Lib3dsFile structure.
 *  If the structure cannot be allocated, NULL is returned.
 *
 * \ingroup file
 */
Lib3dsFile*
lib3ds_file_new() {
    Lib3dsFile *file;

    file = (Lib3dsFile*)calloc(sizeof(Lib3dsFile), 1);
    if (!file) {
        return(0);
    }
    file->mesh_version = 3;
    file->master_scale = 1.0f;
    file->keyf_revision = 5;
    strcpy(file->name, "LIB3DS");

    file->frames = 100;
    file->segment_from = 0;
    file->segment_to = 100;
    file->current_frame = 0;

    return(file);
}


/*!
 * Free a Lib3dsFile object and all of its resources.
 *
 * \param file The Lib3dsFile object to be freed.
 *
 * \ingroup file
 */
void
lib3ds_file_free(Lib3dsFile* file) {
    assert(file);
    lib3ds_file_material_reserve(file, 0, TRUE);
    lib3ds_file_camera_reserve(file, 0, TRUE);
    lib3ds_file_light_reserve(file, 0, TRUE);
    lib3ds_file_mesh_reserve(file, 0, TRUE);
    {
        Lib3dsNode *p, *q;

        for (p = file->nodes; p; p = q) {
            q = p->next;
            lib3ds_node_free(p);
        }
    }
    free(file);
}


/*!
 * Evaluate all of the nodes in this Lib3dsFile object.
 *
 * \param file The Lib3dsFile object to be evaluated.
 * \param t time value, between 0. and file->frames
 *
 * \see lib3ds_node_eval
 *
 * \ingroup file
 */
void
lib3ds_file_eval(Lib3dsFile *file, float t) {
    Lib3dsNode *p;

    for (p = file->nodes; p != 0; p = p->next) {
        lib3ds_node_eval(p, t);
    }
}


static Lib3dsBool
named_object_read(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;
    char name[64];
    Lib3dsWord chunk;
    Lib3dsMesh *mesh = NULL;
    Lib3dsCamera *camera = NULL;
    Lib3dsLight *light = NULL;
    Lib3dsDword object_flags;

    if (!lib3ds_chunk_read_start(&c, LIB3DS_NAMED_OBJECT, io)) {
        return(FALSE);
    }
    if (!lib3ds_io_read_string(io, name, 64)) {
        return(FALSE);
    }
    lib3ds_chunk_dump_info("  NAME=%s", name);
    lib3ds_chunk_read_tell(&c, io);

    object_flags = 0;
    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_N_TRI_OBJECT: {
                mesh = lib3ds_mesh_new(name);
                if (!mesh) {
                    return(FALSE);
                }
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_mesh_read(file, mesh, io)) {
                    return(FALSE);
                }
                lib3ds_file_mesh_insert(file, mesh, -1);
            }
            break;

            case LIB3DS_N_CAMERA: {
                camera = lib3ds_camera_new(name);
                if (!camera) {
                    return(FALSE);
                }
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_camera_read(camera, io)) {
                    return(FALSE);
                }
                lib3ds_file_camera_insert(file, camera, -1);
            }
            break;

            case LIB3DS_N_DIRECT_LIGHT: {
                light = lib3ds_light_new(name);
                if (!light) {
                    return(FALSE);
                }
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_light_read(light, io)) {
                    return(FALSE);
                }
                lib3ds_file_light_insert(file, light, -1);
            }
            break;

            case LIB3DS_OBJ_HIDDEN:
                object_flags |= LIB3DS_OBJECT_HIDDEN;
                break;

            case LIB3DS_OBJ_DOESNT_CAST:
                object_flags |= LIB3DS_OBJECT_DOESNT_CAST;
                break;

            case LIB3DS_OBJ_VIS_LOFTER:
                object_flags |= LIB3DS_OBJECT_VIS_LOFTER;
                break;

            case LIB3DS_OBJ_MATTE:
                object_flags |= LIB3DS_OBJECT_MATTE;
                break;

            case LIB3DS_OBJ_DONT_RCVSHADOW:
                object_flags |= LIB3DS_OBJECT_DONT_RCVSHADOW;
                break;

            case LIB3DS_OBJ_FAST:
                object_flags |= LIB3DS_OBJECT_FAST;
                break;

            case LIB3DS_OBJ_FROZEN:
                object_flags |= LIB3DS_OBJECT_FROZEN;
                break;

            default:
                lib3ds_chunk_unknown(chunk);
        }
    }

    if (mesh)
        mesh->object_flags = object_flags;
    if (camera)
        camera->object_flags = object_flags;
    if (light)
        light->object_flags = object_flags;

    lib3ds_chunk_read_end(&c, io);
    return(TRUE);
}


static Lib3dsBool
ambient_read(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;
    Lib3dsBool have_lin = FALSE;

    if (!lib3ds_chunk_read_start(&c, LIB3DS_AMBIENT_LIGHT, io)) {
        return(FALSE);
    }

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_LIN_COLOR_F: {
                int i;
                for (i = 0; i < 3; ++i) {
                    file->ambient[i] = lib3ds_io_read_float(io);
                }
            }
            have_lin = TRUE;
            break;

            case LIB3DS_COLOR_F: {
                /* gamma corrected color chunk
                   replaced in 3ds R3 by LIN_COLOR_24 */
                if (!have_lin) {
                    int i;
                    for (i = 0; i < 3; ++i) {
                        file->ambient[i] = lib3ds_io_read_float(io);
                    }
                }
            }
            break;

            default:
                lib3ds_chunk_unknown(chunk);
        }
    }

    lib3ds_chunk_read_end(&c, io);
    return(TRUE);
}


static Lib3dsBool
mdata_read(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;

    if (!lib3ds_chunk_read_start(&c, LIB3DS_MDATA, io)) {
        return(FALSE);
    }

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_MESH_VERSION: {
                file->mesh_version = lib3ds_io_read_intd(io);
            }
            break;

            case LIB3DS_MASTER_SCALE: {
                file->master_scale = lib3ds_io_read_float(io);
            }
            break;

            case LIB3DS_SHADOW_MAP_SIZE:
            case LIB3DS_LO_SHADOW_BIAS:
            case LIB3DS_HI_SHADOW_BIAS:
            case LIB3DS_SHADOW_SAMPLES:
            case LIB3DS_SHADOW_RANGE:
            case LIB3DS_SHADOW_FILTER:
            case LIB3DS_RAY_BIAS: {
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_shadow_read(&file->shadow, io)) {
                    return(FALSE);
                }
            }
            break;

            case LIB3DS_VIEWPORT_LAYOUT:
            case LIB3DS_DEFAULT_VIEW: {
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_viewport_read(&file->viewport, io)) {
                    return(FALSE);
                }
            }
            break;

            case LIB3DS_O_CONSTS: {
                int i;
                for (i = 0; i < 3; ++i) {
                    file->construction_plane[i] = lib3ds_io_read_float(io);
                }
            }
            break;

            case LIB3DS_AMBIENT_LIGHT: {
                lib3ds_chunk_read_reset(&c, io);
                if (!ambient_read(file, io)) {
                    return(FALSE);
                }
            }
            break;

            case LIB3DS_BIT_MAP:
            case LIB3DS_SOLID_BGND:
            case LIB3DS_V_GRADIENT:
            case LIB3DS_USE_BIT_MAP:
            case LIB3DS_USE_SOLID_BGND:
            case LIB3DS_USE_V_GRADIENT: {
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_background_read(&file->background, io)) {
                    return(FALSE);
                }
            }
            break;

            case LIB3DS_FOG:
            case LIB3DS_LAYER_FOG:
            case LIB3DS_DISTANCE_CUE:
            case LIB3DS_USE_FOG:
            case LIB3DS_USE_LAYER_FOG:
            case LIB3DS_USE_DISTANCE_CUE: {
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_atmosphere_read(&file->atmosphere, io)) {
                    return(FALSE);
                }
            }
            break;

            case LIB3DS_MAT_ENTRY: {
                Lib3dsMaterial *material;

                material = lib3ds_material_new();
                if (!material) {
                    return(FALSE);
                }
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_material_read(material, io)) {
                    return(FALSE);
                }
                lib3ds_file_material_insert(file, material, -1);
            }
            break;

            case LIB3DS_NAMED_OBJECT: {
                lib3ds_chunk_read_reset(&c, io);
                if (!named_object_read(file, io)) {
                    return(FALSE);
                }
            }
            break;

            default:
                lib3ds_chunk_unknown(chunk);
        }
    }

    lib3ds_chunk_read_end(&c, io);
    return(TRUE);
}


static Lib3dsBool
kfdata_read(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;
    Lib3dsDword node_number = 0;

    if (!lib3ds_chunk_read_start(&c, LIB3DS_KFDATA, io)) {
        return(FALSE);
    }

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_KFHDR: {
                file->keyf_revision = lib3ds_io_read_word(io);
                if (!lib3ds_io_read_string(io, file->name, 12 + 1)) {
                    return(FALSE);
                }
                file->frames = lib3ds_io_read_intd(io);
            }
            break;

            case LIB3DS_KFSEG: {
                file->segment_from = lib3ds_io_read_intd(io);
                file->segment_to = lib3ds_io_read_intd(io);
            }
            break;

            case LIB3DS_KFCURTIME: {
                file->current_frame = lib3ds_io_read_intd(io);
            }
            break;

            case LIB3DS_VIEWPORT_LAYOUT:
            case LIB3DS_DEFAULT_VIEW: {
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_viewport_read(&file->viewport_keyf, io)) {
                    return(FALSE);
                }
            }
            break;

            case LIB3DS_AMBIENT_NODE_TAG: {
                Lib3dsNode *node;

                node = lib3ds_node_new(LIB3DS_AMBIENT_NODE);
                if (!node) {
                    return(FALSE);
                }
                node->node_id = node_number++;
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_node_read(node, file, io)) {
                    return(FALSE);
                }
                lib3ds_file_insert_node(file, node);
            }
            break;

            case LIB3DS_OBJECT_NODE_TAG: {
                Lib3dsNode *node;

                node = lib3ds_node_new(LIB3DS_OBJECT_NODE);
                if (!node) {
                    return(FALSE);
                }
                node->node_id = node_number++;
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_node_read(node, file, io)) {
                    return(FALSE);
                }
                lib3ds_file_insert_node(file, node);
            }
            break;

            case LIB3DS_CAMERA_NODE_TAG: {
                Lib3dsNode *node;

                node = lib3ds_node_new(LIB3DS_CAMERA_NODE);
                if (!node) {
                    return(FALSE);
                }
                node->node_id = node_number++;
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_node_read(node, file, io)) {
                    return(FALSE);
                }
                lib3ds_file_insert_node(file, node);
            }
            break;

            case LIB3DS_TARGET_NODE_TAG: {
                Lib3dsNode *node;

                node = lib3ds_node_new(LIB3DS_TARGET_NODE);
                if (!node) {
                    return(FALSE);
                }
                node->node_id = node_number++;
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_node_read(node, file, io)) {
                    return(FALSE);
                }
                lib3ds_file_insert_node(file, node);
            }
            break;

            case LIB3DS_LIGHT_NODE_TAG:
            case LIB3DS_SPOTLIGHT_NODE_TAG: {
                Lib3dsNode *node;

                node = lib3ds_node_new(LIB3DS_LIGHT_NODE);
                if (!node) {
                    return(FALSE);
                }
                node->node_id = node_number++;
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_node_read(node, file, io)) {
                    return(FALSE);
                }
                lib3ds_file_insert_node(file, node);
            }
            break;

            case LIB3DS_L_TARGET_NODE_TAG: {
                Lib3dsNode *node;

                node = lib3ds_node_new(LIB3DS_SPOT_NODE);
                if (!node) {
                    return(FALSE);
                }
                node->node_id = node_number++;
                lib3ds_chunk_read_reset(&c, io);
                if (!lib3ds_node_read(node, file, io)) {
                    return(FALSE);
                }
                lib3ds_file_insert_node(file, node);
            }
            break;

            default:
                lib3ds_chunk_unknown(chunk);
        }
    }

    lib3ds_chunk_read_end(&c, io);
    return(TRUE);
}


/*!
 * Read 3ds file data into a Lib3dsFile object.
 *
 * \param file The Lib3dsFile object to be filled.
 * \param io A Lib3dsIo object previously set up by the caller.
 *
 * \return LIB3DS_TRUE on success, LIB3DS_FALSE on failure.
 *
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_read(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;

    if (!lib3ds_chunk_read_start(&c, 0, io)) {
        return(FALSE);
    }
    switch (c.chunk) {
        case LIB3DS_MDATA: {
            lib3ds_chunk_read_reset(&c, io);
            if (!mdata_read(file, io)) {
                return(FALSE);
            }
        }
        break;
        case LIB3DS_M3DMAGIC:
        case LIB3DS_MLIBMAGIC:
        case LIB3DS_CMAGIC: {
            while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
                switch (chunk) {
                    case LIB3DS_M3D_VERSION: {
                        file->mesh_version = lib3ds_io_read_dword(io);
                    }
                    break;
                    case LIB3DS_MDATA: {
                        lib3ds_chunk_read_reset(&c, io);
                        if (!mdata_read(file, io)) {
                            return(FALSE);
                        }
                    }
                    break;
                    case LIB3DS_KFDATA: {
                        lib3ds_chunk_read_reset(&c, io);
                        if (!kfdata_read(file, io)) {
                            return(FALSE);
                        }
                    }
                    break;
                    default:
                        lib3ds_chunk_unknown(chunk);
                }
            }
        }
        break;
        default:
            lib3ds_chunk_unknown(c.chunk);
            return(FALSE);
    }

    lib3ds_chunk_read_end(&c, io);
    return(TRUE);
}


static Lib3dsBool
colorf_write(Lib3dsRgba rgb, Lib3dsIo *io) {
    Lib3dsChunk c;

    c.chunk = LIB3DS_COLOR_F;
    c.size = 18;
    lib3ds_chunk_write(&c, io);
    lib3ds_io_write_rgb(io, rgb);

    c.chunk = LIB3DS_LIN_COLOR_F;
    c.size = 18;
    lib3ds_chunk_write(&c, io);
    lib3ds_io_write_rgb(io, rgb);
    return(TRUE);
}


static Lib3dsBool
object_flags_write(Lib3dsDword flags, Lib3dsIo *io) {
    if (flags) {
        if (flags & LIB3DS_OBJECT_HIDDEN) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_HIDDEN, io))
                return FALSE;
        }
        if (flags & LIB3DS_OBJECT_VIS_LOFTER) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_VIS_LOFTER, io))
                return FALSE;
        }
        if (flags & LIB3DS_OBJECT_DOESNT_CAST) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_DOESNT_CAST, io))
                return FALSE;
        }
        if (flags & LIB3DS_OBJECT_MATTE) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_MATTE, io))
                return FALSE;
        }
        if (flags & LIB3DS_OBJECT_DONT_RCVSHADOW) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_DOESNT_CAST, io))
                return FALSE;
        }
        if (flags & LIB3DS_OBJECT_FAST) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_FAST, io))
                return FALSE;
        }
        if (flags & LIB3DS_OBJECT_FROZEN) {
            if (!lib3ds_chunk_write_switch(LIB3DS_OBJ_FROZEN, io))
                return FALSE;
        }
    }
    return TRUE;
}


static Lib3dsBool
mdata_write(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;

    c.chunk = LIB3DS_MDATA;
    if (!lib3ds_chunk_write_start(&c, io)) {
        return(FALSE);
    }

    { /*---- LIB3DS_MESH_VERSION ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_MESH_VERSION;
        c.size = 10;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_intd(io, file->mesh_version);
    }
    { /*---- LIB3DS_MASTER_SCALE ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_MASTER_SCALE;
        c.size = 10;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_float(io, file->master_scale);
    }
    { /*---- LIB3DS_O_CONSTS ----*/
        int i;
        for (i = 0; i < 3; ++i) {
            if (fabs(file->construction_plane[i]) > LIB3DS_EPSILON) {
                break;
            }
        }
        if (i < 3) {
            Lib3dsChunk c;
            c.chunk = LIB3DS_O_CONSTS;
            c.size = 18;
            lib3ds_chunk_write(&c, io);
            lib3ds_io_write_vector(io, file->construction_plane);
        }
    }

    { /*---- LIB3DS_AMBIENT_LIGHT ----*/
        int i;
        for (i = 0; i < 3; ++i) {
            if (fabs(file->ambient[i]) > LIB3DS_EPSILON) {
                break;
            }
        }
        if (i < 3) {
            Lib3dsChunk c;
            c.chunk = LIB3DS_AMBIENT_LIGHT;
            c.size = 42;
            lib3ds_chunk_write(&c, io);
            colorf_write(file->ambient, io);
        }
    }
    lib3ds_background_write(&file->background, io);
    lib3ds_atmosphere_write(&file->atmosphere, io);
    lib3ds_shadow_write(&file->shadow, io);
    lib3ds_viewport_write(&file->viewport, io);
    {
        int i;
        for (i = 0; i < file->nmaterials; ++i) {
            lib3ds_material_write(file->materials[i], io);
        }
    }
    {
        Lib3dsChunk c;
        int i;

        for (i = 0; i < file->ncameras; ++i) {
            c.chunk = LIB3DS_NAMED_OBJECT;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_io_write_string(io, file->cameras[i]->name);
            lib3ds_camera_write(file->cameras[i], io);
            object_flags_write(file->cameras[i]->object_flags, io);
            lib3ds_chunk_write_end(&c, io);
        }
    }
    {
        Lib3dsChunk c;
        int i;

        for (i = 0; i < file->nlights; ++i) {
            c.chunk = LIB3DS_NAMED_OBJECT;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_io_write_string(io, file->lights[i]->name);
            lib3ds_light_write(file->lights[i], io);
            object_flags_write(file->lights[i]->object_flags, io);
            lib3ds_chunk_write_end(&c, io);
        }
    }
    {
        Lib3dsChunk c;
        int i;

        for (i = 0; i < file->nmeshes; ++i) {
            c.chunk = LIB3DS_NAMED_OBJECT;
            lib3ds_chunk_write_start(&c, io);
            lib3ds_io_write_string(io, file->meshes[i]->name);
            lib3ds_mesh_write(file, file->meshes[i], io);
            object_flags_write(file->meshes[i]->object_flags, io);
            lib3ds_chunk_write_end(&c, io);
        }
    }

    if (!lib3ds_chunk_write_end(&c, io)) {
        return(FALSE);
    }
    return(TRUE);
}



static Lib3dsBool
nodes_write(Lib3dsNode *node, Lib3dsFile *file, Lib3dsIo *io) {
    {
        Lib3dsNode *p;
        for (p = node->childs; p != 0; p = p->next) {
            if (!lib3ds_node_write(p, file, io)) {
                return(FALSE);
            }
            nodes_write(p, file, io);
        }
    }
    return(TRUE);
}


static Lib3dsBool
kfdata_write(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;

    if (!file->nodes) {
        return(TRUE);
    }

    c.chunk = LIB3DS_KFDATA;
    if (!lib3ds_chunk_write_start(&c, io)) {
        return(FALSE);
    }

    { /*---- LIB3DS_KFHDR ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_KFHDR;
        c.size = 6 + 2 + (Lib3dsDword)strlen(file->name) + 1 + 4;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_intw(io, file->keyf_revision);
        lib3ds_io_write_string(io, file->name);
        lib3ds_io_write_intd(io, file->frames);
    }
    { /*---- LIB3DS_KFSEG ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_KFSEG;
        c.size = 14;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_intd(io, file->segment_from);
        lib3ds_io_write_intd(io, file->segment_to);
    }
    { /*---- LIB3DS_KFCURTIME ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_KFCURTIME;
        c.size = 10;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_intd(io, file->current_frame);
    }
    lib3ds_viewport_write(&file->viewport_keyf, io);

    {
        Lib3dsNode *p;
        for (p = file->nodes; p != 0; p = p->next) {
            if (!lib3ds_node_write(p, file, io)) {
                return(FALSE);
            }
            if (!nodes_write(p, file, io)) {
                return(FALSE);
            }
        }
    }

    if (!lib3ds_chunk_write_end(&c, io)) {
        return(FALSE);
    }
    return(TRUE);
}


/*!
 * Write 3ds file data from a Lib3dsFile object to a file.
 *
 * \param file The Lib3dsFile object to be written.
 * \param io A Lib3dsIo object previously set up by the caller.
 *
 * \return LIB3DS_TRUE on success, LIB3DS_FALSE on failure.
 *
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_write(Lib3dsFile *file, Lib3dsIo *io) {
    Lib3dsChunk c;

    c.chunk = LIB3DS_M3DMAGIC;
    if (!lib3ds_chunk_write_start(&c, io)) {
        LIB3DS_ERROR_LOG;
        return(FALSE);
    }

    { /*---- LIB3DS_M3D_VERSION ----*/
        Lib3dsChunk c;

        c.chunk = LIB3DS_M3D_VERSION;
        c.size = 10;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_dword(io, file->mesh_version);
    }

    if (!mdata_write(file, io)) {
        return(FALSE);
    }
    if (!kfdata_write(file, io)) {
        return(FALSE);
    }

    if (!lib3ds_chunk_write_end(&c, io)) {
        return(FALSE);
    }
    return(TRUE);
}


void lib3ds_file_material_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force) {
    assert(file);
    lib3ds_util_reserve_array(&file->materials, &file->nmaterials, &file->materials_size, size, force, (Lib3dsFreeFunc)lib3ds_material_free);
}


void
lib3ds_file_material_insert(Lib3dsFile *file, Lib3dsMaterial *material, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_insert_array(&file->materials, &file->nmaterials, &file->materials_size, material, index);
}


void
lib3ds_file_material_remove(Lib3dsFile *file, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_remove_array(&file->materials, &file->nmaterials, index, (Lib3dsFreeFunc)lib3ds_material_free);
}


Lib3dsIntd
lib3ds_file_material_by_name(Lib3dsFile *file, const char *name) {
    int i;

    ASSERT(file);
    for (i = 0; i < file->nmaterials; ++i) {
        if (strcmp(file->materials[i]->name, name) == 0) {
            return(i);
        }
    }
    return -1;
}


void lib3ds_file_camera_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force) {
    assert(file);
    lib3ds_util_reserve_array(&file->cameras, &file->ncameras, &file->cameras_size, size, force, (Lib3dsFreeFunc)lib3ds_camera_free);
}


void
lib3ds_file_camera_insert(Lib3dsFile *file, Lib3dsCamera *camera, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_insert_array(&file->cameras, &file->ncameras, &file->cameras_size, camera, index);
}


void
lib3ds_file_camera_remove(Lib3dsFile *file, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_remove_array(&file->cameras, &file->ncameras, index, (Lib3dsFreeFunc)lib3ds_camera_free);
}


Lib3dsIntd
lib3ds_file_camera_by_name(Lib3dsFile *file, const char *name) {
    int i;

    ASSERT(file);
    for (i = 0; i < file->ncameras; ++i) {
        if (strcmp(file->cameras[i]->name, name) == 0) {
            return(i);
        }
    }
    return -1;
}


void lib3ds_file_light_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force) {
    assert(file);
    lib3ds_util_reserve_array(&file->lights, &file->nlights, &file->lights_size, size, force, (Lib3dsFreeFunc)lib3ds_light_free);
}


void
lib3ds_file_light_insert(Lib3dsFile *file, Lib3dsLight *light, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_insert_array(&file->lights, &file->nlights, &file->lights_size, light, index);
}


void
lib3ds_file_light_remove(Lib3dsFile *file, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_remove_array(&file->lights, &file->nlights, index, (Lib3dsFreeFunc)lib3ds_light_free);
}


Lib3dsIntd
lib3ds_file_light_by_name(Lib3dsFile *file, const char *name) {
    int i;

    ASSERT(file);
    for (i = 0; i < file->nlights; ++i) {
        if (strcmp(file->lights[i]->name, name) == 0) {
            return(i);
        }
    }
    return -1;
}


void lib3ds_file_mesh_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force) {
    assert(file);
    lib3ds_util_reserve_array(&file->meshes, &file->nmeshes, &file->meshes_size, size, force, (Lib3dsFreeFunc)lib3ds_mesh_free);
}


void
lib3ds_file_mesh_insert(Lib3dsFile *file, Lib3dsMesh *mesh, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_insert_array(&file->meshes, &file->nmeshes, &file->meshes_size, mesh, index);
}


void
lib3ds_file_remove_mesh(Lib3dsFile *file, Lib3dsIntd index) {
    assert(file);
    lib3ds_util_remove_array(&file->meshes, &file->nmeshes, index, (Lib3dsFreeFunc)lib3ds_mesh_free);
}


Lib3dsIntd
lib3ds_file_mesh_by_name(Lib3dsFile *file, const char *name) {
    int i;

    ASSERT(file);
    for (i = 0; i < file->nmeshes; ++i) {
        if (strcmp(file->meshes[i]->name, name) == 0) {
            return(i);
        }
    }
    return -1;
}


/*!
 * Return a node object by name and type.
 *
 * This function performs a recursive search for the specified node.
 * Both name and type must match.
 *
 * \param file The Lib3dsFile to be searched.
 * \param name The target node name.
 * \param type The target node type
 *
 * \return A pointer to the first matching node, or NULL if not found.
 *
 * \see lib3ds_node_by_name
 *
 * \ingroup file
 */
Lib3dsNode*
lib3ds_file_node_by_name(Lib3dsFile *file, const char* name, Lib3dsNodeType type) {
    Lib3dsNode *p, *q;

    ASSERT(file);
    for (p = file->nodes; p != 0; p = p->next) {
        if ((p->type == type) && (strcmp(p->name, name) == 0)) {
            return(p);
        }
        q = lib3ds_node_by_name(p, name, type);
        if (q) {
            return(q);
        }
    }
    return(0);
}


/*!
 * Return a node object by id.
 *
 * This function performs a recursive search for the specified node.
 *
 * \param file The Lib3dsFile to be searched.
 * \param node_id The target node id.
 *
 * \return A pointer to the first matching node, or NULL if not found.
 *
 * \see lib3ds_node_by_id
 *
 * \ingroup file
 */
Lib3dsNode*
lib3ds_file_node_by_id(Lib3dsFile *file, Lib3dsWord node_id) {
    Lib3dsNode *p, *q;

    ASSERT(file);
    for (p = file->nodes; p != 0; p = p->next) {
        if (p->node_id == node_id) {
            return(p);
        }
        q = lib3ds_node_by_id(p, node_id);
        if (q) {
            return(q);
        }
    }
    return(0);
}


/*!
 * Insert a new node into a Lib3dsFile object.
 *
 * If the node's parent_id structure is not LIB3DS_NO_PARENT and the
 * specified parent is found inside the Lib3dsFile object, then the
 * node is inserted as a child of that parent.  If the parent_id
 * structure is LIB3DS_NO_PARENT or the specified parent is not found,
 * then the node is inserted at the top level.
 *
 * Node is inserted in alphabetic order by name.
 *
 * Finally, if any other top-level nodes in file specify this node as
 * their parent, they are relocated as a child of this node.
 *
 * \param file The Lib3dsFile object to be modified.
 * \param node The node to be inserted into file
 *
 * \ingroup file
 */
void
lib3ds_file_insert_node(Lib3dsFile *file, Lib3dsNode *node) {
    Lib3dsNode *parent, *p, *n;

    ASSERT(node);
    ASSERT(!node->next);
    ASSERT(!node->parent);

    parent = 0;
    if (node->parent_id != LIB3DS_NO_PARENT) {
        parent = lib3ds_file_node_by_id(file, node->parent_id);
    }
    node->parent = parent;

    if (!parent) {
        for (p = 0, n = file->nodes; n != 0; p = n, n = n->next) {
            if (strcmp(n->name, node->name) > 0) {
                break;
            }
        }
        if (!p) {
            node->next = file->nodes;
            file->nodes = node;
        } else {
            node->next = p->next;
            p->next = node;
        }
    } else {
        for (p = 0, n = parent->childs; n != 0; p = n, n = n->next) {
            if (strcmp(n->name, node->name) > 0) {
                break;
            }
        }
        if (!p) {
            node->next = parent->childs;
            parent->childs = node;
        } else {
            node->next = p->next;
            p->next = node;
        }
    }

    if (node->node_id != LIB3DS_NO_PARENT) {
        for (n = file->nodes; n != 0; n = p) {
            p = n->next;
            if (n->parent_id == node->node_id) {
                lib3ds_file_remove_node(file, n);
                lib3ds_file_insert_node(file, n);
            }
        }
    }
}


/*!
 * Remove a node from the a Lib3dsFile object.
 *
 * \param file The Lib3dsFile object to be modified.
 * \param node The Lib3dsNode object to be removed from file
 *
 * \return LIB3DS_TRUE on success, LIB3DS_FALSE if node is not found in file
 *
 * \ingroup file
 */
Lib3dsBool
lib3ds_file_remove_node(Lib3dsFile *file, Lib3dsNode *node) {
    Lib3dsNode *p, *n;

    if (node->parent) {
        for (p = 0, n = node->parent->childs; n; p = n, n = n->next) {
            if (n == node) {
                break;
            }
        }
        if (!n) {
            return(FALSE);
        }

        if (!p) {
            node->parent->childs = n->next;
        } else {
            p->next = n->next;
        }
    } else {
        for (p = 0, n = file->nodes; n; p = n, n = n->next) {
            if (n == node) {
                break;
            }
        }
        if (!n) {
            return(FALSE);
        }

        if (!p) {
            file->nodes = n->next;
        } else {
            p->next = n->next;
        }
    }
    return(TRUE);
}


/*!
 * This function computes the bounding box of meshes, cameras and lights
 * defined in the 3D editor.
 *
 * \param file              The Lib3dsFile object to be examined.
 * \param include_meshes    Include meshes in bounding box calculation.
 * \param include_cameras   Include cameras in bounding box calculation.
 * \param include_lights    Include lights in bounding box calculation.
 * \param bmin              Returned minimum x,y,z values.
 * \param bmax              Returned maximum x,y,z values.
 *
 * \ingroup file
 */
void
lib3ds_file_bounding_box_of_objects(Lib3dsFile *file, Lib3dsBool include_meshes,
                                    Lib3dsBool include_cameras, Lib3dsBool include_lights,
                                    Lib3dsVector bmin, Lib3dsVector bmax) {
    bmin[0] = bmin[1] = bmin[2] = FLT_MAX;
    bmax[0] = bmax[1] = bmax[2] = FLT_MIN;

    if (include_meshes) {
        Lib3dsVector lmin, lmax;
        int i;
        for (i = 0; i < file->nmeshes; ++i) {
            lib3ds_mesh_bounding_box(file->meshes[i], lmin, lmax);
            lib3ds_vector_min(bmin, lmin);
            lib3ds_vector_max(bmax, lmax);
        }
    }
    if (include_cameras) {
        int i;
        for (i = 0; i < file->ncameras; ++i) {
            lib3ds_vector_min(bmin, file->cameras[i]->position);
            lib3ds_vector_max(bmax, file->cameras[i]->position);
            lib3ds_vector_min(bmin, file->cameras[i]->target);
            lib3ds_vector_max(bmax, file->cameras[i]->target);
        }
    }
    if (include_lights) {
        int i;
        for (i = 0; i < file->ncameras; ++i) {
            lib3ds_vector_min(bmin, file->lights[i]->position);
            lib3ds_vector_max(bmax, file->lights[i]->position);
            if (file->lights[i]->spot_light) {
                lib3ds_vector_min(bmin, file->lights[i]->spot);
                lib3ds_vector_max(bmax, file->lights[i]->spot);
            }
        }
    }
}


static void
file_bounding_box_of_nodes_impl(Lib3dsNode *node, Lib3dsFile *file, Lib3dsBool include_meshes,
                                Lib3dsBool include_cameras, Lib3dsBool include_lights,
                                Lib3dsVector bmin, Lib3dsVector bmax) {
    switch (node->type) {
        case LIB3DS_OBJECT_NODE:
            if (include_meshes) {
                Lib3dsIntd index;

                index = lib3ds_file_mesh_by_name(file, node->data.object.instance);
                if (index < 0)
                    index = lib3ds_file_mesh_by_name(file, node->name);
                if (index >= 0) {
                    Lib3dsMesh *mesh;
                    Lib3dsMatrix inv_matrix, M;
                    Lib3dsVector v;
                    unsigned i;

                    mesh = file->meshes[index];
                    lib3ds_matrix_copy(inv_matrix, mesh->matrix);
                    lib3ds_matrix_inv(inv_matrix);
                    lib3ds_matrix_copy(M, node->matrix);
                    lib3ds_matrix_translate_xyz(M, -node->data.object.pivot[0], -node->data.object.pivot[1], -node->data.object.pivot[2]);
                    lib3ds_matrix_mult(M, inv_matrix);

                    for (i = 0; i < mesh->nvertices; ++i) {
                        lib3ds_vector_transform(v, M, mesh->vertices[i]);
                        lib3ds_vector_min(bmin, v);
                        lib3ds_vector_max(bmax, v);
                    }
                }
            }
            break;

        case LIB3DS_CAMERA_NODE:
        case LIB3DS_TARGET_NODE:
            if (include_cameras) {
                Lib3dsVector z, v;
                lib3ds_vector_zero(z);
                lib3ds_vector_transform(v, node->matrix, z);
                lib3ds_vector_min(bmin, v);
                lib3ds_vector_max(bmax, v);
            }
            break;

        case LIB3DS_LIGHT_NODE:
        case LIB3DS_SPOT_NODE:
            if (include_lights) {
                Lib3dsVector z, v;
                lib3ds_vector_zero(z);
                lib3ds_vector_transform(v, node->matrix, z);
                lib3ds_vector_min(bmin, v);
                lib3ds_vector_max(bmax, v);
            }
            break;
    }
    {
        Lib3dsNode *p = node->childs;
        while (p) {
            file_bounding_box_of_nodes_impl(p, file, include_meshes, include_cameras, include_lights, bmin, bmax);
            p = p->next;
        }
    }
}


/*!
 * This function computes the bounding box of mesh, camera and light instances
 * defined in the Keyframer.
 *
 * \param file              The Lib3dsFile object to be examined.
 * \param include_meshes    Include meshes in bounding box calculation.
 * \param include_cameras   Include cameras in bounding box calculation.
 * \param include_lights    Include lights in bounding box calculation.
 * \param bmin              Returned minimum x,y,z values.
 * \param bmax              Returned maximum x,y,z values.
 *
 * \ingroup file
 */
void
lib3ds_file_bounding_box_of_nodes(Lib3dsFile *file, Lib3dsBool include_meshes,
                                  Lib3dsBool include_cameras, Lib3dsBool include_lights,
                                  Lib3dsVector bmin, Lib3dsVector bmax) {
    Lib3dsNode *p;

    bmin[0] = bmin[1] = bmin[2] = FLT_MAX;
    bmax[0] = bmax[1] = bmax[2] = FLT_MIN;
    p = file->nodes;
    while (p) {
        file_bounding_box_of_nodes_impl(p, file, include_meshes, include_cameras, include_lights, bmin, bmax);
        p = p->next;
    }
}
