/**
 * Definition of OccurrencesFile class.
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

#include "occurrences_file.hh"

#include <om_occurrences.hh>  // List of occurrences.
#include <om_log.hh>
#include <list.cpp>           // Template.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Win32 defines
#ifdef WIN32
#include <fcntl.h>
#include <io.h>

// Default coordinate system for occurrence points.
//


#define open	_open
#define close	_close
#define fdopen  _fdopen

#else
#include <fcntl.h>
#include <unistd.h>
#endif


/****************************************************************/
/*********************** Occurrences File ***********************/

/*******************/
/*** Constructor ***/

OccurrencesFile::OccurrencesFile( char *file_name,
				  char *coord_system )
{
  _coord_system = coord_system;
  addOccurrences( file_name );
}


/******************/
/*** Destructor ***/

OccurrencesFile::~OccurrencesFile()
{
  // Empty the occurrences list.
  Occurrences *sp;
  for ( f_sp.head(); sp = f_sp.get(); f_sp.next() )
    delete( sp );
}


/**********************/
/*** add Ocurrences ***/
int
OccurrencesFile::addOccurrences( char *file_name )
{
  // Opens the input file.
  FILE *file = fopen( file_name, "r" );
  if ( ! file )
    {
      g_log.warn( "Can't open file %s.", file_name );
      return 0;
    }

  // Fixme: read this from file.
  Scalar error     	= -1.0;
  Scalar abundance 	= 1.0;
  int    num_attributes = 0;
  Scalar *attributes    = 0;


  // Columns to be read.
  char  sp1[256];
  char  sp2[256];
  double x, y;

  // Read all occurrences line by line inserting into the
  // appropriate object.
  char line[256];
  char *pos;
  while ( fgets( line, 256, file ) )
    {
      // Remove \r that DOS loves to use.
      if ( pos = strchr( line, '\r' ) )
	*pos = '\0';

      if ( *line != '#' &&
	   sscanf( line,"%s%s%lf%lf",sp1,sp2,&x,&y ) == 4 )
	{
	  // Build the occurrences list name using the first two
	  // columns. (originally genus + species)
	  strcat( sp1, " " );
	  strcat( sp1, sp2 );

	  Coord lg = Coord( x );
	  Coord lt = Coord( y );

	  insertOcurrence( sp1, lg, lt, error, abundance,
			   num_attributes, attributes );
	}
    }

  fclose( file );
  return 0;
}


/**************/
/*** remove ***/
Occurrences *
OccurrencesFile::remove( char *name )
{
  Occurrences *sp;
  for ( f_sp.head(); sp = f_sp.get(); f_sp.next() )
    if ( ! strcasecmp( name, sp->name() ) )
      return f_sp.remove();

  return 0; 
}


/*************/
/*** print ***/
void
OccurrencesFile::printOccurrences( char *msg )
{
  printf( "%s", msg );

  Occurrences *sp;
  for ( f_sp.head(); sp = f_sp.get(); f_sp.next() )
    sp->print( "\n****************" );
}


/************************/
/*** insert Ocurrence ***/
int
OccurrencesFile::insertOcurrence( char *name, Coord lg, Coord lt,
				  Scalar error, Scalar abundance,
				  int num_attributes,
				  Scalar *attributes )
{
  Occurrences *sp;

  // If an occurrences group with the same name already exists, 
  // just add another occurrence to it.
  for ( f_sp.head(); sp = f_sp.get(); f_sp.next() )
    if ( ! strcasecmp( sp->name(), name ) )
      {
        sp->insert( lg, lt, error, abundance, num_attributes,
		    attributes );
        return 0;
      }

  // If the occurrences group does not exist, create it.
  //
  // Obs: the new group is included at the beginning of the list
  // because other occurrences from the same group are often in
  // the same portion of the file. Therefore we get better
  // performance.
  f_sp.insertFirst( sp = new Occurrences( name, _coord_system ) );
  sp->insert( lg, lt, error, abundance, num_attributes,
	      attributes );

  return 1;
}
