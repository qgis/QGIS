class QgsRasterIdentifyResult
{
%TypeHeaderCode
#include <qgsrasteridentifyresult.h>
%End
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
    bool isValid() const;

    /** \brief Get results format */
    QgsRaster::IdentifyFormat format() const;

    /** \brief Get results. Results are different for each format:
     * QgsRaster::IdentifyFormatValue: map of values for each band, keys are band numbers (from 1).
     * QgsRaster::IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer (WMS)
     * QgsRaster::IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
     */
    QMap<int, QVariant> results() const;

    /** Set map of optional parameters */
    void setParams( const QMap<QString, QVariant> & theParams );

    /** Get map of optional parameters */
    QMap<QString, QVariant> params() const;

    /** \brief Get error */
    QgsError error() const;

    /** \brief Set error */
    void setError( const QgsError & theError );
};

