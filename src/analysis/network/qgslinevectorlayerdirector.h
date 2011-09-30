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
#ifndef QGSLINEVECTORLAYERDIRECTORH
#define QGSLINEVECTORLAYERDIRECTORH

//QT4 includes

//QGIS includes

// Road-graph plugin includes
#include "qgsgraphdirector.h"

//forward declarations
class QgsGraphBuilderInterface;
class QgsVectorLayer;

/**
* \ingroup networkanalysis
* \class QgsLineVectorLayerDirector
* \brief Determine making the graph from vector line layer
*/
class QgsLineVectorLayerDirector : public QgsGraphDirector
{
  public:
    /**
     * @param vl  source vector layer
     * @param directionFieldId feield contain road direction value
     * @param directDirectionValue value for one-way road
     * @param reverseDirectionValue value for reverse one-way road
     * @param bothDirectionValue value for road
     * @param defaultDirection 1 - direct direction, 2 - reverse direction, 3 - both direction
     */
    QgsLineVectorLayerDirector( QgsVectorLayer* vl,
                                int directionFieldId,
                                const QString& directDirectionValue,
                                const QString& reverseDirectionValue,
                                const QString& bothDirectionValue,
                                int defaultDirection
                              );

    //! Destructor
    virtual ~QgsLineVectorLayerDirector();

    /*
     * MANDATORY DIRECTOR PROPERTY DECLARATION
     */
    void makeGraph( QgsGraphBuilderInterface *builder,
                    const QVector< QgsPoint >& additionalPoints,
                    QVector< QgsPoint>& tiedPoints ) const;

    QString name() const;


  private:

    QgsVectorLayer *mVectorLayer;

    int mDirectionFieldId;

    QString mDirectDirectionValue;

    QString mReverseDirectionValue;

    QString mBothDirectionValue;

    //FIXME: need enum
    int mDefaultDirection;
};

#endif //QGSLINEVECTORLAYERGRAPHDIRECTORH
