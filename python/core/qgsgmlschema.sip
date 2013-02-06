//typedef QMap<int, QgsField> QgsFieldMap;

class QgsGmlSchema: QObject 
{

%TypeHeaderCode
#include <qgsgmlschema.h>
%End

  public:
    QgsGmlSchema();
    ~QgsGmlSchema();

    /** Get fields info from XSD */
    bool parseXSD( const QByteArray &xml );

    /** Guess GML schema from data if XSD does not exist.
      * Currently only recognizes UMN Mapserver GetFeatureInfo GML response.
      * @param data GML data
      * @return true in case of success */
    bool guessSchema( const QByteArray &data );

    QStringList typeNames() const;

    QList<QgsField> fields( const QString & typeName );

    QStringList geometryAttributes( const QString & typeName );
};
