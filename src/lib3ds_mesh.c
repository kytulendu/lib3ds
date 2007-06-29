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
 * $Id: mesh.c,v 1.29 2007/06/20 17:04:08 jeh Exp $
 */
#include "lib3ds_impl.h"


/*!
 * \defgroup mesh Meshes
 */


static Lib3dsBool
face_array_read(Lib3dsFile *file, Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;
    int i;
    int faces;

    if (!lib3ds_chunk_read_start(&c, LIB3DS_FACE_ARRAY, io)) {
        return (FALSE);
    }
    lib3ds_mesh_alloc_faces(mesh, 0);

    faces = lib3ds_io_read_word(io);
    if (faces) {
        lib3ds_mesh_alloc_faces(mesh, faces);
        for (i = 0; i < faces; ++i) {
            mesh->faces[i].material = -1;
            mesh->faces[i].index[0] = lib3ds_io_read_word(io);
            mesh->faces[i].index[1] = lib3ds_io_read_word(io);
            mesh->faces[i].index[2] = lib3ds_io_read_word(io);
            mesh->faces[i].flags = lib3ds_io_read_word(io);
        }
        lib3ds_chunk_read_tell(&c, io);

        while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
            switch (chunk) {
                case LIB3DS_SMOOTH_GROUP: {
                    unsigned i;

                    for (i = 0; i < mesh->nfaces; ++i) {
                        mesh->faces[i].smoothing = lib3ds_io_read_dword(io);
                    }
                }
                break;

                case LIB3DS_MSH_MAT_GROUP: {
                    char name[64];
                    unsigned faces;
                    unsigned i;
                    unsigned index;
                    int material;

                    lib3ds_io_read_string(io, name, 64);
                    material = lib3ds_file_material_by_name(file, name);

                    faces = lib3ds_io_read_word(io);
                    for (i = 0; i < faces; ++i) {
                        index = lib3ds_io_read_word(io);
                        if (index < mesh->nfaces) {
                            mesh->faces[index].material = material;
                        } else {
                            assert(0);
                        }
                    }
                }
                break;

                case LIB3DS_MSH_BOXMAP: {
                    char name[64];

                    if (!lib3ds_io_read_string(io, name, 64)) {
                        return (FALSE);
                    }
                    strcpy(mesh->box_map.front, name);
                    if (!lib3ds_io_read_string(io, name, 64)) {
                        return (FALSE);
                    }
                    strcpy(mesh->box_map.back, name);
                    if (!lib3ds_io_read_string(io, name, 64)) {
                        return (FALSE);
                    }
                    strcpy(mesh->box_map.left, name);
                    if (!lib3ds_io_read_string(io, name, 64)) {
                        return (FALSE);
                    }
                    strcpy(mesh->box_map.right, name);
                    if (!lib3ds_io_read_string(io, name, 64)) {
                        return (FALSE);
                    }
                    strcpy(mesh->box_map.top, name);
                    if (!lib3ds_io_read_string(io, name, 64)) {
                        return (FALSE);
                    }
                    strcpy(mesh->box_map.bottom, name);
                }
                break;

                default:
                    lib3ds_chunk_unknown(chunk);
            }
        }

    }
    lib3ds_chunk_read_end(&c, io);
    return (TRUE);
}


/*!
 * Create and return a new empty mesh object.
 *
 * Mesh is initialized with the name and an identity matrix; all
 * other fields are zero.
 *
 * See Lib3dsFaceFlag for definitions of per-face flags.
 *
 * \param name Mesh name.  Must not be NULL.  Must be < 64 characters.
 *
 * \return mesh object or NULL on error.
 *
 * \ingroup mesh
 */
Lib3dsMesh*
lib3ds_mesh_new(const char *name) {
    Lib3dsMesh *mesh;

    ASSERT(name);
    ASSERT(strlen(name) < 64);

    mesh = (Lib3dsMesh*) calloc(sizeof(Lib3dsMesh), 1);
    if (!mesh) {
        return (0);
    }
    strcpy(mesh->name, name);
    lib3ds_matrix_identity(mesh->matrix);
    mesh->map_data.maptype = LIB3DS_MAP_NONE;
    return (mesh);
}


/*!
 * Free a mesh object and all of its resources.
 *
 * \param mesh Mesh object to be freed.
 *
 * \ingroup mesh
 */
void
lib3ds_mesh_free(Lib3dsMesh *mesh) {
    lib3ds_mesh_alloc_vertices(mesh, 0);
    lib3ds_mesh_alloc_flags(mesh, 0);
    lib3ds_mesh_alloc_texcos(mesh, 0);
    lib3ds_mesh_alloc_faces(mesh, 0);
    memset(mesh, 0, sizeof(Lib3dsMesh));
    free(mesh);
}


void
lib3ds_mesh_alloc_vertices(Lib3dsMesh *mesh, Lib3dsDword nvertices) {
    assert(mesh);
    mesh->vertices = realloc(mesh->vertices, sizeof(Lib3dsVector) * nvertices);
    if (mesh->nvertices < nvertices) {
        memset(&mesh->vertices[mesh->nvertices], 0, sizeof(Lib3dsVector) * (nvertices - mesh->nvertices));
    }
    mesh->nvertices = nvertices;
}


void
lib3ds_mesh_alloc_flags(Lib3dsMesh *mesh, Lib3dsDword nflags) {
    assert(mesh);
    mesh->flags = realloc(mesh->flags, sizeof(Lib3dsWord) * nflags);
    if (mesh->nflags < nflags) {
        memset(&mesh->flags[mesh->nflags], 0, sizeof(Lib3dsWord) * (nflags - mesh->nflags));
    }
    mesh->nflags = nflags;
}


void
lib3ds_mesh_alloc_texcos(Lib3dsMesh *mesh, Lib3dsDword ntexcos) {
    assert(mesh);
    mesh->texcos = realloc(mesh->texcos, sizeof(Lib3dsTexco) * ntexcos);
    if (mesh->ntexcos < ntexcos) {
        memset(&mesh->texcos[mesh->ntexcos], 0, sizeof(Lib3dsTexco) * (ntexcos - mesh->ntexcos));
    }
    mesh->ntexcos = ntexcos;
}


void
lib3ds_mesh_alloc_faces(Lib3dsMesh *mesh, Lib3dsDword nfaces) {
    assert(mesh);
    mesh->faces = realloc(mesh->faces, sizeof(Lib3dsFace) * nfaces);
    if (mesh->nfaces < nfaces) {
        memset(&mesh->faces[mesh->nfaces], 0, sizeof(Lib3dsFace) * (nfaces - mesh->nfaces));
    }
    mesh->nfaces = nfaces;
}


/*!
 * Find the bounding box of a mesh object.
 *
 * \param mesh The mesh object
 * \param bmin Returned bounding box
 * \param bmax Returned bounding box
 *
 * \ingroup mesh
 */
void
lib3ds_mesh_bounding_box(Lib3dsMesh *mesh, Lib3dsVector bmin, Lib3dsVector bmax) {
    unsigned i;
    bmin[0] = bmin[1] = bmin[2] = FLT_MAX;
    bmax[0] = bmax[1] = bmax[2] = FLT_MIN;

    for (i = 0; i < mesh->nvertices; ++i) {
        lib3ds_vector_min(bmin, mesh->vertices[i]);
        lib3ds_vector_max(bmax, mesh->vertices[i]);
    }
}


typedef struct Lib3dsFaces {
    struct Lib3dsFaces *next;
    Lib3dsIntd index;
} Lib3dsFaces;


/*!
 * Calculates the vertex normals corresponding to the smoothing group
 * settings for each face of a mesh.
 *
 * \param mesh      A pointer to the mesh to calculate the normals for.
 * \param normals   A pointer to a buffer to store the calculated
 *                  normals. The buffer must have the size:
 *                  3*sizeof(Lib3dsVector)*mesh->nfaces.
 *
 * To allocate the normal buffer do for example the following:
 * \code
 *  Lib3dsVector *normals = malloc(3*sizeof(Lib3dsVector)*mesh->nfaces);
 * \endcode
 *
 * To access the normal of the i-th vertex of the j-th face do the
 * following:
 * \code
 *   normals[3*j+i]
 * \endcode
 *
 * \ingroup mesh
 */
void
lib3ds_mesh_calculate_normals(Lib3dsMesh *mesh, Lib3dsVector *normals) {
    Lib3dsFaces **fl;
    Lib3dsFaces *fa;
    Lib3dsVector *fn;
    unsigned i, j, k;
    Lib3dsVector *N;
    int N_size = 128;

    if (!mesh->nfaces) {
        return;
    }

    fl = calloc(sizeof(Lib3dsFaces*), mesh->nvertices);
    fa = calloc(sizeof(Lib3dsFaces), 3 * mesh->nfaces);
    fn = calloc(sizeof(Lib3dsVector), mesh->nfaces);
    N = calloc(sizeof(Lib3dsVector), N_size);

    k = 0;
    for (i = 0; i < mesh->nfaces; ++i) {
        Lib3dsFace *f = &mesh->faces[i];

        assert(f->index[0] < mesh->nvertices);
        assert(f->index[1] < mesh->nvertices);
        assert(f->index[2] < mesh->nvertices);

        for (j = 0; j < 3; ++j) {
            Lib3dsFaces* l = &fa[k++];
            assert(f->index[j] < mesh->nvertices);
            l->index = i;
            l->next = fl[f->index[j]];
            fl[f->index[j]] = l;
        }

        lib3ds_vector_normal(
            fn[i],
            mesh->vertices[f->index[0]],
            mesh->vertices[f->index[1]],
            mesh->vertices[f->index[2]]
        );
    }

    for (i = 0; i < mesh->nfaces; ++i) {
        Lib3dsFace *f = &mesh->faces[i];
        for (j = 0; j < 3; ++j) {
            Lib3dsVector n;
            Lib3dsFaces *p;
            Lib3dsFace *pf;
            int k, l;
            int found;

            assert(f->index[j] < mesh->nvertices);

            if (f->smoothing) {
                lib3ds_vector_zero(n);
                k = 0;
                for (p = fl[f->index[j]]; p; p = p->next) {
                    pf = &mesh->faces[p->index];

                    found = 0;
                    for (l = 0; l < k; ++l) {
                        if (l >= N_size) {
                            N_size *= 2;
                            N = realloc(N, sizeof(Lib3dsVector) * N_size);
                        }
                        if (fabs(lib3ds_vector_dot(N[l], fn[p->index]) - 1.0) < 1e-5) {
                            found = 1;
                            break;
                        }
                    }

                    if (!found) {
                        if (f->smoothing & pf->smoothing) {
                            lib3ds_vector_add(n, n, fn[p->index]);
                            lib3ds_vector_copy(N[k], fn[p->index]);
                            ++k;
                        }
                    }
                }
            } else {
                lib3ds_vector_copy(n, fn[i]);
            }

            lib3ds_vector_normalize(n);
            lib3ds_vector_copy(normals[3*i+j], n);
        }
    }

    free(N);
    free(fn);
    free(fa);
    free(fl);
}


/*!
 * This function prints data associated with the specified mesh such as
 * vertex and point lists.
 *
 * \param mesh  Points to a mesh that you wish to view the data for.
 *
 * \return None
 *
 * \warning WIN32: Should only be used in a console window not in a GUI.
 *
 * \ingroup mesh
 */
void
lib3ds_mesh_dump(Lib3dsMesh *mesh) {
    unsigned i;
    Lib3dsVector p;

    ASSERT(mesh);
    printf("  %s vertices=%ld faces=%ld\n",
           mesh->name,
           mesh->nvertices,
           mesh->nfaces
          );
    printf("  matrix:\n");
    lib3ds_matrix_dump(mesh->matrix);
    printf("  point list:\n");
    for (i = 0; i < mesh->nvertices; ++i) {
        lib3ds_vector_copy(p, mesh->vertices[i]);
        printf("    %8f %8f %8f\n", p[0], p[1], p[2]);
    }
    printf("  facelist:\n");
    for (i = 0; i < mesh->nfaces; ++i) {
        printf("    %4d %4d %4d  smoothing:%X  flags:%X  material:\"%s\"\n",
               mesh->faces[i].index[0],
               mesh->faces[i].index[1],
               mesh->faces[i].index[2],
               (unsigned) mesh->faces[i].smoothing,
               mesh->faces[i].flags,
               mesh->faces[i].material
              );
    }
}


/*!
 * \ingroup mesh
 */
Lib3dsBool
lib3ds_mesh_read(Lib3dsFile *file, Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;

    if (!lib3ds_chunk_read_start(&c, LIB3DS_N_TRI_OBJECT, io)) {
        return (FALSE);
    }

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_MESH_MATRIX: {
                int i, j;

                lib3ds_matrix_identity(mesh->matrix);
                for (i = 0; i < 4; i++) {
                    for (j = 0; j < 3; j++) {
                        mesh->matrix[i][j] = lib3ds_io_read_float(io);
                    }
                }
                break;
            }

            case LIB3DS_MESH_COLOR: {
                mesh->color = lib3ds_io_read_byte(io);
                break;
            }

            case LIB3DS_POINT_ARRAY: {
                unsigned i;
                unsigned nvertices;

                lib3ds_mesh_alloc_vertices(mesh, 0);
                nvertices = lib3ds_io_read_word(io);
                if (nvertices) {
                    lib3ds_mesh_alloc_vertices(mesh, nvertices);
                    for (i = 0; i < mesh->nvertices; ++i) {
                        lib3ds_io_read_vector(io, mesh->vertices[i]);
                    }
                    ASSERT((!mesh->nflags) || (mesh->nvertices == mesh->nflags));
                    ASSERT((!mesh->ntexcos) || (mesh->nvertices == mesh->ntexcos));
                }
                break;
            }

            case LIB3DS_POINT_FLAG_ARRAY: {
                unsigned i;
                unsigned nflags;

                lib3ds_mesh_alloc_flags(mesh, 0);
                nflags = lib3ds_io_read_word(io);
                if (nflags) {
                    lib3ds_mesh_alloc_flags(mesh, nflags);
                    for (i = 0; i < mesh->nflags; ++i) {
                        mesh->flags[i] = lib3ds_io_read_word(io);
                    }
                }
                break;
            }

            case LIB3DS_FACE_ARRAY: {
                lib3ds_chunk_read_reset(&c, io);
                if (!face_array_read(file, mesh, io)) {
                    return (FALSE);
                }
                break;
            }

            case LIB3DS_MESH_TEXTURE_INFO: {
                int i, j;

                for (i = 0; i < 2; ++i) {
                    mesh->map_data.tile[i] = lib3ds_io_read_float(io);
                }
                for (i = 0; i < 3; ++i) {
                    mesh->map_data.pos[i] = lib3ds_io_read_float(io);
                }
                mesh->map_data.scale = lib3ds_io_read_float(io);

                lib3ds_matrix_identity(mesh->map_data.matrix);
                for (i = 0; i < 4; i++) {
                    for (j = 0; j < 3; j++) {
                        mesh->map_data.matrix[i][j] = lib3ds_io_read_float(io);
                    }
                }
                for (i = 0; i < 2; ++i) {
                    mesh->map_data.planar_size[i] = lib3ds_io_read_float(io);
                }
                mesh->map_data.cylinder_height = lib3ds_io_read_float(io);
                break;
            }

            case LIB3DS_TEX_VERTS: {
                unsigned i;
                unsigned ntexcos;

                lib3ds_mesh_alloc_texcos(mesh, 0);
                ntexcos = lib3ds_io_read_word(io);
                if (ntexcos) {
                    lib3ds_mesh_alloc_texcos(mesh, ntexcos);
                    for (i = 0; i < mesh->ntexcos; ++i) {
                        mesh->texcos[i][0] = lib3ds_io_read_float(io);
                        mesh->texcos[i][1] = lib3ds_io_read_float(io);
                    }
                }
                break;
            }

            default:
                lib3ds_chunk_unknown(chunk);
        }
    }

    if (lib3ds_matrix_det(mesh->matrix) < 0.0) {
        /* Flip X coordinate of vertices if mesh matrix
           has negative determinant */
        Lib3dsMatrix inv_matrix, M;
        Lib3dsVector tmp;
        unsigned i;

        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);

        lib3ds_matrix_copy(M, mesh->matrix);
        lib3ds_matrix_scale_xyz(M, -1.0f, 1.0f, 1.0f);
        lib3ds_matrix_mult(M, inv_matrix);

        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }
    }

    lib3ds_chunk_read_end(&c, io);

    return TRUE;
}


static Lib3dsBool
point_array_write(Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;
    unsigned i;

    if (!mesh->nvertices || !mesh->vertices) {
        return (TRUE);
    }
    ASSERT(mesh->nvertices < 0x10000);
    c.chunk = LIB3DS_POINT_ARRAY;
    c.size = 8 + 12 * mesh->nvertices;
    lib3ds_chunk_write(&c, io);

    lib3ds_io_write_word(io, (Lib3dsWord) mesh->nvertices);

    if (lib3ds_matrix_det(mesh->matrix) >= 0.0f) {
        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_io_write_vector(io, mesh->vertices[i]);
        }
    } else {
        /* Flip X coordinate of vertices if mesh matrix
           has negative determinant */
        Lib3dsMatrix inv_matrix, M;
        Lib3dsVector tmp;

        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_copy(M, mesh->matrix);
        lib3ds_matrix_scale_xyz(M, -1.0f, 1.0f, 1.0f);
        lib3ds_matrix_mult(M, inv_matrix);

        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_io_write_vector(io, tmp);
        }
    }

    return (TRUE);
}


static Lib3dsBool
flag_array_write(Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;
    unsigned i;

    if (!mesh->nflags || !mesh->flags) {
        return (TRUE);
    }
    ASSERT(mesh->nflags < 0x10000);
    c.chunk = LIB3DS_POINT_FLAG_ARRAY;
    c.size = 8 + 2 * mesh->nflags;
    lib3ds_chunk_write(&c, io);

    lib3ds_io_write_word(io, (Lib3dsWord) mesh->nflags);
    for (i = 0; i < mesh->nflags; ++i) {
        lib3ds_io_write_word(io, mesh->flags[i]);
    }
    return (TRUE);
}


static Lib3dsBool
face_array_write(Lib3dsFile *file, Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;

    if (!mesh->nfaces || !mesh->faces) {
        return (TRUE);
    }
    ASSERT(mesh->nfaces < 0x10000);
    c.chunk = LIB3DS_FACE_ARRAY;
    if (!lib3ds_chunk_write_start(&c, io)) {
        return (FALSE);
    }
    {
        unsigned i;

        lib3ds_io_write_word(io, (Lib3dsWord) mesh->nfaces);
        for (i = 0; i < mesh->nfaces; ++i) {
            lib3ds_io_write_word(io, mesh->faces[i].index[0]);
            lib3ds_io_write_word(io, mesh->faces[i].index[1]);
            lib3ds_io_write_word(io, mesh->faces[i].index[2]);
            lib3ds_io_write_word(io, mesh->faces[i].flags);
        }
    }

    {
        /*---- MSH_MAT_GROUP ----*/
        Lib3dsChunk c;
        unsigned i, j;
        Lib3dsWord num;
        char *matf = calloc(sizeof(char), mesh->nfaces);
        assert(matf);

        for (i = 0; i < mesh->nfaces; ++i) {
            if (!matf[i] && (mesh->faces[i].material >= 0) && (mesh->faces[i].material < file->nmaterials)) {
                matf[i] = 1;
                num = 1;

                for (j = i + 1; j < mesh->nfaces; ++j) {
                    if (mesh->faces[i].material == mesh->faces[j].material) ++num;
                }

                c.chunk = LIB3DS_MSH_MAT_GROUP;
                c.size = 6 + (Lib3dsDword) strlen(file->materials[mesh->faces[i].material]->name) + 1 + 2 + 2 * num;
                lib3ds_chunk_write(&c, io);
                lib3ds_io_write_string(io, file->materials[mesh->faces[i].material]->name);
                lib3ds_io_write_word(io, num);
                lib3ds_io_write_word(io, (Lib3dsWord) i);

                for (j = i + 1; j < mesh->nfaces; ++j) {
                    if (mesh->faces[i].material == mesh->faces[j].material) {
                        lib3ds_io_write_word(io, (Lib3dsWord) j);
                        matf[j] = 1;
                    }
                }
            }
        }
        free(matf);
    }

    {
        /*---- SMOOTH_GROUP ----*/
        Lib3dsChunk c;
        unsigned i;

        c.chunk = LIB3DS_SMOOTH_GROUP;
        c.size = 6 + 4 * mesh->nfaces;
        lib3ds_chunk_write(&c, io);

        for (i = 0; i < mesh->nfaces; ++i) {
            lib3ds_io_write_dword(io, mesh->faces[i].smoothing);
        }
    }

    {
        /*---- MSH_BOXMAP ----*/
        Lib3dsChunk c;

        if (strlen(mesh->box_map.front) ||
            strlen(mesh->box_map.back) ||
            strlen(mesh->box_map.left) ||
            strlen(mesh->box_map.right) ||
            strlen(mesh->box_map.top) ||
            strlen(mesh->box_map.bottom)) {

            c.chunk = LIB3DS_MSH_BOXMAP;
            if (!lib3ds_chunk_write_start(&c, io)) {
                return (FALSE);
            }

            lib3ds_io_write_string(io, mesh->box_map.front);
            lib3ds_io_write_string(io, mesh->box_map.back);
            lib3ds_io_write_string(io, mesh->box_map.left);
            lib3ds_io_write_string(io, mesh->box_map.right);
            lib3ds_io_write_string(io, mesh->box_map.top);
            lib3ds_io_write_string(io, mesh->box_map.bottom);

            if (!lib3ds_chunk_write_end(&c, io)) {
                return (FALSE);
            }
        }
    }

    if (!lib3ds_chunk_write_end(&c, io)) {
        return (FALSE);
    }
    return (TRUE);
}


static Lib3dsBool
texel_array_write(Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;
    unsigned i;

    if (!mesh->ntexcos || !mesh->texcos) {
        return (TRUE);
    }
    ASSERT(mesh->ntexcos < 0x10000);
    c.chunk = LIB3DS_TEX_VERTS;
    c.size = 8 + 8 * mesh->ntexcos;
    lib3ds_chunk_write(&c, io);

    lib3ds_io_write_word(io, (Lib3dsWord) mesh->ntexcos);
    for (i = 0; i < mesh->ntexcos; ++i) {
        lib3ds_io_write_float(io, mesh->texcos[i][0]);
        lib3ds_io_write_float(io, mesh->texcos[i][1]);
    }
    return (TRUE);
}


/*!
 * \ingroup mesh
 */
Lib3dsBool
lib3ds_mesh_write(Lib3dsFile *file, Lib3dsMesh *mesh, Lib3dsIo *io) {
    Lib3dsChunk c;

    c.chunk = LIB3DS_N_TRI_OBJECT;
    if (!lib3ds_chunk_write_start(&c, io)) {
        return (FALSE);
    }
    if (!point_array_write(mesh, io)) {
        return (FALSE);
    }
    if (!texel_array_write(mesh, io)) {
        return (FALSE);
    }

    if (mesh->map_data.maptype != LIB3DS_MAP_NONE) {   /*---- LIB3DS_MESH_TEXTURE_INFO ----*/
        Lib3dsChunk c;
        int i, j;

        c.chunk = LIB3DS_MESH_TEXTURE_INFO;
        c.size = 92;
        if (!lib3ds_chunk_write(&c, io)) {
            return (FALSE);
        }

        lib3ds_io_write_word(io, mesh->map_data.maptype);

        for (i = 0; i < 2; ++i) {
            lib3ds_io_write_float(io, mesh->map_data.tile[i]);
        }
        for (i = 0; i < 3; ++i) {
            lib3ds_io_write_float(io, mesh->map_data.pos[i]);
        }
        lib3ds_io_write_float(io, mesh->map_data.scale);

        for (i = 0; i < 4; i++) {
            for (j = 0; j < 3; j++) {
                lib3ds_io_write_float(io, mesh->map_data.matrix[i][j]);
            }
        }
        for (i = 0; i < 2; ++i) {
            lib3ds_io_write_float(io, mesh->map_data.planar_size[i]);
        }
        lib3ds_io_write_float(io, mesh->map_data.cylinder_height);
    }

    if (!flag_array_write(mesh, io)) {
        return (FALSE);
    }
    {
        /*---- LIB3DS_MESH_MATRIX ----*/
        Lib3dsChunk c;
        int i, j;

        c.chunk = LIB3DS_MESH_MATRIX;
        c.size = 54;
        if (!lib3ds_chunk_write(&c, io)) {
            return (FALSE);
        }
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 3; j++) {
                lib3ds_io_write_float(io, mesh->matrix[i][j]);
            }
        }
    }

    if (mesh->color) {   /*---- LIB3DS_MESH_COLOR ----*/
        Lib3dsChunk c;

        c.chunk = LIB3DS_MESH_COLOR;
        c.size = 7;
        if (!lib3ds_chunk_write(&c, io)) {
            return (FALSE);
        }
        lib3ds_io_write_byte(io, mesh->color);
    }
    if (!face_array_write(file, mesh, io)) {
        return (FALSE);
    }

    if (!lib3ds_chunk_write_end(&c, io)) {
        return (FALSE);
    }
    return (TRUE);
}

