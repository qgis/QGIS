/***************************************************************************
  qgsgraphbuilder.h
  --------------------------------------
  Date                 : 2010-10-22
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef QGSGRAPHBUILDERINTERFACE
#define QGSGRAPHBUILDERINTERFACE

//QT4 includes
#include <QVector>
#include <QVariant>

//QGIS includes
#include <qgspoint.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsdistancearea.h>

//forward declarations

/**
* \ingroup networkanalysis
* \class QgsGraphBuilderInterface
* \brief Determine interface for creating a graph. Contains the settings of the graph. QgsGraphBuilder and QgsGraphDirector is a Builder pattern
*/
class ANALYSIS_EXPORT QgsGraphBuilderInterface
{
  public:
    /**
     * QgsGraphBuilderInterface constructor
     * @param crs Coordinate reference system for new graph vertex
     * @param ctfEnabled enable coordinate transform from source graph CRS to CRS graph
     * @param topologyTolerance sqrt distance between source point as one graph vertex
     * @param ellipsoidID ellipsoid for edge measurement
     */
    QgsGraphBuilderInterface( const QgsCoordinateReferenceSystem& crs, bool ctfEnabled = true, double topologyTolerance = 0.0, const QString& ellipsoidID = "WGS84" ) :
        mCrs( crs ), mCtfEnabled( ctfEnabled ), mTopologyTolerance( topologyTolerance )
    {
      mDa.setSourceCrs( mCrs.srsid() );
      mDa.setEllipsoid( ellipsoidID );
      mDa.setEllipsoidalEnabled( ctfEnabled );
    }

    //! Destructor
    virtual ~QgsGraphBuilderInterface()
    { }

    //! get destinaltion Crs
    QgsCoordinateReferenceSystem& destinationCrs()
    {
      return mCrs;
    }

    //! get coordinate transformation enabled
    bool coordinateTransformationEnabled()
    {
      return mCtfEnabled;
    }

    //! get topology tolerance
    double topologyTolerance()
    {
      return mTopologyTolerance;
    }

    //! get measurement tool
    QgsDistanceArea* distanceArea()
    {
      return &mDa;
    }

    /**
     * add vertex
     * @param id vertex identifier
     * @param pt vertex coordinate
     * @note id and pt are redundant. You can use pt or id to identify the vertex
     */
    virtual void addVertex( int id, const QgsPoint &pt )
    {
      Q_UNUSED( id );
      Q_UNUSED( pt );
    }

    /**
     * add arc
     * @param pt1id first vertex identificator
     * @param pt1   first vertex coordinate
     * @param pt2id second vertex identificator
     * @param pt2   second vertex coordinate
     * @param properties arc properties
     * @note pt1id, pt1 and pt2id, pt2 is a redundant interface. You can use vertex coordinates or their identificators.
     */
    virtual void addArc( int pt1id, const QgsPoint& pt1, int pt2id, const QgsPoint& pt2, const QVector< QVariant >& properties )
    {
      Q_UNUSED( pt1id );
      Q_UNUSED( pt1 );
      Q_UNUSED( pt2id );
      Q_UNUSED( pt2 );
      Q_UNUSED( properties );
    }

  private:
    QgsCoordinateReferenceSystem mCrs;

    QgsDistanceArea mDa;

    bool mCtfEnabled;

    double mTopologyTolerance;

};
#endif //QGSGRAPHBUILDERINTERFACE
