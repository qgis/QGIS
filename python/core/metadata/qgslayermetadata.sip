/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadata.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayerMetadata
{
%Docstring
 A structured metadata store for a map layer.

 QgsLayerMetadata handles storage and management of the metadata
 for a QgsMapLayer. This class is an internal QGIS format with a common
 metadata structure, which allows for code to access the metadata properties for
 layers in a uniform way.

 The metadata store is designed to be compatible with the Dublin Core metadata
 specifications, and will be expanded to allow compatibility with ISO specifications
 in future releases. However, the QGIS internal schema does not represent a superset
 of all existing metadata schemas and accordingly conversion from specific
 metadata formats to QgsLayerMetadata may result in a loss of information.

 This class is designed to follow the specifications detailed in
 the schema definition available at resources/qgis-resource-metadata.xsd
 within the QGIS source code.

 Metadata can be validated through the use of QgsLayerMetadataValidator
 subclasses. E.g. validating against the native QGIS metadata schema can be performed
 using QgsNativeMetadataValidator.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayermetadata.h"
%End
  public:

    typedef QMap< QString, QStringList > KeywordMap;

    struct Constraint
    {

      Constraint( const QString &constraint = QString(), const QString &type = QString() );
%Docstring
 Constructor for Constraint.
%End

      QString type;
%Docstring
 Constraint type. Standard values include 'access' and 'other', however any
 string can be used for the type.
%End

      QString constraint;
%Docstring
 Free-form constraint string.
%End
    };

    typedef QList< QgsLayerMetadata::Constraint > ConstraintList;


    struct Address
    {

      Address( const QString &type = QString(), const QString &address = QString(), const QString &city = QString(), const QString &administrativeArea = QString(), const QString &postalCode = QString(), const QString &country = QString() );
%Docstring
 Constructor for Address.
%End

      QString type;
%Docstring
 Type of address, e.g. 'postal'.
%End

      QString address;
%Docstring
 Free-form physical address component, e.g. '221B Baker St' or 'P.O. Box 196'.
%End

      QString city;
%Docstring
 City or locality name.
%End

      QString administrativeArea;
%Docstring
 Administrative area (state, provice/territory, etc.).
%End

      QString postalCode;
%Docstring
 Postal (or ZIP) code.
%End

      QString country;
%Docstring
 Free-form country string.
%End
    };

    struct Contact
    {

      Contact( const QString &name = QString() );
%Docstring
 Constructor for Contact.
%End

      QString name;
%Docstring
 Name of contact.
%End

      QString organization;
%Docstring
 Organization contact belongs to/represents.
%End

      QString position;
%Docstring
 Position/title of contact.
%End

      QList< QgsLayerMetadata::Address > addresses;
%Docstring
 List of addresses associated with this contact.
%End

      QString voice;
%Docstring
 Voice telephone.
%End

      QString fax;
%Docstring
 Facsimile telephone.
%End

      QString email;
%Docstring
 Electronic mail address.
.. note::

   Do not include mailto: protocol as part of the email address.
%End

      QString role;
%Docstring
 Role of contact. Acceptable values are those from the ISO 19115 CI_RoleCode specifications
 (see http://www.isotc211.org/2005/resources/Codelist/gmxCodelists.xml).
 E.g. 'custodian', 'owner', 'distributor', etc.
%End
    };

    typedef QList< QgsLayerMetadata::Contact > ContactList;


    struct Link
    {

      Link( const QString &name = QString(), const QString &type = QString(), const QString &url = QString() );
%Docstring
 Constructor for Link.
%End

      QString name;
%Docstring
 Short link name. E.g. WMS layer name.
%End

      QString type;
%Docstring
 Link type. It is strongly suggested to use values from the 'identifier'
 column in https://github.com/OSGeo/Cat-Interop/blob/master/LinkPropertyLookupTable.csv
%End

      QString description;
%Docstring
 Abstract text about link.
%End

      QString url;
%Docstring
 Link url.  If the URL is an OWS server, specify the *base* URL only without parameters like service=xxx....
%End

      QString format;
%Docstring
 Format specification of online resource. It is strongly suggested to use GDAL/OGR format values.
%End

      QString mimeType;
%Docstring
 MIME type representative of the online resource response (image/png, application/json, etc.)
%End

      QString size;
%Docstring
 Estimated size (in bytes) of the online resource response.
%End
    };

    typedef QList< QgsLayerMetadata::Link > LinkList;

    QgsLayerMetadata();
%Docstring
 Constructor for QgsLayerMetadata.
%End

    virtual ~QgsLayerMetadata();

    QString identifier() const;
%Docstring
 A reference, URI, URL or some other mechanism to identify the resource.
 \see setIdentifier()
 :rtype: str
%End

    void setIdentifier( const QString &identifier );
%Docstring
 Sets the reference, URI, URL or some other mechanism to identify the resource.
 \see identifier()
%End

    QString parentIdentifier() const;
%Docstring
 A reference, URI, URL or some other mechanism to identify the parent resource that this resource is a part (child) of.
 Returns an empty string if no parent identifier is set.
 \see setParentIdentifier()
 :rtype: str
%End

    void setParentIdentifier( const QString &parentIdentifier );
%Docstring
 Sets a reference, URI, URL or some other mechanism to identify the parent resource that this resource is a part (child) of.
 Set an empty string if no parent identifier is required.
 \see parentIdentifier()
%End

    QString language() const;
%Docstring
 Returns the human language associated with the resource. Usually the returned string
 will follow either the ISO 639.2 or ISO 3166 specifications, e.g. 'ENG' or 'SPA', however
 this is not a hard requirement and the caller must account for non compliant
 values.
 \see setLanguage()
 :rtype: str
%End

    void setLanguage( const QString &language );
%Docstring
 Sets the human ``language`` associated with the resource. While a formal vocabulary is not imposed,
 ideally values should be taken from the ISO 639.2 or ISO 3166 specifications,
 e.g. 'ENG' or 'SPA' (ISO 639.2) or 'EN-AU' (ISO 3166).
 \see language()
%End

    QString type() const;
%Docstring
 Returns the nature of the resource.  While a formal vocabulary is not imposed, it is advised
 to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.
 \see setType()
 :rtype: str
%End

    void setType( const QString &type );
%Docstring
 Sets the ``type`` (nature) of the resource.  While a formal vocabulary is not imposed, it is advised
 to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.
 \see type()
%End

    QString title() const;
%Docstring
 Returns the human readable name of the resource, typically displayed in search results.
 \see setTitle()
 :rtype: str
%End

    void setTitle( const QString &title );
%Docstring
 Sets the human readable ``title`` (name) of the resource, typically displayed in search results.
 \see title()
%End

    QString abstract() const;
%Docstring
 Returns a free-form description of the resource.
 \see setAbstract()
 :rtype: str
%End

    void setAbstract( const QString &abstract );
%Docstring
 Sets a free-form ``abstract`` (description) of the resource.
 \see abstract()
%End

    QString fees() const;
%Docstring
 Returns any fees associated with using the resource.
 An empty string will be returned if no fees are set.
 \see setFees()
 :rtype: str
%End

    void setFees( const QString &fees );
%Docstring
 Sets the ``fees`` associated with using the resource.
 Use an empty string if no fees are set.
 \see fees()
%End

    QgsLayerMetadata::ConstraintList constraints() const;
%Docstring
 Returns a list of constraints associated with using the resource.
 \see setConstraints()
 :rtype: QgsLayerMetadata.ConstraintList
%End

    void setConstraints( const QgsLayerMetadata::ConstraintList &constraints );
%Docstring
 Sets the list of ``constraints`` associated with using the resource.
 \see constraints()
%End

    QStringList rights() const;
%Docstring
 Returns a list of attribution or copyright strings associated with the resource.
 \see setRights()
 :rtype: list of str
%End

    void setRights( const QStringList &rights );
%Docstring
 Sets a list of ``rights`` (attribution or copyright strings) associated with the resource.
 \see rights()
%End

    QStringList licenses() const;
%Docstring
 Returns a list of licenses associated with the resource (examples: http://opendefinition.org/licenses/).
 \see setLicenses()
 :rtype: list of str
%End

    void setLicenses( const QStringList &licenses );
%Docstring
 Sets a list of ``licenses`` associated with the resource.
 (examples: http://opendefinition.org/licenses/).
 \see licenses()
%End

    QStringList history() const;
%Docstring
 Returns a freeform description of the history or lineage of the resource.
 \see setHistory()
 :rtype: list of str
%End

    void setHistory( const QStringList &history );
%Docstring
 Sets the freeform description of the ``history`` or lineage of the resource.
 Any existing history items will be overwritten.
 \see addHistoryItem()
 \see history()
%End

    void addHistoryItem( const QString &text );
%Docstring
 Adds a single history ``text`` to the end of the existing history list.
 \see history()
 \see setHistory()
%End

    QString encoding() const;
%Docstring
 Returns the character encoding of the data in the resource. An empty string will be returned if no encoding is set.
 \see setEncoding()
 :rtype: str
%End

    void setEncoding( const QString &encoding );
%Docstring
 Sets the character ``encoding`` of the data in the resource. Use an empty string if no encoding is set.
 \see encoding()
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
 Returns the coordinate reference system described by the layer's metadata.

 Note that this has no link to QgsMapLayer.crs(). While in most cases these
 two systems are likely to be identical, it is possible to have a layer
 with a different CRS described by it's accompanying metadata versus the
 CRS which is actually used to display and manipulate the layer within QGIS.
 This may be the case when a layer has an incorrect CRS within its metadata
 and a user has manually overridden the layer's CRS within QGIS.
 \see setCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the coordinate reference system for the layer's metadata.

 Note that this has no link to QgsMapLayer.setCrs(). Setting the layer's
 CRS via QgsMapLayer.setCrs() does not affect the layer's metadata CRS,
 and changing the CRS from the metadata will not change the layer's
 CRS or how it is projected within QGIS.

 While ideally these two systems are likely to be identical, it is possible to have a layer
 with a different CRS described by it's accompanying metadata versus the
 CRS which is actually used to display and manipulate the layer within QGIS.
 This may be the case when a layer has an incorrect CRS within its metadata
 and a user has manually overridden the layer's CRS within QGIS.
 \see setCrs()
%End

    KeywordMap keywords() const;
%Docstring
 Returns the keywords map, which is a set of descriptive keywords associated with the resource.

 The map key is the vocabulary string and map value is a list of keywords for that vocabulary.

 The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
 associated with keyword list.

 \see setKeywords()
 \see keywordVocabularies()
 :rtype: KeywordMap
%End

    void setKeywords( const KeywordMap &keywords );
%Docstring
 Sets the ``keywords`` map, which is a set of descriptive keywords associated with the resource.

 The map key is the vocabulary string and map value is a list of keywords for that vocabulary.
 Calling this replaces any existing keyword vocabularies.

 The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
 associated with keyword list.

 \see keywords()
 \see addKeywords()
%End

    void addKeywords( const QString &vocabulary, const QStringList &keywords );
%Docstring
 Adds a list of descriptive ``keywords`` for a specified ``vocabulary``. Any existing
 keywords for the same vocabulary will be replaced. Other vocabularies
 will not be affected.

 The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
 associated with keyword list.

 \see setKeywords()
%End

    QStringList keywordVocabularies() const;
%Docstring
 Returns a list of keyword vocabularies contained in the metadata.

 The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
 associated with keyword list.

 \see keywords()
 :rtype: list of str
%End

    QStringList keywords( const QString &vocabulary ) const;
%Docstring
 Returns a list of keywords for the specified ``vocabulary``.
 If the vocabulary is not contained in the metadata, an empty
 list will be returned.

 The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
 associated with keyword list.

 \see keywordVocabularies()
 :rtype: list of str
%End

    QgsLayerMetadata::ContactList contacts() const;
%Docstring
 Returns a list of contact persons or entities associated with the resource.
 \see setContacts()
 :rtype: QgsLayerMetadata.ContactList
%End

    void setContacts( const QgsLayerMetadata::ContactList &contacts );
%Docstring
 Sets the list of ``contacts`` or entities associated with the resource. Any existing contacts
 will be replaced.
 \see contacts()
 \see addContact()
%End

    void addContact( const QgsLayerMetadata::Contact &contact );
%Docstring
 Adds an individual ``contact`` to the existing contacts.
 \see contacts()
 \see setContacts()
%End

    QgsLayerMetadata::LinkList links() const;
%Docstring
 Returns a list of online resources associated with the resource.
 \see setLinks()
 :rtype: QgsLayerMetadata.LinkList
%End

    void setLinks( const QgsLayerMetadata::LinkList &links );
%Docstring
 Sets the list of online resources associated with the resource. Any existing links
 will be replaced.
 \see links()
 \see addLink()
%End

    void addLink( const QgsLayerMetadata::Link &link );
%Docstring
 Adds an individual ``link`` to the existing links.
 \see links()
 \see setLinks()
%End

    void saveToLayer( QgsMapLayer *layer ) const;
%Docstring
 Saves the metadata to a ``layer``'s custom properties (see QgsMapLayer.setCustomProperty() ).
 \see readFromLayer()
%End

    void readFromLayer( const QgsMapLayer *layer );
%Docstring
 Reads the metadata state from a ``layer``'s custom properties (see QgsMapLayer.customProperty() ).
 \see saveToLayer()
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadata.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
