/**
 * Se an OpenModeller object reading parameters from a request file.
 * 
 * @file
 * @author Mauro E S Muñoz (mauro@cria.org.br)
 * @date   2004-06-22
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

#include "request_file.hh"
#include "file_parser.hh"
#include "occurrences_file.hh"

#include <om.hh>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/**************************************************************/
/************************ Request File ************************/

RequestFile::RequestFile()  
{ 
  _occurrences = 0; 
}

RequestFile::~RequestFile() 
{
  if (_occurrences)
    { delete _occurrences; }
}

/*****************/
/*** configure ***/
int
RequestFile::configure( OpenModeller *om, char *request_file )
{
  FileParser fp( request_file );

  _occurrences_set = setOccurrences( om, fp );
  _environment_set = setEnvironment( om, fp );
  _projection_set  = setProjection ( om, fp );
  _outputmap_set   = setOutputMap  ( om, fp );
  _algorithm_set   = setAlgorithm  ( om, fp );

  // Returns ZERO if all was set correctly.
  return 5 - _occurrences_set - _environment_set -
    _projection_set - _outputmap_set - _algorithm_set;
}


/***********************/
/*** set Occurrences ***/
int
RequestFile::setOccurrences( OpenModeller *om, FileParser &fp )
{
  // Obtain the Well Known Text string for the localities
  // coordinate system.
  char *oc_cs = fp.get( "WKT coord system" );

  // Get the name of the file containing localities
  char *oc_file = fp.get( "Species file" );

  // Get the name of the taxon being modelled!
  char *oc_name = fp.get( "Species" );

  _occurrences = readOccurrences( oc_file, oc_name, oc_cs );

  // Populate the occurences list from the localities file
  return om->setOccurrences( _occurrences );
}


/***********************/
/*** set Environment ***/
int
RequestFile::setEnvironment( OpenModeller *om, FileParser &fp )
{
  // Mask to select the desired species occurrence points
  char *mask = fp.get( "Mask" );

  // Categorical environmental maps and the number of these maps.
  char *cat_label = "Categorical map";
  int  ncat  = fp.count( cat_label );
  char **cat = new char *[ncat];

  // Continuous environmental maps and the number of these maps.
  char *map_label = "Map";
  int  nmap  = fp.count( map_label );
  char **map = new char *[nmap];

  // Initiate the environment with all maps.
  fp.getAll( cat_label, cat );
  fp.getAll( map_label, map );
  int resp = om->setEnvironment( ncat, cat, nmap, map, mask );

  delete cat;
  delete map;

  return resp;
}


/**********************/
/*** set Projection ***/
int
RequestFile::setProjection( OpenModeller *om, FileParser &fp )
{
  // Categorical environmental maps and the number of these maps.
  char *cat_label = "Categorical output map";
  int  ncat  = fp.count( cat_label );
  char **cat = 0;
  if ( ncat )
    cat = new char *[ncat];

  // Continuous environmental maps and the number of these maps.
  char *map_label = "Output Map";
  int  nmap  = fp.count( map_label );
  char **map = 0;
  if ( nmap )
    map = new char *[nmap];

  // It is ok to not set the projection.
  if ( ! (ncat + nmap) )
    return 1;

  // Initiate the environment with all maps.
  fp.getAll( cat_label, cat );
  fp.getAll( map_label, map );
  int resp = om->setProjection( ncat, cat, nmap, map );

  delete cat;
  delete map;

  return resp;
}


/***********************/
/*** set OutputMap ***/
int
RequestFile::setOutputMap( OpenModeller *om, FileParser &fp )
{
  // Get the details for the output Map
  char *mask   = fp.get( "Output mask" );
  char *file   = fp.get( "Output file" );
  char *format = fp.get( "Output format" );

  // Used to scale the model results e.g. from [0,1] to
  // [0,255] - useful for image generation.
  char *multiplier = fp.get( "Scale" );

  // Make sure the basic variables have been defined in the
  // parameter file...
  if ( ! file )
    {
      g_log( "The 'Output file' file name was not specified!\n" );
      return 0;
    }
  if ( ! format )
    {
      g_log( "The 'Output format' was not specified!\n" );
      return 0;
    }
  if ( ! multiplier )
    multiplier = "255.0";

  return om->setOutputMap( atof(multiplier), file, mask, format );
}


/***********************/
/*** set Algorithm ***/
int
RequestFile::setAlgorithm( OpenModeller *om, FileParser &fp )
{
  // Find out which model algorithm is to be used.
  AlgMetadata *metadata;
  char *alg_id = fp.get( "Algorithm" );

  // Try to used the algorithm specified in the request file.
  // If it can not be used, return 0.
  if ( ! (metadata = om->algorithmMetadata( alg_id )) )
    return 0;

  // Obtain any model parameter specified in the request file.
  char *param_label = "Parameter";
  int   req_nparam  = fp.count( param_label );
  char **req_param  = new char *[req_nparam];

  // read parameters from file into req_param parameters
  fp.getAll( param_label, req_param );

  // For resulting parameters storage.
  int nparam = metadata->nparam;
  AlgParameter *param = new AlgParameter[nparam];

  // Read from console the parameters not set by request
  // file. Fills 'param' with all 'metadata->nparam' parameters
  // set.
  readParameters( param, metadata, req_nparam, req_param );

  // Set the model algorithm to be used by the controller
  int resp = om->setAlgorithm( metadata->id, nparam, param );

  delete[] param;
  delete[] req_param;

  return resp;
}


/************************/
/*** read Occurrences ***/
Occurrences *
RequestFile::readOccurrences( char *file, char *name,
                              char *coord_system )
{
  OccurrencesFile oc_file( file, coord_system );

  // Take last species from the list, which corresponds to the
  // first inside the file.
  if ( ! name )
    {
      oc_file.tail();
      name = oc_file.get()->name();
    }

  return oc_file.remove( name );
}


/***********************/
/*** read Parameters ***/
int
RequestFile::readParameters( AlgParameter *result,
                             AlgMetadata *metadata,
                             int str_nparam,
                             char **str_param )
{
  AlgParamMetadata *param = metadata->param;
  AlgParamMetadata *end   = param + metadata->nparam;

  // For each algorithm parameter metadata...
  for ( ; param < end; param++, result++ )
    {
      // The resulting name is equal the name set in
      // algorithm's metadata.
      result->setId( param->id );

      // Read the resulting value from str_param.
      char *value = extractParameter( result->id(), str_nparam,
                                      str_param );

      // If the parameter is not referenced in the file, set it
      // with the default value extracted from the parameter
      // metadata.
      if ( ! value )
        value = param->typical;

      result->setValue( value );
    }

  return metadata->nparam;
}


/*************************/
/*** extract Parameter ***/
/**
 * Search for 'name' in the 'nvet' elements of the vector 'vet'.
 * If the string 'name' is in the begining of some string vet[i]
 * then returns a pointer to the next character of vet[i],
 * otherwise returns 0.
 */
char *
RequestFile::extractParameter( char *name, int nvet, char **vet )
{
  int length = strlen( name );
  char **end = vet + nvet;

  while ( vet < end )
    if ( ! strncmp( name, *vet++, length ) )
      return *(vet-1) + length;

  return 0;
}


