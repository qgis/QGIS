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
    QgsGeometryValidationModel( QgsGeometryValidationService *geometryValidationService, QObject *parent = nullptr );
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QgsVectorLayer *currentLayer() const;
    void setCurrentLayer( QgsVectorLayer *currentLayer );

  private:
    QgsGeometryValidationService *mGeometryValidationService = nullptr;
    QgsVectorLayer *mCurrentLayer = nullptr;
    mutable QgsExpression mDisplayExpression;
    mutable QgsExpressionContext mExpressionContext;
};

#endif // QGSGEOMETRYVALIDATIONMODEL_H
