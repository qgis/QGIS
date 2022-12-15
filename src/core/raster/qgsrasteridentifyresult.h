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
 * \brief Raster identify results container.
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
    QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat format, const QMap<int, QVariant> &results );

    /**
     * \brief Constructor. Creates invalid result with error.
     *  \param error the error
     */
    QgsRasterIdentifyResult( const QgsError &error );

    virtual ~QgsRasterIdentifyResult() = default;

    //! \brief Returns TRUE if valid
    bool isValid() const { return mValid; }

    //! Returns the results format.
    Qgis::RasterIdentifyFormat format() const { return mFormat; }

    /**
     * Returns the identify results. Results are different for each format:
     *
     * - QgsRaster::IdentifyFormatValue: a map of values for each band, where keys are band numbers (from 1).
     * - QgsRaster::IdentifyFormatFeature: a map of WMS sublayer keys and lists of QgsFeatureStore values.
     * - QgsRaster::IdentifyFormatHtml: a map of WMS sublayer keys and HTML strings.
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
    Qgis::RasterIdentifyFormat mFormat = Qgis::RasterIdentifyFormat::Undefined;

    // TODO: better hierarchy (sublayer multiple feature sets)?
    // TODO?: results are not consistent for different formats (per band x per sublayer)

    //! \brief Results
    QMap<int, QVariant> mResults;

    //! \brief Additional params (e.g. request url used by WMS)
    QMap<QString, QVariant> mParams;

    //! \brief Error
    QgsError mError;
};

#endif


