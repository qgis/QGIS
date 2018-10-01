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

  public slots:
    void setCurrentLayer( QgsVectorLayer *currentLayer );

  private slots:
    void onGeometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<std::shared_ptr<QgsSingleGeometryCheckError> > &errors );
    void onGeometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid );
    void onTopologyChecksUpdated( QgsVectorLayer *layer, const QList<std::shared_ptr<QgsGeometryCheckError> > &errors );
    void onTopologyChecksCleared( QgsVectorLayer *layer );

  private:
    struct FeatureErrors
    {
      FeatureErrors()
      {}

      FeatureErrors( QgsFeatureId fid )
        : fid( fid )
      {}

      QgsFeatureId fid; // TODO INITIALIZE PROPERLY
      QList<std::shared_ptr<QgsSingleGeometryCheckError>> errors;
    };

    int errorsForFeature( QgsVectorLayer *layer, QgsFeatureId fid );

    QgsGeometryValidationService *mGeometryValidationService = nullptr;
    QgsVectorLayer *mCurrentLayer = nullptr;
    mutable QgsExpression mDisplayExpression;
    mutable QgsExpressionContext mExpressionContext;

    QMap<QgsVectorLayer *, QList< FeatureErrors > > mErrorStorage;
    QMap<QgsVectorLayer *, QList< std::shared_ptr< QgsGeometryCheckError > > > mTopologyErrorStorage;
};

#endif // QGSGEOMETRYVALIDATIONMODEL_H
