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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsraster.h"
#include "qgserror.h"

/**
 * \ingroup core
 * Raster identify results container.
 */
class CORE_EXPORT QgsRasterIdentifyResult
{
  public:

    /**
     * Constructor for QgsRasterIdentifyResult.
     */
    QgsRasterIdentifyResult() = default;

    /**
     * \brief Constructor. Creates valid result.
     *  \param format the result format
     *  \param results the results
     */
    QgsRasterIdentifyResult( QgsRaster::IdentifyFormat format, const QMap<int, QVariant> &results );

    /**
     * \brief Constructor. Creates invalid result with error.
     *  \param error the error
     */
    QgsRasterIdentifyResult( const QgsError &error );

    virtual ~QgsRasterIdentifyResult() = default;

    //! \brief Returns TRUE if valid
    bool isValid() const { return mValid; }

    //! Returns the results format.
    QgsRaster::IdentifyFormat format() const { return mFormat; }

    /**
     * Returns the identify results. Results are different for each format:
     * QgsRaster::IdentifyFormatValue: map of values for each band, keys are band numbers (from 1).
     * QgsRaster::IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer (WMS)
     * QgsRaster::IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
     */
    QMap<int, QVariant> results() const { return mResults; }

    //! Sets map of optional parameters
    void setParams( const QMap<QString, QVariant> &params ) { mParams = params; }

    //! Gets map of optional parameters
    QMap<QString, QVariant> params() const { return mParams; }

    //! Returns the last error
    QgsError error() const { return mError; }

    //! Sets the last error
    void setError( const QgsError &error ) { mError = error;}

  private:
    //! \brief Is valid
    bool mValid = false;

    //! \brief Results format
    QgsRaster::IdentifyFormat mFormat = QgsRaster::IdentifyFormatUndefined;

    //! \brief Results
    // TODO: better hierarchy (sublayer multiple feature sets)?
    // TODO?: results are not consistent for different formats (per band x per sublayer)
    QMap<int, QVariant> mResults;

    //! \brief Additional params (e.g. request url used by WMS)
    QMap<QString, QVariant> mParams;

    //! \brief Error
    QgsError mError;
};

#endif


