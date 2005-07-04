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

#include <openmodeller/om.hh>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/**************************************************************/
/************************ Request File ************************/

RequestFile::RequestFile() :
  _occurrencesSet(0),
  _environmentSet(0),
  _projectionSet(0),
  _occurrences(),
  _nonNativeProjection( false ),
  _projectionCategoricalMap(),
  _projectionMap(),
  _inputMask(),
  _outputMask(),
  _inputModelFile(),
  _outputModelFile(),
  _projectionFile(),
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

  _inputModelFile = fp.get( "Input model" );

  _occurrencesSet = setOccurrences( om, fp );
  _environmentSet = setEnvironment( om, fp );
  _projectionSet  = setProjection ( om, fp );
  _algorithmSet   = setAlgorithm  ( om, fp );

  _outputModelFile = fp.get( "Output model" );

  // Returns ZERO if all was set correctly.
  return 4 - _occurrencesSet - _environmentSet -
    _projectionSet - _algorithmSet;
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

  // If user provided a serialized model
  if ( ! _inputModelFile.empty() ) {

    // Warn if unnecessary parameters were specified 
    if ( ! oc_cs.empty() )
      g_log.warn( "'WKT coord system' will be ignored since 'Input model' has been specified...\n" );

    if ( ! oc_file.empty() )
      g_log.warn( "'Species file' will be ignored since 'Input model' has been specified...\n" );

    if ( ! oc_name.empty() )
      g_log.warn( "'Species' will be ignored since 'Input model' has been specified...\n" );

    return 1;
  }

  // When a model needs to be created, 'WKT coord system' and 
  // 'Species file' are mandatory parameters
  if ( oc_cs.empty() ) {
    g_log.error( 0, "'WKT coord system' was not specified!\n" );
    return 0;
  }

  if ( oc_file.empty() ) {
    g_log.error( 0, "'Species file' was not specified!\n" );
    return 0;
  }

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
  _inputMask = fp.get( "Mask" );

  // Initiate the environment with all maps.
  std::vector<std::string> cat = fp.getAll( "Categorical map" );
  std::vector<std::string> map = fp.getAll( "Map" );

  // If user provided a serialized model
  if ( ! _inputModelFile.empty() ) {

    // Warn if unnecessary parameters were specified 
    if ( ! _inputMask.empty() )
      g_log.warn( "'Mask' will be ignored since 'Input model' has been specified...\n" );

    if ( cat.size() > 0 )
      g_log.warn( "'Categorical map' will be ignored since 'Input model' has been specified...\n" );

    if ( map.size() > 0 )
      g_log.warn( "'Map' will be ignored since 'Input model' has been specified...\n" );

    return 1;
  }

  // When a model needs to be created, there should be at least one input map
  if ( ! (cat.size() + map.size()) ) {

    g_log.error( 0, "At least one 'Map' or 'Categorical map' needs to be specified!\n" );
    return 0;
  }

  // Mask is also mandatory
  if ( _inputMask.empty() ) {
    g_log.error( 0, "'Mask' was not specified!\n" );
    return 0;
  }

  // Set input environment
  om->setEnvironment( cat, map, _inputMask );

  return 1;
}


/**********************/
/*** set Projection ***/
int
RequestFile::setProjection( OpenModeller *om, FileParser &fp )
{

  _projectionFile = fp.get( "Output file" );

  if ( _projectionFile.empty() ) {

    g_log.error( 0, "'Output file' was not specified!\n" );
    return 0;
  }

  // Categorical environmental maps and the number of these maps.
  _projectionCategoricalMap = fp.getAll( "Categorical output map" );

  // Continuous environmental maps and the number of these maps.
  _projectionMap = fp.getAll( "Output Map" );

  // If user provided a serialized model
  if ( _inputModelFile.empty() ) {

    // note: should we accept native projections using environment from serialized models?
    _nonNativeProjection = true;

    // So, assume that in this case projection maps are mandatory.
    if ( ! (_projectionCategoricalMap.size() + _projectionMap.size()) ) {

      g_log.error( 0, "At least one 'Output map' or 'Categorical output map' needs to be specified!\n" );
      return 0;
    }
  }
  else {

    // It is ok to not set the projection.
    if ( ! (_projectionCategoricalMap.size() + _projectionMap.size()) ) {

      g_log("Projection not set: using training Environment for projection\n");
      _nonNativeProjection = false;
    }
    else {

      _nonNativeProjection = true;
    }
  }

  // Get the output mask
  _outputMask = fp.get( "Output mask" );

  if ( _nonNativeProjection && _outputMask.empty() ) {

    g_log.error( 0, "'Output mask' was not specified!\n" );
    return 0;
  }

  // Template header to be used by the generated map
  std::string format = fp.get( "Output format" );

  if ( ! format.empty() ) {

    _outputFormat = MapFormat( format.c_str() );
  }

  // hard coded for now: 8-bit grey tiffs
  _outputFormat.setFormat( MapFormat::GreyTiff );

  // Overwrite output extent with values from mask
  const std::string maskFile = ( _nonNativeProjection ) ? _outputMask.c_str() : _inputMask.c_str();

  Raster mask( maskFile );

  Header h = mask.header();

  _outputFormat.setXMin( h.xmin );
  _outputFormat.setYMin( h.ymin );
  _outputFormat.setXMax( h.xmax );
  _outputFormat.setYMax( h.ymax );

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

  // If user provided a serialized model
  if ( ! _inputModelFile.empty() ) {
    // Warn if unnecessary parameters were specified 
    if ( ! alg_id.empty() )
      g_log.warn( "'Algorithm' will be ignored since 'Input model' has been specified...\n" );

    return 1;
  }

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

    g_log.error( 0, "Algorithm '%s' specified in the request file was not found\n", 
                 alg_id.c_str() );
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
    g_log.error( 0, "Could not set the algorithm to be used\n" );

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


/************************/
/*** get Occurrences ***/
OccurrencesPtr
RequestFile::getOccurrences( )
{
  if ( ! _occurrences )
    g_log.error( 0, "Could not read occurrences from request file. Make sure 'Species file' has been specified.\n" );

  return _occurrences;
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
  for ( ; param < end; param++, result++ ) {

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
  // If user provided a serialized model, just load it
  if ( ! _inputModelFile.empty() ) {

    char* file_name = new char [_inputModelFile.size() + 1];
    strcpy( file_name, _inputModelFile.c_str() );

    ConfigurationPtr conf = Configuration::readXml( file_name );
      
    om->setConfiguration( conf );

    delete[] file_name;

    return;
  }

  // Build model
  if ( ! om->createModel() ) {
    g_log.error( 1, "Error during model creation: %s\n", om->error() );
  }

  // Serialize model, if requested
  if ( _inputModelFile.empty() && ! _outputModelFile.empty() ) {

    char* file_name = new char [_outputModelFile.size() + 1];
    strcpy( file_name, _outputModelFile.c_str() );

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
  if ( _projectionSet == 0 )
    g_log.error( 1, "Error during projection: Request not properly initialized\n" );

  if ( !_nonNativeProjection ) {

    om->createMap( _projectionFile.c_str(), _outputFormat );
  }
  else {

    EnvironmentPtr env = createEnvironment( _projectionCategoricalMap, _projectionMap, _outputMask );

    om->createMap( env, _projectionFile.c_str(), _outputFormat );
  }
}
