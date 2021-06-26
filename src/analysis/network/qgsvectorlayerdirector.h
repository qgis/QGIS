/***************************************************************************
  qgsvectorlayerdirector.h
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

#ifndef QGSVECTORLAYERDIRECTOR_H
#define QGSVECTORLAYERDIRECTOR_H

#include "qgis.h"

#include "qgsgraphdirector.h"
#include "qgis_analysis.h"

class QgsGraphBuilderInterface;
class QgsFeatureSource;

/**
* \ingroup analysis
* \class QgsVectorLayerDirector
* \brief Determine making the graph from vector line layer
* \since QGIS 3.0
*/
class ANALYSIS_EXPORT QgsVectorLayerDirector : public QgsGraphDirector
{
    Q_OBJECT

  public:

    /**
     * Edge direction
     * Edge can be one-way with direct flow (one can move only from the start
     * point to the end point), one-way with reversed flow (one can move only
     * from the end point to the start point) and bidirectional or two-way
     * (one can move in any direction)
     */
    enum Direction
    {
      DirectionForward,     //!< One-way direct
      DirectionBackward,    //!< One-way reversed
      DirectionBoth,        //!< Two-way
    };

    /**
     * Default constructor
     * \param source feature source representing network
     * \param directionFieldId field containing direction value
     * \param directDirectionValue value for direct one-way road
     * \param reverseDirectionValue value for reversed one-way road
     * \param bothDirectionValue value for two-way (bidirectional) road
     * \param defaultDirection default direction. Will be used if corresponding
     * attribute value is not set or does not equal to the given values
     */
    QgsVectorLayerDirector( QgsFeatureSource *source,
                            int directionFieldId,
                            const QString &directDirectionValue,
                            const QString &reverseDirectionValue,
                            const QString &bothDirectionValue,
                            Direction defaultDirection
                          );

    /*
     * MANDATORY DIRECTOR PROPERTY DECLARATION
     */
    void makeGraph( QgsGraphBuilderInterface *builder,
                    const QVector< QgsPointXY > &additionalPoints,
                    QVector< QgsPointXY> &snappedPoints SIP_OUT,
                    QgsFeedback *feedback = nullptr ) const override;

    QString name() const override;

  private:
    QgsFeatureSource *mSource = nullptr;
    int mDirectionFieldId = -1;
    QString mDirectDirectionValue;
    QString mReverseDirectionValue;
    QString mBothDirectionValue;
    Direction mDefaultDirection = DirectionBoth;

    QgsAttributeList requiredAttributes() const;
    Direction directionForFeature( const QgsFeature &feature ) const;
};

#endif // QGSVECTORLAYERDIRECTOR_H
