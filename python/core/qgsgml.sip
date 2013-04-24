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

    QMap<qint64, QgsFeature* > featuresMap() const;

};
