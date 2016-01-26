//typedef QMap<int, QgsField> QgsFieldMap;

/* Description of feature class in GML */
class QgsGmlFeatureClass
{
  public:
    QgsGmlFeatureClass();
    QgsGmlFeatureClass( const QString& name, const QString& path );

    QList<QgsField> & fields();

    int fieldIndex( const QString & name );

    QString path() const;

    QStringList & geometryAttributes();
};

class QgsGmlSchema : QObject
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
      * Supports only UTF-8, UTF-16, ISO-8859-1, US-ASCII XML encodings.
      * @param data GML data
      * @return true in case of success */
    bool guessSchema( const QByteArray &data );

    /** Get list of dot separated paths to feature classes parsed from GML or XSD */
    QStringList typeNames() const;

    /** Get fields for type/class name parsed from GML or XSD */
    QList<QgsField> fields( const QString & typeName );

    /** Get list of geometry attributes for type/class name */
    QStringList geometryAttributes( const QString & typeName );

    /** Get error if parseXSD() or guessSchema() failed */
    QgsError error() const;
};
