/**
 * Declaration of Occurrences class.
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


#ifndef _OCCURRENCESHH_
#define _OCCURRENCESHH_

#include "defs.hh"
#include "list.hh"

class Occurrence;


/****************************************************************/
/************************* Occurrences **************************/

/** 
 * Representation of a set of occurrences.
 *
 */
class Occurrences
{
  typedef DList<Occurrence *> LstOccur;


public:

  Occurrences( char *name, char *id );
  ~Occurrences();

  char *name()   { return f_name; }
  char *id()     { return f_id;   }

  /** Insert an occurrence. */
  void insert( Coord longitude, Coord latitude, float pop=1.0 );


  /** Number of occurrences. */
  int numOccurrences()   { return f_ocur.Length(); }

  /** Navigate on the list of occurrences. */
  void head()        { f_ocur.Head(); }
  void next()        { f_ocur.Next(); }
  Occurrence *get()  { return f_ocur.Get(); }

  /** Remove from list the current coordinate, returning it. */
  Occurrence *remove()  { return f_ocur.Delete(); }
  

  /** Print occurrence data and its points. */
  void print( char *msg="" );


private:

  char *f_name; ///< A name for the list of occurrences (e.g. species name).
  char *f_id;   ///< An identifier for the list of occurrences.

  LstOccur f_ocur;  ///< Coordinates of the occurrences.
};


#endif
