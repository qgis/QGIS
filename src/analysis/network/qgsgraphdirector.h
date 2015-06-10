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
#ifndef QGSGRAPHDIRECTORH
#define QGSGRAPHDIRECTORH

//QT4 includes
#include <QObject>
#include <QVector>
#include <QList>

//QGIS includes
#include <qgspoint.h>
#include "qgsarcproperter.h"

//forward declarations
class QgsGraphBuilderInterface;

/**
 * \ingroup networkanalysis
 * \class QgsGraphDirector
 * \brief Determine making the graph. QgsGraphBuilder and QgsGraphDirector is a builder patter.
 */
class ANALYSIS_EXPORT QgsGraphDirector : public QObject
{
    Q_OBJECT

  signals:
    void buildProgress( int, int ) const;
    void buildMessage( QString ) const;

  public:
    //! Destructor
    virtual ~QgsGraphDirector() { }

    /**
     * Make a graph using RgGraphBuilder
     *
     * @param builder   The graph builder
     *
     * @param additionalPoints  Vector of points that must be tied to the graph
     *
     * @param tiedPoints  Vector of tied points
     *
     * @note if tiedPoints[i]==QgsPoint(0.0,0.0) then tied failed.
     */
    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPoint > &additionalPoints,
                            QVector< QgsPoint > &tiedPoints ) const
    {
      Q_UNUSED( builder );
      Q_UNUSED( additionalPoints );
      Q_UNUSED( tiedPoints );
    }

    void addProperter( QgsArcProperter* prop )
    {
      mProperterList.push_back( prop );
    }

    /**
     * return Director name
     */
    virtual QString name() const = 0;

  protected:
    QList<QgsArcProperter*> mProperterList;
};
#endif //QGSGRAPHDIRECTORH
