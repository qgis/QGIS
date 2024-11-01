/***************************************************************************
                      qgsgeometryvalidationmodel.h
                     --------------------------------------
Date                 : 6.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEOMETRYVALIDATIONMODEL_H
#define QGSGEOMETRYVALIDATIONMODEL_H

#include <QAbstractItemModel>

#include "qgsgeometryvalidationservice.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"

class QgsGeometryValidationModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Roles
    {
      FeatureExtentRole = Qt::UserRole,
      ProblemExtentRole,
      ErrorGeometryRole,
      ErrorFeatureIdRole,
      FeatureGeometryRole,
      ErrorLocationGeometryRole,
      GeometryCheckErrorRole,
      DetailsRole
    };

    QgsGeometryValidationModel( QgsGeometryValidationService *geometryValidationService, QObject *parent = nullptr );

    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    QgsVectorLayer *currentLayer() const;

  signals:

    /**
     * Emitted before single geometry check results are removed.
     * This is guaranteed to be emitted before the models regular
     * aboutToRemoveRows() signal.
     */
    void aboutToRemoveSingleGeometryCheck();

  public slots:
    void setCurrentLayer( QgsVectorLayer *currentLayer );

  private slots:
    void onSingleGeometryCheckCleared( QgsVectorLayer *layer );
    void onGeometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<std::shared_ptr<QgsSingleGeometryCheckError>> &errors );
    void onGeometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid );
    void onTopologyChecksUpdated( QgsVectorLayer *layer, const QList<std::shared_ptr<QgsGeometryCheckError>> &errors );
    void onTopologyChecksCleared( QgsVectorLayer *layer );
    void onTopologyErrorUpdated( QgsVectorLayer *layer, QgsGeometryCheckError *error );

  private:
    struct FeatureErrors
    {
        FeatureErrors() = default;

        FeatureErrors( QgsFeatureId fid )
          : fid( fid )
        {}

        QgsFeatureId fid = FID_NULL;
        QList<std::shared_ptr<QgsSingleGeometryCheckError>> errors;
    };

    int errorsForFeature( QgsVectorLayer *layer, QgsFeatureId fid );

    QgsFeature getFeature( QgsFeatureId fid ) const;

    QgsGeometryValidationService *mGeometryValidationService = nullptr;
    QgsVectorLayer *mCurrentLayer = nullptr;
    mutable QgsExpression mDisplayExpression;
    mutable QStringList mRequiredAttributes;
    mutable QgsExpressionContext mExpressionContext;

    QMap<QgsVectorLayer *, QList<FeatureErrors>> mErrorStorage;
    QMap<QgsVectorLayer *, QList<std::shared_ptr<QgsGeometryCheckError>>> mTopologyErrorStorage;
    mutable QgsFeature mCachedFeature;
};

#endif // QGSGEOMETRYVALIDATIONMODEL_H
