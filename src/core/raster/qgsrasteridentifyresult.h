/***************************************************************************
      qgsrasteridentifyresult.h
     --------------------------------------
    Date                 : Apr 8, 2013
    Copyright            : (C) 2013 by Radim Blazek
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

#ifndef QGSRASTERIDENTIFYRESULT_H
#define QGSRASTERIDENTIFYRESULT_H

#include "qgis.h"
#include "qgslogger.h"
#include "qgsrasterdataprovider.h"
#include "qgsraster.h"

/** \ingroup core
 * Raster identify results container.
 */
class CORE_EXPORT QgsRasterIdentifyResult
{
  public:
    QgsRasterIdentifyResult();

    /** \brief Constructor. Creates valid result.
     *  @param theFormat the result format
     *  @param theResults the results
     */
    QgsRasterIdentifyResult( QgsRaster::IdentifyFormat theFormat, QMap<int, QVariant> theResults );

    /** \brief Constructor. Creates invalid result with error.
     *  @param theError the error
     */
    QgsRasterIdentifyResult( QgsError theError );

    virtual ~QgsRasterIdentifyResult();

    /** \brief Returns true if valid */
    bool isValid() const { return mValid; }

    /** \brief Get results format */
    QgsRaster::IdentifyFormat format() const { return mFormat; }

    /** \brief Get results. Results are different for each format:
     * QgsRaster::IdentifyFormatValue: map of values for each band, keys are band numbers (from 1).
     * QgsRaster::IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer (WMS)
     * QgsRaster::IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
     */
    QMap<int, QVariant> results() const { return mResults; }

    /** Set map of optional parameters */
    void setParams( const QMap<QString, QVariant> & theParams ) { mParams = theParams; }

    /** Get map of optional parameters */
    QMap<QString, QVariant> params() const { return mParams; }

    /** \brief Get error */
    QgsError error() const { return mError; }

    /** \brief Set error */
    void setError( const QgsError & theError ) { mError = theError;}

  private:
    /** \brief Is valid */
    bool mValid;

    /** \brief Results format */
    QgsRaster::IdentifyFormat mFormat;

    /** \brief Results */
    // TODO: better hierarchy (sublayer multiple feature sets)?
    // TODO?: results are not consistent for different formats (per band x per sublayer)
    QMap<int, QVariant> mResults;

    /** \brief Additional params (e.g. request url used by WMS) */
    QMap<QString, QVariant> mParams;

    /** \brief Error */
    QgsError mError;
};

#endif


