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

/**
 * \ingroup core
 * Provides a list of features based on filter conditions.
 * Features are fetched asynchronously.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeaturePickerModel : public QgsFeaturePickerModelBase<QgsFeatureByIdExpressionValuesGatherer>
{
    Q_OBJECT

  public:

    /**
     * Create a new QgsFeaturePickerModel, optionally specifying a \a parent.
     */
    explicit QgsFeaturePickerModel( QObject *parent = nullptr );
    ~QgsFeaturePickerModel() override;

    /**
     * Allows specifying one value that does not need to match the filter criteria but will
     * still be available in the model as NULL value(s).
     * \since QGIS 3.10
     */
    void setExtraIdentifierValueToNull() override;


  private:
    void requestToReloadCurrentFeature( QgsFeatureRequest &request ) override;

    void setCurrentFeatureUnguarded( const QgsFeatureId &featureId );

    QgsFeatureId mFeatureFidToReload = FID_NULL;

};

#endif // QGSFEATUREPICKERMODEL_H
