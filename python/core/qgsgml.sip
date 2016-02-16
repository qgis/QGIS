class QgsGml: QObject
{
%TypeHeaderCode
#include <qgsgml.h>
%End

  public:
    QgsGml(
      const QString& typeName,
      const QString& geometryAttribute,
      const QgsFields & fields );

    ~QgsGml();

    /** Does the Http GET request to the wfs server
     *  Supports only UTF-8, UTF-16, ISO-8859-1, ISO-8859-1 XML encodings.
     *  @param uri GML URL
     *  @param wkbType wkbType to retrieve
     *  @param extent retrieved extents
     *  @param userName username for authentication
     *  @param password password for authentication
     *  @param authcfg authentication configuration id
     *  @return 0 in case of success
     *  @note available in python as getFeaturesUri
     */
    int getFeatures( const QString& uri,
                     QGis::WkbType* wkbType,
                     QgsRectangle* extent = 0,
                     const QString& userName = QString(),
                     const QString& password = QString(),
                     const QString& authcfg = QString() ) /PyName=getFeaturesUri/;

    /** Read from GML data. Constructor uri param is ignored
     *  Supports only UTF-8, UTF-16, ISO-8859-1, ISO-8859-1 XML encodings.
     */
    int getFeatures( const QByteArray &data, QGis::WkbType* wkbType, QgsRectangle* extent = 0 );

    /** Get parsed features for given type name */
    QMap<qint64, QgsFeature* > featuresMap() const;

    /** Get feature ids map */
    QMap<qint64, QString > idsMap() const;

    /** Returns features spatial reference system
      @note Added in QGIS 2.1 */
    QgsCoordinateReferenceSystem crs() const;

  signals:
    void dataReadProgress( int progress );
    void totalStepsUpdate( int totalSteps );
    //also emit signal with progress and totalSteps together (this is better for the status message)
    void dataProgressAndSteps( int progress, int totalSteps );

};
