/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgmlschema.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsGmlFeatureClass
{
%Docstring
 Description of feature class in GML
%End

%TypeHeaderCode
#include "qgsgmlschema.h"
%End
  public:

    QgsGmlFeatureClass();
%Docstring
 Constructor for QgsGmlFeatureClass.
%End
    QgsGmlFeatureClass( const QString &name, const QString &path );

    QList<QgsField> &fields();
%Docstring
 :rtype: list of QgsField
%End

    int fieldIndex( const QString &name );
%Docstring
 :rtype: int
%End

    QString path() const;
%Docstring
 :rtype: str
%End

    QStringList &geometryAttributes();
%Docstring
 :rtype: list of str
%End

};

class QgsGmlSchema : QObject
{

%TypeHeaderCode
#include "qgsgmlschema.h"
%End
  public:
    QgsGmlSchema();

    bool parseXSD( const QByteArray &xml );
%Docstring
Get fields info from XSD
 :rtype: bool
%End

    bool guessSchema( const QByteArray &data );
%Docstring
 Guess GML schema from data if XSD does not exist.
 Currently only recognizes UMN Mapserver GetFeatureInfo GML response.
 Supports only UTF-8, UTF-16, ISO-8859-1, US-ASCII XML encodings.
 \param data GML data
 :return: true in case of success *
 :rtype: bool
%End

    QStringList typeNames() const;
%Docstring
Get list of dot separated paths to feature classes parsed from GML or XSD
 :rtype: list of str
%End

    QList<QgsField> fields( const QString &typeName );
%Docstring
Get fields for type/class name parsed from GML or XSD
 :rtype: list of QgsField
%End

    QStringList geometryAttributes( const QString &typeName );
%Docstring
Get list of geometry attributes for type/class name
 :rtype: list of str
%End

    QgsError error() const;
%Docstring
Get error if parseXSD() or guessSchema() failed
 :rtype: QgsError
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgmlschema.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
