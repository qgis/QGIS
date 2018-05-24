/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <memory>

#include "mdal_binary_dat.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"

#include <math.h>

static const int CT_VERSION   = 3000;
static const int CT_OBJTYPE   = 100;
static const int CT_SFLT      = 110;
static const int CT_SFLG      = 120;
static const int CT_BEGSCL    = 130;
static const int CT_BEGVEC    = 140;
static const int CT_VECTYPE   = 150;
static const int CT_OBJID     = 160;
static const int CT_NUMDATA   = 170;
static const int CT_NUMCELLS  = 180;
static const int CT_NAME      = 190;
static const int CT_TS        = 200;
static const int CT_ENDDS     = 210;
static const int CT_RT_JULIAN = 240;
static const int CT_TIMEUNITS = 250;

static const int CT_2D_MESHES = 3;
static const int CT_FLOAT_SIZE = 4;
static const int CF_FLAG_SIZE = 1;
static const int CF_FLAG_INT_SIZE = 4;

#define EXIT_WITH_ERROR(error)       {  if (status) *status = (error); return; }

static bool read( std::ifstream &in, char *s, int n )
{
  in.read( s, n );
  if ( !in )
    return true; //error
  else
    return false; //OK
}

static bool readIStat( std::ifstream &in, int sflg, char *flag )
{
  if ( sflg == CF_FLAG_SIZE )
  {
    in.read( flag, sflg );
    if ( !in )
      return true; // error
  }
  else
  {
    int istat;
    in.read( ( char * )&istat, sflg );
    if ( !in )
      return true; // error
    else
      *flag = ( istat == 1 );
  }
  return false;
}

MDAL::LoaderBinaryDat::LoaderBinaryDat( const std::string &datFile ):
  mDatFile( datFile )
{
}

/**
 * The DAT format contains "datasets" and each dataset has N-outputs. One output
 * represents data for all vertices/faces for one timestep
 *
 * in TUFLOW results there could be also a special timestep (99999) with maximums
 * we will put it into a separate dataset with name suffixed with "/Maximums"
 *
 * In MDAL we convert one output to one MDAL dataset;
 *
 */
void MDAL::LoaderBinaryDat::load( MDAL::Mesh *mesh, MDAL_Status *status )
{
  if ( status ) *status = MDAL_Status::None;

  if ( !MDAL::fileExists( mDatFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return;
  }

  std::ifstream in( mDatFile, std::ifstream::in | std::ifstream::binary );

  // implementation based on information from:
  // http://www.xmswiki.com/wiki/SMS:Binary_Dataset_Files_*.dat
  if ( !in )
    EXIT_WITH_ERROR( MDAL_Status::Err_FileNotFound ); // Couldn't open the file

  size_t vertexCount = mesh->vertices.size();
  size_t elemCount = mesh->faces.size();

  int card = 0;
  int version;
  int objecttype;
  int sflt;
  int sflg;
  int vectype;
  int objid;
  int numdata;
  int numcells;
  char name[40];
  char istat;
  float time;

  if ( read( in, ( char * )&version, 4 ) )
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  if ( version != CT_VERSION ) // Version should be 3000
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  bool isVector = false;
  std::string baseDatasetName;
  std::vector<std::shared_ptr<Dataset>> datOutputs; // DAT outputs data

  while ( card != CT_ENDDS )
  {
    if ( read( in, ( char * )&card, 4 ) )
    {
      // We've reached the end of the file and there was no ends card
      break;
    }

    switch ( card )
    {

      case CT_OBJTYPE:
        // Object type
        if ( read( in, ( char * )&objecttype, 4 ) || objecttype != CT_2D_MESHES )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_SFLT:
        // Float size
        if ( read( in, ( char * )&sflt, 4 ) || sflt != CT_FLOAT_SIZE )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_SFLG:
        // Flag size
        if ( read( in, ( char * )&sflg, 4 ) )
          if ( sflg != CF_FLAG_SIZE && sflg != CF_FLAG_INT_SIZE )
            EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_BEGSCL:
        isVector = false;
        break;

      case CT_BEGVEC:
        isVector = true;
        break;

      case CT_VECTYPE:
        // Vector type
        if ( read( in, ( char * )&vectype, 4 ) || vectype != 0 )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_OBJID:
        // Object id
        if ( read( in, ( char * )&objid, 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_NUMDATA:
        // Num data
        if ( read( in, ( char * )&numdata, 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        if ( numdata != ( int ) vertexCount )
          EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
        break;

      case CT_NUMCELLS:
        // Num data
        if ( read( in, ( char * )&numcells, 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        if ( numcells != ( int ) elemCount )
          EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
        break;

      case CT_NAME:
        // Name
        if ( read( in, ( char * )&name, 40 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        if ( name[39] != 0 )
          name[39] = 0;
        baseDatasetName = trim( std::string( name ) );
        break;

      case CT_TS:
        // Time step!
        if ( readIStat( in, sflg, &istat ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

        if ( read( in, ( char * )&time, 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

        if ( readVertexTimestep( mesh, datOutputs, time, isVector, istat, sflg, in ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

        break;
    }
  }

  if ( datOutputs.size() == 0 )
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  addDatasets( mesh, baseDatasetName, datOutputs );
}


void MDAL::LoaderBinaryDat::addDatasets( MDAL::Mesh *mesh,
    const std::string &name,
    const std::vector<std::shared_ptr<Dataset>> &datOutputs ) const
{
  for ( const std::shared_ptr<Dataset> &ds : datOutputs )
  {
    ds->uri = mDatFile;
    std::string suffix = ds->name();
    ds->setName( name + suffix );
    ds->isValid = true;
  }

  //https://stackoverflow.com/a/2551785/2838364
  mesh->datasets.insert(
    mesh->datasets.end(),
    datOutputs.begin(),
    datOutputs.end()
  );
}

bool MDAL::LoaderBinaryDat::readVertexTimestep( const MDAL::Mesh *mesh, std::vector<std::shared_ptr<MDAL::Dataset> > &datOutputs, float time,
    bool isVector, bool hasStatus, int sflg, std::ifstream &in )
{
  size_t vertexCount = mesh->vertices.size();
  size_t faceCount = mesh->faces.size();

  std::shared_ptr<MDAL::Dataset> dataset( new MDAL::Dataset );
  dataset->isScalar = !isVector; //everything else to be set in addDatasets
  dataset->setMetadata( "time", std::to_string( time / 3600.0f ) );
  dataset->values.resize( vertexCount );
  dataset->active.resize( faceCount );
  dataset->isOnVertices = true;

  // name will be set properly in the addDatasets at the end.
  if ( time == 99999 ) // Special TUFLOW dataset with maximus
    dataset->setName( "/Maximums" );

  bool active = true;
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( hasStatus )
    {
      if ( readIStat( in, sflg, ( char * )&active ) )
        return true; //error

    }
    dataset->active[i] = active;
  }

  for ( size_t i = 0; i < vertexCount; ++i )
  {
    if ( isVector )
    {
      float x, y;

      if ( read( in, ( char * )&x, 4 ) )
        return true; //error
      if ( read( in, ( char * )&y, 4 ) )
        return true; //error

      dataset->values[i].x = x;
      dataset->values[i].y = y;
    }
    else
    {
      float scalar;

      if ( read( in, ( char * )&scalar, 4 ) )
        return true; //error

      dataset->values[i].x = scalar;
    }
  }

  datOutputs.push_back( std::move( dataset ) );

  return false; //OK
}
