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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrange.h"
#include "qgsrasterinterface.h"

#include <QList>

/**
 * \ingroup core
  * \brief Raster pipe that deals with null values.
*/
class CORE_EXPORT QgsRasterNuller : public QgsRasterInterface
{
  public:
    QgsRasterNuller( QgsRasterInterface *input = nullptr );

    struct NoData
    {
        double min;
        double max;
    };

    QgsRasterNuller *clone() const override SIP_FACTORY;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    void setNoData( int bandNo, const QgsRasterRangeList &noData );

    QgsRasterRangeList noData( int bandNo ) const { return mNoData.value( bandNo - 1 ); }

    //! Sets the output no data value.
    void setOutputNoDataValue( int bandNo, double noData );

  private:
    // no data indexed from 0
    QVector<QgsRasterRangeList> mNoData;
    // no data to be set in output, indexed from 0
    QVector<double> mOutputNoData;
    QVector<bool> mHasOutputNoData;
};

#endif // QGSRASTERNULLER_H
