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
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgscolorrampimpl.h"
#include "qgslocaleawarenumericlineeditdelegate.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMimeData>
#include <QTextStream>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif


QgsPalettedRendererWidget::QgsPalettedRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent ): QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  mCalculatingProgressBar->hide();
  mCancelButton->hide();

  mContextMenu = new QMenu( tr( "Options" ), this );
  mContextMenu->addAction( tr( "Change Color…" ), this, SLOT( changeColor() ) );
  mContextMenu->addAction( tr( "Change Opacity…" ), this, SLOT( changeOpacity() ) );
  mContextMenu->addAction( tr( "Change Label…" ), this, SLOT( changeLabel() ) );

  mAdvancedMenu = new QMenu( tr( "Advanced Options" ), this );
  QAction *mLoadFromLayerAction = mAdvancedMenu->addAction( tr( "Load Classes from Layer" ) );
  connect( mLoadFromLayerAction, &QAction::triggered, this, &QgsPalettedRendererWidget::loadFromLayer );
  QAction *loadFromFile = mAdvancedMenu->addAction( tr( "Load Color Map from File…" ) );
  connect( loadFromFile, &QAction::triggered, this, &QgsPalettedRendererWidget::loadColorTable );
  QAction *exportToFile = mAdvancedMenu->addAction( tr( "Export Color Map to File…" ) );
  connect( exportToFile, &QAction::triggered, this, &QgsPalettedRendererWidget::saveColorTable );


  mButtonAdvanced->setMenu( mAdvancedMenu );

  mModel = new QgsPalettedRendererModel( this );
  mProxyModel = new QgsPalettedRendererProxyModel( this );
  mProxyModel->setSourceModel( mModel );
  mTreeView->setSortingEnabled( false );
  mTreeView->setModel( mProxyModel );

  connect( this, &QgsPalettedRendererWidget::widgetChanged, this, [ = ]
  {
    mProxyModel->sort( QgsPalettedRendererModel::Column::ValueColumn );
  } );

#ifdef ENABLE_MODELTEST
  new ModelTest( mModel, this );
#endif

  mTreeView->setItemDelegateForColumn( QgsPalettedRendererModel::ColorColumn, new QgsColorSwatchDelegate( this ) );
  mValueDelegate = new QgsLocaleAwareNumericLineEditDelegate( Qgis::DataType::UnknownDataType, this );
  mTreeView->setItemDelegateForColumn( QgsPalettedRendererModel::ValueColumn, mValueDelegate );

  mTreeView->setColumnWidth( QgsPalettedRendererModel::ColorColumn, Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 6.6 );
  mTreeView->setContextMenuPolicy( Qt::CustomContextMenu );
  mTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mTreeView->setDragEnabled( true );
  mTreeView->setAcceptDrops( true );
  mTreeView->setDropIndicatorShown( true );
  mTreeView->setDragDropMode( QAbstractItemView::InternalMove );
  mTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTreeView->setDefaultDropAction( Qt::MoveAction );

  connect( mTreeView, &QTreeView::customContextMenuRequested, this, [ = ]( QPoint ) { mContextMenu->exec( QCursor::pos() ); } );

  btnColorRamp->setShowRandomColorRamp( true );

  connect( btnColorRamp, &QgsColorRampButton::colorRampChanged, this, &QgsPalettedRendererWidget::applyColorRamp );

  mBandComboBox->setLayer( mRasterLayer );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }
    setFromRenderer( mRasterLayer->renderer() );
  }

  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterRendererWidget::widgetChanged );
  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
  connect( mDeleteEntryButton, &QPushButton::clicked, this, &QgsPalettedRendererWidget::deleteEntry );
  connect( mButtonDeleteAll, &QPushButton::clicked, mModel, &QgsPalettedRendererModel::deleteAll );
  connect( mAddEntryButton, &QPushButton::clicked, this, &QgsPalettedRendererWidget::addEntry );
  connect( mClassifyButton, &QPushButton::clicked, this, &QgsPalettedRendererWidget::classify );

  if ( mRasterLayer && mRasterLayer->dataProvider() )
  {
    mLoadFromLayerAction->setEnabled( !mRasterLayer->dataProvider()->colorTable( mBandComboBox->currentBand() ).isEmpty() );
  }
  else
  {
    mLoadFromLayerAction->setEnabled( false );
  }

  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( QgsMapLayer * ) >( &QgsProject::layerWillBeRemoved ), this, &QgsPalettedRendererWidget::layerWillBeRemoved );
  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsPalettedRendererWidget::bandChanged );
}

QgsPalettedRendererWidget::~QgsPalettedRendererWidget()
{
  if ( mGatherer )
  {
    mGatherer->stop();
    mGatherer->wait(); // mGatherer is deleted when wait completes
  }
}

QgsRasterRenderer *QgsPalettedRendererWidget::renderer()
{
  QgsPalettedRasterRenderer::ClassData classes = mProxyModel->classData();
  int bandNumber = mBandComboBox->currentBand();

  QgsPalettedRasterRenderer *r = new QgsPalettedRasterRenderer( mRasterLayer->dataProvider(), bandNumber, classes );
  if ( !btnColorRamp->isNull() )
  {
    r->setSourceColorRamp( btnColorRamp->colorRamp() );
  }
  return r;
}

void QgsPalettedRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsPalettedRasterRenderer *pr = dynamic_cast<const QgsPalettedRasterRenderer *>( r );
  if ( pr )
  {
    mBand = pr->band();
    whileBlocking( mBandComboBox )->setBand( mBand );

    //read values and colors and fill into tree widget
    mModel->setClassData( pr->classes() );

    if ( pr->sourceColorRamp() )
    {
      whileBlocking( btnColorRamp )->setColorRamp( pr->sourceColorRamp() );
    }
    else
    {
      std::unique_ptr< QgsColorRamp > ramp( new QgsRandomColorRamp() );
      whileBlocking( btnColorRamp )->setColorRamp( ramp.get() );
    }
  }
  else
  {
    loadFromLayer();
    std::unique_ptr< QgsColorRamp > ramp( new QgsRandomColorRamp() );
    whileBlocking( btnColorRamp )->setColorRamp( ramp.get() );
  }

  if ( mRasterLayer && mRasterLayer->dataProvider() )
  {
    mValueDelegate->setDataType( mRasterLayer->dataProvider()->dataType( mBand ) );
  }
}

void QgsPalettedRendererWidget::setSelectionColor( const QItemSelection &selection, const QColor &color )
{
  // don't want to emit widgetChanged multiple times
  disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

  QModelIndex colorIndex;
  const auto constSelection = selection;
  for ( const QItemSelectionRange &range : constSelection )
  {
    const auto constIndexes = range.indexes();
    for ( const QModelIndex &index : constIndexes )
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

  QItemSelection sel = mProxyModel->mapSelectionToSource( mTreeView->selectionModel()->selection() );
  const auto constSel = sel;
  for ( const QItemSelectionRange &range : constSel )
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

  QColor color( 150, 150, 150 );
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( ramp )
  {
    color = ramp->color( 1.0 );
  }
  QModelIndex newEntry = mModel->addEntry( color );
  mTreeView->scrollTo( newEntry );
  mTreeView->selectionModel()->select( mProxyModel->mapFromSource( newEntry ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
  emit widgetChanged();
}

void QgsPalettedRendererWidget::changeColor()
{
  QItemSelection sel = mProxyModel->mapSelectionToSource( mTreeView->selectionModel()->selection() );
  QModelIndex colorIndex = mModel->index( sel.first().top(), QgsPalettedRendererModel::ColorColumn );
  QColor currentColor = mModel->data( colorIndex, Qt::DisplayRole ).value<QColor>();

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
  if ( panel && panel->dockMode() )
  {
    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, currentColor, QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Select Color" ) );
    colorWidget->setAllowOpacity( true );
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

void QgsPalettedRendererWidget::changeOpacity()
{
  QItemSelection sel = mProxyModel->mapSelectionToSource( mTreeView->selectionModel()->selection() );
  QModelIndex colorIndex = mModel->index( sel.first().top(), QgsPalettedRendererModel::ColorColumn );
  QColor currentColor = mModel->data( colorIndex, Qt::DisplayRole ).value<QColor>();

  bool ok;
  double oldOpacity = ( currentColor.alpha() / 255.0 ) * 100.0;
  double opacity = QInputDialog::getDouble( this, tr( "Opacity" ), tr( "Change color opacity [%]" ), oldOpacity, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    int newOpacity = opacity / 100 * 255;

    // don't want to emit widgetChanged multiple times
    disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

    const auto constSel = sel;
    for ( const QItemSelectionRange &range : constSel )
    {
      const auto constIndexes = range.indexes();
      for ( const QModelIndex &index : constIndexes )
      {
        colorIndex = mModel->index( index.row(), QgsPalettedRendererModel::ColorColumn );

        QColor newColor = mModel->data( colorIndex, Qt::DisplayRole ).value<QColor>();
        newColor.setAlpha( newOpacity );
        mModel->setData( colorIndex, newColor, Qt::EditRole );
      }
    }
    connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

    emit widgetChanged();
  }
}

void QgsPalettedRendererWidget::changeLabel()
{
  QItemSelection sel = mProxyModel->mapSelectionToSource( mTreeView->selectionModel()->selection() );
  QModelIndex labelIndex = mModel->index( sel.first().top(), QgsPalettedRendererModel::LabelColumn );
  QString currentLabel = mModel->data( labelIndex, Qt::DisplayRole ).toString();

  bool ok;
  QString newLabel = QInputDialog::getText( this, tr( "Label" ), tr( "Change label" ), QLineEdit::Normal, currentLabel, &ok );
  if ( ok )
  {
    // don't want to emit widgetChanged multiple times
    disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

    const auto constSel = sel;
    for ( const QItemSelectionRange &range : constSel )
    {
      const auto constIndexes = range.indexes();
      for ( const QModelIndex &index : constIndexes )
      {
        labelIndex = mModel->index( index.row(), QgsPalettedRendererModel::LabelColumn );
        mModel->setData( labelIndex, newLabel, Qt::EditRole );
      }
    }
    connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

    emit widgetChanged();
  }
}

void QgsPalettedRendererWidget::applyColorRamp()
{
  std::unique_ptr< QgsColorRamp > ramp( btnColorRamp->colorRamp() );
  if ( !ramp )
  {
    return;
  }

  disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );

  QgsPalettedRasterRenderer::ClassData data = mProxyModel->classData();
  QgsPalettedRasterRenderer::ClassData::iterator cIt = data.begin();

  double numberOfEntries = data.count();
  int i = 0;

  if ( QgsRandomColorRamp *randomRamp = dynamic_cast<QgsRandomColorRamp *>( ramp.get() ) )
  {
    //ramp is a random colors ramp, so inform it of the total number of required colors
    //this allows the ramp to pregenerate a set of visually distinctive colors
    randomRamp->setTotalColorCount( numberOfEntries );
  }

  if ( numberOfEntries > 1 )
    numberOfEntries -= 1; //avoid duplicate first color

  for ( ; cIt != data.end(); ++cIt )
  {
    cIt->color = ramp->color( i / numberOfEntries );
    i++;
  }
  mModel->setClassData( data );

  connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
  emit widgetChanged();
}

void QgsPalettedRendererWidget::loadColorTable()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Color Table from File" ), lastDir );
  if ( !fileName.isEmpty() )
  {
    QgsPalettedRasterRenderer::ClassData classes = QgsPalettedRasterRenderer::classDataFromFile( fileName );
    if ( !classes.isEmpty() )
    {
      disconnect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
      mModel->setClassData( classes );
      emit widgetChanged();
      connect( mModel, &QgsPalettedRendererModel::classesChanged, this, &QgsPalettedRendererWidget::widgetChanged );
    }
    else
    {
      QMessageBox::critical( nullptr, tr( "Load Color Table" ), tr( "Could not interpret file as a raster color table." ) );
    }
  }
}

void QgsPalettedRendererWidget::saveColorTable()
{
  QgsSettings settings;
  QString lastDir = settings.value( QStringLiteral( "lastColorMapDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save Color Table as File" ), lastDir, tr( "Text (*.clr)" ) );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( QLatin1String( ".clr" ), Qt::CaseInsensitive ) )
    {
      fileName = fileName + ".clr";
    }

    QFile outputFile( fileName );
    if ( outputFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream outputStream( &outputFile );
      outputStream << QgsPalettedRasterRenderer::classDataToString( mProxyModel->classData() );
      outputStream.flush();
      outputFile.close();

      QFileInfo fileInfo( fileName );
      settings.setValue( QStringLiteral( "lastColorMapDir" ), fileInfo.absoluteDir().absolutePath() );
    }
    else
    {
      QMessageBox::warning( this, tr( "Save Color Table as File" ), tr( "Write access denied. Adjust the file permissions and try again.\n\n" ) );
    }
  }
}

void QgsPalettedRendererWidget::classify()
{
  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    if ( mGatherer )
    {
      mGatherer->stop();
      return;
    }

    mGatherer = new QgsPalettedRendererClassGatherer( mRasterLayer, mBandComboBox->currentBand(), mModel->classData(), btnColorRamp->colorRamp() );

    connect( mGatherer, &QgsPalettedRendererClassGatherer::progressChanged, mCalculatingProgressBar, [ = ]( int progress )
    {
      mCalculatingProgressBar->setValue( progress );
    } );

    mCalculatingProgressBar->show();
    mCancelButton->show();
    connect( mCancelButton, &QPushButton::clicked, mGatherer, &QgsPalettedRendererClassGatherer::stop );

    connect( mGatherer, &QgsPalettedRendererClassGatherer::collectedClasses, this, &QgsPalettedRendererWidget::gatheredClasses );
    connect( mGatherer, &QgsPalettedRendererClassGatherer::finished, this, &QgsPalettedRendererWidget::gathererThreadFinished );
    mClassifyButton->setText( tr( "Calculating…" ) );
    mClassifyButton->setEnabled( false );
    mGatherer->start();
  }
}

void QgsPalettedRendererWidget::loadFromLayer()
{
  //read default palette settings from layer
  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( provider )
  {
    QList<QgsColorRampShader::ColorRampItem> table = provider->colorTable( mBandComboBox->currentBand() );
    if ( !table.isEmpty() )
    {
      QgsPalettedRasterRenderer::ClassData classes = QgsPalettedRasterRenderer::colorTableToClassData( provider->colorTable( mBandComboBox->currentBand() ) );
      mModel->setClassData( classes );
      emit widgetChanged();
    }
  }
}

void QgsPalettedRendererWidget::bandChanged( int band )
{
  if ( band == mBand )
    return;

  if ( mRasterLayer && mRasterLayer->dataProvider( ) )
  {
    mValueDelegate->setDataType( mRasterLayer->dataProvider( )->dataType( mBand ) );
  }

  bool deleteExisting = false;
  if ( !mModel->classData().isEmpty() )
  {
    int res = QMessageBox::question( this,
                                     tr( "Delete Classification" ),
                                     tr( "The classification band was changed from %1 to %2.\n"
                                         "Should the existing classes be deleted?" ).arg( mBand ).arg( band ),
                                     QMessageBox::Yes | QMessageBox::No );

    deleteExisting = ( res == QMessageBox::Yes );
  }

  mBand = band;
  mModel->blockSignals( true );
  if ( deleteExisting )
    mModel->deleteAll();

  mModel->blockSignals( false );
  emit widgetChanged();
}

void QgsPalettedRendererWidget::gatheredClasses()
{
  if ( !mGatherer || mGatherer->wasCanceled() )
    return;

  mModel->setClassData( mGatherer->classes() );
  emit widgetChanged();
}

void QgsPalettedRendererWidget::gathererThreadFinished()
{
  mGatherer->deleteLater();
  mGatherer = nullptr;
  mClassifyButton->setText( tr( "Classify" ) );
  mClassifyButton->setEnabled( true );
  mCalculatingProgressBar->hide();
  mCancelButton->hide();
}

void QgsPalettedRendererWidget::layerWillBeRemoved( QgsMapLayer *layer )
{
  if ( mGatherer && mRasterLayer == layer )
  {
    mGatherer->stop();
    mGatherer->wait();
  }
}

//
// QgsPalettedRendererModel
//

///@cond PRIVATE
QgsPalettedRendererModel::QgsPalettedRendererModel( QObject *parent )
  : QAbstractItemModel( parent )
{

}

void QgsPalettedRendererModel::setClassData( const QgsPalettedRasterRenderer::ClassData &data )
{
  beginResetModel();
  mData = data;
  endResetModel();
}

QModelIndex QgsPalettedRendererModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    //column out of bounds
    return QModelIndex();
  }

  if ( !parent.isValid() && row >= 0 && row < mData.size() )
  {
    //return an index for the item at this position
    return createIndex( row, column );
  }

  //only top level supported
  return QModelIndex();
}

QModelIndex QgsPalettedRendererModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )

  //all items are top level
  return QModelIndex();
}

int QgsPalettedRendererModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 3;
}

int QgsPalettedRendererModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mData.count();
}

QVariant QgsPalettedRendererModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case ValueColumn:
          return mData.at( index.row() ).value;

        case ColorColumn:
          return mData.at( index.row() ).color;

        case LabelColumn:
          return mData.at( index.row() ).label;
      }
    }

    default:
      break;
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
      return QAbstractItemModel::headerData( section, orientation, role );
  }
  return QAbstractItemModel::headerData( section, orientation, role );
}

bool QgsPalettedRendererModel::setData( const QModelIndex &index, const QVariant &value, int )
{
  if ( !index.isValid() )
    return false;
  if ( index.row() >= mData.length() )
    return false;

  switch ( index.column() )
  {
    case ValueColumn:
    {
      bool ok = false;
      double newValue = value.toDouble( &ok );
      if ( !ok )
        return false;

      mData[ index.row() ].value = newValue;
      emit dataChanged( index, index );
      emit classesChanged();
      return true;
    }

    case ColorColumn:
    {
      mData[ index.row() ].color = value.value<QColor>();
      emit dataChanged( index, index );
      emit classesChanged();
      return true;
    }

    case LabelColumn:
    {
      mData[ index.row() ].label = value.toString();
      emit dataChanged( index, index );
      emit classesChanged();
      return true;
    }
  }

  return false;
}

Qt::ItemFlags QgsPalettedRendererModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QAbstractItemModel::flags( index ) | Qt::ItemIsDropEnabled;

  Qt::ItemFlags f = QAbstractItemModel::flags( index );
  switch ( index.column() )
  {
    case ValueColumn:
    case LabelColumn:
    case ColorColumn:
      f = f | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
      break;
  }
  return f | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool QgsPalettedRendererModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || row >= mData.count() )
    return false;
  if ( parent.isValid() )
    return false;

  for ( int i = row + count - 1; i >= row; --i )
  {
    beginRemoveRows( parent, i, i );
    mData.removeAt( i );
    endRemoveRows();
  }
  emit classesChanged();
  return true;
}

bool QgsPalettedRendererModel::insertRows( int row, int count, const QModelIndex & )
{
  QgsPalettedRasterRenderer::ClassData::const_iterator cIt = mData.constBegin();
  int currentMaxValue = -std::numeric_limits<int>::max();
  for ( ; cIt != mData.constEnd(); ++cIt )
  {
    int value = cIt->value;
    currentMaxValue = std::max( value, currentMaxValue );
  }
  int nextValue = std::max( 0, currentMaxValue + 1 );

  beginInsertRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row; i < row + count; ++i, ++nextValue )
  {
    mData.insert( i, QgsPalettedRasterRenderer::Class( nextValue, QColor( 200, 200, 200 ), QLocale().toString( nextValue ) ) );
  }
  endInsertRows();
  emit classesChanged();
  return true;
}

Qt::DropActions QgsPalettedRendererModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QStringList QgsPalettedRendererModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/x-qgspalettedrenderermodel" );
  return types;
}

QMimeData *QgsPalettedRendererModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  // Create list of rows
  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    if ( !index.isValid() || index.column() != 0 )
      continue;

    stream << index.row();
  }
  mimeData->setData( QStringLiteral( "application/x-qgspalettedrenderermodel" ), encodedData );
  return mimeData;
}

bool QgsPalettedRendererModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & )
{
  Q_UNUSED( column )
  if ( action != Qt::MoveAction ) return true;

  if ( !data->hasFormat( QStringLiteral( "application/x-qgspalettedrenderermodel" ) ) )
    return false;

  QByteArray encodedData = data->data( QStringLiteral( "application/x-qgspalettedrenderermodel" ) );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );

  QVector<int> rows;
  while ( !stream.atEnd() )
  {
    int r;
    stream >> r;
    rows.append( r );
  }

  QgsPalettedRasterRenderer::ClassData newData;
  for ( int i = 0; i < rows.count(); ++i )
    newData << mData.at( rows.at( i ) );

  if ( row < 0 )
    row = mData.count();

  beginInsertRows( QModelIndex(), row, row + rows.count() - 1 );
  for ( int i = 0; i < rows.count(); ++i )
    mData.insert( row + i, newData.at( i ) );
  endInsertRows();
  emit classesChanged();
  return true;
}

QModelIndex QgsPalettedRendererModel::addEntry( const QColor &color )
{
  insertRow( rowCount() );
  QModelIndex newRow = index( mData.count() - 1, 1 );
  setData( newRow, color );
  return newRow;
}

void QgsPalettedRendererModel::deleteAll()
{
  beginResetModel();
  mData.clear();
  endResetModel();
  emit classesChanged();
}

//
// QgsPalettedRendererClassGatherer
//

QgsPalettedRendererClassGatherer::QgsPalettedRendererClassGatherer( QgsRasterLayer *layer, int bandNumber, const QgsPalettedRasterRenderer::ClassData &existingClasses, QgsColorRamp *ramp )
  : mProvider( ( layer && layer->dataProvider() ) ? layer->dataProvider()->clone() : nullptr )
  , mBandNumber( bandNumber )
  , mRamp( ramp )
  , mClasses( existingClasses )
  , mWasCanceled( false )
{}

void QgsPalettedRendererClassGatherer::run()
{
  mWasCanceled = false;

  // allow responsive cancellation
  mFeedback = new QgsRasterBlockFeedback();
  connect( mFeedback, &QgsRasterBlockFeedback::progressChanged, this, &QgsPalettedRendererClassGatherer::progressChanged );

  if ( mProvider )
  {
    QgsPalettedRasterRenderer::ClassData newClasses = QgsPalettedRasterRenderer::classDataFromRaster( mProvider.get(), mBandNumber, mRamp.get(), mFeedback );

    // combine existing classes with new classes
    QgsPalettedRasterRenderer::ClassData::iterator classIt = newClasses.begin();
    emit progressChanged( 0 );
    qlonglong i = 0;
    for ( ; classIt != newClasses.end(); ++classIt )
    {
      // check if existing classes contains this same class
      for ( const QgsPalettedRasterRenderer::Class &existingClass : std::as_const( mClasses ) )
      {
        if ( existingClass.value == classIt->value )
        {
          classIt->color = existingClass.color;
          classIt->label = existingClass.label;
          break;
        }
      }
      i ++;
      emit progressChanged( 100 * ( static_cast< double >( i ) / static_cast<double>( newClasses.count() ) ) );
    }
    mClasses = newClasses;
  }

  // be overly cautious - it's *possible* stop() might be called between deleting mFeedback and nulling it
  mFeedbackMutex.lock();
  delete mFeedback;
  mFeedback = nullptr;
  mFeedbackMutex.unlock();

  emit collectedClasses();
}


QgsPalettedRasterRenderer::ClassData QgsPalettedRendererProxyModel::classData() const
{
  QgsPalettedRasterRenderer::ClassData data;
  for ( int i = 0; i < rowCount( ); ++i )
  {
    data.push_back( qobject_cast<QgsPalettedRendererModel *>( sourceModel() )->classAtIndex( mapToSource( index( i, 0 ) ) ) );
  }
  return data;
}


///@endcond PRIVATE
