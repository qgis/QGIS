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

#include <list.hh>
#include <om_defs.hh>

class Occurrences;


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

  OccurrencesFile( char *file_name, char *coord_system );
  ~OccurrencesFile();

  /** Add the occurrences from the file. */
  int addOccurrences( char *file_name );

  /** Return the number of occurrences. */
  int numOccurrences()  { return f_sp.length(); }

  /** Navigate on the list of occurrences. */
  void head()     { f_sp.head(); }
  void tail()     { f_sp.tail(); }
  void next()     { f_sp.next(); }
  Occurrences *get()   { return f_sp.get(); }

  /** Return the group of occurrences known by 'name' or 0 if
   *  not found.
   */
  Occurrences *remove( char *name );

  void printOccurrences( char *msg="" );


private:

  /** Insert a new occurrence of a group. If the group does not
   *  exist, create it and return != 0. */
  int insertOcurrence( char *sp, Coord lg, Coord lt, Scalar error,
		       Scalar abundance, int num_attributes,
		       Scalar *attributes );


  LstOccurrences f_sp;  ///< List of occurrences read from file.
  char *_coord_system;
};


#endif
