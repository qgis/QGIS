/***************************************************************************
  qgsfeaturefiltermodel.h - QgsFeatureFilterModel
 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREFILTERMODEL_H
#define QGSFEATUREFILTERMODEL_H


#include "qgsfeaturepickermodelbase.h"

/**
 * \ingroup core
 * \brief Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 *
 */
class CORE_EXPORT QgsFeatureFilterModel : public QgsFeaturePickerModelBase
{
    Q_OBJECT

    /**
     * A set of fields of sourceLayer that is unique and should be used to identify features.
     * Normally the primary key field.
     * Needs to match the identifierValue.
     */
    Q_PROPERTY( QStringList identifierFields READ identifierFields WRITE setIdentifierFields NOTIFY identifierFieldsChanged )

    /**
     * The values that identifies the current feature.
     */
    Q_PROPERTY( QVariantList extraIdentifierValues READ extraIdentifierValues WRITE setExtraIdentifierValues NOTIFY extraIdentifierValuesChanged )

  public:

    /**
     * Create a new QgsFeatureFilterModel, optionally specifying a \a parent.
     */
    explicit QgsFeatureFilterModel( QObject *parent = nullptr );

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     * If there are several identifier fields defined, the behavior is not guaranteed
     * \deprecated QGIS 3.10. Use identifierFields() instead.
     */
    Q_DECL_DEPRECATED QString identifierField() const;

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     * \since QGIS 3.10
     */
    QStringList identifierFields() const;

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     * \note This will also reset identifier fields to NULL
     * \since QGIS 3.10
     */
    void setIdentifierFields( const QStringList &identifierFields );

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model as NULL value(s).
     * \since QGIS 3.10
     */
    void setExtraIdentifierValueToNull() override;

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     * \since QGIS 3.10
     */
    QVariantList extraIdentifierValues() const;

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     * \since QGIS 3.10
     */
    void setExtraIdentifierValues( const QVariantList &extraIdentifierValues );


  signals:

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     */
    void identifierFieldsChanged();

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model.
     */
    void extraIdentifierValuesChanged();

  private:
    QgsFeatureExpressionValuesGatherer *createValuesGatherer( const QgsFeatureRequest &request ) const override;

    void requestToReloadCurrentFeature( QgsFeatureRequest &request ) override SIP_FORCE;

    QSet<QString> requestedAttributes() const override;

    QVariant entryIdentifier( const QgsFeatureExpressionValuesGatherer::Entry &entry ) const override;

    QgsFeatureExpressionValuesGatherer::Entry createEntry( const QVariant &identifier ) const override;

    bool compareEntries( const QgsFeatureExpressionValuesGatherer::Entry &a, const QgsFeatureExpressionValuesGatherer::Entry &b ) const override;

    bool identifierIsNull( const QVariant &identifier ) const override;

    QVariant nullIdentifier() const override;

    QStringList mIdentifierFields;
};

#endif // QGSFEATUREFILTERMODEL_H
