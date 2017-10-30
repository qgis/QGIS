/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasteridentifyresult.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRasterIdentifyResult
{
%Docstring
 Raster identify results container.
%End

%TypeHeaderCode
#include "qgsrasteridentifyresult.h"
%End
  public:

    QgsRasterIdentifyResult();
%Docstring
 Constructor for QgsRasterIdentifyResult.
%End

    QgsRasterIdentifyResult( QgsRaster::IdentifyFormat format, const QMap<int, QVariant> &results );
%Docstring
 Constructor. Creates valid result.
  \param format the result format
  \param results the results
%End

    QgsRasterIdentifyResult( const QgsError &error );
%Docstring
 Constructor. Creates invalid result with error.
  \param error the error
%End

    virtual ~QgsRasterIdentifyResult();

    bool isValid() const;
%Docstring
 Returns true if valid
 :rtype: bool
%End

    QgsRaster::IdentifyFormat format() const;
%Docstring
 Get results format
 :rtype: QgsRaster.IdentifyFormat
%End

    QMap<int, QVariant> results() const;
%Docstring
 Get results. Results are different for each format:
 QgsRaster.IdentifyFormatValue: map of values for each band, keys are band numbers (from 1).
 QgsRaster.IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer (WMS)
 QgsRaster.IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
 :rtype: QMap<int, QVariant>
%End

    void setParams( const QMap<QString, QVariant> &params );
%Docstring
Set map of optional parameters
%End

    QMap<QString, QVariant> params() const;
%Docstring
Get map of optional parameters
 :rtype: QMap<str, QVariant>
%End

    QgsError error() const;
%Docstring
 Get error
 :rtype: QgsError
%End

    void setError( const QgsError &error );
%Docstring
 Set error
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasteridentifyresult.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
