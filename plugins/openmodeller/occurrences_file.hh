/**
 * Declaration of OccurrencesFile class.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2003-02-25
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

#ifndef _OCCURRENCES_FILEHH_
#define _OCCURRENCES_FILEHH_

#include "list.hh"

class Occurrences;
class GeoTransform;


/****************************************************************/
/********************* Occurrences File *************************/

/**  
 * Read occurrences data of one or more group from an
 * ASCII file.
 *
 * The file is read and stored as a linked list
 * of objects from the Occurrences class.
 *
 * Allow navigation through the object list.
 *
 * Format:
 *
 * Lines beginning with '#' are ignored!
 *
 * The file must have 5 columns separated by a
 * space (characters: ' ' or '\t').
 *
 * Columns 1,2: Group name (string).
 * Column  3  : longitude (-180.0 <= lat  <= 180.0).
 * Column  4  : latitude: (-90.0 <= long <= 90.0).
 * Column  5  : Group identifier (string).
 */
class OccurrencesFile
{
  typedef Occurrences *PtOccurrences;
  typedef DList<PtOccurrences> LstOccurrences;


public:

  OccurrencesFile( char *in, char *out )  { setCoordSystem(in,out); }
  OccurrencesFile( char *file, char *in, char *out );
  OccurrencesFile( int fd, char *in, char *out );
  ~OccurrencesFile();

  /** Add the occurrences from the file. */
  int addOccurrences( int fd );

  /** Return the number of occurrences. */
  int numOccurrences()  { return f_sp.Length(); }

  /** Navigate on the list of occurrences. */
  void head()     { f_sp.Head(); }
  void tail()     { f_sp.Tail(); }
  void next()     { f_sp.Next(); }
  Occurrences *get()   { return f_sp.Get(); }

  /** Return the group of occurrences known by 'name' or 0 if not found. */
  Occurrences *remove( char *name );

  void print( char *msg="" );


private:

  int setCoordSystem( char *in, char *out );

  /** Insert a new occurrence of a group. If the group does not
   *  exist, create it and return != 0. */
  int insertOcurrence( char *sp, Coord lg, Coord lt );


  LstOccurrences f_sp;  ///< List of occurrences read from file.
  GeoTransform  *f_gt;
};


#endif
