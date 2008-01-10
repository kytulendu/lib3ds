/* -*- c -*- */
#ifndef INCLUDED_LIB3DS_H
#define INCLUDED_LIB3DS_H
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
 * $Id: types.h,v 1.25 2007/06/21 08:36:41 jeh Exp $
 */

#include <stddef.h>
#ifndef _MSC_VER
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#ifdef LIB3DS_EXPORTS
#define LIB3DSAPI __declspec(dllexport)
#else               
#define LIB3DSAPI __declspec(dllimport)
#endif           
#else
#define LIB3DSAPI
#endif

#ifdef _MSC_VER
typedef __int32 Lib3dsBool;
typedef unsigned __int8 Lib3dsByte;
typedef unsigned __int16 Lib3dsWord;
typedef unsigned __int32 Lib3dsDword;
typedef signed __int8 Lib3dsIntb;
typedef signed __int16 Lib3dsIntw;
typedef signed __int32 Lib3dsIntd;
#else
typedef int32_t Lib3dsBool;
typedef uint8_t Lib3dsByte;
typedef uint16_t Lib3dsWord;
typedef uint32_t Lib3dsDword;
typedef int8_t Lib3dsIntb;
typedef int16_t Lib3dsIntw;
typedef int32_t Lib3dsIntd;
#endif

typedef enum Lib3dsIoSeek {
    LIB3DS_SEEK_SET     = 0,
    LIB3DS_SEEK_CUR     = 1,
    LIB3DS_SEEK_END     = 2
} Lib3dsIoSeek;

typedef enum Lib3dsLogLevel {
    LIB3DS_LOG_ERROR    = 0,
    LIB3DS_LOG_WARN     = 1,
    LIB3DS_LOG_INFO     = 2,
    LIB3DS_LOG_DEBUG    = 3
} Lib3dsLogLevel;

typedef struct Lib3dsIo Lib3dsIo;
typedef Lib3dsBool (*Lib3dsIoErrorFunc)(void *self);
typedef long (*Lib3dsIoSeekFunc)(void *self, long offset, Lib3dsIoSeek origin);
typedef long (*Lib3dsIoTellFunc)(void *self);
typedef size_t (*Lib3dsIoReadFunc)(void *self, void *buffer, size_t size);
typedef size_t (*Lib3dsIoWriteFunc)(void *self, const void *buffer, size_t size);
typedef void (*Lib3dsIoLogFunc)(Lib3dsIo *io, Lib3dsLogLevel level, char *msg);

/* Atmosphere settings */
typedef struct Lib3dsAtmosphere {
	Lib3dsBool  use_fog;
	float       fog_color[3];
	Lib3dsBool  fog_background;
	float       fog_near_plane;
	float       fog_near_density;
	float       fog_far_plane;
	float       fog_far_density;
	Lib3dsBool  use_layer_fog;
	Lib3dsDword layer_fog_flags;
	float       layer_fog_color[3];
	float       layer_fog_near_y;
	float       layer_fog_far_y;
	float       layer_fog_density;
	Lib3dsBool  use_dist_cue;
	Lib3dsBool  dist_cue_background;
	float       dist_cue_near_plane;
	float       dist_cue_near_dimming;
	float       dist_cue_far_plane;
	float       dist_cue_far_dimming;
} Lib3dsAtmosphere;

/* Background settings */
typedef struct Lib3dsBackground {
    Lib3dsBool  use_bitmap;
	char        bitmap_name[64];
    Lib3dsBool  use_solid;
    float       solid_color[3];
	Lib3dsBool  use_gradient;
	float       gradient_percent;
	float       gradient_top[3];
	float       gradient_middle[3];
	float       gradient_bottom[3];
} Lib3dsBackground;

/* Shadow settings */
typedef struct Lib3dsShadow {
    short   map_size;           /* Global shadow map size that ranges from 10 to 4096 */
    float   low_bias;           /* Global shadow low bias */
    float   hi_bias;            /* Global shadow hi bias */
    float   filter;             /* Global shadow filter that ranges from 1 (lowest) to 10 (highest) */
    float   ray_bias;           /* Global raytraced shadow bias */
} Lib3dsShadow;

/* Layout view types */
typedef enum Lib3dsViewType {
    LIB3DS_VIEW_TYPE_NOT_USED   = 0,
    LIB3DS_VIEW_TYPE_TOP        = 1,
    LIB3DS_VIEW_TYPE_BOTTOM     = 2,
    LIB3DS_VIEW_TYPE_LEFT       = 3,
    LIB3DS_VIEW_TYPE_RIGHT      = 4,
    LIB3DS_VIEW_TYPE_FRONT      = 5,
    LIB3DS_VIEW_TYPE_BACK       = 6,
    LIB3DS_VIEW_TYPE_USER       = 7,
    LIB3DS_VIEW_TYPE_SPOTLIGHT  = 18,
    LIB3DS_VIEW_TYPE_CAMERA     = 65535
} Lib3dsViewType;

/* Layout styles */
typedef enum Lib3dsLayoutStyle {
    LIB3DS_LAYOUT_SINGLE                    = 0,
    LIB3DS_LAYOUT_TWO_PANE_VERT_SPLIT       = 1,
    LIB3DS_LAYOUT_TWO_PANE_HORIZ_SPLIT      = 2,
    LIB3DS_LAYOUT_FOUR_PANE                 = 3,
    LIB3DS_LAYOUT_THREE_PANE_LEFT_SPLIT     = 4,
    LIB3DS_LAYOUT_THREE_PANE_BOTTOM_SPLIT   = 5,
    LIB3DS_LAYOUT_THREE_PANE_RIGHT_SPLIT    = 6,
    LIB3DS_LAYOUT_THREE_PANE_TOP_SPLIT      = 7,
    LIB3DS_LAYOUT_THREE_PANE_VERT_SPLIT     = 8,
    LIB3DS_LAYOUT_THREE_PANE_HORIZ_SPLIT    = 9,
    LIB3DS_LAYOUT_FOUR_PANE_LEFT_SPLIT      = 10,
    LIB3DS_LAYOUT_FOUR_PANE_RIGHT_SPLIT     = 11
} Lib3dsLayoutStyle;

/* Layout view settings */
typedef struct Lib3dsView {
    Lib3dsWord      type;
    Lib3dsWord      axis_lock;
    Lib3dsIntw      position[2];
    Lib3dsIntw      size[2];
    float           zoom;
    float           center[3];
    float           horiz_angle;
    float           vert_angle;
    char            camera[11];
} Lib3dsView;

#define LIB3DS_LAYOUT_MAX_VIEWS 32

/* Viewport and default view settings */
typedef struct Lib3dsViewport {
	Lib3dsWord      layout_style;
	Lib3dsIntw      layout_active;
	Lib3dsIntw      layout_swap;
	Lib3dsIntw      layout_swap_prior;
	Lib3dsIntw      layout_swap_view;
	Lib3dsWord      layout_position[2];
	Lib3dsWord      layout_size[2];
	Lib3dsDword     layout_nviews;
	Lib3dsView      layout_views[LIB3DS_LAYOUT_MAX_VIEWS];
	Lib3dsWord      default_type;
	float           default_position[3];
	float           default_width;
	float           default_horiz_angle;
	float           default_vert_angle;
	float           default_roll_angle;
	char            default_camera[64];
} Lib3dsViewport;

/* Material texture map flags */
typedef enum Lib3dsTextureMapFlags {
  LIB3DS_DECALE         = 0x0001,
  LIB3DS_MIRROR         = 0x0002,
  LIB3DS_NEGATE         = 0x0008,
  LIB3DS_NO_TILE        = 0x0010,
  LIB3DS_SUMMED_AREA    = 0x0020,
  LIB3DS_ALPHA_SOURCE   = 0x0040,
  LIB3DS_TINT           = 0x0080,
  LIB3DS_IGNORE_ALPHA   = 0x0100,
  LIB3DS_RGB_TINT       = 0x0200
} Lib3dsTextureMapFlags;

/* Material texture map */
typedef struct Lib3dsTextureMap {
    int             user_id;
    void*           user_ptr;
    char            name[64];
    Lib3dsDword     flags;
    float           percent;
    float           blur;
    float           scale[2];
    float           offset[2];
    float           rotation;
    float           tint_1[3];
    float           tint_2[3];
    float           tint_r[3];
    float           tint_g[3];
    float           tint_b[3];
} Lib3dsTextureMap;

/* Auto reflection texture map flags */
typedef enum Lib3dsAutoReflMapFlags {
    LIB3DS_USE_REFL_MAP             = 0x0001,
    LIB3DS_READ_FIRST_FRAME_ONLY    = 0x0004,
    LIB3DS_FLAT_MIRROR              = 0x0008 
} Lib3dsAutoReflMapFlags;

/* Auto reflection texture map anti-aliasing level */
typedef enum Lib3dsAutoReflMapAntiAlias {
    LIB3DS_ANTI_ALIAS_NONE      = 0,
    LIB3DS_ANTI_ALIAS_LOW       = 1,
    LIB3DS_ANTI_ALIAS_MEDIUM    = 2,
    LIB3DS_ANTI_ALIAS_HIGH      = 3
} Lib3dsAutoReflMapAntiAlias;

/* Auto reflection map settings */
typedef struct Lib3dsAutoReflMap {
    Lib3dsDword flags;
    Lib3dsIntd  anti_alias;
    Lib3dsIntd  size;
    Lib3dsIntd  frame_step;
} Lib3dsAutoReflMap;

/* Material shading type */
typedef enum Lib3dsShading {
	LIB3DS_WIRE_FRAME = 0,
	LIB3DS_FLAT       = 1, 
	LIB3DS_GOURAUD    = 2, 
	LIB3DS_PHONG      = 3, 
	LIB3DS_METAL      = 4
} Lib3dsShading; 

/** Material */
typedef struct Lib3dsMaterial {
    unsigned            user_type;          /* == 'MAT' */
    int                 user_id;
    void*               user_ptr;
    char                name[64];			/* Material name */
    float               ambient[3];			/* Material ambient reflectivity */
    float               diffuse[3];			/* Material diffuse reflectivity */
    float               specular[3];		/* Material specular reflectivity */
    float               shininess;		    /* Material specular exponent */
    float               shin_strength;
    Lib3dsBool          use_blur;
    float               blur;
    float               transparency;
    float               falloff;
    Lib3dsBool          additive;
    float               self_ilpct;
    Lib3dsBool          use_falloff;
    Lib3dsBool          self_illum;
    Lib3dsIntw          shading;
    Lib3dsBool          soften;
    Lib3dsBool          face_map;
    Lib3dsBool          two_sided;		/* Material visible from back */
    Lib3dsBool          map_decal;
    Lib3dsBool          use_wire;
    Lib3dsBool          use_wire_abs;
    float               wire_size;
    Lib3dsTextureMap    texture1_map;
    Lib3dsTextureMap    texture1_mask;
    Lib3dsTextureMap    texture2_map;
    Lib3dsTextureMap    texture2_mask;
    Lib3dsTextureMap    opacity_map;
    Lib3dsTextureMap    opacity_mask;
    Lib3dsTextureMap    bump_map;
    Lib3dsTextureMap    bump_mask;
    Lib3dsTextureMap    specular_map;
    Lib3dsTextureMap    specular_mask;
    Lib3dsTextureMap    shininess_map;
    Lib3dsTextureMap    shininess_mask;
    Lib3dsTextureMap    self_illum_map;
    Lib3dsTextureMap    self_illum_mask;
    Lib3dsTextureMap    reflection_map;
    Lib3dsTextureMap    reflection_mask;
    Lib3dsAutoReflMap   autorefl_map;
} Lib3dsMaterial;

/** Object flags for cameras, lights and meshes */
typedef enum Lib3dsObjectFlags {
    LIB3DS_OBJECT_HIDDEN          = 0x01, 
    LIB3DS_OBJECT_VIS_LOFTER      = 0x02, 
    LIB3DS_OBJECT_DOESNT_CAST     = 0x04, 
    LIB3DS_OBJECT_MATTE           = 0x08, 
    LIB3DS_OBJECT_DONT_RCVSHADOW  = 0x10, 
    LIB3DS_OBJECT_FAST            = 0x20, 
    LIB3DS_OBJECT_FROZEN          = 0x40 
} Lib3dsObjectFlags;

/** Camera object */
typedef struct Lib3dsCamera {
    unsigned        user_type;          /* == 'CAM' */
    int             user_id;
    void*           user_ptr;
    char            name[64];
    Lib3dsDword     object_flags; /*< @see Lib3dsObjectFlags */ 
    float           position[3];
    float           target[3];
    float           roll;
    float           fov;
    Lib3dsBool      see_cone;
    float           near_range;
    float           far_range;
} Lib3dsCamera; 

/** Light object */
typedef struct Lib3dsLight {
    unsigned        user_type;          /* == 'LGHT' */
    int             user_id;
    void*           user_ptr;
    char            name[64];
    Lib3dsDword     object_flags; /*< @see Lib3dsObjectFlags */ 
    Lib3dsBool      spot_light;
    Lib3dsBool      see_cone;
    float           color[3];
    float           position[3];
    float           spot[3];
    float           roll;
    Lib3dsBool      off;
    float           outer_range;
    float           inner_range;
    float           multiplier;
    /*const char**  excludes;*/
    float           attenuation;
    Lib3dsBool      rectangular_spot;
    Lib3dsBool      shadowed;
    float           shadow_bias;
    float           shadow_filter;
    Lib3dsIntw      shadow_size;
    float           spot_aspect;
    Lib3dsBool      use_projector;
    char            projector[64];
    Lib3dsIntd      spot_overshoot;
    Lib3dsBool      ray_shadows;
    float           ray_bias;
    float           hot_spot;
    float           fall_off;
} Lib3dsLight; 

/* Texture map projection */
typedef enum {
  LIB3DS_MAP_NONE           = 0xFFFF,
  LIB3DS_MAP_PLANAR         = 0,
  LIB3DS_MAP_CYLINDRICAL    = 1,
  LIB3DS_MAP_SPHERICAL      = 2
} Lib3dsMapType;

/*
 * Meaning of _Lib3dsFace::flags. ABC are points of the current face 
 * (A: is 1st vertex, B is 2nd vertex, C is 3rd vertex) 
 */
typedef enum Lib3dsFaceFlags {
  LIB3DS_FACE_VIS_AC    = 0x01,       /* Bit 0: Edge visibility AC */
  LIB3DS_FACE_VIS_BC    = 0x02,       /* Bit 1: Edge visibility BC */
  LIB3DS_FACE_VIS_AB    = 0x04,       /* Bit 2: Edge visibility AB */
  LIB3DS_FACE_WRAP_U    = 0x08,       /* Bit 3: Face is at tex U wrap seam */
  LIB3DS_FACE_WRAP_V    = 0x10,       /* Bit 4: Face is at tex V wrap seam */
  LIB3DS_FACE_SELECT_3  = (1<<13),    /* Bit 13: Selection of the face in selection 3*/
  LIB3DS_FACE_SELECT_2  = (1<<14),    /* Bit 14: Selection of the face in selection 2*/
  LIB3DS_FACE_SELECT_1  = (1<<15),    /* Bit 15: Selection of the face in selection 1*/
} Lib3dsFaceFlags;

typedef struct Lib3dsVertex {
    float       tex[2];
    float       pos[3];
    Lib3dsDword flags;
} Lib3dsVertex; 

typedef struct Lib3dsFace {
    Lib3dsWord  index[3];
    Lib3dsWord  flags;
    Lib3dsIntd  material;
    Lib3dsDword smoothing_group;
} Lib3dsFace; 

/* Triangular mesh object */
typedef struct Lib3dsMesh {
    unsigned        user_type;          /* == 'MESH' */
    int             user_id;
    void*           user_ptr;
    char            name[64];		     /* Mesh name. Don't use more than 8 characters  */
    Lib3dsDword     object_flags;        /* @see Lib3dsObjectFlags */ 
    Lib3dsByte      color;
    float           matrix[4][4];        /* Transformation matrix for mesh data */
    int             nvertices;		     /* Number of vertices in vertex array (max. 65535) */
    Lib3dsVertex*   vertices;	         /* Point list */
    int             nfaces;	             /* Number of faces in face array (max. 65535) */
	Lib3dsFace*     faces;               /* Array */
	char            box_front[64];
	char            box_back[64];
	char            box_left[64];
	char            box_right[64];
	char            box_top[64];
	char            box_bottom[64];
	Lib3dsWord      map_projection;
	float           map_pos[3];
	float           map_matrix[4][4];
	float           map_scale;
	float           map_tile[2];
	float           map_planar_size[2];
	float           map_cylinder_height;
} Lib3dsMesh; 

typedef enum Lib3dsNodeType {
    LIB3DS_AMBIENT_NODE = 0,
    LIB3DS_OBJECT_NODE  = 1,
    LIB3DS_CAMERA_NODE  = 2,
    LIB3DS_TARGET_NODE  = 3,
    LIB3DS_LIGHT_NODE   = 4,
    LIB3DS_SPOT_NODE    = 5
} Lib3dsNodeType;

#define LIB3DS_NO_PARENT 65535

/*!
 * Node flags #1
 */
typedef enum {
    LIB3DS_HIDDEN   = 0x800
} Lib3dsNodeFlags1;

/*!
 * Node flags #2
 */
typedef enum {
    LIB3DS_SHOW_PATH        = 0x1,
    LIB3DS_SMOOTHING        = 0x2,
    LIB3DS_MOTION_BLUR      = 0x10,
    LIB3DS_MORPH_MATERIALS  = 0x40
} Lib3dsNodeFlags2;

/*!
 * Scene graph node
 */
typedef struct Lib3dsNode {
    unsigned            user_type;          /* == 'MAT' */
    int                 user_id;
    void*               user_ptr;
    struct Lib3dsNode*  next;
    struct Lib3dsNode*  childs;
    struct Lib3dsNode*  parent;
    Lib3dsNodeType      type;
    Lib3dsWord          node_id;
    char                name[64];
    Lib3dsWord          flags1;
    Lib3dsWord          flags2;
    Lib3dsWord          parent_id;
    float               matrix[4][4];
} Lib3dsNode;

typedef enum Lib3dsTcbFlags{
    LIB3DS_USE_TENSION      = 0x0001,
    LIB3DS_USE_CONTINUITY   = 0x0002,
    LIB3DS_USE_BIAS         = 0x0004,
    LIB3DS_USE_EASE_TO      = 0x0008,
    LIB3DS_USE_EASE_FROM    = 0x0010
} Lib3dsTcbFlags;

typedef enum {
    LIB3DS_REPEAT   = 0x0001,
    LIB3DS_SMOOTH   = 0x0002,
    LIB3DS_LOCK_X   = 0x0008,
    LIB3DS_LOCK_Y   = 0x0010,
    LIB3DS_LOCK_Z   = 0x0020,
    LIB3DS_UNLINK_X = 0x0100,
    LIB3DS_UNLINK_Y = 0x0200,
    LIB3DS_UNLINK_Z = 0x0400
} Lib3dsTrackFlags;

typedef enum Lib3dsTrackType {
    LIB3DS_TRACK_BOOL   = 0,
    LIB3DS_TRACK_FLOAT  = 2,
    LIB3DS_TRACK_VECTOR = 3,
    LIB3DS_TRACK_QUAT   = 4
} Lib3dsTrackType;

typedef struct Lib3dsKey {
    Lib3dsIntd  frame;
    Lib3dsWord  flags;
    float       tens;
    float       cont;
    float       bias;
    float       ease_to;
    float       ease_from;
    float       value[4];
} Lib3dsKey;

typedef struct Lib3dsTrack {
    unsigned        user_type;          /* == 'TRCK' */
    int             user_id;
    void*           user_ptr;
    Lib3dsNode*     node;
    Lib3dsDword     flags;
    Lib3dsTrackType type; 
    int             nkeys;
    Lib3dsKey*      keys;   
} Lib3dsTrack;

typedef struct Lib3dsAmbientNode {
    Lib3dsNode      base;
    float           color[3];
    Lib3dsTrack*    color_track;
} Lib3dsAmbientNode;

typedef struct Lib3dsObjectNode {
    Lib3dsNode      base;
    float           pivot[3];
    char            instance[64];
    float           bbox_min[3];
    float           bbox_max[3];
    Lib3dsBool      hide;
    float           pos[3];
    float           rot[4];
    float           scl[3];
    float           morph_smooth;
    char            morph[64];
    Lib3dsTrack*    pos_track;
    Lib3dsTrack*    rot_track;
    Lib3dsTrack*    scl_track;
    Lib3dsTrack*    morph_track;
    Lib3dsTrack*    hide_track;
} Lib3dsObjectNode;

typedef struct Lib3dsCameraNode {
    Lib3dsNode      base;
    float           pos[3];
    float           fov;
    float           roll;
    Lib3dsTrack*    pos_track;
    Lib3dsTrack*    fov_track;
    Lib3dsTrack*    roll_track;
} Lib3dsCameraNode;

typedef struct Lib3dsTargetNode {
    Lib3dsNode      base;
    float           pos[3];
    Lib3dsTrack*    pos_track;
} Lib3dsTargetNode;

typedef struct Lib3dsLightNode {
    Lib3dsNode      base;
    float           pos[3];
    float           color[3];
    float           hotspot;
    float           falloff;
    float           roll;
    Lib3dsTrack*    pos_track;
    Lib3dsTrack*    color_track;
    Lib3dsTrack*    hotspot_track;
    Lib3dsTrack*    falloff_track;
    Lib3dsTrack*    roll_track;
} Lib3dsLightNode;

typedef struct Lib3dsSpotNode {
    Lib3dsNode      base;
    float           pos[3];
    Lib3dsTrack*    pos_track;
} Lib3dsSpotNode;

/*!
    3DS file structure 
 */
typedef struct Lib3dsFile {
    unsigned            user_type;          /* == 'FILE' */
    int                 user_id;
    void*               user_ptr;
    Lib3dsDword         mesh_version;
    Lib3dsWord          keyf_revision;
    char                name[12+1];
    float               master_scale;
    float               construction_plane[3];
    float               ambient[3];
    Lib3dsShadow        shadow;
    Lib3dsBackground    background;
    Lib3dsAtmosphere    atmosphere;
    Lib3dsViewport      viewport;
    Lib3dsViewport      viewport_keyf;
    Lib3dsIntd          frames;
    Lib3dsIntd          segment_from;
    Lib3dsIntd          segment_to;
    Lib3dsIntd          current_frame;
    Lib3dsIntd          materials_size;
    Lib3dsIntd          nmaterials;         
    Lib3dsMaterial**    materials;        
    Lib3dsIntd          cameras_size;  
    Lib3dsIntd          ncameras;                      
    Lib3dsCamera**      cameras;
    Lib3dsIntd          lights_size;
    Lib3dsIntd          nlights;                      
    Lib3dsLight**       lights;
    Lib3dsIntd          meshes_size;
    Lib3dsIntd          nmeshes;                      
    Lib3dsMesh**        meshes;                         
    Lib3dsNode*         nodes;
} Lib3dsFile; 

/** 
    Calculates the ease in/out function.
    @param fp
        [in] Previous frame number.
    @param fc
        [in] Current frame number.
    @param fn
        [in] Next frame number.
    @param ease_from
        [in] Ease in value [0, 1.0]
    @param ease_to
        [in] Ease out value [0, 1.0]
    @return
        The calculated value of the function.
 */
extern LIB3DSAPI float lib3ds_float_ease(
    float fp, 
    float fc, 
    float fn, 
    float ease_from, 
    float ease_to
);

/*! 
    @param a
        [in] First point.
    @param p
        [in] Tangent of first point.
    @param q
        [in] Tangent of second point.
    @param b
        [in] Second point.
    @param t
        [in] 
 */
extern LIB3DSAPI float lib3ds_float_cubic(
    float a, 
    float p, 
    float q, 
    float b, 
    float t);

extern LIB3DSAPI void lib3ds_vector_make(
    float c[3],
    float x,
    float y,
    float z);

/*! 
    Sets all components of a vector to zero.
    @param c
         [in/out] The pointer to the vector. 
 */
extern LIB3DSAPI void lib3ds_vector_zero(
    float c[3] );

/** Copies all components of a vector to another vector. */
extern LIB3DSAPI void lib3ds_vector_copy( 
    float dest[3],  /**< [out] The destination vector.*/
    float src[3]    /**< [in] The source vector. */
);

/*! 
    Negates all components of a vector.
    @param c
        [in/out] The pointer to the vector. 
 */
extern LIB3DSAPI void lib3ds_vector_neg(
    float c[3]);

extern LIB3DSAPI void lib3ds_vector_add(float c[3], float a[3], float b[3]);
extern LIB3DSAPI void lib3ds_vector_sub(float c[3], float a[3], float b[3]);
extern LIB3DSAPI void lib3ds_vector_scalar(float c[3], float k);
extern LIB3DSAPI void lib3ds_vector_cross(float c[3], float a[3], float b[3]);
extern LIB3DSAPI float lib3ds_vector_dot(float a[3], float b[3]);
extern LIB3DSAPI float lib3ds_vector_length(float c[3]);
extern LIB3DSAPI void lib3ds_vector_normalize(float c[3]);
extern LIB3DSAPI void lib3ds_vector_normal(float n[3], float a[3], float b[3], float c[3]);
extern LIB3DSAPI void lib3ds_vector_cubic(float c[3], float a[3], float p[3], float q[3], float b[3], float t);
extern LIB3DSAPI void lib3ds_vector_min(float c[3], float a[3]);
extern LIB3DSAPI void lib3ds_vector_max(float c[3], float a[3]);
extern LIB3DSAPI void lib3ds_vector_transform(float c[3], float m[4][4], float a[3]);

/** @name Quaternions */
/** @{ */
extern LIB3DSAPI void lib3ds_quat_identity(float c[4]);
extern LIB3DSAPI void lib3ds_quat_copy(float dest[4], float src[4]);
extern LIB3DSAPI void lib3ds_quat_axis_angle(float c[4], float axis[3], float angle);
extern LIB3DSAPI void lib3ds_quat_neg(float c[4]);
extern LIB3DSAPI void lib3ds_quat_cnj(float c[4]);
extern LIB3DSAPI void lib3ds_quat_mul(float c[4], float a[4], float b[4]);
extern LIB3DSAPI void lib3ds_quat_scalar(float c[4], float k);
extern LIB3DSAPI void lib3ds_quat_normalize(float c[4]);
extern LIB3DSAPI void lib3ds_quat_inv(float c[4]);
extern LIB3DSAPI float lib3ds_quat_dot(float a[4], float b[4]);
extern LIB3DSAPI float lib3ds_quat_norm(float c[4]);
extern LIB3DSAPI void lib3ds_quat_ln(float c[4]);
extern LIB3DSAPI void lib3ds_quat_ln_dif(float c[4], float a[4], float b[4]);
extern LIB3DSAPI void lib3ds_quat_exp(float c[4]);
extern LIB3DSAPI void lib3ds_quat_slerp(float c[4], float a[4], float b[4], float t);
extern LIB3DSAPI void lib3ds_quat_squad(float c[4], float a[4], float p[4], float q[4], float b[4], float t);
extern LIB3DSAPI void lib3ds_quat_tangent(float c[4], float p[4], float q[4], float n[4]);
/** @} */

/** @name Matrices */
/** @{ */
extern LIB3DSAPI void lib3ds_matrix_zero(float m[4][4]);
extern LIB3DSAPI void lib3ds_matrix_identity(float  m[4][4]);
extern LIB3DSAPI void lib3ds_matrix_copy(float dest[4][4], float src[4][4]);
extern LIB3DSAPI void lib3ds_matrix_neg(float m[4][4]);
extern LIB3DSAPI void lib3ds_matrix_transpose(float m[4][4]);
extern LIB3DSAPI void lib3ds_matrix_add(float m[4][4], float a[4][4], float b[4][4]);
extern LIB3DSAPI void lib3ds_matrix_sub(float m[4][4], float a[4][4], float b[4][4]);
extern LIB3DSAPI void lib3ds_matrix_mult(float m[4][4], float a[4][4], float b[4][4]);
extern LIB3DSAPI void lib3ds_matrix_scalar(float m[4][4], float k);
extern LIB3DSAPI float lib3ds_matrix_det(float m[4][4]);
extern LIB3DSAPI Lib3dsBool lib3ds_matrix_inv(float m[4][4]);
extern LIB3DSAPI void lib3ds_matrix_translate(float m[4][4], float x, float y, float z);
extern LIB3DSAPI void lib3ds_matrix_scale(float m[4][4], float x, float y, float z);
extern LIB3DSAPI void lib3ds_matrix_rotate_quat(float m[4][4], float q[4]);
extern LIB3DSAPI void lib3ds_matrix_rotate(float m[4][4], float angle, float ax, float ay, float az);
extern LIB3DSAPI void lib3ds_matrix_camera(float m[4][4], float pos[3], float tgt[3], float roll);

/** @} */
/** @name io IO Handling */
/** @{ */

extern LIB3DSAPI Lib3dsIo* lib3ds_io_new(void *self, Lib3dsIoSeekFunc seek_func, Lib3dsIoTellFunc tell_func,
                                         Lib3dsIoReadFunc read_func, Lib3dsIoWriteFunc write_func,
                                         Lib3dsIoLogFunc log_func);
extern LIB3DSAPI void lib3ds_io_free(Lib3dsIo *io);

/** @} */
/** @name Objects */
/** @{ */


extern LIB3DSAPI Lib3dsMaterial* lib3ds_material_new();
extern LIB3DSAPI void lib3ds_material_free(Lib3dsMaterial *material);

extern LIB3DSAPI Lib3dsCamera* lib3ds_camera_new(const char *name);
extern LIB3DSAPI void lib3ds_camera_free(Lib3dsCamera *mesh);

extern LIB3DSAPI Lib3dsLight* lib3ds_light_new(const char *name);
extern LIB3DSAPI void lib3ds_light_free(Lib3dsLight *mesh);

extern LIB3DSAPI Lib3dsMesh* lib3ds_mesh_new(const char *name);
extern LIB3DSAPI void lib3ds_mesh_free(Lib3dsMesh *mesh);
extern LIB3DSAPI void lib3ds_mesh_resize_vertices(Lib3dsMesh *mesh, int nvertices);
extern LIB3DSAPI void lib3ds_mesh_resize_faces(Lib3dsMesh *mesh, int nfaces);
extern LIB3DSAPI void lib3ds_mesh_bounding_box(Lib3dsMesh *mesh, float bmin[3], float bmax[3]);
extern LIB3DSAPI void lib3ds_mesh_calculate_face_normals(Lib3dsMesh *mesh, float (*face_normals)[3]);
extern LIB3DSAPI void lib3ds_mesh_calculate_normals(Lib3dsMesh *mesh, float (*normalL)[3]);

/** @} */
/** @name Hierachy, Keyframing */
/** @{ */

extern LIB3DSAPI Lib3dsTrack* lib3ds_track_new(Lib3dsNode *node, Lib3dsTrackType type, Lib3dsDword nkeys);
extern LIB3DSAPI void lib3ds_track_free(Lib3dsTrack *track);
extern LIB3DSAPI void lib3ds_track_resize(Lib3dsTrack *track, int nkeys);
extern LIB3DSAPI void lib3ds_track_eval_bool(Lib3dsTrack *track, Lib3dsBool *b, float t);
extern LIB3DSAPI void lib3ds_track_eval_float(Lib3dsTrack *track, float *f, float t);
extern LIB3DSAPI void lib3ds_track_eval_vector(Lib3dsTrack *track, float v[3], float t);
extern LIB3DSAPI void lib3ds_track_eval_quat(Lib3dsTrack *track, float q[4], float t);

extern LIB3DSAPI Lib3dsNode* lib3ds_node_new(Lib3dsNodeType type);
extern LIB3DSAPI void lib3ds_node_free(Lib3dsNode *node);
extern LIB3DSAPI void lib3ds_node_eval(Lib3dsNode *node, float t);
extern LIB3DSAPI Lib3dsNode* lib3ds_node_by_name(Lib3dsNode *node, const char* name, Lib3dsNodeType type);
extern LIB3DSAPI Lib3dsNode* lib3ds_node_by_id(Lib3dsNode *node, Lib3dsWord node_id);

/** @} */
/** @name Files */
/** @{ */

extern LIB3DSAPI Lib3dsFile* lib3ds_file_load(
    const char *filename);

extern LIB3DSAPI Lib3dsBool lib3ds_file_save(Lib3dsFile *file, const char *filename);
extern LIB3DSAPI Lib3dsFile* lib3ds_file_new();
extern LIB3DSAPI void lib3ds_file_free(Lib3dsFile *file);
extern LIB3DSAPI void lib3ds_file_eval(Lib3dsFile *file, float t);
extern LIB3DSAPI Lib3dsBool lib3ds_file_read(Lib3dsFile *file, Lib3dsIo *io);
extern LIB3DSAPI Lib3dsBool lib3ds_file_write(Lib3dsFile *file, Lib3dsIo *io);
extern LIB3DSAPI void lib3ds_file_material_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force);
extern LIB3DSAPI void lib3ds_file_material_insert(Lib3dsFile *file, Lib3dsMaterial *material, Lib3dsIntd index);
extern LIB3DSAPI void lib3ds_file_material_remove(Lib3dsFile *file, Lib3dsIntd index);
extern LIB3DSAPI Lib3dsIntd lib3ds_file_material_by_name(Lib3dsFile *file, const char *name);
extern LIB3DSAPI void lib3ds_file_camera_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force);
extern LIB3DSAPI void lib3ds_file_camera_insert(Lib3dsFile *file, Lib3dsCamera *camera, Lib3dsIntd index);
extern LIB3DSAPI void lib3ds_file_camera_remove(Lib3dsFile *file, Lib3dsIntd index);
extern LIB3DSAPI Lib3dsIntd lib3ds_file_camera_by_name(Lib3dsFile *file, const char *name);
extern LIB3DSAPI void lib3ds_file_light_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force);
extern LIB3DSAPI void lib3ds_file_light_insert(Lib3dsFile *file, Lib3dsLight *light, Lib3dsIntd index);
extern LIB3DSAPI void lib3ds_file_light_remove(Lib3dsFile *file, Lib3dsIntd index);
extern LIB3DSAPI Lib3dsIntd lib3ds_file_light_by_name(Lib3dsFile *file, const char *name);
extern LIB3DSAPI void lib3ds_file_mesh_reserve(Lib3dsFile *file, Lib3dsIntd size, Lib3dsBool force);
extern LIB3DSAPI void lib3ds_file_mesh_insert(Lib3dsFile *file, Lib3dsMesh *mesh, Lib3dsIntd index);
extern LIB3DSAPI void lib3ds_file_mesh_remove(Lib3dsFile *file, Lib3dsIntd index);
extern LIB3DSAPI Lib3dsIntd lib3ds_file_mesh_by_name(Lib3dsFile *file, const char *name);
extern LIB3DSAPI Lib3dsMesh* lib3ds_file_mesh_for_node(Lib3dsFile *file, Lib3dsNode *node);
extern LIB3DSAPI Lib3dsNode* lib3ds_file_node_by_name(Lib3dsFile *file, const char* name, Lib3dsNodeType type);
extern LIB3DSAPI Lib3dsNode* lib3ds_file_node_by_id(Lib3dsFile *file, Lib3dsWord node_id);
extern LIB3DSAPI void lib3ds_file_insert_node(Lib3dsFile *file, Lib3dsNode *node);
extern LIB3DSAPI void lib3ds_file_remove_node(Lib3dsFile *file, Lib3dsNode *node);
extern LIB3DSAPI void lib3ds_file_minmax_node_id(Lib3dsFile *file, Lib3dsWord *min_id, Lib3dsWord *max_id);
extern LIB3DSAPI void lib3ds_file_bounding_box_of_objects(Lib3dsFile *file, Lib3dsBool include_meshes, Lib3dsBool include_cameras, Lib3dsBool include_lights, float bmin[3], float bmax[3]);
extern LIB3DSAPI void lib3ds_file_bounding_box_of_nodes(Lib3dsFile *file, Lib3dsBool include_meshes, Lib3dsBool include_cameras, Lib3dsBool include_lights, float bmin[3], float bmax[3], float matrix[4][4]);

/** @} */

#ifdef __cplusplus
}
#endif
#endif

