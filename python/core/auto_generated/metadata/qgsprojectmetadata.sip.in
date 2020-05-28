/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgsprojectmetadata.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsProjectMetadata : QgsAbstractMetadataBase
{
%Docstring
A structured metadata store for a map layer.

QgsProjectMetadata handles storage and management of the metadata
for a QgsProject. This class is an internal QGIS format with a common
metadata structure, which allows for code to access the metadata properties for
projects in a uniform way.

The metadata store is designed to be compatible with the Dublin Core metadata
specifications, and will be expanded to allow compatibility with ISO specifications
in future releases. However, the QGIS internal schema does not represent a superset
of all existing metadata schemas and accordingly conversion from specific
metadata formats to QgsProjectMetadata may result in a loss of information.

This class is designed to follow the specifications detailed in
the schema definition available at resources/qgis-project-metadata.xsd
within the QGIS source code.

Metadata can be validated through the use of QgsLayerMetadataValidator
subclasses. E.g. validating against the native QGIS metadata schema can be performed
using QgsNativeProjectMetadataValidator.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsprojectmetadata.h"
%End
  public:

    QgsProjectMetadata();
%Docstring
Constructor for QgsProjectMetadata.
%End

    virtual QgsProjectMetadata *clone() const /Factory/;


    QString author() const;
%Docstring
Returns the project author string.

.. seealso:: :py:func:`setAuthor`
%End

    void setAuthor( const QString &author );
%Docstring
Sets the project ``author`` string

.. seealso:: :py:func:`author`
%End

    QDateTime creationDateTime() const;
%Docstring
Returns the project's creation date/timestamp.

.. seealso:: :py:func:`setCreationDateTime`
%End

    void setCreationDateTime( const QDateTime &creationDateTime );
%Docstring
Sets the project's creation date/timestamp.

.. seealso:: :py:func:`creationDateTime`
%End

    virtual bool readMetadataXml( const QDomElement &metadataElement );

    virtual bool writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const;


    bool operator==( const QgsProjectMetadata &metadataOther ) const;

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgsprojectmetadata.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
