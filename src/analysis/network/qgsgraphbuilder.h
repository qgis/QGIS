/***************************************************************************
  qgsgraphbuilder.h
  --------------------------------------
  Date                 : 2010-10-25
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef QGSGRAPHBUILDERH
#define QGSGRAPHBUILDERH

#include "qgsgraphbuilderintr.h"

//QT4 includes

//QGIS includes
#include <qgsspatialindex.h>

//forward declarations
class QgsDistanceArea;
class QgsCoordinateTransform;
class QgsGraph;

/**
* \ingroup analysis
* \class QgsGraphBuilder
* \brief This class making the QgsGraph object
*/

class ANALYSIS_EXPORT QgsGraphBuilder : public QgsGraphBuilderInterface
{
  private:
    /**
     * \class QgsPointCompare
     * \brief equivalence ratio
     */
    class QgsPointCompare
    {
      public:
        bool operator()( const QgsPoint& a, const QgsPoint& b ) const
        {
          return a.x() == b.x() ? a.y() < b.y() : a.x() < b.x();
        }
    };

  public:
    /**
     * default constructor
     */
    QgsGraphBuilder( const QgsCoordinateReferenceSystem& crs, const QgsDistanceArea& da, bool otfEnabled, double topologyTolerance = 0.0 );
    
    ~QgsGraphBuilder();

    /*
     * MANDATORY BUILDER PROPERTY DECLARATION
     */
    virtual QgsPoint addVertex( const QgsPoint& pt );

    virtual void addArc( const QgsPoint& pt1, const QgsPoint& pt2, const QVector< QVariant >& prop );

    /**
     * return QgsGraph result;
     */
    QgsGraph* graph();
  
  private:  
    // return -1 if pt not found
    int pointId( const QgsPoint& pt );

    QgsSpatialIndex mPointIndex;

    std::map< QgsPoint, int, QgsPointCompare > mPointMap;

    QgsGraph *mGraph;
};
#endif //QGSGRAPHBUILDERH
