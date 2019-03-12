/***************************************************************************
    qgsdatumtransformtablewidget.cpp
     --------------------------------------
    Date                 : 28.11.2017
    Copyright            : (C) 2017 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatumtransformtablewidget.h"

#include "qgscoordinatetransform.h"
#include "qgsdatumtransformdialog.h"


QgsDatumTransformTableModel::QgsDatumTransformTableModel( QObject *parent )
  : QAbstractTableModel( parent )
{
}

void QgsDatumTransformTableModel::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
  reset();
}

void QgsDatumTransformTableModel::removeTransform( const QModelIndexList &indexes )
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
    const QgsDatumTransform::TransformPair transforms = mTransformContext.sourceDestinationDatumTransforms().value( crses );
    sourceTransform = transforms.sourceTransformId;
    destinationTransform = transforms.destinationTransformId;
#ifdef singlesourcedest
  }
#endif

  switch ( role )
  {
    case Qt::FontRole:
    {
      QFont font;
      font.setPointSize( font.pointSize() - 1 );
      return font;
    }
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
      switch ( index.column() )
      {
        case SourceCrsColumn:
          return sourceCrs;
        case SourceTransformColumn:
          if ( sourceTransform != -1 )
          {
            return QgsDatumTransform::datumTransformToProj( sourceTransform );
          }
          break;
        case DestinationCrsColumn:
          return destinationCrs;
        case DestinationTransformColumn:
          if ( destinationTransform != -1 )
          {
            return QgsDatumTransform::datumTransformToProj( destinationTransform );
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
    case Qt::ToolTipRole:
      switch ( section )
      {
        case SourceCrsColumn :
          return tr( "Source CRS" );
        case SourceTransformColumn:
          return tr( "Source Datum Transform" );
        case DestinationCrsColumn:
          return tr( "Destination CRS" );
        case DestinationTransformColumn:
          return tr( "Destination Datum Transform" );
        default:
          break;
      }
      break;
    default:
      break;
  }

  return QVariant();
}


QgsDatumTransformTableWidget::QgsDatumTransformTableWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModel = new QgsDatumTransformTableModel( this );

  mTableView->setModel( mModel );
  mTableView->resizeColumnToContents( 0 );
  mTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
  mTableView->horizontalHeader()->show();
  mTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTableView->setAlternatingRowColors( true );
  connect( mAddButton, &QToolButton::clicked, this, &QgsDatumTransformTableWidget::addDatumTransform );
  connect( mRemoveButton, &QToolButton::clicked, this, &QgsDatumTransformTableWidget::removeDatumTransform );
  connect( mEditButton, &QToolButton::clicked, this, &QgsDatumTransformTableWidget::editDatumTransform );

  connect( mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsDatumTransformTableWidget::selectionChanged );
  mEditButton->setEnabled( false );
}

void QgsDatumTransformTableWidget::addDatumTransform()
{
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem(), QgsCoordinateReferenceSystem(), true );
  if ( dlg.exec() )
  {
    QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > dt = dlg.selectedDatumTransforms();
    QgsCoordinateTransformContext context = mModel->transformContext();
    context.addSourceDestinationDatumTransform( dt.first.first, dt.second.first, dt.first.second, dt.second.second );
    mModel->setTransformContext( context );
    selectionChanged();
  }
}

void QgsDatumTransformTableWidget::removeDatumTransform()
{
  QModelIndexList selectedIndexes = mTableView->selectionModel()->selectedIndexes();
  if ( selectedIndexes.count() > 0 )
  {
    mModel->removeTransform( selectedIndexes );
    selectionChanged();
  }
}

void QgsDatumTransformTableWidget::editDatumTransform()
{
  QModelIndexList selectedIndexes = mTableView->selectionModel()->selectedIndexes();
  if ( selectedIndexes.count() > 0 )
  {
    QgsCoordinateReferenceSystem sourceCrs;
    QgsCoordinateReferenceSystem destinationCrs;
    int sourceTransform = -1;
    int destinationTransform = -1;
    for ( QModelIndexList::const_iterator it = selectedIndexes.constBegin(); it != selectedIndexes.constEnd(); it ++ )
    {
      switch ( it->column() )
      {
        case QgsDatumTransformTableModel::SourceCrsColumn:
          sourceCrs = QgsCoordinateReferenceSystem( mModel->data( *it, Qt::DisplayRole ).toString() );
          break;
        case QgsDatumTransformTableModel::DestinationCrsColumn:
          destinationCrs = QgsCoordinateReferenceSystem( mModel->data( *it, Qt::DisplayRole ).toString() );
          break;
        case QgsDatumTransformTableModel::SourceTransformColumn:
          sourceTransform = mModel->data( *it, Qt::UserRole ).toInt();
          break;
        case QgsDatumTransformTableModel::DestinationTransformColumn:
          destinationTransform = mModel->data( *it, Qt::UserRole ).toInt();
          break;
        default:
          break;
      }
    }
    if ( sourceCrs.isValid() && destinationCrs.isValid() &&
         ( sourceTransform != -1 || destinationTransform != -1 ) )
    {
      QgsDatumTransformDialog dlg( sourceCrs, destinationCrs, true, qMakePair( sourceTransform, destinationTransform ) );
      if ( dlg.exec() )
      {
        QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > dt = dlg.selectedDatumTransforms();
        QgsCoordinateTransformContext context = mModel->transformContext();
        // QMap::insert takes care of replacing existing value
        context.addSourceDestinationDatumTransform( sourceCrs, destinationCrs, dt.first.second, dt.second.second );
        mModel->setTransformContext( context );
      }
    }
  }
}

void QgsDatumTransformTableWidget::selectionChanged( const QItemSelection &, const QItemSelection & )
{
  mEditButton->setEnabled( !mTableView->selectionModel()->selection().empty() );
}
