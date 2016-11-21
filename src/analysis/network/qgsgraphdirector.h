/***************************************************************************
  qgsgraphdirector.h
  --------------------------------------
  Date                 : 2010-10-18
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

#ifndef QGSGRAPHDIRECTOR_H
#define QGSGRAPHDIRECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

#include <qgspoint.h>
#include "qgsnetworkstrategy.h"

class QgsGraphBuilderInterface;

/**
 * \ingroup analysis
 * \class QgsGraphDirector
 * \brief Determine making the graph. QgsGraphBuilder and QgsGraphDirector implemented
 * using "builder" design patter.
 */
class ANALYSIS_EXPORT QgsGraphDirector : public QObject
{
    Q_OBJECT

  signals:
    void buildProgress( int, int ) const;
    void buildMessage( const QString& ) const;

  public:
    //! Destructor
    virtual ~QgsGraphDirector() { }

    /**
     * Make a graph using QgsGraphBuilder
     *
     * @param builder the graph builder
     * @param additionalPoints list of points that should be snapped to the graph
     * @param snappedPoints list of snapped points
     * @note if snappedPoints[i] == QgsPoint(0.0,0.0) then snapping failed.
     */
    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPoint > &additionalPoints,
                            QVector< QgsPoint > &snappedPoints ) const
    {
      Q_UNUSED( builder );
      Q_UNUSED( additionalPoints );
      Q_UNUSED( snappedPoints );
    }

    //! Add optimization strategy
    void addStrategy( QgsNetworkStrategy* prop )
    {
      mStrategies.push_back( prop );
    }

    //! Returns director name
    virtual QString name() const = 0;

  protected:
    QList<QgsNetworkStrategy*> mStrategies;
};

#endif // QGSGRAPHDIRECTOR_H
