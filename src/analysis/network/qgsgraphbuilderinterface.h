/***************************************************************************
  qgsgraphbuilderinterface.h
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

#ifndef QGSGRAPHBUILDERINTERFACE_H
#define QGSGRAPHBUILDERINTERFACE_H

#include <QVector>
#include <QVariant>

#include "qgscoordinatereferencesystem.h"
#include "qgsdistancearea.h"
#include "qgis_analysis.h"

class QgsPoint;

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsgraphbuilder.h>
% End
#endif

/**
* \ingroup analysis
* \class QgsGraphBuilderInterface
* \brief Determine interface for creating a graph. Contains the settings of the graph.
* QgsGraphBuilder and QgsGraphDirector both use a "builder" design pattern
*/
class ANALYSIS_EXPORT QgsGraphBuilderInterface
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsGraphBuilder * >( sipCpp ) != NULL )
      sipType = sipType_QgsGraphBuilder;
    else
      sipType = NULL;
    SIP_END
#endif

  public:

    /**
     * Default constructor
     * \param crs Coordinate reference system for new graph vertex
     * \param ctfEnabled enable coordinate transform from source graph CRS to CRS graph
     * \param topologyTolerance sqrt distance between source point as one graph vertex
     * \param ellipsoidID ellipsoid for edge measurement
     */
    QgsGraphBuilderInterface( const QgsCoordinateReferenceSystem &crs, bool ctfEnabled = true,
                              double topologyTolerance = 0.0, const QString &ellipsoidID = "WGS84" );

    virtual ~QgsGraphBuilderInterface() = default;

    //! Returns destinaltion CRS
    QgsCoordinateReferenceSystem destinationCrs() const
    {
      return mCrs;
    }

    //! Returns coordinate transformation enabled
    bool coordinateTransformationEnabled() const
    {
      return mCtfEnabled;
    }

    //! Returns topology tolerance
    double topologyTolerance() const
    {
      return mTopologyTolerance;
    }

    //! Returns measurement tool
    QgsDistanceArea *distanceArea()
    {
      return &mDa;
    }

    /**
     * Add vertex to the graph
     * \param id vertex identifier
     * \param pt vertex coordinates
     * \note id and pt are redundant. You can use pt or id to identify the vertex
     */
    virtual void addVertex( int id, const QgsPointXY &pt );

    /**
     * Add edge to the graph
     * \param pt1id first vertex identificator
     * \param pt1   first vertex coordinates
     * \param pt2id second vertex identificator
     * \param pt2   second vertex coordinates
     * \param strategies optimization strategies
     * \note pt1id, pt1 and pt2id, pt2 is a redundant interface. You can use vertex coordinates or their identificators.
     */
    virtual void addEdge( int pt1id, const QgsPointXY &pt1, int pt2id, const QgsPointXY &pt2, const QVector< QVariant > &strategies );

  private:
    QgsCoordinateReferenceSystem mCrs;

    QgsDistanceArea mDa;

    bool mCtfEnabled;

    double mTopologyTolerance;

};

// clazy:excludeall=qstring-allocations

#endif // QGSGRAPHBUILDERINTERFACE_H
