//typedef QMap<int, QgsField> QgsFieldMap;

/* Description of feature class in GML */
class QgsGmlFeatureClass
{
  public:
    QgsGmlFeatureClass( );
    QgsGmlFeatureClass( QString name, QString path );

    ~QgsGmlFeatureClass();

    QList<QgsField> & fields();

    int fieldIndex( const QString & name );

    QString path() const;

    QStringList & geometryAttributes();
};

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

    /** Get fields for type/class name parsed from GML or XSD */
    QList<QgsField> fields( const QString & typeName );

    /** Get list of geometry attributes for type/class name */
    QStringList geometryAttributes( const QString & typeName );
};
