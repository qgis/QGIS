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
    ~QgsMetadataWidget();

    void fillSourceFromLayer() const;
%Docstring
 Fill the source field from the layer.
%End

    void addVocabulary() const;
%Docstring
 Add a new vocabulary.
%End

    void removeVocabulary() const;
%Docstring
 Remove a selected vocabulary.
%End

    void addLicence();
%Docstring
 Add a new licence.
%End

    void removeLicence() const;
%Docstring
 Remove a selected licence.
%End

    void addRight();
%Docstring
 Add a new right.
%End

    void removeRight() const;
%Docstring
 Remove a selected right.
%End

    void addConstraint() const;
%Docstring
 Add a new constraint.
%End

    void removeConstraint() const;
%Docstring
 Remove a constraint.
%End

    void fillCrsFromLayer() const;
%Docstring
 Fill the CRS field from the layer.
%End

    void addContact() const;
%Docstring
 Add a new contact.
%End

    void removeContact() const;
%Docstring
 Remove a selected contact.
%End

    void updateContactDetails() const;
%Docstring
 Update the contact details according to the selection in the contact list.
%End

    void addLink() const;
%Docstring
 Add a new link.
%End

    void removeLink() const;
%Docstring
 Remove a selected link.
%End

    void fillComboBox() const;
%Docstring
 Function to fill combobox like language, type.
%End

    void setPropertiesFromLayer() const;
%Docstring
 Fill the wizard from values in the layer metadata object.
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

    void acceptMetadata();
%Docstring
 Saves the metadata to the layer.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmetadatawidget.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
