/***************************************************************************
  qgsfeaturepickermodel.h - QgsFeaturePickerModel
 ---------------------
 begin                : 03.04.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREPICKERMODEL_H
#define QGSFEATUREPICKERMODEL_H

#include <QAbstractItemModel>

#include "qgsconditionalstyle.h"
#include "qgsfeatureexpressionvaluesgatherer.h"
#include "qgsfeaturepickermodelbase.h"

/**
 * \ingroup core
 * \brief Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 *
 */
class CORE_EXPORT QgsFeaturePickerModel : public QgsFeaturePickerModelBase
{
    Q_OBJECT

  public:

    /**
     * Create a new QgsFeaturePickerModel, optionally specifying a \a parent.
     */
    explicit QgsFeaturePickerModel( QObject *parent = nullptr );

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model as NULL value(s).
     * \since QGIS 3.10
     */
    void setExtraIdentifierValueToNull() override;

    //! Set the feature to the given feature id
    void setFeature( const QgsFeatureId &fid );

    /**
     * Returns the current feature
     */
    QgsFeature feature() const;

  signals:
    //! Emitted when the current feature changes
    void featureChanged( const QgsFeature &feature );


  private:
    QgsFeatureExpressionValuesGatherer *createValuesGatherer( const QgsFeatureRequest &request ) const override;

    void requestToReloadCurrentFeature( QgsFeatureRequest &request ) override;

    QVariant entryIdentifier( const QgsFeatureExpressionValuesGatherer::Entry &entry ) const override;

    QgsFeatureExpressionValuesGatherer::Entry createEntry( const QVariant &identifier ) const override;

    bool compareEntries( const QgsFeatureExpressionValuesGatherer::Entry &a, const QgsFeatureExpressionValuesGatherer::Entry &b ) const override;

    bool identifierIsNull( const QVariant &identifier ) const override;

    QVariant nullIdentifier() const override;
};

#endif // QGSFEATUREPICKERMODEL_H
