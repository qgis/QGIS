/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
** Copyright (C) 2001 Robert J. Campbell Jr.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Licensees holding valid dxflib Professional Edition licenses may use 
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/**
 * Defines common DXF codes and constants.
 */

#ifndef DXF_CODES_H
#define DXF_CODES_H

#include "dl_global.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#if defined(__OS2__)||defined(__EMX__)
#define strcasecmp(s,t) stricmp(s,t)
#endif

#if defined(_WIN32)
#define strcasecmp(s,t) _stricmp(s,t)
#endif


#ifdef _WIN32
#undef M_PI
#define M_PI   3.14159265358979323846
#pragma warning(disable : 4800)
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#define DL_DXF_MAXLINE 1024
#define DL_DXF_MAXGROUPCODE 1100

// used to mark invalid vectors:
//#define DL_DXF_MAXDOUBLE 1.0E+10

/**
 * Codes for colors and DXF versions.
 */
class DXFLIB_EXPORT DL_Codes {
public:
    /**
     * Standard DXF colors.
     */
    enum color {
        black = 250,
        green = 3,
        red = 1,
        brown = 15,
        yellow = 2,
        cyan = 4,
        magenta = 6,
        gray = 8,
        blue = 5,
        l_blue = 163,
        l_green = 121,
        l_cyan = 131,
        l_red = 23,
        l_magenta = 221,
        l_gray = 252,
        white = 7,
        bylayer = 256,
        byblock = 0
    };

    /**
     * Version numbers for the DXF Format.
     */
    enum version {
        AC1009,         // R12
        AC1012,
        AC1014,
        AC1015          // R2000
    };
};


// Extended color palette:
// The first entry is only for direct indexing starting with [1]
// Color 1 is red (1,0,0)
const double dxfColors[][3] = {
                                  {0,0,0},                // unused
                                  {1,0,0},                // 1
                                  {1,1,0},
                                  {0,1,0},
                                  {0,1,1},
                                  {0,0,1},
                                  {1,0,1},
                                  {1,1,1},                // black or white
                                  {0.5,0.5,0.5},
                                  {0.75,0.75,0.75},
                                  {1,0,0},                // 10
                                  {1,0.5,0.5},
                                  {0.65,0,0},
                                  {0.65,0.325,0.325},
                                  {0.5,0,0},
                                  {0.5,0.25,0.25},
                                  {0.3,0,0},
                                  {0.3,0.15,0.15},
                                  {0.15,0,0},
                                  {0.15,0.075,0.075},
                                  {1,0.25,0},             // 20
                                  {1,0.625,0.5},
                                  {0.65,0.1625,0},
                                  {0.65,0.4063,0.325},
                                  {0.5,0.125,0},
                                  {0.5,0.3125,0.25},
                                  {0.3,0.075,0},
                                  {0.3,0.1875,0.15},
                                  {0.15,0.0375,0},
                                  {0.15,0.0938,0.075},
                                  {1,0.5,0},              // 30
                                  {1,0.75,0.5},
                                  {0.65,0.325,0},
                                  {0.65,0.4875,0.325},
                                  {0.5,0.25,0},
                                  {0.5,0.375,0.25},
                                  {0.3,0.15,0},
                                  {0.3,0.225,0.15},
                                  {0.15,0.075,0},
                                  {0.15,0.1125,0.075},
                                  {1,0.75,0},             // 40
                                  {1,0.875,0.5},
                                  {0.65,0.4875,0},
                                  {0.65,0.5688,0.325},
                                  {0.5,0.375,0},
                                  {0.5,0.4375,0.25},
                                  {0.3,0.225,0},
                                  {0.3,0.2625,0.15},
                                  {0.15,0.1125,0},
                                  {0.15,0.1313,0.075},
                                  {1,1,0},                // 50
                                  {1,1,0.5},
                                  {0.65,0.65,0},
                                  {0.65,0.65,0.325},
                                  {0.5,0.5,0},
                                  {0.5,0.5,0.25},
                                  {0.3,0.3,0},
                                  {0.3,0.3,0.15},
                                  {0.15,0.15,0},
                                  {0.15,0.15,0.075},
                                  {0.75,1,0},             // 60
                                  {0.875,1,0.5},
                                  {0.4875,0.65,0},
                                  {0.5688,0.65,0.325},
                                  {0.375,0.5,0},
                                  {0.4375,0.5,0.25},
                                  {0.225,0.3,0},
                                  {0.2625,0.3,0.15},
                                  {0.1125,0.15,0},
                                  {0.1313,0.15,0.075},
                                  {0.5,1,0},              // 70
                                  {0.75,1,0.5},
                                  {0.325,0.65,0},
                                  {0.4875,0.65,0.325},
                                  {0.25,0.5,0},
                                  {0.375,0.5,0.25},
                                  {0.15,0.3,0},
                                  {0.225,0.3,0.15},
                                  {0.075,0.15,0},
                                  {0.1125,0.15,0.075},
                                  {0.25,1,0},             // 80
                                  {0.625,1,0.5},
                                  {0.1625,0.65,0},
                                  {0.4063,0.65,0.325},
                                  {0.125,0.5,0},
                                  {0.3125,0.5,0.25},
                                  {0.075,0.3,0},
                                  {0.1875,0.3,0.15},
                                  {0.0375,0.15,0},
                                  {0.0938,0.15,0.075},
                                  {0,1,0},                // 90
                                  {0.5,1,0.5},
                                  {0,0.65,0},
                                  {0.325,0.65,0.325},
                                  {0,0.5,0},
                                  {0.25,0.5,0.25},
                                  {0,0.3,0},
                                  {0.15,0.3,0.15},
                                  {0,0.15,0},
                                  {0.075,0.15,0.075},
                                  {0,1,0.25},             // 100
                                  {0.5,1,0.625},
                                  {0,0.65,0.1625},
                                  {0.325,0.65,0.4063},
                                  {0,0.5,0.125},
                                  {0.25,0.5,0.3125},
                                  {0,0.3,0.075},
                                  {0.15,0.3,0.1875},
                                  {0,0.15,0.0375},
                                  {0.075,0.15,0.0938},
                                  {0,1,0.5},              // 110
                                  {0.5,1,0.75},
                                  {0,0.65,0.325},
                                  {0.325,0.65,0.4875},
                                  {0,0.5,0.25},
                                  {0.25,0.5,0.375},
                                  {0,0.3,0.15},
                                  {0.15,0.3,0.225},
                                  {0,0.15,0.075},
                                  {0.075,0.15,0.1125},
                                  {0,1,0.75},             // 120
                                  {0.5,1,0.875},
                                  {0,0.65,0.4875},
                                  {0.325,0.65,0.5688},
                                  {0,0.5,0.375},
                                  {0.25,0.5,0.4375},
                                  {0,0.3,0.225},
                                  {0.15,0.3,0.2625},
                                  {0,0.15,0.1125},
                                  {0.075,0.15,0.1313},
                                  {0,1,1},                // 130
                                  {0.5,1,1},
                                  {0,0.65,0.65},
                                  {0.325,0.65,0.65},
                                  {0,0.5,0.5},
                                  {0.25,0.5,0.5},
                                  {0,0.3,0.3},
                                  {0.15,0.3,0.3},
                                  {0,0.15,0.15},
                                  {0.075,0.15,0.15},
                                  {0,0.75,1},             // 140
                                  {0.5,0.875,1},
                                  {0,0.4875,0.65},
                                  {0.325,0.5688,0.65},
                                  {0,0.375,0.5},
                                  {0.25,0.4375,0.5},
                                  {0,0.225,0.3},
                                  {0.15,0.2625,0.3},
                                  {0,0.1125,0.15},
                                  {0.075,0.1313,0.15},
                                  {0,0.5,1},              // 150
                                  {0.5,0.75,1},
                                  {0,0.325,0.65},
                                  {0.325,0.4875,0.65},
                                  {0,0.25,0.5},
                                  {0.25,0.375,0.5},
                                  {0,0.15,0.3},
                                  {0.15,0.225,0.3},
                                  {0,0.075,0.15},
                                  {0.075,0.1125,0.15},
                                  {0,0.25,1},             // 160
                                  {0.5,0.625,1},
                                  {0,0.1625,0.65},
                                  {0.325,0.4063,0.65},
                                  {0,0.125,0.5},
                                  {0.25,0.3125,0.5},
                                  {0,0.075,0.3},
                                  {0.15,0.1875,0.3},
                                  {0,0.0375,0.15},
                                  {0.075,0.0938,0.15},
                                  {0,0,1},                // 170
                                  {0.5,0.5,1},
                                  {0,0,0.65},
                                  {0.325,0.325,0.65},
                                  {0,0,0.5},
                                  {0.25,0.25,0.5},
                                  {0,0,0.3},
                                  {0.15,0.15,0.3},
                                  {0,0,0.15},
                                  {0.075,0.075,0.15},
                                  {0.25,0,1},             // 180
                                  {0.625,0.5,1},
                                  {0.1625,0,0.65},
                                  {0.4063,0.325,0.65},
                                  {0.125,0,0.5},
                                  {0.3125,0.25,0.5},
                                  {0.075,0,0.3},
                                  {0.1875,0.15,0.3},
                                  {0.0375,0,0.15},
                                  {0.0938,0.075,0.15},
                                  {0.5,0,1},              // 190
                                  {0.75,0.5,1},
                                  {0.325,0,0.65},
                                  {0.4875,0.325,0.65},
                                  {0.25,0,0.5},
                                  {0.375,0.25,0.5},
                                  {0.15,0,0.3},
                                  {0.225,0.15,0.3},
                                  {0.075,0,0.15},
                                  {0.1125,0.075,0.15},
                                  {0.75,0,1},             // 200
                                  {0.875,0.5,1},
                                  {0.4875,0,0.65},
                                  {0.5688,0.325,0.65},
                                  {0.375,0,0.5},
                                  {0.4375,0.25,0.5},
                                  {0.225,0,0.3},
                                  {0.2625,0.15,0.3},
                                  {0.1125,0,0.15},
                                  {0.1313,0.075,0.15},
                                  {1,0,1},                // 210
                                  {1,0.5,1},
                                  {0.65,0,0.65},
                                  {0.65,0.325,0.65},
                                  {0.5,0,0.5},
                                  {0.5,0.25,0.5},
                                  {0.3,0,0.3},
                                  {0.3,0.15,0.3},
                                  {0.15,0,0.15},
                                  {0.15,0.075,0.15},
                                  {1,0,0.75},             // 220
                                  {1,0.5,0.875},
                                  {0.65,0,0.4875},
                                  {0.65,0.325,0.5688},
                                  {0.5,0,0.375},
                                  {0.5,0.25,0.4375},
                                  {0.3,0,0.225},
                                  {0.3,0.15,0.2625},
                                  {0.15,0,0.1125},
                                  {0.15,0.075,0.1313},
                                  {1,0,0.5},              // 230
                                  {1,0.5,0.75},
                                  {0.65,0,0.325},
                                  {0.65,0.325,0.4875},
                                  {0.5,0,0.25},
                                  {0.5,0.25,0.375},
                                  {0.3,0,0.15},
                                  {0.3,0.15,0.225},
                                  {0.15,0,0.075},
                                  {0.15,0.075,0.1125},
                                  {1,0,0.25},             // 240
                                  {1,0.5,0.625},
                                  {0.65,0,0.1625},
                                  {0.65,0.325,0.4063},
                                  {0.5,0,0.125},
                                  {0.5,0.25,0.3125},
                                  {0.3,0,0.075},
                                  {0.3,0.15,0.1875},
                                  {0.15,0,0.0375},
                                  {0.15,0.075,0.0938},
                                  {0.33,0.33,0.33},       // 250
                                  {0.464,0.464,0.464},
                                  {0.598,0.598,0.598},
                                  {0.732,0.732,0.732},
                                  {0.866,0.866,0.866},
                                  {1,1,1}                 // 255
                              }
                              ;


// AutoCAD VERSION aliases
#define DL_VERSION_R12    DL_Codes::AC1009
#define DL_VERSION_LT2    DL_Codes::AC1009
#define DL_VERSION_R13    DL_Codes::AC1012   // not supported yet
#define DL_VERSION_LT95   DL_Codes::AC1012   // not supported yet
#define DL_VERSION_R14    DL_Codes::AC1014   // not supported yet
#define DL_VERSION_LT97   DL_Codes::AC1014   // not supported yet
#define DL_VERSION_LT98   DL_Codes::AC1014   // not supported yet
#define DL_VERSION_2000   DL_Codes::AC1015
#define DL_VERSION_2002   DL_Codes::AC1015


// DXF Group Codes:

// Strings
#define DL_STRGRP_START      0
#define DL_STRGRP_END        9

// Coordinates
#define DL_CRDGRP_START     10
#define DL_CRDGRP_END       19

// Real values
#define DL_RLGRP_START      38
#define DL_RLGRP_END        59

// Short integer values
#define DL_SHOGRP_START     60
#define DL_SHOGRP_END       79

// New in Release 13,
#define DL_SUBCLASS        100

// More coordinates
#define DL_CRD2GRP_START   210
#define DL_CRD2GRP_END     239

// Extended data strings
#define DL_ESTRGRP_START  1000
#define DL_ESTRGRP_END    1009

// Extended data reals
#define DL_ERLGRP_START   1010
#define DL_ERLGRP_END     1059


#define DL_Y8_COORD_CODE       28
#define DL_Z0_COORD_CODE       30
#define DL_Z8_COORD_CODE       38

#define DL_POINT_COORD_CODE    10
#define DL_INSERT_COORD_CODE   10

#define DL_CRD2GRP_START      210
#define DL_CRD2GRP_END        239

#define DL_THICKNESS            39
#define DL_FIRST_REAL_CODE      THICKNESS
#define DL_LAST_REAL_CODE       59
#define DL_FIRST_INT_CODE       60
#define DL_ATTFLAGS_CODE        70
#define DL_PLINE_FLAGS_CODE     70
#define DL_LAYER_FLAGS_CODE     70
#define DL_FLD_LEN_CODE         73 // Inside ATTRIB resbuf
#define DL_LAST_INT_CODE        79
#define DL_X_EXTRU_CODE        210
#define DL_Y_EXTRU_CODE        220
#define DL_Z_EXTRU_CODE        230
#define DL_COMMENT_CODE        999

// Start and endpoints of a line
#define DL_LINE_START_CODE      10  // Followed by x coord
#define DL_LINE_END_CODE        11  // Followed by x coord

// Some codes used by blocks
#define DL_BLOCK_FLAGS_CODE     70  // An int containing flags
#define DL_BLOCK_BASE_CODE      10  // Origin of block definition
#define DL_XREF_DEPENDENT       16  // If a block contains an XREF
#define DL_XREF_RESOLVED        32  // If a XREF resolved ok
#define DL_REFERENCED           64  // If a block is ref'd in DWG

#define DL_XSCALE_CODE          41
#define DL_YSCALE_CODE          42
#define DL_ANGLE_CODE           50
#define DL_INS_POINT_CODE       10  // Followed by x of ins pnt
#define DL_NAME2_CODE            3  // Second appearance of name

// Some codes used by circle entities
#define DL_CENTER_CODE          10  // Followed by x of center
#define DL_RADIUS_CODE          40  // Followd by radius of circle

#define DL_COND_OP_CODE         -4  // Conditional op,ads_ssget

// When using ads_buildlist you MUST use RTDXF0 instead of these
#define DL_ENTITY_TYPE_CODE      0  // Then there is LINE, 3DFACE..
#define DL_SES_CODE              0  // Start End String Code
#define DL_FILE_SEP_CODE         0  // File separator
#define DL_SOT_CODE              0  // Start Of Table
#define DL_TEXTVAL_CODE          1
#define DL_NAME_CODE             2
#define DL_BLOCK_NAME_CODE       2
#define DL_SECTION_NAME_CODE     2
#define DL_ENT_HAND_CODE         5  // What follows is hexa string
#define DL_TXT_STYLE_CODE        7  // Inside attributes
#define DL_LAYER_NAME_CODE       8  // What follows is layer name
#define DL_FIRST_XCOORD_CODE    10  // Group code x of 1st coord
#define DL_FIRST_YCOORD_CODE    20  // Group code y of 1st coord
#define DL_FIRST_ZCOORD_CODE    30  // Group code z of 1st coord
#define DL_L_START_CODE         10
#define DL_L_END_CODE           11
#define DL_TXTHI_CODE           40
#define DL_SCALE_X_CODE         41
#define DL_SCALE_Y_CODE         42
#define DL_SCALE_Z_CODE         43
#define DL_BULGE_CODE           42  // Used in PLINE verts for arcs
#define DL_ROTATION_CODE        50
#define DL_COLOUR_CODE          62  // What follows is a color int
#define DL_LTYPE_CODE            6  // What follows is a linetype


// Attribute flags
#define DL_ATTS_FOLLOW_CODE     66
#define DL_ATT_TAG_CODE          2
#define DL_ATT_VAL_CODE          1
#define DL_ATT_FLAGS_CODE       70  // 4 1 bit flags as follows...
#define DL_ATT_INVIS_FLAG        1
#define DL_ATT_CONST_FLAG        2
#define DL_ATT_VERIFY_FLAG       4 // Prompt and verify
#define DL_ATT_PRESET_FLAG       8 // No prompt and no verify

// PLINE defines
// Flags
#define DL_OPEN_PLINE       0x00
#define DL_CLOSED_PLINE     0x01
#define DL_POLYLINE3D       0x80
#define DL_PFACE_MESH       0x40
#define DL_PGON_MESH        0x10
// Vertices follow entity, required in POLYLINES
#define DL_VERTS_FOLLOW_CODE   66 // Value should always be 1
#define DL_VERTEX_COORD_CODE   10


// LAYER flags
#define DL_FROZEN           1
#define DL_FROZEN_BY_DEF    2
#define DL_LOCKED           4
#define DL_OBJECT_USED     64   // Object is ref'd in the dwg

#define DL_BLOCK_EN_CODE   -2   // Block entity definition
#define DL_E_NAME          -1   // Entity name

// Extended data codes
#define DL_EXTD_SENTINEL    (-3)
#define DL_EXTD_STR         1000
#define DL_EXTD_APP_NAME    1001
#define DL_EXTD_CTL_STR     1002
#define DL_EXTD_LYR_STR     1003
#define DL_EXTD_CHUNK       1004
#define DL_EXTD_HANDLE      1005
#define DL_EXTD_POINT       1010
#define DL_EXTD_POS         1011
#define DL_EXTD_DISP        1012
#define DL_EXTD_DIR         1013
#define DL_EXTD_FLOAT       1040
#define DL_EXTD_DIST        1041
#define DL_EXTD_SCALE       1042
#define DL_EXTD_INT16       1070
#define DL_EXTD_INT32       1071

// UCS codes for use in ads_trans
#define DL_WCS_TRANS_CODE      0
#define DL_UCS_TRANS_CODE      1
#define DL_DCS_TRANS_CODE      2
#define DL_PCS_TRANS_CODE      3

#endif

