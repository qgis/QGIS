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
* \ingroup networkanalysis
* \class QgsGraphBuilder
* \brief This class making the QgsGraph object
*/

class ANALYSIS_EXPORT QgsGraphBuilder : public QgsGraphBuilderInterface
{
  public:
    /**
     * default constructor
     */
    QgsGraphBuilder( const QgsCoordinateReferenceSystem& crs, bool otfEnabled = true, double topologyTolerance = 0.0, const QString& ellipsoidID = "WGS84" );

    ~QgsGraphBuilder();

    /*
     * MANDATORY BUILDER PROPERTY DECLARATION
     */
    virtual void addVertex( int id, const QgsPoint& pt );

    virtual void addArc( int pt1id, const QgsPoint& pt1, int pt2id, const QgsPoint& pt2, const QVector< QVariant >& prop );

    /**
     * return QgsGraph result;
     */
    QgsGraph* graph();

  private:

    QgsGraph *mGraph;
};
#endif //QGSGRAPHBUILDERH
