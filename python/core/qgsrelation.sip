/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrelation.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRelation
{

%TypeHeaderCode
#include "qgsrelation.h"
%End
  public:

    enum RelationStrength
    {
      Association,
      Composition

    };


    QgsRelation();
%Docstring
Default constructor. Creates an invalid relation.
%End

    static QgsRelation createFromXml( const QDomNode &node );
%Docstring
Creates a relation from an XML structure. Used for reading .qgs projects.

:param node: The dom node containing the relation information

:return: A relation
%End

    void writeXml( QDomNode &node, QDomDocument &doc ) const;
%Docstring
Writes a relation to an XML structure. Used for saving .qgs projects

:param node: The parent node in which the relation will be created
:param doc:  The document in which the relation will be saved
%End

    void setId( const QString &id );
%Docstring
Set an id for this relation
%End

    void setName( const QString &name );
%Docstring
Set a name for this relation
%End

    void setStrength( const RelationStrength &strength );
%Docstring
Set a strength for this relation

.. versionadded:: 3.0
%End

    void setReferencingLayer( const QString &id );
%Docstring
Set the referencing (child) layer id. This layer will be searched in the registry.
%End

    void setReferencedLayer( const QString &id );
%Docstring
Set the referenced (parent) layer id. This layer will be searched in the registry.
%End

    void addFieldPair( const QString &referencingField, const QString &referencedField );
%Docstring
Add a field pairs which is part of this relation
The first element of each pair are the field names of the foreign key.
The second element of each pair are the field names of the matching primary key.

:param referencingField:  The field name on the referencing (child) layer (FK)
:param referencedField:   The field name on the referenced (parent) layer  (PK)
%End


    QgsFeatureIterator getRelatedFeatures( const QgsFeature &feature ) const;
%Docstring
Creates an iterator which returns all the features on the referencing (child) layer
which have a foreign key pointing to the provided feature.

:param feature: A feature from the referenced (parent) layer

:return: An iterator with all the referenced features

.. seealso:: :py:func:`getRelatedFeaturesRequest`

.. seealso:: :py:func:`getRelatedFeaturesFilter`
%End

    QgsFeatureRequest getRelatedFeaturesRequest( const QgsFeature &feature ) const;
%Docstring
Creates a request to return all the features on the referencing (child) layer
which have a foreign key pointing to the provided feature.

:param feature: A feature from the referenced (parent) layer

:return: A request for all the referencing features

.. seealso:: :py:func:`getRelatedFeatures`

.. seealso:: :py:func:`getRelatedFeaturesFilter`
%End

    QString getRelatedFeaturesFilter( const QgsFeature &feature ) const;
%Docstring
Returns a filter expression which returns all the features on the referencing (child) layer
which have a foreign key pointing to the provided feature.

:param feature: A feature from the referenced (parent) layer

:return: expression filter string for all the referencing features

.. versionadded:: 2.16

.. seealso:: :py:func:`getRelatedFeatures`

.. seealso:: :py:func:`getRelatedFeaturesRequest`
%End

    QgsFeatureRequest getReferencedFeatureRequest( const QgsAttributes &attributes ) const;
%Docstring
Creates a request to return the feature on the referenced (parent) layer
which is referenced by the provided feature.

:param attributes: An attribute vector containing the foreign key

:return: A request the referenced feature

.. note::

   not available in Python bindings
%End

    QgsFeatureRequest getReferencedFeatureRequest( const QgsFeature &feature ) const;
%Docstring
Creates a request to return the feature on the referenced (parent) layer
which is referenced by the provided feature.

:param feature: A feature from the referencing (child) layer

:return: A request the referenced feature
%End

    QgsFeature getReferencedFeature( const QgsFeature &feature ) const;
%Docstring
Creates a request to return the feature on the referenced (parent) layer
which is referenced by the provided feature.

:param feature: A feature from the referencing (child) layer

:return: A request the referenced feature
%End

    QString name() const;
%Docstring
Returns a human readable name for this relation. Mostly used as title for the children.

.. seealso:: :py:func:`id`

:return: A name
%End

    RelationStrength strength() const;
%Docstring
Returns the relation strength as a string

:return: strength

.. versionadded:: 3.0
%End

    QString id() const;
%Docstring
A (project-wide) unique id for this relation

:return: The id
%End

    void generateId();
%Docstring
Generate a (project-wide) unique id for this relation

.. versionadded:: 3.0
%End

    QString referencingLayerId() const;
%Docstring
Access the referencing (child) layer's id
This is the layer which has the field(s) which point to another layer

:return: The id of the referencing layer
%End

    QgsVectorLayer *referencingLayer() const;
%Docstring
Access the referencing (child) layer
This is the layer which has the field(s) which point to another layer

:return: The referencing layer
%End

    QString referencedLayerId() const;
%Docstring
Access the referenced (parent) layer's id

:return: The id of the referenced layer
%End

    QgsVectorLayer *referencedLayer() const;
%Docstring
Access the referenced (parent) layer

:return: referenced layer
%End

    QMap< QString, QString > fieldPairs() const;
%Docstring
Returns the field pairs which form this relation
The first element of each pair are the field names of the foreign key.
The second element of each pair are the field names of the matching primary key.

:return: The fields forming the relation
%End
%MethodCode
    const QList< QgsRelation::FieldPair > &pairs = sipCpp->fieldPairs();
    sipRes = new QMap< QString, QString >();
    Q_FOREACH ( const QgsRelation::FieldPair &pair, pairs )
    {
      sipRes->insert( pair.first, pair.second );
    }
%End

    QgsAttributeList referencedFields() const;
%Docstring
Returns a list of attributes used to form the referenced fields
(most likely primary key) on the referenced (parent) layer.

:return: A list of attributes
%End

    QgsAttributeList referencingFields() const;
%Docstring
Returns a list of attributes used to form the referencing fields
(foreign key) on the referencing (child) layer.

:return: A list of attributes
%End

    bool isValid() const;
%Docstring
Returns the validity of this relation. Don't use the information if it's not valid.

:return: true if the relation is valid
%End

    bool hasEqualDefinition( const QgsRelation &other ) const;
%Docstring
Compares the two QgsRelation, ignoring the name and the ID.

:param other: The other relation

:return: true if they are similar

.. versionadded:: 3.0
%End

    QString resolveReferencedField( const QString &referencingField ) const;
%Docstring
Get the referenced field counterpart given a referencing field.

.. versionadded:: 3.0
%End

    QString resolveReferencingField( const QString &referencedField ) const;
%Docstring
Get the referencing field counterpart given a referenced field.

.. versionadded:: 3.0
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsrelation.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
