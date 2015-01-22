/***************************************************************************
 qgssymbolslist.cpp
 ---------------------
 begin                : June 2012
 copyright            : (C) 2012 by Arunmozhi
 email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgssymbolslistwidget.h"

#include "qgsstylev2managerdialog.h"

#include "qgssymbolv2.h"
#include "qgsstylev2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsmarkersymbollayerv2.h"

#include "qgsapplication.h"

#include <QString>
#include <QStringList>
#include <QPainter>
#include <QIcon>
#include <QStandardItemModel>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QScopedPointer>

// check if the layer has an "en masse" size expression
// to return an active QgsDataDefined, the size and offset expression
// should match the "en masse" pattern

inline
QString rotateEnMasse( double additionalRotation, const QString & exprStr )
{
  return additionalRotation 
    ? QgsExpression( QString::number( additionalRotation )+" + ("+exprStr+")" ).dump()
    : QgsExpression( exprStr ).dump();
}

inline
QString scaleEnMasse( double scaleFactor, const QString & exprStr )
{
  return ( qAbs(scaleFactor - 1) > 1e-6 ) 
    ? QgsExpression( QString::number( scaleFactor )+"*("+exprStr+")" ).dump()
    : QgsExpression( exprStr ).dump();
}

inline
QString scaleEnMasseOffset( double scaleFactorX, double scaleFactorY, const QString & exprStr )
{
  return QgsExpression( 
      (scaleFactorX ? "tostring("+QString::number(scaleFactorX)+"*("+exprStr+"))" : "'0'")+ 
        "|| ',' || "+
      (scaleFactorY ? "tostring("+QString::number(scaleFactorY)+"*("+exprStr+"))" : "'0'") ).dump(); 
}

template < class MarkerSymbolLayerIterator >
QgsDataDefined enMasseRotationExpression( double angle, MarkerSymbolLayerIterator begin, MarkerSymbolLayerIterator end )
{
  QScopedPointer< QgsExpression > expr;

  // find the base of the "en masse" pattern
  for ( MarkerSymbolLayerIterator it = begin; !expr.data() && it != end; ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2 *>( *it );
    if ( layer->angle() == angle && layer->dataDefinedProperty( "angle" ) )
      expr.reset( new QgsExpression( layer->dataDefinedPropertyString( "angle" ) ) );
  }

  // check that all layers angle expressions match the "en masse" pattern
  const QString exprStr( expr.data() ? expr->dump() : "" );
  for ( MarkerSymbolLayerIterator it = begin; expr.data() && it != end; ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2 *>( *it );
    const QString sizeExpr( QgsExpression( layer->dataDefinedPropertyString( "angle" ) ).dump() );
    if ( rotateEnMasse( layer->angle() - angle, exprStr ) != sizeExpr )
      expr.reset();
  }

  return QgsDataDefined( expr.data() );
}

template < class MarkerSymbolLayerIterator >
void setEnMasseRotationExpression( const QString & exprStr, double angle, MarkerSymbolLayerIterator begin, MarkerSymbolLayerIterator end )
{
  for ( MarkerSymbolLayerIterator it = begin; it != end; ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2 *>( *it );
    if ( !exprStr.length() )
      layer->removeDataDefinedProperty( "angle" );
    else
      layer->setDataDefinedProperty( "angle", rotateEnMasse( layer->angle() - angle, exprStr ) );
  }
}

template < class MarkerSymbolLayerIterator >
QgsDataDefined enMasseSizeExpression( double size, MarkerSymbolLayerIterator begin, MarkerSymbolLayerIterator end )
{
  if ( !size ) return QgsDataDefined();

  QScopedPointer< QgsExpression > expr;

  // find the base of the "en masse" pattern
  for ( MarkerSymbolLayerIterator it = begin; !expr.data() && it != end; ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2 *>( *it );
    if ( layer->size() == size && layer->dataDefinedProperty( "size" ) )
      expr.reset( new QgsExpression( layer->dataDefinedPropertyString( "size" ) ) );
  }

  // check that all layers size expressions match the "en masse" pattern
  const QString exprStr( expr.data() ? expr->dump() : "" );
  for ( MarkerSymbolLayerIterator it = begin; expr.data() && it != end; ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2 *>( *it );
    const QString sizeExpr( QgsExpression( layer->dataDefinedPropertyString( "size" ) ).dump() );
    const QString offsetExpr( QgsExpression( layer->dataDefinedPropertyString( "offset" ) ).dump() );
    if ( scaleEnMasse( layer->size() / size, exprStr ) != sizeExpr 
      || ( (layer->offset().x() || layer->offset().y() ) && scaleEnMasseOffset( layer->offset().x() / size, layer->offset().y() / size, exprStr ) != offsetExpr ) )
    {
      expr.reset();
    }
  }

  return QgsDataDefined( expr.data() );
}

template < class MarkerSymbolLayerIterator >
void setEnMasseSizeExpression( const QString & exprStr, double size, MarkerSymbolLayerIterator begin, MarkerSymbolLayerIterator end )
{
  if ( size == 0 ) return;

  for ( MarkerSymbolLayerIterator it = begin; it != end; ++it )
  {
    QgsMarkerSymbolLayerV2* layer = static_cast<QgsMarkerSymbolLayerV2 *>( *it );
    if ( !exprStr.length() )
    {
      layer->removeDataDefinedProperty( "size" );
      layer->removeDataDefinedProperty( "offset" );
    }
    else
    {
      layer->setDataDefinedProperty( "size", scaleEnMasse( layer->size()/size, exprStr ) );
      if ( layer->offset().x() || layer->offset().y() ) 
        layer->setDataDefinedProperty( "offset", scaleEnMasseOffset( layer->offset().x()/size, layer->offset().y()/size, exprStr ) );
    }
  }
}

template < class LineSymbolLayerIterator >
QgsDataDefined enMasseWidthExpression( double width, LineSymbolLayerIterator begin, LineSymbolLayerIterator end )
{
  if ( !width ) return QgsDataDefined();

  QScopedPointer< QgsExpression > expr;

  // find the base of the "en masse" pattern
  for ( LineSymbolLayerIterator it = begin; !expr.data() && it != end; ++it )
  {
    QgsLineSymbolLayerV2* layer = static_cast<QgsLineSymbolLayerV2*>( *it );
    if ( layer->width() == width && layer->dataDefinedProperty( "width" ) )
      expr.reset( new QgsExpression( layer->dataDefinedPropertyString( "width" ) ) );
  }

  // check that all layers width expressions match the "en masse" pattern
  const QString exprStr( expr.data() ? expr->dump() : "" );
  for ( LineSymbolLayerIterator it = begin; expr.data() && it != end; ++it )
  {
    QgsLineSymbolLayerV2* layer = static_cast<QgsLineSymbolLayerV2*>( *it );
    const QString sizeExpr( QgsExpression( layer->dataDefinedPropertyString( "width" ) ).dump() );
    const QString offsetExpr( QgsExpression( layer->dataDefinedPropertyString( "offset" ) ).dump() );
    if ( scaleEnMasse( layer->width() / width, exprStr ) != sizeExpr 
      || ( layer->offset() && scaleEnMasse( layer->offset() / width, exprStr ) != offsetExpr ) )
    {
      expr.reset();
    }
  }

  return QgsDataDefined( expr.data() );
}

template < class LineSymbolLayerIterator >
void setEnMasseWidthExpression( const QString & exprStr, double width, LineSymbolLayerIterator begin, LineSymbolLayerIterator end )
{
  if ( width == 0 ) return;

  for ( LineSymbolLayerIterator it = begin; it != end; ++it )
  {
    QgsLineSymbolLayerV2* layer = static_cast<QgsLineSymbolLayerV2*>( *it );
    if ( !exprStr.length() )
    {
      layer->removeDataDefinedProperty( "width" );
      layer->removeDataDefinedProperty( "offset" );
    }
    else
    {
      layer->setDataDefinedProperty( "width", scaleEnMasse( layer->width()/width, exprStr ) );
      if ( layer->offset() || layer->offset() ) 
        layer->setDataDefinedProperty( "offset", scaleEnMasse( layer->offset()/width, exprStr ) );
    }
  }
}

QgsSymbolsListWidget::QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent ) : QWidget( parent )
{
  mSymbol = symbol;
  mStyle = style;

  setupUi( this );

  mSymbolUnitWidget->setUnits( QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), 1 );

  btnAdvanced->hide(); // advanced button is hidden by default
  if ( menu ) // show it if there is a menu pointer
  {
    btnAdvanced->setMenu( menu );
    btnAdvanced->show();
  }

  // populate the groups
  groupsCombo->addItem( "" );
  populateGroups();
  QStringList groups = style->smartgroupNames();
  foreach ( QString group, groups )
  {
    groupsCombo->addItem( group, QVariant( "smart" ) );
  }

  QStandardItemModel* model = new QStandardItemModel( viewSymbols );
  viewSymbols->setModel( model );
  connect( viewSymbols->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ), this, SLOT( setSymbolFromStyle( const QModelIndex & ) ) );

  connect( mStyle, SIGNAL( symbolSaved( QString, QgsSymbolV2* ) ), this, SLOT( symbolAddedToStyle( QString, QgsSymbolV2* ) ) );
  connect( openStyleManagerButton, SIGNAL( pressed() ), this, SLOT( openStyleManager() ) );

  lblSymbolName->setText( "" );
  populateSymbolView();

  if ( mSymbol )
  {
    updateSymbolInfo();
  }

  // select correct page in stacked widget
  // there's a correspondence between symbol type number and page numbering => exploit it!
  stackedWidget->setCurrentIndex( symbol->type() );
  connect( btnColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setSymbolColor( const QColor& ) ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerAngle( double ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerSize( double ) ) );
  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( setLineWidth( double ) ) );

  connect( mRotationDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( setMarkerRotationExpression( const QString& ) ) );
  connect( mRotationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( setActiveMarkerRotationExpression( bool ) ) );
  connect( mSizeDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( setMarkerSizeExpression( const QString& ) ) );
  connect( mSizeDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( setActiveMarkerSizeExpression( bool ) ) );
  connect( mWidthDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( setLineWidthExpression( const QString& ) ) );
  connect( mWidthDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( setActiveLineWidthExpression( bool ) ) );




  // Live color updates are not undoable to child symbol layers
  btnColor->setAcceptLiveUpdates( false );
  btnColor->setAllowAlpha( true );
  btnColor->setColorDialogTitle( tr( "Select color" ) );
  btnColor->setContext( "symbology" );
}

void QgsSymbolsListWidget::populateGroups( QString parent, QString prepend )
{
  QgsSymbolGroupMap groups = mStyle->childGroupNames( parent );
  QgsSymbolGroupMap::const_iterator i = groups.constBegin();
  while ( i != groups.constEnd() )
  {
    QString text;
    if ( !prepend.isEmpty() )
    {
      text = prepend + "/" + i.value();
    }
    else
    {
      text = i.value();
    }
    groupsCombo->addItem( text, QVariant( i.key() ) );
    populateGroups( i.value(), text );
    ++i;
  }
}

void QgsSymbolsListWidget::populateSymbolView()
{
  populateSymbols( mStyle->symbolNames() );
}

void QgsSymbolsListWidget::populateSymbols( QStringList names )
{
  QSize previewSize = viewSymbols->iconSize();
  QPixmap p( previewSize );
  QPainter painter;

  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( viewSymbols->model() );
  if ( !model )
  {
    return;
  }
  model->clear();

  for ( int i = 0; i < names.count(); i++ )
  {
    QgsSymbolV2* s = mStyle->symbol( names[i] );
    if ( s->type() != mSymbol->type() )
    {
      delete s;
      continue;
    }
    QStandardItem* item = new QStandardItem( names[i] );
    item->setData( names[i], Qt::UserRole ); //so we can load symbol with that name
    item->setText( names[i] );
    item->setToolTip( names[i] );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    // Set font to 10points to show reasonable text
    QFont itemFont = item->font();
    itemFont.setPointSize( 10 );
    item->setFont( itemFont );
    // create preview icon
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( s, previewSize );
    item->setIcon( icon );
    // add to model
    model->appendRow( item );
    delete s;
  }
}

void QgsSymbolsListWidget::openStyleManager()
{
  QgsStyleV2ManagerDialog dlg( mStyle, this );
  dlg.exec();

  populateSymbolView();
}

void QgsSymbolsListWidget::setSymbolColor( const QColor& color )
{
  mSymbol->setColor( color );
  emit changed();
}

void QgsSymbolsListWidget::setMarkerAngle( double angle )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  if ( markerSymbol->angle() == angle )
    return;
  markerSymbol->setAngle( angle );
  emit changed();
}

void QgsSymbolsListWidget::setActiveMarkerRotationExpression( bool active )
{
    if ( active )
        setMarkerRotationExpression( mRotationDDBtn->currentDefinition() );
    else
        setMarkerRotationExpression( QString() );
    spinAngle->setEnabled( !active );
}

void QgsSymbolsListWidget::setMarkerRotationExpression( const QString & definition )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  QgsSymbolLayerV2List layers = markerSymbol->symbolLayers();
  if (// shall we remove datadefined expressions for layers ?
      ( enMasseRotationExpression( markerSymbol->angle(), layers.begin(), layers.end() ).isActive() && !definition.length()  )
      // shall we set the "en masse" expression for properties ?
      || definition.length() )
  {
    setEnMasseRotationExpression( definition, markerSymbol->angle(), layers.begin(), layers.end() );
    emit changed();
  }
}

void QgsSymbolsListWidget::setMarkerSize( double size )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  if ( markerSymbol->size() == size )
    return;
  markerSymbol->setSize( size );
  emit changed();
}

void QgsSymbolsListWidget::setActiveMarkerSizeExpression( bool active )
{
    if ( active )
        setMarkerSizeExpression( mSizeDDBtn->currentDefinition() );
    else
        setMarkerSizeExpression( QString() );
    spinSize->setEnabled( !active );
}

void QgsSymbolsListWidget::setMarkerSizeExpression( const QString & definition )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  QgsSymbolLayerV2List layers = markerSymbol->symbolLayers();
  if (// shall we remove datadefined expressions for layers ?
      ( enMasseSizeExpression( markerSymbol->size(), layers.begin(), layers.end() ).isActive() && !definition.length()  )
      // shall we set the "en masse" expression for properties ?
      || definition.length() )
  {
    setEnMasseSizeExpression( definition, markerSymbol->size(), layers.begin(), layers.end() );
    emit changed();
  }
}

void QgsSymbolsListWidget::setLineWidth( double width )
{
  QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
  if ( lineSymbol->width() == width )
    return;
  lineSymbol->setWidth( width );
  emit changed();
}

void QgsSymbolsListWidget::setActiveLineWidthExpression( bool active )
{
    if ( active )
        setLineWidthExpression( mSizeDDBtn->currentDefinition() );
    else
        setLineWidthExpression( QString() );
    spinWidth->setEnabled( !active );
}


void QgsSymbolsListWidget::setLineWidthExpression( const QString & definition )
{
  QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
  QgsSymbolLayerV2List layers = lineSymbol->symbolLayers();
  if (// shall we remove datadefined expressions for layers ?
      ( enMasseWidthExpression( lineSymbol->width(), layers.begin(), layers.end() ).isActive() && !definition.length()  )
      // shall we set the "en masse" expression for properties ?
      || definition.length() )
  {
    setEnMasseWidthExpression( definition, lineSymbol->width(), layers.begin(), layers.end() );
    emit changed();
  }
}

void QgsSymbolsListWidget::symbolAddedToStyle( QString name, QgsSymbolV2* symbol )
{
  Q_UNUSED( name );
  Q_UNUSED( symbol );
  populateSymbolView();
}

void QgsSymbolsListWidget::addSymbolToStyle()
{
  bool ok;
  QString name = QInputDialog::getText( this, tr( "Symbol name" ),
                                        tr( "Please enter name for the symbol:" ), QLineEdit::Normal, tr( "New symbol" ), &ok );
  if ( !ok || name.isEmpty() )
    return;

  // check if there is no symbol with same name
  if ( mStyle->symbolNames().contains( name ) )
  {
    int res = QMessageBox::warning( this, tr( "Save symbol" ),
                                    tr( "Symbol with name '%1' already exists. Overwrite?" )
                                    .arg( name ),
                                    QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
  }

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( name, mSymbol->clone() );

  // make sure the symbol is stored
  mStyle->saveSymbol( name, mSymbol->clone(), 0, QStringList() );
  populateSymbolView();
}

void QgsSymbolsListWidget::on_mSymbolUnitWidget_changed()
{
  if ( mSymbol )
  {
    QgsSymbolV2::OutputUnit unit = static_cast<QgsSymbolV2::OutputUnit>( mSymbolUnitWidget->getUnit() );
    mSymbol->setOutputUnit( unit );
    mSymbol->setMapUnitScale( mSymbolUnitWidget->getMapUnitScale() );

    emit changed();
  }
}

void QgsSymbolsListWidget::on_mTransparencySlider_valueChanged( int value )
{
  if ( mSymbol )
  {
    double alpha = 1 - ( value / 255.0 );
    mSymbol->setAlpha( alpha );
    displayTransparency( alpha );
    emit changed();
  }
}

void QgsSymbolsListWidget::displayTransparency( double alpha )
{
  double transparencyPercent = ( 1 - alpha ) * 100;
  mTransparencyLabel->setText( tr( "Transparency %1%" ).arg(( int ) transparencyPercent ) );
}

void QgsSymbolsListWidget::updateSymbolColor()
{
  btnColor->blockSignals( true );
  btnColor->setColor( mSymbol->color() );
  btnColor->blockSignals( false );
}

void QgsSymbolsListWidget::updateSymbolInfo()
{
  updateSymbolColor();

  if ( mSymbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
    spinSize->setValue( markerSymbol->size() );
    spinAngle->setValue( markerSymbol->angle() );

    QgsSymbolLayerV2List layers = markerSymbol->symbolLayers();
    {
      QgsDataDefined dd = enMasseSizeExpression( markerSymbol->size(), layers.begin(), layers.end() );
      mSizeDDBtn->init( markerSymbol->layer(), &dd, QgsDataDefinedButton::Double, "En masse size expression" );
      if ( mSizeDDBtn->isActive() ) spinSize->setEnabled( false );
    }
    {
      QgsDataDefined dd = enMasseRotationExpression( markerSymbol->angle(), layers.begin(), layers.end() );
      mRotationDDBtn->init( markerSymbol->layer(), &dd, QgsDataDefinedButton::Double, "En masse rotation expression" );
      if ( mRotationDDBtn->isActive() ) spinAngle->setEnabled( false );
    }
  }
  else if ( mSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
    spinWidth->setValue( lineSymbol->width() );

    {
      QgsSymbolLayerV2List layers = lineSymbol->symbolLayers();
      QgsDataDefined dd = enMasseWidthExpression( lineSymbol->width(), layers.begin(), layers.end() );
      mWidthDDBtn->init( lineSymbol->layer(), &dd, QgsDataDefinedButton::Double, "En masse width expression" );
      if ( mWidthDDBtn->isActive() ) spinWidth->setEnabled( false );
    }
  }

  mSymbolUnitWidget->blockSignals( true );
  mSymbolUnitWidget->setUnit( mSymbol->outputUnit() );
  mSymbolUnitWidget->setMapUnitScale( mSymbol->mapUnitScale() );
  mSymbolUnitWidget->blockSignals( false );

  mTransparencySlider->blockSignals( true );
  double transparency = 1 - mSymbol->alpha();
  mTransparencySlider->setValue( transparency * 255 );
  displayTransparency( mSymbol->alpha() );
  mTransparencySlider->blockSignals( false );
}

void QgsSymbolsListWidget::setSymbolFromStyle( const QModelIndex & index )
{
  QString symbolName = index.data( Qt::UserRole ).toString();
  lblSymbolName->setText( symbolName );
  // get new instance of symbol from style
  QgsSymbolV2* s = mStyle->symbol( symbolName );
  QgsSymbolV2::OutputUnit unit = s->outputUnit();
  // remove all symbol layers from original symbol
  while ( mSymbol->symbolLayerCount() )
    mSymbol->deleteSymbolLayer( 0 );
  // move all symbol layers to our symbol
  while ( s->symbolLayerCount() )
  {
    QgsSymbolLayerV2* sl = s->takeSymbolLayer( 0 );
    mSymbol->appendSymbolLayer( sl );
  }
  mSymbol->setAlpha( s->alpha() );
  mSymbol->setOutputUnit( unit );
  // delete the temporary symbol
  delete s;

  updateSymbolInfo();
  emit changed();
}

void QgsSymbolsListWidget::on_groupsCombo_currentIndexChanged( int index )
{
  QStringList symbols;
  QString text = groupsCombo->itemText( index );
  // List all symbols when empty list item is selected
  if ( text.isEmpty() )
  {
    symbols = mStyle->symbolNames();
  }
  else
  {
    int groupid;
    if ( groupsCombo->itemData( index ).toString() == "smart" )
    {
      groupid = mStyle->smartgroupId( text );
      symbols = mStyle->symbolsOfSmartgroup( QgsStyleV2::SymbolEntity, groupid );
    }
    else
    {
      groupid = groupsCombo->itemData( index ).toInt();
      symbols = mStyle->symbolsOfGroup( QgsStyleV2::SymbolEntity, groupid );
    }
  }
  populateSymbols( symbols );
}

void QgsSymbolsListWidget::on_groupsCombo_editTextChanged( const QString &text )
{
  QStringList symbols = mStyle->findSymbols( QgsStyleV2::SymbolEntity, text );
  populateSymbols( symbols );
}
