/*
    Copyright (C) 2008 by Jan Eric Kyprianidis <www.kyprianidis.com>
    All rights reserved.
    
    This program is free  software: you can redistribute it and/or modify 
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 2.1 of the License, or 
    (at your option) any later version.

    Thisprogram  is  distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
    GNU Lesser General Public License for more details.
    
    You should  have received a copy of the GNU Lesser General Public License
    along with  this program; If not, see <http://www.gnu.org/licenses/>. 
*/
#include <lib3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <vector>


#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif


/*!
\example 3ds2obj.c

Converts a <i>3DS</i> file to an <i>OBJ</i> file.

\code
Syntax: 3ds2obj [options] filename [options]\n"

Options:
  -h           This help\n"
\endcode
*/


static void
help() {
    fprintf(stderr,
            "3ds2obj\n"
            "Copyright (C) 2008 by Jan Eric Kyprianidis <www.kyprianidis.com>\n"
            "All rights reserved.\n"
            "\n"
            "Syntax: 3dsdump [options] 3ds-file obj-file\n"
            "\n"
            "Options:\n"
            "  -h           This help\n"
            "\n"
           );
    exit(1);     
}


static const char* input = 0;
static const char* output = 0;
static int max_vertices = 0;
static int max_texcos = 0;
static int max_normals = 0;


static void parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
                help();
            } else {
                help();
            }
        } else {
            if (!input) {
                input = argv[i];
            } else if (!output) {
                output = argv[i];
            } else {
                help();
            }
        }
    }
    if (!input || !output) {
        help();
    }
}


static void save_mesh(FILE *o, Lib3dsFile *f, Lib3dsMeshInstanceNode *node) {
    Lib3dsMesh *mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode*)node);
    if (!mesh || !mesh->vertices) return;

    fprintf(o, "# object %s\n", node->base.name);
    fprintf(o, "g %s\n", node->instance_name[0]? node->instance_name : node->base.name);

    bool export_texcos = (mesh->texcos != 0);
    bool export_normals = (mesh->faces != 0);

    for (int i = 0; i < mesh->nvertices; ++i) {
        fprintf(o, "v %f %f %f\n", mesh->vertices[i][0], 
                                   mesh->vertices[i][1], 
                                   mesh->vertices[i][2]);
    }
    fprintf(o, "# %d vertices\n", mesh->nvertices);

    if (export_texcos) {
        for (int i = 0; i < mesh->nvertices; ++i) {
            fprintf(o, "vt %f %f\n", mesh->texcos[i][0], 
                                     mesh->texcos[i][1]);
        }
        fprintf(o, "# %d texture vertices\n", mesh->nvertices);
    }

    if (export_normals) {
        float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
        lib3ds_mesh_calculate_vertex_normals(mesh, normals);
        for (int i = 0; i < 3 * mesh->nfaces; ++i) {
            fprintf(o, "vn %f %f %f\n", normals[i][0],
                                        normals[i][1],
                                        normals[i][2]);
        }
        free(normals);  
        fprintf(o, "# %d normals\n", 3 * mesh->nfaces);
    }

    switch ((int)export_texcos | ((int)export_normals << 1)) {
        case 0:
            for (int i = 0; i < mesh->nfaces; ++i) {
                fprintf(o, "f %d %d %d\n", 
                    mesh->faces[i].index[0] + max_vertices + 1,
                    mesh->faces[i].index[1] + max_vertices + 1,
                    mesh->faces[i].index[2] + max_vertices + 1);
            }
            break;

        case 1:
            for (int i = 0; i < mesh->nfaces; ++i) {
                fprintf(o, "f %d/%d %d/%d %d/%d\n", 
                    mesh->faces[i].index[0] + max_vertices + 1,
                    mesh->faces[i].index[0] + max_texcos + 1,
                    mesh->faces[i].index[1] + max_vertices + 1,
                    mesh->faces[i].index[1] + max_texcos + 1,
                    mesh->faces[i].index[2] + max_vertices + 1,
                    mesh->faces[i].index[2] + max_texcos + 1);
            }
            break;

        case 2:
            for (int i = 0; i < mesh->nfaces; ++i) {
                fprintf(o, "#smoothing: 0x%x\n", mesh->faces[i].smoothing_group);
                fprintf(o, "f %d//%d %d//%d %d//%d\n", 
                    mesh->faces[i].index[0] + max_vertices + 1,
                    3 * i + 0 + max_normals + 1,
                    mesh->faces[i].index[1] + max_vertices + 1,
                    3 * i + 1 + max_normals + 1,
                    mesh->faces[i].index[2] + max_vertices + 1,
                    3 * i + 2 + max_normals + 1);
            }
            break;

        case 3:
            for (int i = 0; i < mesh->nfaces; ++i) {
                fprintf(o, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
                    mesh->faces[i].index[0] + max_vertices + 1,
                    mesh->faces[i].index[0] + max_texcos + 1,
                    3 * i + 0 + max_normals + 1,
                    mesh->faces[i].index[1] + max_vertices + 1,
                    mesh->faces[i].index[1] + max_texcos + 1,
                    3 * i + 1 + max_normals + 1,
                    mesh->faces[i].index[2] + max_vertices + 1,
                    mesh->faces[i].index[2] + max_texcos + 1,
                    3 * i + 2 + max_normals + 1);
            }
            break;
    }

    max_vertices += mesh->nvertices;
    if (export_texcos) 
        max_texcos += mesh->nvertices;
    if (export_normals) 
        max_normals += 3 * mesh->nfaces;
}


static void save_node(FILE *o, Lib3dsFile *f, Lib3dsNode *first_node) {
    for (Lib3dsNode *p = first_node; p; p = p->next) {
        if (p->type == LIB3DS_NODE_MESH_INSTANCE) {
            save_mesh(o, f, (Lib3dsMeshInstanceNode*)p);
            save_node(o, f, p->childs);
        }
    }
}


int main(int argc, char **argv) {
    Lib3dsFile *f;
    FILE *o;

    parse_args(argc, argv);

    f = lib3ds_file_open(input);
    if (!f) {
        fprintf(stderr, "***ERROR***\nLoading file failed: %s\n", input);
        exit(1);
    }

    o = fopen(output, "wt");
    if (!f) {
        fprintf(stderr, "***ERROR***\nCreating output file failed: %s\n", output);
        exit(1);
    }

    if (!f->nodes)
        lib3ds_file_create_nodes_for_meshes(f);
    lib3ds_file_eval(f, 0);
    save_node(o, f, f->nodes);

    fclose(o);

    lib3ds_file_free(f);
    return 0;
}
