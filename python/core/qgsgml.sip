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

    /** Read from GML data. */
    int getFeatures( const QByteArray &data, QGis::WkbType* wkbType, QgsRectangle* extent = 0 );

    /** Get parsed features for given type name */
    QMap<qint64, QgsFeature* > featuresMap() const;

    /** Get feature ids map */
    QMap<qint64, QString > idsMap() const;

};
