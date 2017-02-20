/***************************************************************************
    qgsrelation.sip
     --------------------------------------
    Date                 : 29.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsRelation
{
%TypeHeaderCode
#include <qgsrelation.h>
%End
  public:

    QgsRelation();
    static QgsRelation createFromXml( const QDomNode& node );

    void writeXml( QDomNode& node, QDomDocument& doc ) const;

    void setId( const QString& id );

    void setName( const QString& name );

    void setReferencingLayer( const QString& id );

    void setReferencedLayer( const QString& id );
    void addFieldPair( const QString& referencingField, const QString& referencedField );
    // void addFieldPair( const FieldPair& fieldPair );

    QgsFeatureIterator getRelatedFeatures( const QgsFeature& feature ) const;

    QgsFeatureRequest getRelatedFeaturesRequest( const QgsFeature& feature ) const;
    QString getRelatedFeaturesFilter( const QgsFeature& feature ) const;
    QgsFeatureRequest getReferencedFeatureRequest( const QgsAttributes& attributes ) const;
    QgsFeatureRequest getReferencedFeatureRequest( const QgsFeature& feature ) const;
    QgsFeature getReferencedFeature( const QgsFeature& feature ) const;
    QString name() const;

    QString id() const;

    void generateId();

    QString referencingLayerId() const;

    QgsVectorLayer* referencingLayer() const;

    QString referencedLayerId() const;

    QgsVectorLayer* referencedLayer() const;

    /**
     * Returns the field pairs which form this relation
     * The first element of each pair are the field names fo the foreign key.
     * The second element of each pair are the field names of the matching primary key.
     *
     * @return The fields forming the relation
     */
    QMap< QString, QString > fieldPairs() const;
%MethodCode
    const QList< QgsRelation::FieldPair >& pairs = sipCpp->fieldPairs();
    sipRes = new QMap< QString, QString >();
    Q_FOREACH( const QgsRelation::FieldPair& pair, pairs )
    {
        sipRes->insert( pair.first, pair.second );
    }
%End

    QgsAttributeList referencedFields() const;

    QgsAttributeList referencingFields() const;

    bool isValid() const;
    bool hasEqualDefinition( const QgsRelation& other ) const;

    QString resolveReferencedField( const QString& referencingField ) const;
    QString resolveReferencingField( const QString& referencedField ) const;
};
