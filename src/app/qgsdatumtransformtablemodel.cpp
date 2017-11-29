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

void QgsDatumTransformTableModel::removeTransform( QModelIndexList indexes )
{
  QgsCoordinateReferenceSystem sourceCrs;
  QgsCoordinateReferenceSystem destinationCrs;
  for ( QModelIndexList::const_iterator it = indexes.constBegin(); it != indexes.constEnd(); it ++ )
  {
    if ( it->column() == SourceCrsColumn )
    {
      sourceCrs = QgsCoordinateReferenceSystem( data( *it, Qt::DisplayRole ).toString() );
    }
    if ( it->column() == DestinationCrsColumn )
    {
      destinationCrs = QgsCoordinateReferenceSystem( data( *it, Qt::DisplayRole ).toString() );
    }
    if ( sourceCrs.isValid() && destinationCrs.isValid() )
    {
      mTransformContext.removeSourceDestinationDatumTransform( sourceCrs, destinationCrs );
      reset();
      break;
    }
  }
}


int QgsDatumTransformTableModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mTransformContext.sourceDestinationDatumTransforms().count()
#ifdef singlesourcedest
         + mTransformContext.sourceDatumTransforms().count()
         + mTransformContext.destinationDatumTransforms().count()
#endif
         ;
}

int QgsDatumTransformTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 4;
}

QVariant QgsDatumTransformTableModel::data( const QModelIndex &index, int role ) const
{
  QString sourceCrs;
  QString destinationCrs;
  int sourceTransform = -1;
  int destinationTransform = -1;

#ifdef singlesourcedest
  if ( index.row() < mTransformContext.sourceDestinationDatumTransforms().count() )
  {
#endif
    QPair< QString, QString> crses = mTransformContext.sourceDestinationDatumTransforms().keys().at( index.row() );
    sourceCrs = crses.first;
    destinationCrs = crses.second;
    QPair< int, int> transforms = mTransformContext.sourceDestinationDatumTransforms().value( crses );
    sourceTransform = transforms.first;
    destinationTransform = transforms.second;
#ifdef singlesourcedest
  }
#endif

  switch ( role )
  {
    case Qt::DisplayRole:
      switch ( index.column() )
      {
        case SourceCrsColumn:
          return sourceCrs;
        case SourceTransformColumn:
          if ( sourceTransform != -1 )
          {
            return QgsCoordinateTransform::datumTransformString( sourceTransform );
          }
          break;
        case DestinationCrsColumn:
          return destinationCrs;
        case DestinationTransformColumn:
          if ( destinationTransform != -1 )
          {
            return QgsCoordinateTransform::datumTransformString( destinationTransform );
          }
          break;
        default:
          break;
      }
      break;
    case Qt::UserRole:
      switch ( index.column() )
      {
        case SourceTransformColumn:
          return sourceTransform;
        case DestinationTransformColumn:
          return destinationTransform;
        default:
          break;
      }
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
