/**
 * Declaration of GColor class.
 * 
 * @file
 * @author Mauro E S Muñoz <mauro@cria.org.br>
 * @date 2003-10-25
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

#ifndef _COLORHH_
#define _COLORHH_


#define NUM_COLOR 256          // Maximum value of each color channel.
#define MAX_COLOR (NUM_COLOR - 1)
#define MID_COLOR (NUM_COLOR > 1)
#define MIN_COLOR 0


/**********************************************************************/
/******************************** GColor ******************************/

/** 
 *  Stores a RGB color. It can be set to be used with 8, 16
 *  and 24 bit representation of colors, but only one can be
 *  setted (class attribute)... just for performancy.
 *  
 */

/**********/
class GColor
{

public:

  GColor()             { r = g = b = 0; }
  GColor( GColor &c )  { r = c.r; g = c.g; b = c.b; }

  GColor( int red, int green, int blue ) { r=red; g=green; b=blue; }
  GColor( int grey )                     { r = g = b = grey;  }

  // In the range [0, 1).
  GColor( double red, double green, double blue );
  GColor( double grey );

  GColor &operator=( GColor &c );
  GColor &operator=( int grey );

  GColor &set( int rd, int gr, int bl ) { r=rd; g=gr; bl=b; return *this; }

  /** Conversions to primitive types */
  operator unsigned long();


  /** Video depth: 8, 16, 24 */
  static void setDepth( int depth );

  /** Scale by 'esc' and crop to fit the range. */
  GColor &scale( double esc );

  // Get normalized cores in [0,1].
  double red()     { return double(r) / MAX_COLOR; }
  double green()   { return double(g) / MAX_COLOR; }
  double blue()    { return double(b) / MAX_COLOR; }


  // Debug
  void print( char *msg="" );


  int r;  ///< Red
  int g;  ///< Green
  int b;  ///< Blue


  /*** basic colors ***/
  static GColor Black;
  static GColor Grey;
  static GColor White;
  static GColor Red;
  static GColor Green;
  static GColor Blue;


private:

  static int f_depth;  // Video depth.
};


#endif


