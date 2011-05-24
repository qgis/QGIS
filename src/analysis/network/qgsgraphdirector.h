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
#include "qgsedgeproperter.h"

//forward declarations
class QgsGraphBuilderInterface;

/**
 * \ingroup analysis
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
    virtual ~QgsGraphDirector() { };

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
    virtual void makeGraph( QgsGraphBuilderInterface* builder,
                            const QVector< QgsPoint >& additionalPoints,
                            QVector< QgsPoint>& tiedPoints ) const = 0;
    
    void addProperter( QgsEdgeProperter* prop )
    {
      mProperterList.push_back( prop );
    }
    
    /**
     * return Director name
     */
    virtual QString name() const = 0;
  
  protected:
    QList<QgsEdgeProperter*> mProperterList;
};
#endif //QGSGRAPHDIRECTORH
