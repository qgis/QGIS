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


QgsDatumTransformTableWidget::QgsDatumTransformTableWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    mTableView->setModel( mModel );
    mTableView->resizeColumnToContents( 0 );
    mTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
    mTableView->horizontalHeader()->show();
    mTableView->setSelectionMode( QAbstractItemView::SingleSelection );
    mTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
    connect( mAddButton, &QToolButton::clicked, this, &QgsDatumTransformTableWidget::addDatumTransform );
    connect( mRemoveButton, &QToolButton::clicked, this, &QgsDatumTransformTableWidget::removeDatumTransform );
    connect( mEditButton, &QToolButton::clicked, this, &QgsDatumTransformTableWidget::editDatumTransform );

}

QgsDatumTransformTableWidget::~QgsDatumTransformTableWidget()
{
}


void QgsDatumTransformTableWidget::addDatumTransform()
{
  QgsDatumTransformDialog *dlg = new QgsDatumTransformDialog();
  if ( dlg->exec() )
  {
    QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > dt = dlg->selectedDatumTransforms();
    QgsCoordinateTransformContext context = mModel->transformContext();
    context.addSourceDestinationDatumTransform( dt.first.first, dt.second.first, dt.first.second, dt.second.second );
    mModel->setTransformContext( context );
  }
}

void QgsDatumTransformTableWidget::removeDatumTransform()
{
  QModelIndexList selectedIndexes = mTableView->selectionModel()->selectedIndexes();
  if ( selectedIndexes.count() > 0 )
  {
    mModel->removeTransform( selectedIndexes );
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
      if ( it->column() == QgsDatumTransformTableModel::SourceCrsColumn )
      {
        sourceCrs = QgsCoordinateReferenceSystem( mModel->data( *it, Qt::DisplayRole ).toString() );
      }
      if ( it->column() == QgsDatumTransformTableModel::DestinationCrsColumn )
      {
        destinationCrs = QgsCoordinateReferenceSystem( mModel->data( *it, Qt::DisplayRole ).toString() );
      }
      if ( it->column() == QgsDatumTransformTableModel::SourceTransformColumn )
      {
        sourceTransform = mModel->data( *it, Qt::UserRole ).toInt();
      }
      if ( it->column() == QgsDatumTransformTableModel::DestinationTransformColumn )
      {
        destinationTransform = mModel->data( *it, Qt::UserRole ).toInt();
      }
    }
    if ( sourceCrs.isValid() && destinationCrs.isValid() &&
         ( sourceTransform != -1 || destinationTransform != -1 ) )
    {
      QgsDatumTransformDialog *dlg = new QgsDatumTransformDialog( sourceCrs, destinationCrs, qMakePair( sourceTransform, destinationTransform ) );
      if ( dlg->exec() )
      {
        QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > dt = dlg->selectedDatumTransforms();
        QgsCoordinateTransformContext context = mModel->transformContext();
        // QMap::insert takes care of replacing existing value
        context.addSourceDestinationDatumTransform( sourceCrs, destinationCrs, dt.first.second, dt.second.second );
        mModel->setTransformContext( context );
      }
    }
  }
}
