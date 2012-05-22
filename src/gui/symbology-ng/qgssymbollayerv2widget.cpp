/***************************************************************************
    qgssymbollayerv2widget.cpp - symbol layer widgets

    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbollayerv2widget.h"

#include "qgslinesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"

#include "characterwidget.h"
#include "qgsdashspacedialog.h"
#include "qgssymbolv2propertiesdialog.h"
#include "qgssvgcache.h"

#include "qgsapplication.h"

#include "qgslogger.h"

#include <QAbstractButton>
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QPainter>
#include <QSettings>
#include <QStandardItemModel>
#include <QSvgRenderer>



QgsSimpleLineSymbolLayerV2Widget::QgsSimpleLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );

  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( penWidthChanged() ) );
  connect( btnChangeColor, SIGNAL( clicked() ), this, SLOT( colorChanged() ) );
  connect( cboPenStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  connect( spinOffset, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( cboCapStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  connect( cboJoinStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( penStyleChanged() ) );
  updatePatternIcon();

}

void QgsSimpleLineSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SimpleLine" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleLineSymbolLayerV2*>( layer );

  // set values
  spinWidth->setValue( mLayer->width() );
  btnChangeColor->setColor( mLayer->color() );
  spinOffset->setValue( mLayer->offset() );
  cboPenStyle->blockSignals( true );
  cboJoinStyle->blockSignals( true );
  cboCapStyle->blockSignals( true );
  cboPenStyle->setPenStyle( mLayer->penStyle() );
  cboJoinStyle->setPenJoinStyle( mLayer->penJoinStyle() );
  cboCapStyle->setPenCapStyle( mLayer->penCapStyle() );
  cboPenStyle->blockSignals( false );
  cboJoinStyle->blockSignals( false );
  cboCapStyle->blockSignals( false );

  //use a custom dash pattern?
  bool useCustomDashPattern = mLayer->useCustomDashPattern();
  mChangePatternButton->setEnabled( useCustomDashPattern );
  label_3->setEnabled( !useCustomDashPattern );
  cboPenStyle->setEnabled( !useCustomDashPattern );
  mCustomCheckBox->blockSignals( true );
  mCustomCheckBox->setCheckState( useCustomDashPattern ? Qt::Checked : Qt::Unchecked );
  mCustomCheckBox->blockSignals( false );
  updatePatternIcon();
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleLineSymbolLayerV2Widget::penWidthChanged()
{
  mLayer->setWidth( spinWidth->value() );
  updatePatternIcon();
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::colorChanged()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mLayer->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mLayer->color(), this );
#endif
  if ( !color.isValid() )
    return;
  mLayer->setColor( color );
  btnChangeColor->setColor( mLayer->color() );
  updatePatternIcon();
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::penStyleChanged()
{
  mLayer->setPenStyle( cboPenStyle->penStyle() );
  mLayer->setPenJoinStyle( cboJoinStyle->penJoinStyle() );
  mLayer->setPenCapStyle( cboCapStyle->penCapStyle() );
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::offsetChanged()
{
  mLayer->setOffset( spinOffset->value() );
  updatePatternIcon();
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::on_mCustomCheckBox_stateChanged( int state )
{
  bool checked = ( state == Qt::Checked );
  mChangePatternButton->setEnabled( checked );
  label_3->setEnabled( !checked );
  cboPenStyle->setEnabled( !checked );

  mLayer->setUseCustomDashPattern( checked );
  emit changed();
}

void QgsSimpleLineSymbolLayerV2Widget::on_mChangePatternButton_clicked()
{
  QgsDashSpaceDialog d( mLayer->customDashVector() );
  if ( d.exec() == QDialog::Accepted )
  {
    mLayer->setCustomDashVector( d.dashDotVector() );
    updatePatternIcon();
    emit changed();
  }
}

void QgsSimpleLineSymbolLayerV2Widget::updatePatternIcon()
{
  if ( !mLayer )
  {
    return;
  }
  QgsSimpleLineSymbolLayerV2* layerCopy = dynamic_cast<QgsSimpleLineSymbolLayerV2*>( mLayer->clone() );
  if ( !layerCopy )
  {
    return;
  }
  layerCopy->setUseCustomDashPattern( true );
  QIcon buttonIcon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( layerCopy, QgsSymbolV2::MM, mChangePatternButton->iconSize() );
  mChangePatternButton->setIcon( buttonIcon );
  delete layerCopy;
}


///////////


QgsSimpleMarkerSymbolLayerV2Widget::QgsSimpleMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );

  QSize size = lstNames->iconSize();
  QStringList names;
  names << "circle" << "rectangle" << "diamond" << "pentagon" << "cross" << "cross2" << "triangle"
  << "equilateral_triangle" << "star" << "regular_star" << "arrow" << "line" << "arrowhead" << "filled_arrowhead";
  double markerSize = DEFAULT_POINT_SIZE * 2;
  for ( int i = 0; i < names.count(); ++i )
  {
    QgsSimpleMarkerSymbolLayerV2* lyr = new QgsSimpleMarkerSymbolLayerV2( names[i], QColor( 200, 200, 200 ), QColor( 0, 0, 0 ), markerSize );
    QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( lyr, QgsSymbolV2::MM, size );
    QListWidgetItem* item = new QListWidgetItem( icon, QString(), lstNames );
    item->setData( Qt::UserRole, names[i] );
    delete lyr;
  }

  connect( lstNames, SIGNAL( currentRowChanged( int ) ), this, SLOT( setName() ) );
  connect( btnChangeColorBorder, SIGNAL( clicked() ), this, SLOT( setColorBorder() ) );
  connect( btnChangeColorFill, SIGNAL( clicked() ), this, SLOT( setColorFill() ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
}

void QgsSimpleMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SimpleMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleMarkerSymbolLayerV2*>( layer );

  // set values
  QString name = mLayer->name();
  for ( int i = 0; i < lstNames->count(); ++i )
  {
    if ( lstNames->item( i )->data( Qt::UserRole ).toString() == name )
    {
      lstNames->setCurrentRow( i );
      break;
    }
  }
  btnChangeColorBorder->setColor( mLayer->borderColor() );
  btnChangeColorFill->setColor( mLayer->color() );
  spinSize->setValue( mLayer->size() );
  spinAngle->setValue( mLayer->angle() );

  // without blocking signals the value gets changed because of slot setOffset()
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );
}

QgsSymbolLayerV2* QgsSimpleMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleMarkerSymbolLayerV2Widget::setName()
{
  mLayer->setName( lstNames->currentItem()->data( Qt::UserRole ).toString() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setColorBorder()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor borderColor = QColorDialog::getColor( mLayer->borderColor(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor borderColor = QColorDialog::getColor( mLayer->borderColor(), this );
#endif
  if ( !borderColor.isValid() )
    return;
  mLayer->setBorderColor( borderColor );
  btnChangeColorBorder->setColor( mLayer->borderColor() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setColorFill()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mLayer->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mLayer->color(), this );
#endif
  if ( !color.isValid() )
    return;
  mLayer->setColor( color );
  btnChangeColorFill->setColor( mLayer->color() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setSize()
{
  mLayer->setSize( spinSize->value() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setAngle()
{
  mLayer->setAngle( spinAngle->value() );
  emit changed();
}

void QgsSimpleMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}


///////////

QgsSimpleFillSymbolLayerV2Widget::QgsSimpleFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );

  connect( btnChangeColor, SIGNAL( clicked() ), this, SLOT( setColor() ) );
  connect( cboFillStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setBrushStyle() ) );
  connect( btnChangeBorderColor, SIGNAL( clicked() ), this, SLOT( setBorderColor() ) );
  connect( spinBorderWidth, SIGNAL( valueChanged( double ) ), this, SLOT( borderWidthChanged() ) );
  connect( cboBorderStyle, SIGNAL( currentIndexChanged( int ) ), this, SLOT( borderStyleChanged() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( offsetChanged() ) );
}

void QgsSimpleFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SimpleFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSimpleFillSymbolLayerV2*>( layer );

  // set values
  btnChangeColor->setColor( mLayer->color() );
  cboFillStyle->setBrushStyle( mLayer->brushStyle() );
  btnChangeBorderColor->setColor( mLayer->borderColor() );
  cboBorderStyle->setPenStyle( mLayer->borderStyle() );
  spinBorderWidth->setValue( mLayer->borderWidth() );
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );
}

QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSimpleFillSymbolLayerV2Widget::setColor()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mLayer->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mLayer->color(), this );
#endif
  if ( !color.isValid() )
    return;
  mLayer->setColor( color );
  btnChangeColor->setColor( mLayer->color() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::setBorderColor()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mLayer->borderColor(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mLayer->borderColor(), this );
#endif
  if ( !color.isValid() )
    return;
  mLayer->setBorderColor( color );
  btnChangeBorderColor->setColor( mLayer->borderColor() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::setBrushStyle()
{
  mLayer->setBrushStyle( cboFillStyle->brushStyle() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::borderWidthChanged()
{
  mLayer->setBorderWidth( spinBorderWidth->value() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::borderStyleChanged()
{
  mLayer->setBorderStyle( cboBorderStyle->penStyle() );
  emit changed();
}

void QgsSimpleFillSymbolLayerV2Widget::offsetChanged()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

///////////

QgsMarkerLineSymbolLayerV2Widget::QgsMarkerLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );

  connect( spinInterval, SIGNAL( valueChanged( double ) ), this, SLOT( setInterval( double ) ) );
  connect( btnChangeMarker, SIGNAL( clicked() ), this, SLOT( setMarker() ) );
  connect( chkRotateMarker, SIGNAL( clicked() ), this, SLOT( setRotate() ) );
  connect( spinOffset, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( radInterval, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radVertex, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radVertexLast, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radVertexFirst, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
  connect( radCentralPoint, SIGNAL( clicked() ), this, SLOT( setPlacement() ) );
}

void QgsMarkerLineSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "MarkerLine" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsMarkerLineSymbolLayerV2*>( layer );

  // set values
  spinInterval->setValue( mLayer->interval() );
  chkRotateMarker->setChecked( mLayer->rotateMarker() );
  spinOffset->setValue( mLayer->offset() );
  if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::Interval )
    radInterval->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::Vertex )
    radVertex->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::LastVertex )
    radVertexLast->setChecked( true );
  else if ( mLayer->placement() == QgsMarkerLineSymbolLayerV2::CentralPoint )
    radCentralPoint->setChecked( true );
  else
    radVertexFirst->setChecked( true );
  updateMarker();
  setPlacement(); // update gui
}

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsMarkerLineSymbolLayerV2Widget::setInterval( double val )
{
  mLayer->setInterval( val );
  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setMarker()
{
  QgsSymbolV2PropertiesDialog dlg( mLayer->subSymbol(), mVectorLayer, this );
  if ( dlg.exec() == 0 )
    return;
  updateMarker();

  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setRotate()
{
  mLayer->setRotateMarker( chkRotateMarker->isChecked() );
  emit changed();
}

void QgsMarkerLineSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( spinOffset->value() );
  emit changed();
}


void QgsMarkerLineSymbolLayerV2Widget::updateMarker()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLayer->subSymbol(), btnChangeMarker->iconSize() );
  btnChangeMarker->setIcon( icon );
}

void QgsMarkerLineSymbolLayerV2Widget::setPlacement()
{
  bool interval = radInterval->isChecked();
  spinInterval->setEnabled( interval );
  //mLayer->setPlacement( interval ? QgsMarkerLineSymbolLayerV2::Interval : QgsMarkerLineSymbolLayerV2::Vertex );
  if ( radInterval->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::Interval );
  else if ( radVertex->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::Vertex );
  else if ( radVertexLast->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::LastVertex );
  else if ( radVertexFirst->isChecked() )
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::FirstVertex );
  else
    mLayer->setPlacement( QgsMarkerLineSymbolLayerV2::CentralPoint );

  emit changed();
}

///////////


QgsSvgMarkerSymbolLayerV2Widget::QgsSvgMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  viewGroups->setHeaderHidden( true );

  populateList();

  connect( viewImages->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setName( const QModelIndex& ) ) );
  connect( viewGroups->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( populateIcons( const QModelIndex& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle() ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
}

#include <QTime>
#include <QAbstractListModel>
#include <QPixmapCache>
#include <QStyle>

class QgsSvgListModel : public QAbstractListModel
{
  public:
    QgsSvgListModel( QObject* parent ) : QAbstractListModel( parent )
    {
      mSvgFiles = QgsSvgMarkerSymbolLayerV2::listSvgFiles();
    }

    // Constructor to create model for icons in a specific path
    QgsSvgListModel( QObject* parent, QString path ) : QAbstractListModel( parent )
    {
      mSvgFiles = QgsSvgMarkerSymbolLayerV2::listSvgFilesAt( path );
    }

    int rowCount( const QModelIndex & parent = QModelIndex() ) const
    {
      Q_UNUSED( parent );
      return mSvgFiles.count();
    }

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const
    {
      QString entry = mSvgFiles.at( index.row() );

      if ( role == Qt::DecorationRole ) // icon
      {
        QPixmap pixmap;
        if ( !QPixmapCache::find( entry, pixmap ) )
        {
          // render SVG file
          QColor fill, outline;
          double outlineWidth;
          bool fillParam, outlineParam, outlineWidthParam;
          QgsSvgCache::instance()->containsParams( entry, fillParam, fill, outlineParam, outline, outlineWidthParam, outlineWidth );

          const QImage& img = QgsSvgCache::instance()->svgAsImage( entry, 30, fill, outline, outlineWidth, 3.5 /*appr. 88 dpi*/, 1.0 );
          pixmap = QPixmap::fromImage( img );
          QPixmapCache::insert( entry, pixmap );
        }

        return pixmap;
      }
      else if ( role == Qt::UserRole || role == Qt::ToolTipRole )
      {
        return entry;
      }

      return QVariant();
    }

  protected:
    QStringList mSvgFiles;
};

class QgsSvgGroupsModel : public QStandardItemModel
{
  public:
    QgsSvgGroupsModel( QObject* parent ) : QStandardItemModel( parent )
    {
      QStringList svgPaths = QgsApplication::svgPaths();
      QStandardItem *parentItem = invisibleRootItem();

      for ( int i = 0; i < svgPaths.size(); i++ )
      {
        QDir dir( svgPaths[i] );
        QStandardItem *baseGroup;

        if ( dir.path().contains( QgsApplication::pkgDataPath() ) )
        {
          baseGroup = new QStandardItem( QString( "App Symbols" ) );
        }
        else if ( dir.path().contains( QgsApplication::qgisSettingsDirPath() ) )
        {
          baseGroup = new QStandardItem( QString( "User Symbols" ) );
        }
        else
        {
          baseGroup = new QStandardItem( dir.dirName() );
        }
        baseGroup->setData( QVariant( svgPaths[i] ) );
        baseGroup->setEditable( false );
        baseGroup->setCheckable( false );
        baseGroup->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
        baseGroup->setToolTip( dir.path() );
        parentItem->appendRow( baseGroup );
        createTree( baseGroup );
        QgsDebugMsg( QString( "SVG base path %1: %2" ).arg( i ).arg( baseGroup->data().toString() ) );
      }
    }
  private:
    void createTree( QStandardItem* &parentGroup )
    {
      QDir parentDir( parentGroup->data().toString() );
      foreach( QString item, parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
      {
        QStandardItem* group = new QStandardItem( item );
        group->setData( QVariant( parentDir.path() + "/" + item ) );
        group->setEditable( false );
        group->setCheckable( false );
        group->setToolTip( parentDir.path() + "/" + item );
        group->setIcon( QgsApplication::style()->standardIcon( QStyle::SP_DirIcon ) );
        parentGroup->appendRow( group );
        createTree( group );
      }
    }
};

void QgsSvgMarkerSymbolLayerV2Widget::populateList()
{
  QgsSvgGroupsModel* g = new QgsSvgGroupsModel( viewGroups );
  viewGroups->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    viewGroups->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  // Initally load the icons in the List view without any grouping
  QgsSvgListModel* m = new QgsSvgListModel( viewImages );
  viewImages->setModel( m );
}

void QgsSvgMarkerSymbolLayerV2Widget::populateIcons( const QModelIndex& idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QgsSvgListModel* m = new QgsSvgListModel( viewImages, path );
  viewImages->setModel( m );

  connect( viewImages->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setName( const QModelIndex& ) ) );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setGuiForSvg( const QgsSvgMarkerSymbolLayerV2* layer )
{
  if ( !layer )
  {
    return;
  }

  //activate gui for svg parameters only if supported by the svg file
  bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth;
  QgsSvgCache::instance()->containsParams( layer->path(), hasFillParam, defaultFill, hasOutlineParam, defaultOutline, hasOutlineWidthParam, defaultOutlineWidth );
  mChangeColorButton->setEnabled( hasFillParam );
  mChangeBorderColorButton->setEnabled( hasOutlineParam );
  mBorderWidthSpinBox->setEnabled( hasOutlineWidthParam );

  mFileLineEdit->blockSignals( true );
  mFileLineEdit->setText( layer->path() );
  mFileLineEdit->blockSignals( false );

  mBorderWidthSpinBox->blockSignals( true );
  mBorderWidthSpinBox->setValue( layer->outlineWidth() );
  mBorderWidthSpinBox->blockSignals( false );

}


void QgsSvgMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "SvgMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsSvgMarkerSymbolLayerV2*>( layer );

  // set values

  QAbstractItemModel* m = viewImages->model();
  QItemSelectionModel* selModel = viewImages->selectionModel();
  for ( int i = 0; i < m->rowCount(); i++ )
  {
    QModelIndex idx( m->index( i, 0 ) );
    if ( m->data( idx ).toString() == mLayer->path() )
    {
      selModel->select( idx, QItemSelectionModel::SelectCurrent );
      selModel->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
      setName( idx );
      break;
    }
  }



  spinSize->setValue( mLayer->size() );
  spinAngle->setValue( mLayer->angle() );

  // without blocking signals the value gets changed because of slot setOffset()
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );

  setGuiForSvg( mLayer );

}

QgsSymbolLayerV2* QgsSvgMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsSvgMarkerSymbolLayerV2Widget::setName( const QModelIndex& idx )
{
  QString name = idx.data( Qt::UserRole ).toString();
  mLayer->setPath( name );
  mFileLineEdit->setText( name );

  setGuiForSvg( mLayer );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setSize()
{
  mLayer->setSize( spinSize->value() );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setAngle()
{
  mLayer->setAngle( spinAngle->value() );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mFileToolButton_clicked()
{
  QSettings s;
  QString file = QFileDialog::getOpenFileName( 0,
                 tr( "Select SVG file" ),
                 s.value( "/UI/lastSVGMarkerDir" ).toString(),
                 tr( "SVG files" ) + " (*.svg)" );
  QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() )
  {
    return;
  }
  mFileLineEdit->setText( file );
  mLayer->setPath( file );
  s.setValue( "/UI/lastSVGMarkerDir", fi.absolutePath() );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mFileLineEdit_textEdited( const QString& text )
{
  if ( !QFileInfo( text ).exists() )
  {
    return;
  }
  mLayer->setPath( text );
  setGuiForSvg( mLayer );
  emit changed();
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mChangeColorButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }
  QColor c = QColorDialog::getColor( mLayer->fillColor() );
  if ( c.isValid() )
  {
    mLayer->setFillColor( c );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mChangeBorderColorButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }
  QColor c = QColorDialog::getColor( mLayer->outlineColor() );
  if ( c.isValid() )
  {
    mLayer->setOutlineColor( c );
    emit changed();
  }
}

void QgsSvgMarkerSymbolLayerV2Widget::on_mBorderWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOutlineWidth( d );
    emit changed();
  }
}

///////////////

QgsLineDecorationSymbolLayerV2Widget::QgsLineDecorationSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );

  connect( btnChangeColor, SIGNAL( clicked() ), this, SLOT( colorChanged() ) );
  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( penWidthChanged() ) );
}

void QgsLineDecorationSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "LineDecoration" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsLineDecorationSymbolLayerV2*>( layer );

  // set values
  btnChangeColor->setColor( mLayer->color() );
  spinWidth->setValue( mLayer->width() );
}

QgsSymbolLayerV2* QgsLineDecorationSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsLineDecorationSymbolLayerV2Widget::colorChanged()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mLayer->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mLayer->color(), this );
#endif
  if ( !color.isValid() )
    return;
  mLayer->setColor( color );
  btnChangeColor->setColor( mLayer->color() );
  emit changed();
}

void QgsLineDecorationSymbolLayerV2Widget::penWidthChanged()
{
  mLayer->setWidth( spinWidth->value() );
  emit changed();
}

/////////////

#include <QFileDialog>

QgsSVGFillSymbolLayerWidget::QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = 0;
  setupUi( this );
  mSvgTreeView->setHeaderHidden( true );
  insertIcons();
  updateOutlineIcon();

  connect( mSvgListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setFile( const QModelIndex& ) ) );
  connect( mSvgTreeView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( populateIcons( const QModelIndex& ) ) );
}

void QgsSVGFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer )
  {
    return;
  }

  if ( layer->layerType() != "SVGFill" )
  {
    return;
  }

  mLayer = dynamic_cast<QgsSVGFillSymbolLayer*>( layer );
  if ( mLayer )
  {
    double width = mLayer->patternWidth();
    mTextureWidthSpinBox->setValue( width );
    mSVGLineEdit->setText( mLayer->svgFilePath() );
    mRotationSpinBox->setValue( mLayer->angle() );
  }
  updateOutlineIcon();
  updateParamGui();
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsSVGFillSymbolLayerWidget::on_mBrowseToolButton_clicked()
{
  QString filePath = QFileDialog::getOpenFileName( 0, tr( "Select svg texture file" ) );
  if ( !filePath.isNull() )
  {
    mSVGLineEdit->setText( filePath );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mTextureWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setPatternWidth( d );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mSVGLineEdit_textChanged( const QString & text )
{
  if ( !mLayer )
  {
    return;
  }

  QFileInfo fi( text );
  if ( !fi.exists() )
  {
    return;
  }
  mLayer->setSvgFilePath( text );
  emit changed();
  updateParamGui();
}

void QgsSVGFillSymbolLayerWidget::setFile( const QModelIndex& item )
{
  mSVGLineEdit->setText( item.data( Qt::UserRole ).toString() );
  updateParamGui();
}

void QgsSVGFillSymbolLayerWidget::insertIcons()
{
  QgsSvgGroupsModel* g = new QgsSvgGroupsModel( mSvgTreeView );
  mSvgTreeView->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    mSvgTreeView->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  QgsSvgListModel* m = new QgsSvgListModel( mSvgListView );
  mSvgListView->setModel( m );
}

void QgsSVGFillSymbolLayerWidget::populateIcons( const QModelIndex& idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QgsSvgListModel* m = new QgsSvgListModel( mSvgListView, path );
  mSvgListView->setModel( m );

  connect( mSvgListView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( setFile( const QModelIndex& ) ) );
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::on_mChangeOutlinePushButton_clicked()
{
  QgsSymbolV2PropertiesDialog dlg( mLayer->subSymbol(), mVectorLayer, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    return;
  }

  updateOutlineIcon();
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::on_mRotationSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setAngle( d );
  }
  emit changed();
}

void QgsSVGFillSymbolLayerWidget::updateOutlineIcon()
{
  if ( mLayer )
  {
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLayer->subSymbol(), mChangeOutlinePushButton->iconSize() );
    mChangeOutlinePushButton->setIcon( icon );
  }
}

void QgsSVGFillSymbolLayerWidget::updateParamGui()
{
  //activate gui for svg parameters only if supported by the svg file
  bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
  QColor defaultFill, defaultOutline;
  double defaultOutlineWidth;
  QgsSvgCache::instance()->containsParams( mSVGLineEdit->text(), hasFillParam, defaultFill, hasOutlineParam, defaultOutline, hasOutlineWidthParam, defaultOutlineWidth );
  mChangeColorButton->setEnabled( hasFillParam );
  mChangeBorderColorButton->setEnabled( hasOutlineParam );
  mBorderWidthSpinBox->setEnabled( hasOutlineWidthParam );
}

void QgsSVGFillSymbolLayerWidget::on_mChangeColorButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }
  QColor c = QColorDialog::getColor( mLayer->svgFillColor() );
  if ( c.isValid() )
  {
    mLayer->setSvgFillColor( c );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mChangeBorderColorButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }
  QColor c = QColorDialog::getColor( mLayer->svgOutlineColor() );
  if ( c.isValid() )
  {
    mLayer->setSvgOutlineColor( c );
    emit changed();
  }
}

void QgsSVGFillSymbolLayerWidget::on_mBorderWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setSvgOutlineWidth( d );
    emit changed();
  }
}

/////////////

QgsLinePatternFillSymbolLayerWidget::QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ):
    QgsSymbolLayerV2Widget( parent, vl ), mLayer( 0 )
{
  setupUi( this );
}

void QgsLinePatternFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "LinePatternFill" )
  {
    return;
  }

  QgsLinePatternFillSymbolLayer* patternLayer = static_cast<QgsLinePatternFillSymbolLayer*>( layer );
  if ( patternLayer )
  {
    mLayer = patternLayer;
    mAngleSpinBox->setValue( mLayer->lineAngle() );
    mDistanceSpinBox->setValue( mLayer->distance() );
    mLineWidthSpinBox->setValue( mLayer->lineWidth() );
    mOffsetSpinBox->setValue( mLayer->offset() );
  }
}

QgsSymbolLayerV2* QgsLinePatternFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsLinePatternFillSymbolLayerWidget::on_mAngleSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setLineAngle( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mDistanceSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDistance( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mLineWidthSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setLineWidth( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mOffsetSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setOffset( d );
    emit changed();
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mColorPushButton_clicked()
{
  if ( mLayer )
  {
    QColor c = QColorDialog::getColor( mLayer->color() );
    if ( c.isValid() )
    {
      mLayer->setColor( c );
      emit changed();
    }
  }
}

void QgsLinePatternFillSymbolLayerWidget::on_mOutlinePushButton_clicked()
{
  if ( mLayer )
  {
    QgsSymbolV2PropertiesDialog dlg( mLayer->subSymbol(), mVectorLayer, this );
    if ( dlg.exec() == QDialog::Rejected )
    {
      return;
    }

    //updateOutlineIcon();
    emit changed();
  }
}

/////////////

QgsPointPatternFillSymbolLayerWidget::QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ):
    QgsSymbolLayerV2Widget( parent, vl ), mLayer( 0 )
{
  setupUi( this );
  updateMarkerIcon();
}


void QgsPointPatternFillSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( !layer || layer->layerType() != "PointPatternFill" )
  {
    return;
  }

  mLayer = static_cast<QgsPointPatternFillSymbolLayer*>( layer );
  mHorizontalDistanceSpinBox->setValue( mLayer->distanceX() );
  mVerticalDistanceSpinBox->setValue( mLayer->distanceY() );
  mHorizontalDisplacementSpinBox->setValue( mLayer->displacementX() );
  mVerticalDisplacementSpinBox->setValue( mLayer->displacementY() );
  updateMarkerIcon();
}

QgsSymbolLayerV2* QgsPointPatternFillSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsPointPatternFillSymbolLayerWidget::updateMarkerIcon()
{
  if ( mLayer )
  {
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLayer->subSymbol(), mChangeMarkerButton->iconSize() );
    mChangeMarkerButton->setIcon( icon );
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDistanceSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDistanceX( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDistanceSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDistanceY( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mHorizontalDisplacementSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDisplacementX( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mVerticalDisplacementSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setDisplacementY( d );
    emit changed();
  }
}

void QgsPointPatternFillSymbolLayerWidget::on_mChangeMarkerButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsSymbolV2PropertiesDialog dlg( mLayer->subSymbol(), mVectorLayer, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    return;
  }

  updateMarkerIcon();
  emit changed();
}

/////////////

QgsFontMarkerSymbolLayerV2Widget::QgsFontMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );
  widgetChar = new CharacterWidget;
  scrollArea->setWidget( widgetChar );

  connect( cboFont, SIGNAL( currentFontChanged( const QFont & ) ), this, SLOT( setFontFamily( const QFont& ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setSize( double ) ) );
  connect( btnColor, SIGNAL( clicked() ), this, SLOT( setColor() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setAngle( double ) ) );
  connect( spinOffsetX, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( spinOffsetY, SIGNAL( valueChanged( double ) ), this, SLOT( setOffset() ) );
  connect( widgetChar, SIGNAL( characterSelected( const QChar & ) ), this, SLOT( setCharacter( const QChar & ) ) );
}


void QgsFontMarkerSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "FontMarker" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsFontMarkerSymbolLayerV2*>( layer );

  // set values
  cboFont->setCurrentFont( QFont( mLayer->fontFamily() ) );
  spinSize->setValue( mLayer->size() );
  btnColor->setColor( mLayer->color() );
  spinAngle->setValue( mLayer->angle() );

  //block
  spinOffsetX->blockSignals( true );
  spinOffsetX->setValue( mLayer->offset().x() );
  spinOffsetX->blockSignals( false );
  spinOffsetY->blockSignals( true );
  spinOffsetY->setValue( mLayer->offset().y() );
  spinOffsetY->blockSignals( false );

}

QgsSymbolLayerV2* QgsFontMarkerSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsFontMarkerSymbolLayerV2Widget::setFontFamily( const QFont& font )
{
  mLayer->setFontFamily( font.family() );
  widgetChar->updateFont( font );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setColor()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mLayer->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mLayer->color(), this );
#endif
  if ( !color.isValid() )
    return;
  mLayer->setColor( color );
  btnColor->setColor( mLayer->color() );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setSize( double size )
{
  mLayer->setSize( size );
  //widgetChar->updateSize(size);
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setAngle( double angle )
{
  mLayer->setAngle( angle );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setCharacter( const QChar& chr )
{
  mLayer->setCharacter( chr );
  emit changed();
}

void QgsFontMarkerSymbolLayerV2Widget::setOffset()
{
  mLayer->setOffset( QPointF( spinOffsetX->value(), spinOffsetY->value() ) );
  emit changed();
}


///////////////


QgsCentroidFillSymbolLayerV2Widget::QgsCentroidFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent )
    : QgsSymbolLayerV2Widget( parent, vl )
{
  mLayer = NULL;

  setupUi( this );

  connect( btnChangeMarker, SIGNAL( clicked() ), this, SLOT( setMarker() ) );
}

void QgsCentroidFillSymbolLayerV2Widget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "CentroidFill" )
    return;

  // layer type is correct, we can do the cast
  mLayer = static_cast<QgsCentroidFillSymbolLayerV2*>( layer );

  // set values
  updateMarker();
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2Widget::symbolLayer()
{
  return mLayer;
}

void QgsCentroidFillSymbolLayerV2Widget::setMarker()
{
  QgsSymbolV2PropertiesDialog dlg( mLayer->subSymbol(), mVectorLayer, this );
  if ( dlg.exec() == 0 )
    return;
  updateMarker();

  emit changed();
}

void QgsCentroidFillSymbolLayerV2Widget::updateMarker()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLayer->subSymbol(), btnChangeMarker->iconSize() );
  btnChangeMarker->setIcon( icon );
}
