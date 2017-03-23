/***************************************************************************
                         qgspalettedrendererwidget.cpp
                         -----------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspalettedrendererwidget.h"
#include "qgspalettedrasterrenderer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgscolordialog.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QMenu>

QgsPalettedRendererWidget::QgsPalettedRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent ): QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  contextMenu = new QMenu( tr( "Options" ), this );
  contextMenu->addAction( tr( "Change color" ), this, SLOT( changeColor() ) );
  contextMenu->addAction( tr( "Change transparency" ), this, SLOT( changeTransparency() ) );

  mModel = new QgsPalettedRendererModel( this );
  mTreeView->setSortingEnabled( false );
  mTreeView->setModel( mModel );

  mSwatchDelegate = new QgsColorSwatchDelegate( this );
  mTreeView->setItemDelegateForColumn( QgsPalettedRendererModel::ColorColumn, mSwatchDelegate );

  mTreeView->setColumnWidth( QgsPalettedRendererModel::ColorColumn, 50 );
  mTreeView->setContextMenuPolicy( Qt::CustomContextMenu );
  mTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( mTreeView, &QTreeView::customContextMenuRequested,  [ = ]( const QPoint & ) { contextMenu->exec( QCursor::pos() ); }
         );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    //fill available bands into combo box
    int nBands = provider->bandCount();
    for ( int i = 1; i <= nBands; ++i ) //band numbering seem to start at 1
    {
      mBandComboBox->addItem( displayBandName( i ), i );
    }

    setFromRenderer( mRasterLayer->renderer() );
    connect( mBandComboBox, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( widgetChanged() ) );
  }

  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
  connect( mDeleteEntryButton, &QPushButton::clicked, this, &QgsPalettedRendererWidget::deleteEntry );
  connect( mAddEntryButton, &QPushButton::clicked, this, &QgsPalettedRendererWidget::addEntry );
}

QgsRasterRenderer *QgsPalettedRendererWidget::renderer()
{
  QgsPalettedRasterRenderer::ClassData classes = mModel->classData();
  int bandNumber = mBandComboBox->currentData().toInt();
  return new QgsPalettedRasterRenderer( mRasterLayer->dataProvider(), bandNumber, classes );
}

void QgsPalettedRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsPalettedRasterRenderer *pr = dynamic_cast<const QgsPalettedRasterRenderer *>( r );
  if ( pr )
  {
    //read values and colors and fill into tree widget
    mModel->setClassData( pr->classes() );
  }
  else
  {
    //read default palette settings from layer
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( provider )
    {
      QgsPalettedRasterRenderer::ClassData classes = QgsPalettedRasterRenderer::colorTableToClassData( provider->colorTable( mBandComboBox->currentData().toInt() ) );
      mModel->setClassData( classes );
    }
  }
}

void QgsPalettedRendererWidget::setSelectionColor( const QItemSelection &selection, const QColor &color )
{
  // don't want to emit widgetChanged multiple times
  disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

  QModelIndex colorIndex;
  Q_FOREACH ( const QItemSelectionRange &range, selection )
  {
    Q_FOREACH ( const QModelIndex &index, range.indexes() )
    {
      colorIndex = mModel->index( index.row(), QgsPalettedRendererModel::ColorColumn );
      mModel->setData( colorIndex, color, Qt::EditRole );
    }
  }
  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

  emit widgetChanged();
}

void QgsPalettedRendererWidget::deleteEntry()
{
  // don't want to emit widgetChanged multiple times
  disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

  QItemSelection sel = mTreeView->selectionModel()->selection();
  Q_FOREACH ( const QItemSelectionRange &range, sel )
  {
    if ( range.isValid() )
      mModel->removeRows( range.top(), range.bottom() - range.top() + 1, range.parent() );
  }

  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

  emit widgetChanged();
}

void QgsPalettedRendererWidget::addEntry()
{
  disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
  mModel->insertRow( mModel->rowCount() );
  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
  emit widgetChanged();
}

void QgsPalettedRendererWidget::changeColor()
{
  QItemSelection sel = mTreeView->selectionModel()->selection();

  QModelIndex colorIndex = mModel->index( sel.first().top(), QgsPalettedRendererModel::ColorColumn );
  QColor currentColor = mModel->data( colorIndex, Qt::DisplayRole ).value<QColor>();

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, currentColor, QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Select color" ) );
    colorWidget->setAllowAlpha( true );
    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, [ = ]( const QColor & color ) { setSelectionColor( sel, color ); } );
    panel->openPanel( colorWidget );
  }
  else
  {
    // modal dialog version... yuck
    QColor newColor = QgsColorDialog::getColor( currentColor, this, QStringLiteral( "Change color" ), true );
    if ( newColor.isValid() )
    {
      setSelectionColor( sel, newColor );
    }
  }
}

void QgsPalettedRendererWidget::changeTransparency()
{
  QItemSelection sel = mTreeView->selectionModel()->selection();

  QModelIndex colorIndex = mModel->index( sel.first().top(), QgsPalettedRendererModel::ColorColumn );
  QColor currentColor = mModel->data( colorIndex, Qt::DisplayRole ).value<QColor>();

  bool ok;
  double oldTransparency = ( currentColor.alpha() / 255.0 ) * 100.0;
  double transparency = QInputDialog::getDouble( this, tr( "Transparency" ), tr( "Change color transparency [%]" ), oldTransparency, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    int newTransparency = transparency / 100 * 255;

    // don't want to emit widgetChanged multiple times
    disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

    Q_FOREACH ( const QItemSelectionRange &range, sel )
    {
      Q_FOREACH ( const QModelIndex &index, range.indexes() )
      {
        colorIndex = mModel->index( index.row(), QgsPalettedRendererModel::ColorColumn );

        QColor newColor = mModel->data( colorIndex, Qt::DisplayRole ).value<QColor>();
        newColor.setAlpha( newTransparency );
        mModel->setData( colorIndex, newColor, Qt::EditRole );
      }
    }
    connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

    emit widgetChanged();
  }
}

//
// QgsPalettedRendererModel
//

///@cond PRIVATE
QgsPalettedRendererModel::QgsPalettedRendererModel( QObject *parent )
  : QAbstractTableModel( parent )
{

}

void QgsPalettedRendererModel::setClassData( const QgsPalettedRasterRenderer::ClassData &data )
{
  beginResetModel();
  mData = data;
  endResetModel();
}

int QgsPalettedRendererModel::columnCount( const QModelIndex & ) const
{
  return 3;
}

int QgsPalettedRendererModel::rowCount( const QModelIndex & ) const
{
  return mData.count();
}

QVariant QgsPalettedRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= mData.count() )
    return QVariant();
  if ( index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case ValueColumn:
          return mData.keys().at( index.row() );

        case ColorColumn:
          return mData.value( mData.keys().at( index.row() ) ).color;

        case LabelColumn:
          return mData.value( mData.keys().at( index.row() ) ).label;
      }
    }

    default:
      return QVariant();
  }

  return QVariant();
}

QVariant QgsPalettedRendererModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( orientation )
  {
    case Qt::Vertical:
      return QVariant();

    case Qt::Horizontal:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        {
          switch ( section )
          {
            case ValueColumn:
              return tr( "Value" );

            case ColorColumn:
              return tr( "Color" );

            case LabelColumn:
              return tr( "Label" );
          }
        }

      }
      break;
    }

    default:
      return QAbstractTableModel::headerData( section, orientation, role );
  }
  return QAbstractTableModel::headerData( section, orientation, role );
}

bool QgsPalettedRendererModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.row() < 0 || index.row() >= mData.count() )
    return false;
  if ( index.column() < 0 || index.column() >= columnCount() )
    return false;

  switch ( role )
  {
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case ValueColumn:
        {
          // make sure value is unique
          bool ok = false;
          int newValue = value.toInt( &ok );
          if ( !ok )
            return false;

          for ( int i = 0; i < rowCount(); ++i )
          {
            if ( i == index.row() )
              continue;
            if ( data( QgsPalettedRendererModel::index( i, ValueColumn ) ).toInt() == newValue )
            {
              return false;
            }
          }

          QgsPalettedRasterRenderer::ClassData newData;
          QgsPalettedRasterRenderer::ClassData::const_iterator cIt = mData.constBegin();
          for ( int i = 0; cIt != mData.constEnd(); ++i, ++cIt )
          {
            if ( i == index.row() )
              newData.insert( newValue, cIt.value() );
            else
              newData.insert( cIt.key(), cIt.value() );
          }
          mData = newData;

          emit dataChanged( index, index, QVector< int >() << Qt::EditRole << Qt::DisplayRole );
          emit classesChanged();
          return true;
        }

        case ColorColumn:
        {
          int pixValue = mData.keys().at( index.row() );
          mData[ pixValue ].color = value.value<QColor>();
          emit dataChanged( index, index, QVector< int >() << Qt::EditRole << Qt::DisplayRole );
          emit classesChanged();
          return true;
        }

        case LabelColumn:
        {
          int pixValue = mData.keys().at( index.row() );
          mData[ pixValue ].label = value.toString();
          emit dataChanged( index, index, QVector< int >() << Qt::EditRole << Qt::DisplayRole );
          emit classesChanged();
          return true;
        }
      }
    }
  }

  return QAbstractTableModel::setData( index, value, role );
}

Qt::ItemFlags QgsPalettedRendererModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags f = QAbstractTableModel::flags( index );
  switch ( index.column() )
  {
    case ValueColumn:
    case LabelColumn:
    case ColorColumn:
      f = f | Qt::ItemIsEditable;
      break;
  }
  return f;
}

bool QgsPalettedRendererModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || row >= mData.count() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int i = 0; i < count; i++ )
  {
    mData.remove( mData.keys().at( row ) );
  }

  endRemoveRows();
  return true;
}

bool QgsPalettedRendererModel::insertRows( int row, int count, const QModelIndex & )
{
  beginInsertRows( QModelIndex(), row, row + count - 1 );

  QgsPalettedRasterRenderer::ClassData::const_iterator cIt = mData.constBegin();
  int currentMaxValue = -INT_MAX;
  for ( ; cIt != mData.constEnd(); ++cIt )
  {
    int value = cIt.key();
    currentMaxValue = qMax( value, currentMaxValue );
  }
  int nextValue = qMax( 0, currentMaxValue + 1 );

  QgsPalettedRasterRenderer::ClassData newData;
  cIt = mData.constBegin();
  bool insertedRows = false;
  for ( int i = 0; !insertedRows; ++i )
  {
    if ( i == row )
    {
      for ( int j = nextValue; j < nextValue + count; ++j )
      {
        newData.insert( j, QgsPalettedRasterRenderer::Class( QColor( 200, 200, 200 ), QString::number( j ) ) );
      }
      insertedRows = true;
    }
    if ( cIt != mData.constEnd() )
    {
      newData.insert( cIt.key(), cIt.value() );
      cIt++;
    }
  }
  mData = newData;
  endInsertRows();
  return true;
}

///@endcond PRIVATE

