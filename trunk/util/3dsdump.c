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
 * $Id: 3dsdump.c,v 1.4 2007/06/14 11:29:39 jeh Exp $
 */
#include <lib3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif


/*!
\example 3dsdump.c

Displays information about the content of a <i>3DS</i> file.

\code
Syntax: 3dsdump [options] filename [options]\n"

Options:
  -h           This help\n"
  -d level     Set log level (0=ERROR, 1=WARN, 2=INFO, 3=DEBUG)
  -m           Dump materials
  -t           Dump trimeshes
  -i           Dump instances
  -c           Dump cameras
  -l           Dump lights
  -n           Dump node hierarchy
  -w filename  Write new 3ds file to disk
\endcode
*/


static void
help() {
    fprintf(stderr,
            "The 3D Studio File Format Library - 3dsdump\n"
            "Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.kyprianidis.com>\n"
            "All rights reserved.\n"
            "\n"
            "Syntax: 3dsdump [options] filename [options]\n"
            "\n"
            "Options:\n"
            "  -h           This help\n"
            "  -d=level     Set log level (0=ERROR, 1=WARN, 2=INFO, 3=DEBUG)\n"
            "  -m           Dump materials\n"
            "  -t           Dump trimeshes\n"
            "  -i           Dump instances\n"
            "  -c           Dump cameras\n"
            "  -l           Dump lights\n"
            "  -n           Dump node hierarchy\n"
            "  -w=filename  Write new 3ds file to disk\n"
            "\n"
           );
    exit(1);
}


typedef enum Flags {
    LIB3DSDUMP_MATERIALS  = 0x0004,
    LIB3DSDUMP_TRIMESHES  = 0x0008,
    LIB3DSDUMP_INSTANCES  = 0x0010,
    LIB3DSDUMP_CAMERAS    = 0x0020,
    LIB3DSDUMP_LIGHTS     = 0x0040,
    LIB3DSDUMP_NODES      = 0x0080
} Flags;


static const char* filename = 0;
static const char* output = 0;
static unsigned flags = 0;
static int  log_level = LIB3DS_LOG_INFO;


static void
parse_args(int argc, char **argv) {
    int i;

    for (i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
                help();
            } else if ((argv[i][1] == 'd') && (argv[i][2] == '='))  {
                log_level =  atoi(&argv[i][3]);
            } else if (argv[i][1] == 'm') {
                flags |= LIB3DSDUMP_MATERIALS;
            } else if (argv[i][1] == 't') {
                flags |= LIB3DSDUMP_TRIMESHES;
            } else if (argv[i][1] == 'i') {
                flags |= LIB3DSDUMP_INSTANCES;
            } else if (argv[i][1] == 'c') {
                flags |= LIB3DSDUMP_CAMERAS;
            } else if (argv[i][1] == 'l') {
                flags |= LIB3DSDUMP_LIGHTS;
            } else if (argv[i][1] == 'n') {
                flags |= LIB3DSDUMP_NODES;
            } else if ((argv[i][1] == 'w') && (argv[i][2] == '='))  {
                output =  &argv[i][3];
            } else {
                help();
            }
        } else {
            if (filename) {
                help();
            }
            filename = argv[i];
        }
    }
    if (!filename) {
        help();
    }
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
    return (fread(buffer, 1, size, f));
}


static size_t
fileio_write_func(void *self, const void *buffer, size_t size) {
    FILE *f = (FILE*)self;
    return (fwrite(buffer, 1, size, f));
}


static void 
fileio_log_func(Lib3dsIo *io, Lib3dsLogLevel level, char *msg)
{
    static const char * level_str[] = {
        "ERROR", "WARN", "INFO", "DEBUG"
    };
    if (log_level >=  level) {
        printf("%5s : %s\n", level_str[level], msg);
    }
}


static void
matrix_dump(Lib3dsMatrix matrix) {
    int i, j;

    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            printf("%f ", matrix[j][i]);
        }
        printf("\n");
    }
}


static void
viewport_dump(Lib3dsViewport *vp) {
    Lib3dsView *view;
    unsigned i;
    assert(vp);

    printf("  viewport:\n");
    printf("    layout:\n");
    printf("      style:       %d\n", vp->layout.style);
    printf("      active:      %d\n", vp->layout.active);
    printf("      swap:        %d\n", vp->layout.swap);
    printf("      swap_prior:  %d\n", vp->layout.swap_prior);
    printf("      position:    %d,%d\n", vp->layout.position[0], vp->layout.position[1]);
    printf("      size:        %d,%d\n", vp->layout.size[0], vp->layout.size[1]);
    printf("      views:       %ld\n", vp->layout.nviews);

    for (i = 0; i < vp->layout.nviews; ++i) {
        view = &vp->layout.views[i];

        printf("        view %d:\n", i);
        printf("          type:         %d\n", view->type);
        printf("          axis_lock:    %d\n", view->axis_lock);
        printf("          position:     (%d,%d)\n", view->position[0], view->position[1]);
        printf("          size:         (%d,%d)\n", view->size[0], view->size[1]);
        printf("          zoom:         %g\n", view->zoom);
        printf("          center:       (%g,%g,%g)\n", view->center[0], view->center[1], view->center[2]);
        printf("          horiz_angle:  %g\n", view->horiz_angle);
        printf("          vert_angle:   %g\n", view->vert_angle);
        printf("          camera:       %s\n", view->camera);
    }

    printf("    default_view:\n");
    printf(" type:         %d\n", vp->default_view.type);
    printf(" position:     (%g,%g,%g)\n", vp->default_view.position[0], vp->default_view.position[1], vp->default_view.position[2]);
    printf(" width:        %g\n", vp->default_view.width);
    printf(" horiz_angle:  %g\n", vp->default_view.horiz_angle);
    printf(" vert_angle:   %g\n", vp->default_view.vert_angle);
    printf(" roll_angle:   %g\n", vp->default_view.roll_angle);
    printf(" camera:       %s\n", vp->default_view.camera);
}


static void
texture_dump(const char *maptype, Lib3dsTextureMap *texture) {
    assert(texture);
    if (strlen(texture->name) == 0) {
        return;
    }
    printf("  %s:\n", maptype);
    printf("    name:        %s\n", texture->name);
    printf("    flags:       %X\n", (unsigned)texture->flags);
    printf("    percent:     %f\n", texture->percent);
    printf("    blur:        %f\n", texture->blur);
    printf("    scale:       (%f, %f)\n", texture->scale[0], texture->scale[1]);
    printf("    offset:      (%f, %f)\n", texture->offset[0], texture->offset[1]);
    printf("    rotation:    %f\n", texture->rotation);
    printf("    tint_1:      (%f, %f, %f)\n",
        texture->tint_1[0], texture->tint_1[1], texture->tint_1[2]);
    printf("    tint_2:      (%f, %f, %f)\n",
        texture->tint_2[0], texture->tint_2[1], texture->tint_2[2]);
    printf("    tint_r:      (%f, %f, %f)\n",
        texture->tint_r[0], texture->tint_r[1], texture->tint_r[2]);
    printf("    tint_g:      (%f, %f, %f)\n",
        texture->tint_g[0], texture->tint_g[1], texture->tint_g[2]);
    printf("    tint_b:      (%f, %f, %f)\n",
        texture->tint_b[0], texture->tint_b[1], texture->tint_b[2]);
}


static void
lib3ds_material_dump(Lib3dsMaterial *material) {
    assert(material);
    printf("  name:          %s\n", material->name);
    printf("  ambient:       (%f, %f, %f)\n",
        material->ambient[0], material->ambient[1], material->ambient[2]);
    printf("  diffuse:       (%f, %f, %f)\n",
        material->diffuse[0], material->diffuse[1], material->diffuse[2]);
    printf("  specular:      (%f, %f, %f)\n",
        material->specular[0], material->specular[1], material->specular[2]);
    printf("  shininess:     %f\n", material->shininess);
    printf("  shin_strength: %f\n", material->shin_strength);
    printf("  use_blur:      %s\n", material->use_blur ? "yes" : "no");
    printf("  blur:          %f\n", material->blur);
    printf("  falloff:       %f\n", material->falloff);
    printf("  additive:      %s\n", material->additive ? "yes" : "no");
    printf("  use_falloff:   %s\n", material->use_falloff ? "yes" : "no");
    printf("  self_illum:    %s\n", material->self_illum ? "yes" : "no");
    printf("  self_ilpct:    %f\n", material->self_ilpct);
    printf("  shading:       %d\n", material->shading);
    printf("  soften:        %s\n", material->soften ? "yes" : "no");
    printf("  face_map:      %s\n", material->face_map ? "yes" : "no");
    printf("  two_sided:     %s\n", material->two_sided ? "yes" : "no");
    printf("  map_decal:     %s\n", material->map_decal ? "yes" : "no");
    printf("  use_wire:      %s\n", material->use_wire ? "yes" : "no");
    printf("  use_wire_abs:  %s\n", material->use_wire_abs ? "yes" : "no");
    printf("  wire_size:     %f\n", material->wire_size);
    texture_dump("texture1_map", &material->texture1_map);
    texture_dump("texture1_mask", &material->texture1_mask);
    texture_dump("texture2_map", &material->texture2_map);
    texture_dump("texture2_mask", &material->texture2_mask);
    texture_dump("opacity_map", &material->opacity_map);
    texture_dump("opacity_mask", &material->opacity_mask);
    texture_dump("bump_map", &material->bump_map);
    texture_dump("bump_mask", &material->bump_mask);
    texture_dump("specular_map", &material->specular_map);
    texture_dump("specular_mask", &material->specular_mask);
    texture_dump("shininess_map", &material->shininess_map);
    texture_dump("shininess_mask", &material->shininess_mask);
    texture_dump("self_illum_map", &material->self_illum_map);
    texture_dump("self_illum_mask", &material->self_illum_mask);
    texture_dump("reflection_map", &material->reflection_map);
    texture_dump("reflection_mask", &material->reflection_mask);
    printf("  autorefl_map:\n");
    printf("    flags        %X\n", (unsigned)material->autorefl_map.flags);
    printf("    level        %d\n", (int)material->autorefl_map.level);
    printf("    size         %d\n", (int)material->autorefl_map.size);
    printf("    frame_step   %d\n", (int)material->autorefl_map.frame_step);
    printf("\n");
}


static void
camera_dump(Lib3dsCamera *camera) {
    assert(camera);
    printf("  name:       %s\n", camera->name);
    printf("  position:   (%f, %f, %f)\n",
        camera->position[0], camera->position[1], camera->position[2]);
    printf("  target      (%f, %f, %f)\n",
        camera->target[0], camera->target[1], camera->target[2]);
    printf("  roll:       %f\n", camera->roll);
    printf("  fov:        %f\n", camera->fov);
    printf("  see_cone:   %s\n", camera->see_cone ? "yes" : "no");
    printf("  near_range: %f\n", camera->near_range);
    printf("  far_range:  %f\n", camera->far_range);
    printf("\n");
}


static void
light_dump(Lib3dsLight *light) {
    assert(light);
    printf("  name:             %s\n", light->name);
    printf("  spot_light:       %s\n", light->spot_light ? "yes" : "no");
    printf("  see_cone:         %s\n", light->see_cone ? "yes" : "no");
    printf("  color:            (%f, %f, %f)\n",
        light->color[0], light->color[1], light->color[2]);
    printf("  position          (%f, %f, %f)\n",
        light->position[0], light->position[1], light->position[2]);
    printf("  spot              (%f, %f, %f)\n",
        light->spot[0], light->spot[1], light->spot[2]);
    printf("  roll:             %f\n", light->roll);
    printf("  off:              %s\n", light->off ? "yes" : "no");
    printf("  outer_range:      %f\n", light->outer_range);
    printf("  inner_range:      %f\n", light->inner_range);
    printf("  multiplier:       %f\n", light->multiplier);
    printf("  attenuation:      %f\n", light->attenuation);
    printf("  rectangular_spot: %s\n", light->rectangular_spot ? "yes" : "no");
    printf("  shadowed:         %s\n", light->shadowed ? "yes" : "no");
    printf("  shadow_bias:      %f\n", light->shadow_bias);
    printf("  shadow_filter:    %f\n", light->shadow_filter);
    printf("  shadow_size:      %d\n", light->shadow_size);
    printf("  spot_aspect:      %f\n", light->spot_aspect);
    printf("  use_projector:    %s\n", light->use_projector ? "yes" : "no");
    printf("  projector:        %s\n", light->projector);
    printf("  spot_overshoot:   %d\n", (int)light->spot_overshoot);
    printf("  ray_shadows:      %s\n", light->ray_shadows ? "yes" : "no");
    printf("  ray_bias:         %f\n", light->ray_bias);
    printf("  hot_spot:         %f\n", light->hot_spot);
    printf("  fall_off:         %f\n", light->fall_off);
    printf("\n");
}


static void
mesh_dump(Lib3dsMesh *mesh) {
    unsigned i;
    Lib3dsVector p;

    assert(mesh);
    printf("  %s vertices=%ld faces=%ld\n",
        mesh->name,
        mesh->nvertices,
        mesh->nfaces
        );
    printf("  matrix:\n");
    matrix_dump(mesh->matrix);
    printf("  point list:\n");
    for (i = 0; i < mesh->nvertices; ++i) {
        lib3ds_vector_copy(p, mesh->vertices[i]);
        printf("    %8f %8f %8f\n", p[0], p[1], p[2]);
    }
    printf("  facelist:\n");
    for (i = 0; i < mesh->nfaces; ++i) {
        printf("    %4d %4d %4d  smoothing:%X  flags:%X  material:\"%d\"\n",
            mesh->faces[i].index[0],
            mesh->faces[i].index[1],
            mesh->faces[i].index[2],
            (unsigned) mesh->faces[i].smoothing,
            mesh->faces[i].flags,
            mesh->faces[i].material
            );
    }
}


static void
dump_instances(Lib3dsNode *node, const char* parent) {
    Lib3dsNode *p;
    char name[255];

    strcpy(name, parent);
    strcat(name, ".");
    strcat(name, node->name);
    if (node->type == LIB3DS_OBJECT_NODE) {
        printf("  %s : %s\n", name, node->data.object.instance);
    }
    for (p = node->childs; p != 0; p = p->next) {
        dump_instances(p, parent);
    }
}


static const char* node_names_table[] = {
    "***Unknown***",
    "Ambient",
    "Object",
    "Camera",
    "Target",
    "Light",
    "Spot"
};


static void
node_dump(Lib3dsNode *node, Lib3dsIntd level) {
    Lib3dsNode *p;
    char l[128];

    assert(node);
    memset(l, ' ', 2*level);
    l[2*level] = 0;

    if (node->type == LIB3DS_OBJECT_NODE) {
        printf("%s%s [%s] (%s)\n",
            l,
            node->name,
            node->data.object.instance,
            node_names_table[node->type]
        );
    } else {
        printf("%s%s (%s)\n",
            l,
            node->name,
            node_names_table[node->type]
        );
    }

    for (p = node->childs; p != 0; p = p->next) {
        node_dump(p, level + 1);
    }
}


int
main(int argc, char **argv) {
    FILE *file;
    Lib3dsFile *f = 0;
    Lib3dsIo *io;
    Lib3dsBool result;
    int i;

    parse_args(argc, argv);

    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "***ERROR***\nFile not found: %s\n", filename);
        exit(1);
    }

    f = lib3ds_file_new();
    io = lib3ds_io_new(
        file,
        fileio_seek_func,
        fileio_tell_func,
        fileio_read_func,
        fileio_write_func,
        fileio_log_func
        );

    result =  lib3ds_file_read(f, io);

    lib3ds_io_free(io);
    fclose(file);
 
    if (!result)
    {
        fprintf(stderr, "***ERROR***\nLoading file failed: %s\n", filename);
        exit(1);
    }

    if (flags & LIB3DSDUMP_MATERIALS) {
        printf("Dumping materials:\n");
        //lib3ds_file_dump_materials(f);
        printf("\n");
    }
    if (flags & LIB3DSDUMP_TRIMESHES) {
        printf("Dumping meshes:\n");
        for (i = 0; i < f->nmeshes; ++i) mesh_dump(f->meshes[i]);
        printf("\n");
    }
    if (flags & LIB3DSDUMP_INSTANCES) {
        Lib3dsNode *p;
        printf("Dumping instances:\n");
        for (p = f->nodes; p != 0; p = p->next) {
            dump_instances(p, "");
        }
        printf("\n");
    }
    if (flags & LIB3DSDUMP_CAMERAS) {
        printf("Dumping cameras:\n");
        for (i = 0; i < f->ncameras; ++i) camera_dump(f->cameras[i]);
        printf("\n");
    }
    if (flags & LIB3DSDUMP_LIGHTS) {
        printf("Dumping lights:\n");
        for (i = 0; i < f->nlights; ++i) light_dump(f->lights[i]);
        printf("\n");
    }
    if (flags & LIB3DSDUMP_NODES) {
        Lib3dsNode *p;
        printf("Dumping node hierarchy:\n");
        for (p = f->nodes; p != 0; p = p->next) {
            node_dump(p, 1);
        }
        printf("\n");
    }
    if (output) {
        if (!lib3ds_file_save(f, output)) {
            printf("***ERROR**** Writing %s\n", output);
        }
    }

    lib3ds_file_free(f);
    
    {
        char buf[255];
        gets(buf);
    }
    return 0;
}
