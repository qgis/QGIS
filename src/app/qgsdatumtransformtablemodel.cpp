#include "qgsdatumtransformtablemodel.h"
#include "qgscoordinatetransform.h"

QgsDatumTransformTableModel::QgsDatumTransformTableModel( QObject *parent )
  : QAbstractTableModel( parent )
{
}

void QgsDatumTransformTableModel::setTransformContext( QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
  reset();
}


int QgsDatumTransformTableModel::rowCount( const QModelIndex &parent ) const
{
  return mTransformContext.sourceDestinationDatumTransforms().count()
         + mTransformContext.sourceDatumTransforms().count()
         + mTransformContext.destinationDatumTransforms().count();
}

int QgsDatumTransformTableModel::columnCount( const QModelIndex &parent ) const
{
  return 4;
}

QVariant QgsDatumTransformTableModel::data( const QModelIndex &index, int role ) const
{
  QString sourceCrs;
  QString destinationCrs;
  int sourceTransform = -1;
  int destinationTransform = -1;

  if ( index.row() < mTransformContext.sourceDestinationDatumTransforms().count() )
  {
    QPair< QString, QString> crses = mTransformContext.sourceDestinationDatumTransforms().keys().at( index.row() );
    sourceCrs = crses.first;
    destinationCrs = crses.second;
    QPair< int, int> transforms = mTransformContext.sourceDestinationDatumTransforms().value( crses );
    sourceTransform = transforms.first;
    destinationTransform = transforms.second;
  }

  switch ( role )
  {
    case Qt::DisplayRole:
      switch ( index.column() )
      {
        case SourceCrsColumn:
          return sourceCrs;
        case SourceTransformColumn:
          if (sourceTransform != -1)
          {
              return QgsCoordinateTransform::datumTransformString( sourceTransform );
          }
          break;
        case DestinationCrsColumn:
          return destinationCrs;
        case DestinationTransformColumn:
          if (sourceTransform != -1)
          {
              return QgsCoordinateTransform::datumTransformString( destinationTransform );
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  return QVariant();
}

QVariant QgsDatumTransformTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Vertical )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
      switch ( section )
      {
        case SourceCrsColumn :
          return tr( "Source CRS" );
        case SourceTransformColumn:
          return tr( "Source datum transform" );
        case DestinationCrsColumn:
          return tr( "Destination CRS" );
        case DestinationTransformColumn:
          return tr( "Destination datum transform" );
        default:
          break;
      }
      break;
    default:
      break;
  }

  return QVariant();
}
