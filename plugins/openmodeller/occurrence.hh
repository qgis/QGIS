/**
 * Declaration of Occurrence class.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2003-25-02
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


#ifndef _OCCURRENCEHH_
#define _OCCURRENCEHH_

#include <defs.hh>


/**
 * Geographic coordinates (longitude, latitude) in decimal degrees
 * and the number of individuals.
 * 
 */
class Occurrence
{
public:

  Occurrence( Coord px, Coord py, float p )  {x=px; y=py; pop=p;}

  Coord x, y;
  float pop;  ///< Population size.
};

typedef Occurrence *PtOccurrence;


#endif
