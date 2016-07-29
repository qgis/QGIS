/***************************************************************************
               qgscoordinatetransform_p.h
               --------------------------
    begin                : July 2016
    copyright            : (C) 2016 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATETRANSFORMPRIVATE_H
#define QGSCOORDINATETRANSFORMPRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QSharedData>
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsapplication.h"

extern "C"
{
#include <proj_api.h>
}
#include <sqlite3.h>

#include <QStringList>

class QgsCoordinateTransformPrivate : public QSharedData
{

  public:

    explicit QgsCoordinateTransformPrivate()
        : mIsValid( false )
        , mShortCircuit( false )
        , mSourceProjection( nullptr )
        , mDestinationProjection( nullptr )
        , mSourceDatumTransform( -1 )
        , mDestinationDatumTransform( -1 )
    {
      setFinder();
    }

    QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem& source,
                                   const QgsCoordinateReferenceSystem& destination )
        : mIsValid( false )
        , mShortCircuit( false )
        , mSourceCRS( source )
        , mDestCRS( destination )
        , mSourceProjection( nullptr )
        , mDestinationProjection( nullptr )
        , mSourceDatumTransform( -1 )
        , mDestinationDatumTransform( -1 )
    {
      setFinder();
      initialise();
    }

    QgsCoordinateTransformPrivate( const QgsCoordinateTransformPrivate& other )
        : QSharedData( other )
        , mIsValid( other.mIsValid )
        , mShortCircuit( other.mShortCircuit )
        , mSourceCRS( other.mSourceCRS )
        , mDestCRS( other.mDestCRS )
        , mSourceProjection( nullptr )
        , mDestinationProjection( nullptr )
        , mSourceDatumTransform( other.mSourceDatumTransform )
        , mDestinationDatumTransform( other.mDestinationDatumTransform )
    {
      //must reinitialize to setup mSourceProjection and mDestinationProjection
      initialise();
    }

    ~QgsCoordinateTransformPrivate()
    {
      // free the proj objects
      if ( mSourceProjection )
      {
        pj_free( mSourceProjection );
      }
      if ( mDestinationProjection )
      {
        pj_free( mDestinationProjection );
      }
    }

    bool initialise()
    {
      mShortCircuit = true;
      mIsValid = false;

      if ( !mSourceCRS.isValid() )
      {
        // Pass through with no projection since we have no idea what the layer
        // coordinates are and projecting them may not be appropriate
        QgsDebugMsg( "Source CRS is invalid!" );
        return false;
      }

      if ( !mDestCRS.isValid() )
      {
        //No destination projection is set so we set the default output projection to
        //be the same as input proj.
        mDestCRS = mSourceCRS;
        QgsDebugMsg( "Destination CRS is invalid!" );
        return false;
      }

      mIsValid = true;

      bool useDefaultDatumTransform = ( mSourceDatumTransform == - 1 && mDestinationDatumTransform == -1 );

      // init the projections (destination and source)

      pj_free( mSourceProjection );
      QString sourceProjString = mSourceCRS.toProj4();
      if ( !useDefaultDatumTransform )
      {
        sourceProjString = stripDatumTransform( sourceProjString );
      }
      if ( mSourceDatumTransform != -1 )
      {
        sourceProjString += ( ' ' + datumTransformString( mSourceDatumTransform ) );
      }

      pj_free( mDestinationProjection );
      QString destProjString = mDestCRS.toProj4();
      if ( !useDefaultDatumTransform )
      {
        destProjString = stripDatumTransform( destProjString );
      }
      if ( mDestinationDatumTransform != -1 )
      {
        destProjString += ( ' ' +  datumTransformString( mDestinationDatumTransform ) );
      }

      if ( !useDefaultDatumTransform )
      {
        addNullGridShifts( sourceProjString, destProjString );
      }

      mSourceProjection = pj_init_plus( sourceProjString.toUtf8() );
      mDestinationProjection = pj_init_plus( destProjString.toUtf8() );

#ifdef COORDINATE_TRANSFORM_VERBOSE
      QgsDebugMsg( "From proj : " + mSourceCRS.toProj4() );
      QgsDebugMsg( "To proj   : " + mDestCRS.toProj4() );
#endif

      if ( !mDestinationProjection || !mSourceProjection )
      {
        mIsValid = false;
      }

#ifdef COORDINATE_TRANSFORM_VERBOSE
      if ( mIsValid )
      {
        QgsDebugMsg( "------------------------------------------------------------" );
        QgsDebugMsg( "The OGR Coordinate transformation for this layer was set to" );
        QgsLogger::debug<QgsCoordinateReferenceSystem>( "Input", mSourceCRS, __FILE__, __FUNCTION__, __LINE__ );
        QgsLogger::debug<QgsCoordinateReferenceSystem>( "Output", mDestCRS, __FILE__, __FUNCTION__, __LINE__ );
        QgsDebugMsg( "------------------------------------------------------------" );
      }
      else
      {
        QgsDebugMsg( "------------------------------------------------------------" );
        QgsDebugMsg( "The OGR Coordinate transformation FAILED TO INITIALISE!" );
        QgsDebugMsg( "------------------------------------------------------------" );
      }
#else
      if ( !mIsValid )
      {
        QgsDebugMsg( "Coordinate transformation failed to initialize!" );
      }
#endif

      //XXX todo overload == operator for QgsCoordinateReferenceSystem
      //at the moment srs.parameters contains the whole proj def...soon it wont...
      //if (mSourceCRS->toProj4() == mDestCRS->toProj4())
      if ( mSourceCRS == mDestCRS )
      {
        // If the source and destination projection are the same, set the short
        // circuit flag (no transform takes place)
        mShortCircuit = true;
        QgsDebugMsgLevel( "Source/Dest CRS equal, shortcircuit is set.", 3 );
      }
      else
      {
        // Transform must take place
        mShortCircuit = false;
        QgsDebugMsgLevel( "Source/Dest CRS not equal, shortcircuit is not set.", 3 );
      }
      return mIsValid;
    }

    /** Removes +nadgrids and +towgs84 from proj4 string*/
    QString stripDatumTransform( const QString& proj4 ) const
    {
      QStringList parameterSplit = proj4.split( '+', QString::SkipEmptyParts );
      QString currentParameter;
      QString newProjString;

      for ( int i = 0; i < parameterSplit.size(); ++i )
      {
        currentParameter = parameterSplit.at( i );
        if ( !currentParameter.startsWith( "towgs84", Qt::CaseInsensitive )
             && !currentParameter.startsWith( "nadgrids", Qt::CaseInsensitive ) )
        {
          newProjString.append( '+' );
          newProjString.append( currentParameter );
          newProjString.append( ' ' );
        }
      }
      return newProjString;
    }

    static QString datumTransformString( int datumTransform )
    {
      QString transformString;

      sqlite3* db;
      int openResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().constData(), &db, SQLITE_OPEN_READONLY, 0 );
      if ( openResult != SQLITE_OK )
      {
        sqlite3_close( db );
        return transformString;
      }

      sqlite3_stmt* stmt;
      QString sql = QString( "SELECT coord_op_method_code,p1,p2,p3,p4,p5,p6,p7 FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( datumTransform );
      int prepareRes = sqlite3_prepare( db, sql.toAscii(), sql.size(), &stmt, nullptr );
      if ( prepareRes != SQLITE_OK )
      {
        sqlite3_finalize( stmt );
        sqlite3_close( db );
        return transformString;
      }

      if ( sqlite3_step( stmt ) == SQLITE_ROW )
      {
        //coord_op_methode_code
        int methodCode = sqlite3_column_int( stmt, 0 );
        if ( methodCode == 9615 ) //ntv2
        {
          transformString = "+nadgrids=" + QString( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 1 ) ) );
        }
        else if ( methodCode == 9603 || methodCode == 9606 || methodCode == 9607 )
        {
          transformString += "+towgs84=";
          double p1 = sqlite3_column_double( stmt, 1 );
          double p2 = sqlite3_column_double( stmt, 2 );
          double p3 = sqlite3_column_double( stmt, 3 );
          double p4 = sqlite3_column_double( stmt, 4 );
          double p5 = sqlite3_column_double( stmt, 5 );
          double p6 = sqlite3_column_double( stmt, 6 );
          double p7 = sqlite3_column_double( stmt, 7 );
          if ( methodCode == 9603 ) //3 parameter transformation
          {
            transformString += QString( "%1,%2,%3" ).arg( p1 ).arg( p2 ).arg( p3 );
          }
          else //7 parameter transformation
          {
            transformString += QString( "%1,%2,%3,%4,%5,%6,%7" ).arg( p1 ).arg( p2 ).arg( p3 ).arg( p4 ).arg( p5 ).arg( p6 ).arg( p7 );
          }
        }
      }

      sqlite3_finalize( stmt );
      sqlite3_close( db );
      return transformString;
    }

    /** In certain situations, null grid shifts have to be added to src / dst proj string*/
    void addNullGridShifts( QString& srcProjString, QString& destProjString ) const
    {
      //if one transformation uses ntv2, the other one needs to be null grid shift
      if ( mDestinationDatumTransform == -1 && srcProjString.contains( "+nadgrids" ) ) //add null grid if source transformation is ntv2
      {
        destProjString += " +nadgrids=@null";
        return;
      }
      if ( mSourceDatumTransform == -1 && destProjString.contains( "+nadgrids" ) )
      {
        srcProjString += " +nadgrids=@null";
        return;
      }

      //add null shift grid for google mercator
      //(see e.g. http://trac.osgeo.org/proj/wiki/FAQ#ChangingEllipsoidWhycantIconvertfromWGS84toGoogleEarthVirtualGlobeMercator)
      if ( mSourceCRS.authid().compare( "EPSG:3857", Qt::CaseInsensitive ) == 0 && mSourceDatumTransform == -1 )
      {
        srcProjString += " +nadgrids=@null";
      }
      if ( mDestCRS.authid().compare( "EPSG:3857", Qt::CaseInsensitive ) == 0 && mDestinationDatumTransform == -1 )
      {
        destProjString += " +nadgrids=@null";
      }
    }


    //! Flag to indicate whether the transform is valid (ie has a valid
    //! source and destination crs)
    bool mIsValid;

    /**
     * Flag to indicate that the source and destination coordinate systems are
     * equal and not transformation needs to be done
     */
    bool mShortCircuit;

    //! QgsCoordinateReferenceSystem of the source (layer) coordinate system
    QgsCoordinateReferenceSystem mSourceCRS;

    //! QgsCoordinateReferenceSystem of the destination (map canvas) coordinate system
    QgsCoordinateReferenceSystem mDestCRS;

    //! Proj4 data structure of the source projection (layer coordinate system)
    projPJ mSourceProjection;

    //! Proj4 data structure of the destination projection (map canvas coordinate system)
    projPJ mDestinationProjection;

    int mSourceDatumTransform;
    int mDestinationDatumTransform;

    void setFinder()
    {
#if 0
      // Attention! It should be possible to set PROJ_LIB
      // but it can happen that it was previously set by installer
      // (version 0.7) and the old installation was deleted

      // Another problem: PROJ checks if pj_finder was set before
      // PROJ_LIB environment variable. pj_finder is probably set in
      // GRASS gproj library when plugin is loaded, consequently
      // PROJ_LIB is ignored

      pj_set_finder( finder );
#endif
    }
};

/// @endcond

#endif // QGSCOORDINATETRANSFORMPRIVATE_H
