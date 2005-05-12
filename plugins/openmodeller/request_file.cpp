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

#include <om_sampler.hh>

#include <om.hh>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/**************************************************************/
/************************ Request File ************************/

RequestFile::RequestFile() :
  _occurrences_set(0),
  _environment_set(0),
  _projection_set(0),
  _occurrences(),
  _nonNativeProjection( false ),
  _cat(),
  _map(),
  _mask(),
  _model_file(),
  _projection_file(),
  _outputFormat()
{ 
}

RequestFile::~RequestFile() 
{
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
  _algorithm_set   = setAlgorithm  ( om, fp );

  _model_file = fp.get( "Output model" );

  // Returns ZERO if all was set correctly.
  return 4 - _occurrences_set - _environment_set -
    _projection_set - _algorithm_set;
}


/***********************/
/*** set Occurrences ***/
int
RequestFile::setOccurrences( OpenModeller *om, FileParser &fp )
{
  // Obtain the Well Known Text string for the localities
  // coordinate system.
  std::string oc_cs = fp.get( "WKT coord system" );

  // Get the name of the file containing localities
  std::string oc_file = fp.get( "Species file" );

  // Get the name of the taxon being modelled!
  std::string oc_name = fp.get( "Species" );

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
  std::string mask = fp.get( "Mask" );

  // Initiate the environment with all maps.
  std::vector<std::string> cat = fp.getAll( "Categorical map" );
  std::vector<std::string> map = fp.getAll( "Map" );
  om->setEnvironment( cat, map, mask );

  return 1;
}


/**********************/
/*** set Projection ***/
int
RequestFile::setProjection( OpenModeller *om, FileParser &fp )
{

  _projection_file = fp.get( "Output file" );
  if ( _projection_file.empty() )
    {
      g_log( "The 'Output file' file name was not specified!\n" );
      return 0;
    }

  std::string format = fp.get( "Output format" );
  if ( ! format.empty() )
    {
      _outputFormat = MapFormat( format.c_str() );
    }

  // Categorical environmental maps and the number of these maps.
  _cat = fp.getAll( "Categorical output map" );

  // Continuous environmental maps and the number of these maps.
  _map = fp.getAll( "Output Map" );

  // It is ok to not set the projection.
  if ( ! (_cat.size() + _map.size()) )
    {
      g_log("Projection not set: using training Environment for projection\n");
      _nonNativeProjection = false;
      return 1;
    }

  _nonNativeProjection = true;

  // Get the details for the output Map
  _mask = fp.get( "Output mask" );
  if ( _mask.empty() )
    {
      g_log( "The 'Output mask' file name was not specified!\n" );
      return 0;
    }

  // Make sure the basic variables have been defined in the
  // parameter file...

  return 1;

}


/***********************/
/*** set Algorithm ***/
int
RequestFile::setAlgorithm( OpenModeller *om, FileParser &fp )
{
  // Find out which model algorithm is to be used.
  AlgMetadata const *metadata;
  std::string alg_id = fp.get( "Algorithm" );

  // Note: console tries to get an algorithm from user input
  // if it was not specified in the request file.
  if ( alg_id.empty() )
    return 0;

  // Try to use the algorithm specified in the request file.
  // If it cannot be used, return 0.
  try {
    // An exception here means that the algorithm wasn't found.
    metadata = om->algorithmMetadata( alg_id.c_str() );
  }
  catch (...) {
    g_log( "Algorithm '%s' specified in the request file was not found\n", alg_id.c_str() );
    return 0;
  }

  // Obtain any model parameter specified in the request file.
  // read parameters from file into req_param parameters
  std::vector<std::string> req_param = fp.getAll( "Parameter" );

  // For resulting parameters storage.
  int nparam = metadata->nparam;
  AlgParameter *param = new AlgParameter[nparam];

  // Read from console the parameters not set by request
  // file. Fills 'param' with all 'metadata->nparam' parameters
  // set.
  readParameters( param, metadata, req_param );

  // Set the model algorithm to be used by the controller
  int resp = om->setAlgorithm( metadata->id, nparam, param );

  if ( resp == 0 )
    g_log.error( 1, "Resp from setAlg was zero\n" );

  delete[] param;

  return resp;
}


/************************/
/*** read Occurrences ***/
OccurrencesPtr
RequestFile::readOccurrences( std::string file, std::string name,
                              std::string coord_system )
{
  OccurrencesFile oc_file( file.c_str(), coord_system.c_str() );

  return oc_file.remove( name.c_str() );
}


/***********************/
/*** read Parameters ***/
int
RequestFile::readParameters( AlgParameter *result,
                             AlgMetadata const *metadata,
                             std::vector<std::string> str_param )
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
      std::string value = extractParameter( result->id(), str_param );

      // If the parameter is not referenced in the file, set it
      // with the default value extracted from the parameter
      // metadata.
      if ( value.empty() )
        value = param->typical;

      result->setValue( value.c_str() );
    }

  return metadata->nparam;
}


/*************************/
/*** extract Parameter ***/
std::string
RequestFile::extractParameter( std::string const name, 
			       std::vector<std::string> vet )
{
  int length = name.length();
  std::vector<std::string>::iterator it = vet.begin();
  std::vector<std::string>::iterator end = vet.end();

  while ( it != end ) {
    if ( name == (*it).substr(0, length) ) {
      return (*it).substr( length );
    }
    ++it;
  }

  return "";
}


/******************/
/*** make Model ***/
void
RequestFile::makeModel( OpenModeller *om )
{
  // Build model
  if ( ! om->createModel() ) {
    g_log.error( 1, "Error during model creation: %s\n", om->error() );
  }

  // Serialize model, if requested
  if ( ! _model_file.empty() ) {

    char* file_name = new char [_model_file.size()];
    strcpy( file_name, _model_file.c_str() );

    ConfigurationPtr cfg = om->getConfiguration();
    Configuration::writeXml( cfg, file_name );

    delete[] file_name;
  }
}


/***********************/
/*** make Projection ***/
void
RequestFile::makeProjection( OpenModeller *om )
{
  if ( _projection_set == 0 )
    g_log.error( 1, "Error during projection: Request not properly initialized\n" );

  if ( !_nonNativeProjection ) {

    om->createMap( _projection_file.c_str() );
  }
  else {

    EnvironmentPtr env = createEnvironment( _cat, _map, _mask );

    om->createMap( env, _projection_file.c_str(), _outputFormat );
  }
}
