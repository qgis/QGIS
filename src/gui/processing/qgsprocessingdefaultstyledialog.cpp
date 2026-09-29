/***************************************************************************
                             qgsprocessingdefaultstyledialog.cpp
                             ------------------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingdefaultstyledialog.h"

#include "qgsapplication.h"
#include "qgsfilewidget.h"
#include "qgsgui.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingdefaultstyleregistry.h"
#include "qgsprocessingregistry.h"

#include <QString>

#include "moc_qgsprocessingdefaultstyledialog.cpp"

using namespace Qt::StringLiterals;
///@cond PRIVATE

//
// QgsProcessingDefaultStyleDelegate
//

QgsProcessingDefaultStyleDelegate::QgsProcessingDefaultStyleDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{}

QWidget *QgsProcessingDefaultStyleDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == 1 )
  {
    auto *editor = new QgsFileWidget( parent );
    editor->setStorageMode( QgsFileWidget::StorageMode::GetFile );
    editor->setFilter( u"%1 (*.qml *.QML)"_s.arg( tr( "QGIS Layer Style File" ) ) );

    return editor;
  }
  return QStyledItemDelegate::createEditor( parent, option, index );
}

void QgsProcessingDefaultStyleDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  if ( auto *fileWidget = qobject_cast<QgsFileWidget *>( editor ) )
  {
    fileWidget->setFilePath( index.model()->data( index, Qt::EditRole ).toString() );
  }
  else
  {
    QStyledItemDelegate::setEditorData( editor, index );
  }
}

void QgsProcessingDefaultStyleDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( auto *fileWidget = qobject_cast<QgsFileWidget *>( editor ) )
  {
    model->setData( index, fileWidget->filePath(), Qt::EditRole );
  }
  else
  {
    QStyledItemDelegate::setModelData( editor, model, index );
  }
}

void QgsProcessingDefaultStyleDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & ) const
{
  editor->setGeometry( option.rect );
}

//
// QgsProcessingDefaultStylesModel
//

QgsProcessingDefaultStylesModel::QgsProcessingDefaultStylesModel( const QgsProcessingAlgorithm *algorithm, QObject *parent )
  : QAbstractTableModel( parent )
{
  mAlgorithmId = algorithm->id();
  QgsProcessingDefaultStyleRegistry *defaultStyleRegistry = QgsApplication::processingRegistry()->defaultStyleRegistry();

  const QgsProcessingOutputDefinitions outputDefs = algorithm->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *output : outputDefs )
  {
    if ( dynamic_cast<const QgsProcessingOutputVectorLayer *>( output )
         || dynamic_cast<const QgsProcessingOutputRasterLayer *>( output )
         || dynamic_cast<const QgsProcessingOutputVectorTileLayer *>( output )
         || dynamic_cast<const QgsProcessingOutputPointCloudLayer *>( output ) )
    {
      OutputItem item;
      item.name = output->name();
      item.description = output->description();
      item.stylePath = defaultStyleRegistry->defaultStyleForOutput( mAlgorithmId, output->name() );
      mItems.append( item );
    }
  }
}

int QgsProcessingDefaultStylesModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mItems.size();
}

int QgsProcessingDefaultStylesModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return 2;
}

QVariant QgsProcessingDefaultStylesModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mItems.size() )
    return QVariant();

  const OutputItem &item = mItems.at( index.row() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
      switch ( index.column() )
      {
        case Column::OutputName:
        {
          return item.description.isEmpty() ? item.name : item.description;
        }

        case Column::StylePath:
          return item.stylePath;

        default:
          break;
      }
      break;

    default:
      break;
  }

  return QVariant();
}

bool QgsProcessingDefaultStylesModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.isValid() && index.column() == Column::StylePath && role == Qt::EditRole )
  {
    mItems[index.row()].stylePath = value.toString();
    emit dataChanged( index, index, { role, Qt::DisplayRole } );
    return true;
  }
  return false;
}

Qt::ItemFlags QgsProcessingDefaultStylesModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if ( index.column() == Column::StylePath )
  {
    flags |= Qt::ItemIsEditable;
  }
  return flags;
}

QVariant QgsProcessingDefaultStylesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( orientation )
  {
    case Qt::Horizontal:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        {
          switch ( section )
          {
            case Column::OutputName:
              return tr( "Output" );
            case Column::StylePath:
              return tr( "Style File" );
            default:
              break;
          }
          break;
        }

        default:
          break;
      }

      break;
    }
    case Qt::Vertical:
      break;
  }

  return QAbstractTableModel::headerData( section, orientation, role );
}

void QgsProcessingDefaultStylesModel::saveStyles()
{
  QgsProcessingDefaultStyleRegistry *registry = QgsApplication::processingRegistry()->defaultStyleRegistry();
  for ( const OutputItem &item : std::as_const( mItems ) )
  {
    registry->setDefaultStyleForOutput( mAlgorithmId, item.name, item.stylePath );
  }
  registry->saveStyles();
}
///@endcond

//
// QgsProcessingDefaultStyleDialog
//

QgsProcessingDefaultStyleDialog::QgsProcessingDefaultStyleDialog( const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QDialog( parent )
  , mAlgorithm( algorithm )
{
  setupUi( this );

  setObjectName( "QgsProcessingDefaultStyleDialog" );
  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( algorithm->displayName() );

  mModel = new QgsProcessingDefaultStylesModel( mAlgorithm, this );
  mDelegate = new QgsProcessingDefaultStyleDelegate( this );
  mTableStyles->setModel( mModel );
  mTableStyles->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mTableStyles->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
  mTableStyles->setSelectionBehavior( QAbstractItemView::SelectRows );

  mTableStyles->setItemDelegateForColumn( 1, mDelegate );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
}

void QgsProcessingDefaultStyleDialog::accept()
{
  mModel->saveStyles();
  QDialog::accept();
}

void QgsProcessingDefaultStyleDialog::showEvent( QShowEvent *event )
{
  QDialog::showEvent( event );

  if ( !mInitialWidthsSet )
  {
    mInitialWidthsSet = true;

    int halfWidth = mTableStyles->viewport()->width() / 2;
    mTableStyles->setColumnWidth( 0, halfWidth );
    mTableStyles->setColumnWidth( 1, halfWidth );
  }
}
