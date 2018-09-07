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
//static const int CT_RT_JULIAN = 240;
//static const int CT_TIMEUNITS = 250;
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
    in.read( reinterpret_cast< char * >( &istat ), sflg );
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

  if ( read( in, reinterpret_cast< char * >( &version ), 4 ) )
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  if ( version != CT_VERSION ) // Version should be 3000
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(); // DAT datasets
  group->uri = mDatFile;
  group->isOnVertices = true;

  // in TUFLOW results there could be also a special timestep (99999) with maximums
  // we will put it into a separate dataset
  std::shared_ptr<DatasetGroup> groupMax = std::make_shared< DatasetGroup >();
  groupMax->uri = mDatFile;
  groupMax->isOnVertices = true;

  while ( card != CT_ENDDS )
  {
    if ( read( in, reinterpret_cast< char * >( &card ), 4 ) )
    {
      // We've reached the end of the file and there was no ends card
      break;
    }

    switch ( card )
    {

      case CT_OBJTYPE:
        // Object type
        if ( read( in, reinterpret_cast< char * >( &objecttype ), 4 ) || objecttype != CT_2D_MESHES )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_SFLT:
        // Float size
        if ( read( in, reinterpret_cast< char * >( &sflt ), 4 ) || sflt != CT_FLOAT_SIZE )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_SFLG:
        // Flag size
        if ( read( in, reinterpret_cast< char * >( &sflg ), 4 ) )
          if ( sflg != CF_FLAG_SIZE && sflg != CF_FLAG_INT_SIZE )
            EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_BEGSCL:
        group->isScalar = true;
        groupMax->isScalar = true;
        break;

      case CT_BEGVEC:
        group->isScalar = false;
        groupMax->isScalar = false;
        break;

      case CT_VECTYPE:
        // Vector type
        if ( read( in, reinterpret_cast< char * >( &vectype ), 4 ) || vectype != 0 )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_OBJID:
        // Object id
        if ( read( in, reinterpret_cast< char * >( &objid ), 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        break;

      case CT_NUMDATA:
        // Num data
        if ( read( in, reinterpret_cast< char * >( &numdata ), 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        if ( numdata != static_cast< int >( vertexCount ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
        break;

      case CT_NUMCELLS:
        // Num data
        if ( read( in, reinterpret_cast< char * >( &numcells ), 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        if ( numcells != static_cast< int >( elemCount ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_IncompatibleMesh );
        break;

      case CT_NAME:
        // Name
        if ( read( in, reinterpret_cast< char * >( &name ), 40 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );
        if ( name[39] != 0 )
          name[39] = 0;
        group->setName( trim( std::string( name ) ) );
        groupMax->setName( group->name() + "/Maximums" );
        break;

      case CT_TS:
        // Time step!
        if ( readIStat( in, sflg, &istat ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

        if ( read( in, reinterpret_cast< char * >( &time ), 4 ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

        double t = static_cast<double>( time );
        if ( readVertexTimestep( mesh, group, groupMax, t, istat, sflg, in ) )
          EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

        break;
    }
  }

  if ( !group || group->datasets.size() == 0 )
    EXIT_WITH_ERROR( MDAL_Status::Err_UnknownFormat );

  mesh->datasetGroups.push_back( group );
}

bool MDAL::LoaderBinaryDat::readVertexTimestep( const MDAL::Mesh *mesh,
    std::shared_ptr<DatasetGroup> group,
    std::shared_ptr<DatasetGroup> groupMax,
    double time,
    bool hasStatus,
    int sflg,
    std::ifstream &in )
{
  assert( group && groupMax && ( group->isScalar == groupMax->isScalar ) );
  bool isScalar = group->isScalar;

  size_t vertexCount = mesh->vertices.size();
  size_t faceCount = mesh->faces.size();

  std::shared_ptr<MDAL::Dataset> dataset = std::make_shared< MDAL::Dataset >();
  dataset->values.resize( vertexCount );
  dataset->active.resize( faceCount );
  dataset->parent = group.get();

  bool active = true;
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( hasStatus )
    {
      if ( readIStat( in, sflg, reinterpret_cast< char * >( &active ) ) )
        return true; //error

    }
    dataset->active[i] = active;
  }

  for ( size_t i = 0; i < vertexCount; ++i )
  {
    if ( !isScalar )
    {
      float x, y;

      if ( read( in, reinterpret_cast< char * >( &x ), 4 ) )
        return true; //error
      if ( read( in, reinterpret_cast< char * >( &y ), 4 ) )
        return true; //error

      dataset->values[i].x = static_cast< double >( x );
      dataset->values[i].y = static_cast< double >( y );
    }
    else
    {
      float scalar;

      if ( read( in, reinterpret_cast< char * >( &scalar ), 4 ) )
        return true; //error

      dataset->values[i].x = static_cast< double >( scalar );
    }
  }

  if ( MDAL::equals( time, 99999.0 ) ) // Special TUFLOW dataset with maximus
  {
    dataset->time = time;
    groupMax->datasets.push_back( dataset );
  }
  else
  {
    dataset->time = time; // TODO read TIMEUNITS
    group->datasets.push_back( dataset );
  }
  return false; //OK
}
