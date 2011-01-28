/***************************************************************************
  linevectorlayerdirector.h
  --------------------------------------
  Date                 : 2010-10-20
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
#ifndef ROADGRAPH_LINEVECTORLAYERDIRECTOR
#define ROADGRAPH_LINEVECTORLAYERDIRECTOR

//QT4 includes

//QGIS includes

// Road-graph plugin includes
#include "graphdirector.h"

//forward declarations
class RgGraphBuilder;
class QgsVectorLayer;

/**
* \class RgLineVectorLayerDirector
* \brief Determine making the graph from vector line layer
*/
class RgLineVectorLayerDirector : public RgGraphDirector
{
  public:
    RgLineVectorLayerDirector( const QString& layerId, 
                               int directionFiledId,
                               const QString& directDirectionValue,
                               const QString& reverseDirectionValue,
                               const QString& bothDirectionValue,
                               int defaultDirection, 
                               const QString& speedValueUnit = QString("m/s"),
                               int speedFieldId = -1,
                               double defaultSpeed = 1.0 );

    //! Destructor
    virtual ~RgLineVectorLayerDirector();
    /**
     * MANDATORY DIRECTOR PROPERTY DECLARATION
     */
    void makeGraph( RgGraphBuilder * ) const;

    QString name() const;
  
  private:

    QgsVectorLayer* myLayer() const;

  private:
  
    QString mLayerId;

    int mSpeedFieldId;

    double mDefaultSpeed;
    
    QString mSpeedUnitName;

    int mDirectionFieldId;

    QString mDirectDirectionValue;

    QString mReverseDirectionValue;

    QString mBothDirectionValue;
    
    //FIXME: need enum
    int mDefaultDirection;
};
#endif //GRAPHDIRECTOR
