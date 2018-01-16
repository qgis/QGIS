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

    struct SpatialExtent
    {

      QgsCoordinateReferenceSystem extentCrs;

      QgsBox3d bounds;
    };

    struct Extent
    {
      public:

        QList< QgsLayerMetadata::SpatialExtent > spatialExtents() const;
%Docstring
Spatial extents of the resource.

.. seealso:: :py:func:`setSpatialExtents`
%End

        void setSpatialExtents( const QList< QgsLayerMetadata::SpatialExtent > &extents );
%Docstring
Sets the spatial ``extents`` of the resource.

.. seealso:: :py:func:`spatialExtents`
%End

        QList< QgsDateTimeRange > temporalExtents() const;
%Docstring
Temporal extents of the resource. Use QgsDateTimeRange.isInstant() to determine
whether the temporal extent is a range or a single point in time.
If QgsDateTimeRange.isInfinite() returns true then the temporal extent
is considered to be indeterminate and continuous.

.. seealso:: :py:func:`setTemporalExtents`
%End

        void setTemporalExtents( const QList< QgsDateTimeRange > &extents );
%Docstring
Sets the temporal ``extents`` of the resource.

.. seealso:: :py:func:`temporalExtents`
%End


    };

    struct Constraint
    {

      Constraint( const QString &constraint = QString(), const QString &type = QString() );
%Docstring
Constructor for Constraint.
%End

      QString type;

      QString constraint;
    };

    typedef QList< QgsLayerMetadata::Constraint > ConstraintList;


    struct Address
    {

      Address( const QString &type = QString(), const QString &address = QString(), const QString &city = QString(), const QString &administrativeArea = QString(), const QString &postalCode = QString(), const QString &country = QString() );
%Docstring
Constructor for Address.
%End

      QString type;

      QString address;

      QString city;

      QString administrativeArea;

      QString postalCode;

      QString country;
    };

    struct Contact
    {

      Contact( const QString &name = QString() );
%Docstring
Constructor for Contact.
%End

      QString name;

      QString organization;

      QString position;

      QList< QgsLayerMetadata::Address > addresses;

      QString voice;

      QString fax;

      QString email;

      QString role;
    };

    typedef QList< QgsLayerMetadata::Contact > ContactList;


    struct Link
    {

      Link( const QString &name = QString(), const QString &type = QString(), const QString &url = QString() );
%Docstring
Constructor for Link.
%End

      QString name;

      QString type;

      QString description;

      QString url;

      QString format;

      QString mimeType;

      QString size;
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

.. seealso:: :py:func:`setIdentifier`
%End

    void setIdentifier( const QString &identifier );
%Docstring
Sets the reference, URI, URL or some other mechanism to identify the resource.

.. seealso:: :py:func:`identifier`
%End

    QString parentIdentifier() const;
%Docstring
A reference, URI, URL or some other mechanism to identify the parent resource that this resource is a part (child) of.
Returns an empty string if no parent identifier is set.

.. seealso:: :py:func:`setParentIdentifier`
%End

    void setParentIdentifier( const QString &parentIdentifier );
%Docstring
Sets a reference, URI, URL or some other mechanism to identify the parent resource that this resource is a part (child) of.
Set an empty string if no parent identifier is required.

.. seealso:: :py:func:`parentIdentifier`
%End

    QString language() const;
%Docstring
Returns the human language associated with the resource. Usually the returned string
will follow either the ISO 639.2 or ISO 3166 specifications, e.g. 'ENG' or 'SPA', however
this is not a hard requirement and the caller must account for non compliant
values.

.. seealso:: :py:func:`setLanguage`
%End

    void setLanguage( const QString &language );
%Docstring
Sets the human ``language`` associated with the resource. While a formal vocabulary is not imposed,
ideally values should be taken from the ISO 639.2 or ISO 3166 specifications,
e.g. 'ENG' or 'SPA' (ISO 639.2) or 'EN-AU' (ISO 3166).

.. seealso:: :py:func:`language`
%End

    QString type() const;
%Docstring
Returns the nature of the resource.  While a formal vocabulary is not imposed, it is advised
to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.

.. seealso:: :py:func:`setType`
%End

    void setType( const QString &type );
%Docstring
Sets the ``type`` (nature) of the resource.  While a formal vocabulary is not imposed, it is advised
to use the ISO 19115 MD_ScopeCode values. E.g. 'dataset' or 'series'.

.. seealso:: :py:func:`type`
%End

    QString title() const;
%Docstring
Returns the human readable name of the resource, typically displayed in search results.

.. seealso:: :py:func:`setTitle`
%End

    void setTitle( const QString &title );
%Docstring
Sets the human readable ``title`` (name) of the resource, typically displayed in search results.

.. seealso:: :py:func:`title`
%End

    QString abstract() const;
%Docstring
Returns a free-form description of the resource.

.. seealso:: :py:func:`setAbstract`
%End

    void setAbstract( const QString &abstract );
%Docstring
Sets a free-form ``abstract`` (description) of the resource.

.. seealso:: :py:func:`abstract`
%End

    QString fees() const;
%Docstring
Returns any fees associated with using the resource.
An empty string will be returned if no fees are set.

.. seealso:: :py:func:`setFees`
%End

    void setFees( const QString &fees );
%Docstring
Sets the ``fees`` associated with using the resource.
Use an empty string if no fees are set.

.. seealso:: :py:func:`fees`
%End

    QgsLayerMetadata::ConstraintList constraints() const;
%Docstring
Returns a list of constraints associated with using the resource.

.. seealso:: :py:func:`setConstraints`
%End

    void addConstraint( const QgsLayerMetadata::Constraint &constraint );
%Docstring
Adds an individual constraint to the existing constraints.

.. seealso:: :py:func:`constraints`

.. seealso:: :py:func:`setConstraints`
%End

    void setConstraints( const QgsLayerMetadata::ConstraintList &constraints );
%Docstring
Sets the list of ``constraints`` associated with using the resource.

.. seealso:: :py:func:`constraints`
%End

    QStringList rights() const;
%Docstring
Returns a list of attribution or copyright strings associated with the resource.

.. seealso:: :py:func:`setRights`
%End

    void setRights( const QStringList &rights );
%Docstring
Sets a list of ``rights`` (attribution or copyright strings) associated with the resource.

.. seealso:: :py:func:`rights`
%End

    QStringList licenses() const;
%Docstring
Returns a list of licenses associated with the resource (examples: http://opendefinition.org/licenses/).

.. seealso:: :py:func:`setLicenses`
%End

    void setLicenses( const QStringList &licenses );
%Docstring
Sets a list of ``licenses`` associated with the resource.
(examples: http://opendefinition.org/licenses/).

.. seealso:: :py:func:`licenses`
%End

    QStringList history() const;
%Docstring
Returns a freeform description of the history or lineage of the resource.

.. seealso:: :py:func:`setHistory`
%End

    void setHistory( const QStringList &history );
%Docstring
Sets the freeform description of the ``history`` or lineage of the resource.
Any existing history items will be overwritten.

.. seealso:: :py:func:`addHistoryItem`

.. seealso:: :py:func:`history`
%End

    void addHistoryItem( const QString &text );
%Docstring
Adds a single history ``text`` to the end of the existing history list.

.. seealso:: :py:func:`history`

.. seealso:: :py:func:`setHistory`
%End

    QString encoding() const;
%Docstring
Returns the character encoding of the data in the resource. An empty string will be returned if no encoding is set.

.. seealso:: :py:func:`setEncoding`
%End

    void setEncoding( const QString &encoding );
%Docstring
Sets the character ``encoding`` of the data in the resource. Use an empty string if no encoding is set.

.. seealso:: :py:func:`encoding`
%End


    QgsLayerMetadata::Extent &extent();
%Docstring
Returns the spatial and temporal extents associated with the resource.

.. seealso:: :py:func:`setExtent`
%End

    void setExtent( const QgsLayerMetadata::Extent &extent );
%Docstring
Sets the spatial and temporal extents associated with the resource.

.. seealso:: :py:func:`setExtent`
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the coordinate reference system described by the layer's metadata.

Note that this has no link to :py:func:`QgsMapLayer.crs()` While in most cases these
two systems are likely to be identical, it is possible to have a layer
with a different CRS described by it's accompanying metadata versus the
CRS which is actually used to display and manipulate the layer within QGIS.
This may be the case when a layer has an incorrect CRS within its metadata
and a user has manually overridden the layer's CRS within QGIS.

The CRS described here should either match the CRS from the layer :py:func:`QgsMapLayer.crs()`
or the CRS from the data provider.

This property should also match the CRS property used in the spatial extent.

.. seealso:: :py:func:`setCrs`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the coordinate reference system for the layer's metadata.

Note that this has no link to :py:func:`QgsMapLayer.setCrs()` Setting the layer's
CRS via QgsMapLayer.setCrs() does not affect the layer's metadata CRS,
and changing the CRS from the metadata will not change the layer's
CRS or how it is projected within QGIS.

While ideally these two systems are likely to be identical, it is possible to have a layer
with a different CRS described by it's accompanying metadata versus the
CRS which is actually used to display and manipulate the layer within QGIS.
This may be the case when a layer has an incorrect CRS within its metadata
and a user has manually overridden the layer's CRS within QGIS.

The CRS described here should either match the CRS from the layer :py:func:`QgsMapLayer.crs()`
or the CRS from the data provider.

This property should also match the CRS property used in the spatial extent.

.. seealso:: :py:func:`crs`
%End

    KeywordMap keywords() const;
%Docstring
Returns the keywords map, which is a set of descriptive keywords associated with the resource.

The map key is the vocabulary string and map value is a list of keywords for that vocabulary.

The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
associated with keyword list.

.. seealso:: :py:func:`setKeywords`

.. seealso:: :py:func:`keywordVocabularies`
%End

    void setKeywords( const KeywordMap &keywords );
%Docstring
Sets the ``keywords`` map, which is a set of descriptive keywords associated with the resource.

The map key is the vocabulary string and map value is a list of keywords for that vocabulary.
Calling this replaces any existing keyword vocabularies.

The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
associated with keyword list.

.. seealso:: :py:func:`keywords`

.. seealso:: :py:func:`addKeywords`
%End

    void addKeywords( const QString &vocabulary, const QStringList &keywords );
%Docstring
Adds a list of descriptive ``keywords`` for a specified ``vocabulary``. Any existing
keywords for the same vocabulary will be replaced. Other vocabularies
will not be affected.

The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
associated with keyword list.

.. seealso:: :py:func:`setKeywords`
%End

    bool removeKeywords( const QString &vocabulary );
%Docstring
Remove a vocabulary from the list.

.. seealso:: :py:func:`setKeywords`

.. seealso:: :py:func:`addKeywords`
%End

    QStringList keywordVocabularies() const;
%Docstring
Returns a list of keyword vocabularies contained in the metadata.

The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
associated with keyword list.

.. seealso:: :py:func:`keywords`
%End

    QStringList keywords( const QString &vocabulary ) const;
%Docstring
Returns a list of keywords for the specified ``vocabulary``.
If the vocabulary is not contained in the metadata, an empty
list will be returned.

The vocabulary string is a reference (URI/URL preferred) to a codelist or vocabulary
associated with keyword list.

.. seealso:: :py:func:`keywordVocabularies`
%End

    QStringList categories() const;
%Docstring
Returns categories of the resource.
Categories are stored using a special vocabulary 'gmd:topicCategory' in keywords.

.. seealso:: :py:func:`keywords`
%End

    void setCategories( const QStringList &categories );
%Docstring
Sets categories of the resource.
Categories are stored using a special vocabulary 'gmd:topicCategory' in keywords.

.. seealso:: :py:func:`keywords`
%End

    QgsLayerMetadata::ContactList contacts() const;
%Docstring
Returns a list of contact persons or entities associated with the resource.

.. seealso:: :py:func:`setContacts`
%End

    void setContacts( const QgsLayerMetadata::ContactList &contacts );
%Docstring
Sets the list of ``contacts`` or entities associated with the resource. Any existing contacts
will be replaced.

.. seealso:: :py:func:`contacts`

.. seealso:: :py:func:`addContact`
%End

    void addContact( const QgsLayerMetadata::Contact &contact );
%Docstring
Adds an individual ``contact`` to the existing contacts.

.. seealso:: :py:func:`contacts`

.. seealso:: :py:func:`setContacts`
%End

    QgsLayerMetadata::LinkList links() const;
%Docstring
Returns a list of online resources associated with the resource.

.. seealso:: :py:func:`setLinks`
%End

    void setLinks( const QgsLayerMetadata::LinkList &links );
%Docstring
Sets the list of online resources associated with the resource. Any existing links
will be replaced.

.. seealso:: :py:func:`links`

.. seealso:: :py:func:`addLink`
%End

    void addLink( const QgsLayerMetadata::Link &link );
%Docstring
Adds an individual ``link`` to the existing links.

.. seealso:: :py:func:`links`

.. seealso:: :py:func:`setLinks`
%End

    void saveToLayer( QgsMapLayer *layer ) const;
%Docstring
Saves the metadata to a ``layer``'s custom properties (see QgsMapLayer.setCustomProperty() ).

.. seealso:: :py:func:`readFromLayer`
%End

    void readFromLayer( const QgsMapLayer *layer );
%Docstring
Reads the metadata state from a ``layer``'s custom properties (see QgsMapLayer.customProperty() ).

.. seealso:: :py:func:`saveToLayer`
%End

    bool readMetadataXml( const QDomElement &metadataElement );
%Docstring
Sets state from Dom document

:param metadataElement: The Dom element corresponding to ``resourceMetadata'' tag

:return: true if successful
%End

    bool writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const;
%Docstring
Stores state in Dom node

:param metadataElement: is a Dom element corresponding to ``resourceMetadata'' tag
:param document: is a the dom document being written

:return: true if successful
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadata.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
