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
#include "qgisapp.h"


QgsDatumTransformTableModel::QgsDatumTransformTableModel( QObject *parent )
  : QAbstractTableModel( parent )
{
}

void QgsDatumTransformTableModel::setTransformContext( const QgsCoordinateTransformContext &context )
{
  beginResetModel();
  mTransformContext = context;
  endResetModel();
}

void QgsDatumTransformTableModel::removeTransform( const QModelIndexList &indexes )
{
  QgsCoordinateReferenceSystem sourceCrs;
  QgsCoordinateReferenceSystem destinationCrs;
  for ( QModelIndexList::const_iterator it = indexes.constBegin(); it != indexes.constEnd(); ++it )
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
      beginResetModel();
      mTransformContext.removeCoordinateOperation( sourceCrs, destinationCrs );
      endResetModel();
      break;
    }
  }
}


int QgsDatumTransformTableModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
#if PROJ_VERSION_MAJOR>=6
  return mTransformContext.coordinateOperations().count();
#else
  Q_NOWARN_DEPRECATED_PUSH
  return mTransformContext.sourceDestinationDatumTransforms().count();
  Q_NOWARN_DEPRECATED_POP
#endif
}

int QgsDatumTransformTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
#if PROJ_VERSION_MAJOR>=6
  return 4;
#else
  return 4;
#endif
}

QVariant QgsDatumTransformTableModel::data( const QModelIndex &index, int role ) const
{
  QString sourceCrs;
  QString destinationCrs;
#if PROJ_VERSION_MAJOR>=6
  QPair< QString, QString> crses = mTransformContext.coordinateOperations().keys().at( index.row() );
#else
  int sourceTransform = -1;
  int destinationTransform = -1;
  Q_NOWARN_DEPRECATED_PUSH
  QPair< QString, QString> crses = mTransformContext.sourceDestinationDatumTransforms().keys().at( index.row() );
  Q_NOWARN_DEPRECATED_POP
#endif
  sourceCrs = crses.first;
  destinationCrs = crses.second;
#if PROJ_VERSION_MAJOR>=6
  const QString proj = mTransformContext.coordinateOperations().value( crses );
#else
  Q_NOWARN_DEPRECATED_PUSH
  const QgsDatumTransform::TransformPair transforms = mTransformContext.sourceDestinationDatumTransforms().value( crses );
  Q_NOWARN_DEPRECATED_POP
  sourceTransform = transforms.sourceTransformId;
  destinationTransform = transforms.destinationTransformId;
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
        case DestinationCrsColumn:
          return destinationCrs;

#if PROJ_VERSION_MAJOR>=6
        case ProjDefinitionColumn:
          return proj;
#else
        case SourceTransformColumn:
          if ( sourceTransform != -1 )
          {
            Q_NOWARN_DEPRECATED_PUSH
            return QgsDatumTransform::datumTransformToProj( sourceTransform );
            Q_NOWARN_DEPRECATED_POP
          }
          break;

        case DestinationTransformColumn:
          if ( destinationTransform != -1 )
          {
            Q_NOWARN_DEPRECATED_PUSH
            return QgsDatumTransform::datumTransformToProj( destinationTransform );
            Q_NOWARN_DEPRECATED_POP
          }
          break;
#endif

        default:
          break;
      }
      break;

    case Qt::CheckStateRole:
#if PROJ_VERSION_MAJOR>=6
      switch ( index.column() )
      {
        case AllowFallbackColumn:
          return mTransformContext.allowFallbackTransform( QgsCoordinateReferenceSystem( crses.first ), QgsCoordinateReferenceSystem( crses.second ) ) ? Qt::Checked : Qt::Unchecked;
        default:
          break;
      }
      break;
#endif

    case Qt::UserRole:
#if PROJ_VERSION_MAJOR>=6
      return proj;
#else
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
#endif
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
        case DestinationCrsColumn:
          return tr( "Destination CRS" );

#if PROJ_VERSION_MAJOR>=6
        case ProjDefinitionColumn:
          return tr( "Operation" );
        case AllowFallbackColumn:
          return tr( "Allow Fallback Transforms" );
#else
        case SourceTransformColumn:
          return tr( "Source Datum Transform" );
        case DestinationTransformColumn:
          return tr( "Destination Datum Transform" );
#endif
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
  connect( mEditButton, &QToolButton::clicked, this, [ = ]
  {
    QModelIndexList selectedIndexes = mTableView->selectionModel()->selectedIndexes();
    if ( selectedIndexes.count() > 0 )
    {
      editDatumTransform( selectedIndexes.at( 0 ) );
    }
  } );

  connect( mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsDatumTransformTableWidget::selectionChanged );

  connect( mTableView, &QTableView::doubleClicked, this, [ = ]( const QModelIndex & index )
  {
    editDatumTransform( index );
  } );
  mEditButton->setEnabled( false );
}

void QgsDatumTransformTableWidget::addDatumTransform()
{
  QgsDatumTransformDialog dlg( QgsCoordinateReferenceSystem(), QgsCoordinateReferenceSystem(), true, false, false, QPair< int, int >(), nullptr, nullptr, QString(), QgisApp::instance()->mapCanvas() );
  if ( dlg.exec() )
  {
    const QgsDatumTransformDialog::TransformInfo dt = dlg.selectedDatumTransform();
    QgsCoordinateTransformContext context = mModel->transformContext();
    Q_NOWARN_DEPRECATED_PUSH
    context.addSourceDestinationDatumTransform( dt.sourceCrs, dt.destinationCrs, dt.sourceTransformId, dt.destinationTransformId );
    Q_NOWARN_DEPRECATED_POP
    context.addCoordinateOperation( dt.sourceCrs, dt.destinationCrs, dt.proj, dt.allowFallback );
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

void QgsDatumTransformTableWidget::editDatumTransform( const QModelIndex &index )
{
  QString proj;
  int sourceTransform = -1;
  int destinationTransform = -1;

  QgsCoordinateReferenceSystem sourceCrs = QgsCoordinateReferenceSystem( mModel->data( mModel->index( index.row(), QgsDatumTransformTableModel::SourceCrsColumn ), Qt::DisplayRole ).toString() );
  QgsCoordinateReferenceSystem destinationCrs = QgsCoordinateReferenceSystem( mModel->data( mModel->index( index.row(), QgsDatumTransformTableModel::DestinationCrsColumn ), Qt::DisplayRole ).toString() );

  bool allowFallback = true;
#if PROJ_VERSION_MAJOR>=6
  proj = mModel->data( mModel->index( index.row(), QgsDatumTransformTableModel::ProjDefinitionColumn ), Qt::UserRole ).toString();
  allowFallback = mModel->data( mModel->index( index.row(), QgsDatumTransformTableModel::AllowFallbackColumn ), Qt::CheckStateRole ) == Qt::Checked;
#else
  sourceTransform = mModel->data( mModel->index( index.row(), QgsDatumTransformTableModel::SourceTransformColumn ), Qt::UserRole ).toInt();
  destinationTransform = mModel->data( mModel->index( index.row(), QgsDatumTransformTableModel::DestinationTransformColumn ), Qt::UserRole ).toInt();
#endif

#if PROJ_VERSION_MAJOR>=6
  if ( sourceCrs.isValid() && destinationCrs.isValid() )
#else
  if ( sourceCrs.isValid() && destinationCrs.isValid() &&
       ( sourceTransform != -1 || destinationTransform != -1 ) )
#endif
  {
    QgsDatumTransformDialog dlg( sourceCrs, destinationCrs, true, false, false, qMakePair( sourceTransform, destinationTransform ), nullptr, nullptr, proj, QgisApp::instance()->mapCanvas(), allowFallback );
    if ( dlg.exec() )
    {
      const QgsDatumTransformDialog::TransformInfo dt = dlg.selectedDatumTransform();
      QgsCoordinateTransformContext context = mModel->transformContext();
      if ( sourceCrs != dt.sourceCrs || destinationCrs != dt.destinationCrs )
      {
        context.removeCoordinateOperation( sourceCrs, destinationCrs );
        Q_NOWARN_DEPRECATED_PUSH
        context.removeSourceDestinationDatumTransform( sourceCrs, destinationCrs );
        Q_NOWARN_DEPRECATED_POP
      }
      // QMap::insert takes care of replacing existing value
      Q_NOWARN_DEPRECATED_PUSH
      context.addSourceDestinationDatumTransform( dt.sourceCrs, dt.destinationCrs, dt.sourceTransformId, dt.destinationTransformId );
      Q_NOWARN_DEPRECATED_POP
      context.addCoordinateOperation( dt.sourceCrs, dt.destinationCrs, dt.proj, dt.allowFallback );
      mModel->setTransformContext( context );
    }
  }
}

void QgsDatumTransformTableWidget::selectionChanged( const QItemSelection &, const QItemSelection & )
{
  mEditButton->setEnabled( !mTableView->selectionModel()->selection().empty() );
}
