/**
 * Definition of types and macros of general use.
 * 
 * @file
 * @author Mauro E S Munoz
 * @date   2003-01-24
 * $Id$
 * 
 * LICENSE INFORMATION 
 * 
 * Copyright(c) 2003 by CRIA -
 * Centro de Referencia em Informacao Ambiental
 *
 * http://www.cria.org.br
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details:
 * 
 * http://www.gnu.org/copyleft/gpl.html
 */



#ifndef _DEFSHH_
#define _DEFSHH_


// Types.
//
typedef unsigned char uchar;
typedef unsigned int  uint;
typedef unsigned long ulong;


// Math types.
//
typedef double Real;
typedef double Angle;

// Must be 'float' or 'double':
typedef double Coord;    ///< Type of map coordinates.
typedef double Scalar;   ///< Type of map values.


// Types for funtion pointers.
//
typedef double (*doubleFunc)(double);
typedef float  (*floatFunc)(float);


#define Zero        (1e-8)
#define Abs(x)      ((x) < 0 ? -(x) : x)
#define IsZero(x)   ((x) > -Zero && (x) < Zero)
#define Min( a, b ) ((a) < (b) ? (a) : (b))
#define Max( a, b ) ((a) > (b) ? (a) : (b))


// windows only defs
#ifdef WIN32

#define strcasecmp _stricmp

#endif


#endif




