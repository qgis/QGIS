/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmetadatawidget.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMetadataWidget : QWidget
{
%Docstring
 A wizard to edit metadata on a map layer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsmetadatawidget.h"
%End
  public:

    QgsMetadataWidget( QWidget *parent, QgsMapLayer *layer = 0 );
%Docstring
 Constructor for the wizard.
%End

    void saveMetadata( QgsLayerMetadata &layerMetadata ) const;
%Docstring
 Save all fields in a QgsLayerMetadata object.
%End

    bool checkMetadata() const;
%Docstring
 Check if values in the wizard are correct.
 :rtype: bool
%End

    void acceptMetadata();
%Docstring
 Saves the metadata to the layer.
%End

    static QMap<QString, QString> parseLanguages();
%Docstring
 Returns a list of languages available by default in the wizard.
 :rtype: QMap<str, QString>
%End

    static QStringList parseLicenses();
%Docstring
 Returns a list of licences available by default in the wizard.
 :rtype: list of str
%End

    static QStringList parseLinkTypes();
%Docstring
 Returns a list of link types available by default in the wizard.
.. seealso:: https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv
 :rtype: list of str
%End

    static QStringList parseMimeTypes();
%Docstring
 Returns a list of MIME types available by default in the wizard.
.. seealso:: https://fr.wikipedia.org/wiki/Type_MIME
 :rtype: list of str
%End

    static QMap<QString, QString> parseTypes();
%Docstring
 Returns a list of types available by default in the wizard.
 :rtype: QMap<str, QString>
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmetadatawidget.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
