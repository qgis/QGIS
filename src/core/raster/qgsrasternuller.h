/***************************************************************************
                         qgsrasternuller.h
                         -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERNULLER_H
#define QGSRASTERNULLER_H

#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"

#include <QList>

/** \ingroup core
  * Raster pipe that deals with null values.
*/
class CORE_EXPORT QgsRasterNuller : public QgsRasterInterface
{
  public:
    QgsRasterNuller( QgsRasterInterface* input = 0 );
    ~QgsRasterNuller();

    struct NoData
    {
      double min;
      double max;
    };

    QgsRasterInterface * clone() const;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    QgsRasterBlock* block( int bandNo, const QgsRectangle &extent, int width, int height );

    void setNoData( QList<QgsRasterNuller::NoData> noData ) { mNoData = noData; }

    QList<QgsRasterNuller::NoData> noData() const { return mNoData; }

    /** \brief Set output no data value. */
    void setOutputNoData( double noData ) { mOutputNoData = noData; }

  private:
    QList<QgsRasterNuller::NoData> mNoData;
    // no data to be set in output
    double mOutputNoData;
};

#endif // QGSRASTERNULLER_H
