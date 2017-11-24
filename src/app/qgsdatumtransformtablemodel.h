#ifndef QGSDATUMTRANSFORMTABLEMODEL_H
#define QGSDATUMTRANSFORMTABLEMODEL_H


#include <QAbstractTableModel>

#include "qgis_app.h"
#include "qgscoordinatetransformcontext.h"

class APP_EXPORT QgsDatumTransformTableModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    enum TableColumns
    {
      SourceCrsColumn  = 0,
      SourceTransformColumn,
      DestinationCrsColumn,
      DestinationTransformColumn,
    };

    QgsDatumTransformTableModel( QObject *parent = nullptr );

    void setTransformContext( QgsCoordinateTransformContext &context );

    QgsCoordinateTransformContext transformContext() {return mTransformContext;}

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

  private:

    QgsCoordinateTransformContext mTransformContext;
};

#endif // QGSDATUMTRANSFORMTABLEMODEL_H
