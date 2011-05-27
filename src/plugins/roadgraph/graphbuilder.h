/***************************************************************************
  graphbuilder.h
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
#ifndef ROADGRAPH_GRAPHBUILDER
#define ROADGRAPH_GRAPHBUILDER

#include "utils.h"

//QT4 includes

//QGIS includes
#include <qgspoint.h>
#include <qgscoordinatereferencesystem.h>

//forward declarations

/**
* \class RgGraphDirector
* \brief Determine making the graph, contained the settings
*/
class RgGraphBuilder
{
  public:
    //! Constructor
    RgGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool coordinateTransform, double topologyTolerance = 0.0 );

    //! Destructor
    virtual ~RgGraphBuilder();

    /**
     * get destinaltion Crs
     */
    QgsCoordinateReferenceSystem& destinationCrs();

    /**
     * get topology tolerance factor
     */
    double topologyTolerance();

    /**
     * coordinate transform Enabled
     */
    bool coordinateTransformEnabled() const;

    /**
     * add vertex
     */
    virtual QgsPoint addVertex( const QgsPoint& pt ) = 0;

    /**
     * add arc
     */
    virtual void addArc( const QgsPoint& pt1, const QgsPoint& pt2, double cost, double speed, int featureId ) = 0;

  private:
    QgsCoordinateReferenceSystem mCrs;

    double mTopologyToleraceFactor;

    bool mCoordinateTransformEnabled;
};
#endif //GRAPHBUILDER
