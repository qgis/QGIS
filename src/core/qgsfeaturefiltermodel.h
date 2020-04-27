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
 * Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeatureFilterModel : public QgsFeaturePickerModelBase<QgsFeatureByIdentifierFieldsExpressionValuesGatherer>
{
    Q_OBJECT

    /**
     * A field of sourceLayer that is unique and should be used to identify features.
     * Normally the primary key field.
     * Needs to match the identifierValue.
     */
    Q_PROPERTY( QString identifierFields READ identifierFields WRITE setIdentifierFields NOTIFY identifierFieldChangeds )

  public:

    /**
     * Extra roles that can be used to fetch data from this model.
     */
    enum Role
    {
      IdentifierValueRole = Qt::UserRole, //!< \deprecated Use IdentifierValuesRole instead
      IdentifierValuesRole, //!< Used to retrieve the identifierValues (primary keys) of a feature.
      ValueRole //!< Used to retrieve the displayExpression of a feature.
    };

    /**
     * Create a new QgsFeatureFilterModel, optionally specifying a \a parent.
     */
    explicit QgsFeatureFilterModel( QObject *parent = nullptr );

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     * If there are several identifier fields defined, the behavior is not guaranteed
     * \deprecated since QGIS 3.10 use identifierFields instead
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

    QVariantList extraIdentifierValue() const override;


  signals:

    /**
     * The identifier field should be a unique field that can be used to identify individual features.
     * It is normally set to the primary key of the layer.
     */
    void identifierFieldsChanged();

  protected:
    QgsFeatureByIdentifierFieldsExpressionValuesGatherer createValuesGatherer( const QgsFeatureRequest &request ) const override;


  private:
    void requestToReloadCurrentFeature( QgsFeatureRequest &request ) override;

    QSet<QString> requestedAttributes() const override;

    QStringList mIdentifierFields;
};

#endif // QGSFEATUREFILTERMODEL_H
