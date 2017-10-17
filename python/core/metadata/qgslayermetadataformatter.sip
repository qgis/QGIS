/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadataformatter.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayerMetadataFormatter
{
%Docstring
 Class for metadata formatter.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayermetadataformatter.h"
%End
  public:

    QgsLayerMetadataFormatter( const QgsLayerMetadata &metadata );
%Docstring
 Constructor for QgsLayerMetadataFormatter
%End

    QString accessSectionHtml() const;
%Docstring
 Formats the "Access" section according to a ``metadata`` object.
 This will return a HTML table.
 :rtype: str
%End

    QString contactsSectionHtml() const;
%Docstring
 Formats the "Contacts" section according to a ``metadata`` object.
 This will return a HTML table.
 :rtype: str
%End

    QString extentSectionHtml() const;
%Docstring
 Formats the "Extents" section according to a ``metadata`` object.
 This will return a HTML table.
 :rtype: str
%End

    QString identificationSectionHtml() const;
%Docstring
 Formats the "Identification" section according to a ``metadata`` object.
 This will return a HTML table.
 :rtype: str
%End

    QString historySectionHtml() const;
%Docstring
 Formats the "History" section according to a ``metadata`` object.
 This will return a HTML table.
 :rtype: str
%End

    QString linksSectionHtml() const;
%Docstring
 Formats the "Links" section according to a ``metadata`` object.
 This will return a HTML table.
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadataformatter.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
