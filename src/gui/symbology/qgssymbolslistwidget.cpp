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

#include "qgsstylemanagerdialog.h"
#include "qgsstylesavedialog.h"

#include "qgssymbol.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmapcanvas.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgsauxiliarystorage.h"

#include <QAction>
#include <QString>
#include <QStringList>
#include <QPainter>
#include <QIcon>
#include <QStandardItemModel>
#include <QColorDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>


QgsSymbolsListWidget::QgsSymbolsListWidget( QgsSymbol *symbol, QgsStyle *style, QMenu *menu, QWidget *parent, QgsVectorLayer *layer )
  : QWidget( parent )
  , mSymbol( symbol )
  , mStyle( style )
  , mLayer( layer )
{
  setupUi( this );
  connect( mSymbolUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSymbolsListWidget::mSymbolUnitWidget_changed );
  connect( groupsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSymbolsListWidget::groupsCombo_currentIndexChanged );
  spinAngle->setClearValue( 0 );

  mSymbolUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  btnAdvanced->hide(); // advanced button is hidden by default
  if ( menu ) // show it if there is a menu pointer
  {
    mAdvancedMenu = menu;
    btnAdvanced->show();
    btnAdvanced->setMenu( mAdvancedMenu );
  }
  else
  {
    btnAdvanced->setMenu( new QMenu( this ) );
  }
  mClipFeaturesAction = new QAction( tr( "Clip features to canvas extent" ), this );
  mClipFeaturesAction->setCheckable( true );
  connect( mClipFeaturesAction, &QAction::toggled, this, &QgsSymbolsListWidget::clipFeaturesToggled );

  QStandardItemModel *model = new QStandardItemModel( viewSymbols );
  viewSymbols->setModel( model );
  connect( viewSymbols->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSymbolsListWidget::setSymbolFromStyle );

  connect( mStyle, &QgsStyle::symbolSaved, this, &QgsSymbolsListWidget::symbolAddedToStyle );
  connect( mStyle, &QgsStyle::groupsModified, this, &QgsSymbolsListWidget::populateGroups );

  connect( openStyleManagerButton, &QPushButton::pressed, this, &QgsSymbolsListWidget::openStyleManager );

  lblSymbolName->clear();

  populateGroups();

  if ( mSymbol )
  {
    updateSymbolInfo();
  }

  // select correct page in stacked widget
  // there's a correspondence between symbol type number and page numbering => exploit it!
  stackedWidget->setCurrentIndex( symbol->type() );
  connect( btnColor, &QgsColorButton::colorChanged, this, &QgsSymbolsListWidget::setSymbolColor );
  connect( spinAngle, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSymbolsListWidget::setMarkerAngle );
  connect( spinSize, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSymbolsListWidget::setMarkerSize );
  connect( spinWidth, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSymbolsListWidget::setLineWidth );

  registerDataDefinedButton( mRotationDDBtn, QgsSymbolLayer::PropertyAngle );
  connect( mRotationDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateDataDefinedMarkerAngle );
  registerDataDefinedButton( mSizeDDBtn, QgsSymbolLayer::PropertySize );
  connect( mSizeDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateDataDefinedMarkerSize );
  registerDataDefinedButton( mWidthDDBtn, QgsSymbolLayer::PropertyStrokeWidth );
  connect( mWidthDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsSymbolsListWidget::updateDataDefinedLineWidth );

  connect( this, &QgsSymbolsListWidget::changed, this, &QgsSymbolsListWidget::updateAssistantSymbol );
  updateAssistantSymbol();

  // Live color updates are not undoable to child symbol layers
  btnColor->setAcceptLiveUpdates( false );
  btnColor->setAllowOpacity( true );
  btnColor->setColorDialogTitle( tr( "Select Color" ) );
  btnColor->setContext( QStringLiteral( "symbology" ) );
  connect( btnSaveSymbol, &QPushButton::clicked, this, &QgsSymbolsListWidget::saveSymbol );

  connect( mOpacityWidget, &QgsOpacityWidget::opacityChanged, this, &QgsSymbolsListWidget::opacityChanged );
}

QgsSymbolsListWidget::~QgsSymbolsListWidget()
{
  // This action was added to the menu by this widget, clean it up
  // The menu can be passed in the constructor, so may live longer than this widget
  btnAdvanced->menu()->removeAction( mClipFeaturesAction );
}

void QgsSymbolsListWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key )
{
  button->setProperty( "propertyKey", key );
  button->registerExpressionContextGenerator( this );

  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsSymbolsListWidget::createAuxiliaryField );
}

void QgsSymbolsListWidget::createAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  if ( !mLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  QgsSymbolLayer::Property key = static_cast<  QgsSymbolLayer::Property >( button->propertyKey() );
  const QgsPropertyDefinition def = QgsSymbolLayer::propertyDefinitions()[key];

  // create property in auxiliary storage if necessary
  if ( !mLayer->auxiliaryLayer()->exists( def ) )
    mLayer->auxiliaryLayer()->addAuxiliaryField( def );

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );

  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
  switch ( key )
  {
    case QgsSymbolLayer::PropertyAngle:
      if ( markerSymbol )
        markerSymbol->setDataDefinedAngle( button->toProperty() );
      break;
    case QgsSymbolLayer::PropertySize:
      if ( markerSymbol )
      {
        markerSymbol->setDataDefinedSize( button->toProperty() );
        markerSymbol->setScaleMethod( QgsSymbol::ScaleDiameter );
      }
      break;
    case QgsSymbolLayer::PropertyStrokeWidth:
      if ( lineSymbol )
        lineSymbol->setDataDefinedWidth( button->toProperty() );
    default:
      break;
  }

  emit changed();
}

void QgsSymbolsListWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
  Q_FOREACH ( QgsUnitSelectionWidget *unitWidget, findChildren<QgsUnitSelectionWidget *>() )
  {
    unitWidget->setMapCanvas( mContext.mapCanvas() );
  }
#if 0
  Q_FOREACH ( QgsPropertyOverrideButton *ddButton, findChildren<QgsPropertyOverrideButton *>() )
  {
    if ( ddButton->assistant() )
      ddButton->assistant()->setMapCanvas( mContext.mapCanvas() );
  }
#endif
}

QgsSymbolWidgetContext QgsSymbolsListWidget::context() const
{
  return mContext;
}

void QgsSymbolsListWidget::populateGroups()
{
  groupsCombo->blockSignals( true );
  groupsCombo->clear();

  groupsCombo->addItem( tr( "Favorites" ), QVariant( "favorite" ) );
  groupsCombo->addItem( tr( "All Symbols" ), QVariant( "all" ) );

  int index = 2;
  QStringList tags = mStyle->tags();
  if ( tags.count() > 0 )
  {
    tags.sort();
    groupsCombo->insertSeparator( index );
    Q_FOREACH ( const QString &tag, tags )
    {
      groupsCombo->addItem( tag, QVariant( "tag" ) );
      index++;
    }
  }

  QStringList groups = mStyle->smartgroupNames();
  if ( groups.count() > 0 )
  {
    groups.sort();
    groupsCombo->insertSeparator( index + 1 );
    Q_FOREACH ( const QString &group, groups )
    {
      groupsCombo->addItem( group, QVariant( "smartgroup" ) );
    }
  }
  groupsCombo->blockSignals( false );

  QgsSettings settings;
  index = settings.value( QStringLiteral( "qgis/symbolsListGroupsIndex" ), 0 ).toInt();
  groupsCombo->setCurrentIndex( index );

  populateSymbolView();
}

void QgsSymbolsListWidget::populateSymbolView()
{
  QStringList symbols;
  QString text = groupsCombo->currentText();
  int id;

  if ( groupsCombo->currentData().toString() == QLatin1String( "favorite" ) )
  {
    symbols = mStyle->symbolsOfFavorite( QgsStyle::SymbolEntity );
  }
  else if ( groupsCombo->currentData().toString() == QLatin1String( "all" ) )
  {
    symbols = mStyle->symbolNames();
  }
  else if ( groupsCombo->currentData().toString() == QLatin1String( "smartgroup" ) )
  {
    id = mStyle->smartgroupId( text );
    symbols = mStyle->symbolsOfSmartgroup( QgsStyle::SymbolEntity, id );
  }
  else
  {
    id = mStyle->tagId( text );
    symbols = mStyle->symbolsWithTag( QgsStyle::SymbolEntity, id );
  }

  symbols.sort();
  populateSymbols( symbols );
}

void QgsSymbolsListWidget::populateSymbols( const QStringList &names )
{
  QSize previewSize = viewSymbols->iconSize();

  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( viewSymbols->model() );
  if ( !model )
  {
    return;
  }
  model->clear();

  for ( int i = 0; i < names.count(); i++ )
  {
    QgsSymbol *s = mStyle->symbol( names[i] );
    if ( s->type() != mSymbol->type() )
    {
      delete s;
      continue;
    }
    QStringList tags = mStyle->tagsOfSymbol( QgsStyle::SymbolEntity, names[i] );
    QStandardItem *item = new QStandardItem( names[i] );
    item->setData( names[i], Qt::UserRole ); //so we can load symbol with that name
    item->setText( names[i] );
    item->setToolTip( QStringLiteral( "<b>%1</b><br><i>%2</i>" ).arg( names[i], tags.count() > 0 ? tags.join( QStringLiteral( ", " ) ) : tr( "Not tagged" ) ) );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    // Set font to 10points to show reasonable text
    QFont itemFont = item->font();
    itemFont.setPointSize( 10 );
    item->setFont( itemFont );
    // create preview icon
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( s, previewSize, 15 );
    item->setIcon( icon );
    // add to model
    model->appendRow( item );
    delete s;
  }
}

void QgsSymbolsListWidget::openStyleManager()
{
  QgsStyleManagerDialog dlg( mStyle, this );
  dlg.exec();

  populateSymbolView();
}

void QgsSymbolsListWidget::clipFeaturesToggled( bool checked )
{
  if ( !mSymbol )
    return;

  mSymbol->setClipFeaturesToExtent( checked );
  emit changed();
}

void QgsSymbolsListWidget::setSymbolColor( const QColor &color )
{
  mSymbol->setColor( color );
  emit changed();
}

void QgsSymbolsListWidget::setMarkerAngle( double angle )
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  if ( markerSymbol->angle() == angle )
    return;
  markerSymbol->setAngle( angle );
  emit changed();
}

void QgsSymbolsListWidget::updateDataDefinedMarkerAngle()
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  QgsProperty dd( mRotationDDBtn->toProperty() );

  spinAngle->setEnabled( !mRotationDDBtn->isActive() );

  QgsProperty symbolDD( markerSymbol->dataDefinedAngle() );

  if ( // shall we remove datadefined expressions for layers ?
    ( !symbolDD && !dd )
    // shall we set the "en masse" expression for properties ?
    || dd )
  {
    markerSymbol->setDataDefinedAngle( dd );
    emit changed();
  }
}

void QgsSymbolsListWidget::setMarkerSize( double size )
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  if ( markerSymbol->size() == size )
    return;
  markerSymbol->setSize( size );
  emit changed();
}

void QgsSymbolsListWidget::updateDataDefinedMarkerSize()
{
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
  QgsProperty dd( mSizeDDBtn->toProperty() );

  spinSize->setEnabled( !mSizeDDBtn->isActive() );

  QgsProperty symbolDD( markerSymbol->dataDefinedSize() );

  if ( // shall we remove datadefined expressions for layers ?
    ( !symbolDD && !dd )
    // shall we set the "en masse" expression for properties ?
    || dd )
  {
    markerSymbol->setDataDefinedSize( dd );
    markerSymbol->setScaleMethod( QgsSymbol::ScaleDiameter );
    emit changed();
  }
}

void QgsSymbolsListWidget::setLineWidth( double width )
{
  QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
  if ( lineSymbol->width() == width )
    return;
  lineSymbol->setWidth( width );
  emit changed();
}

void QgsSymbolsListWidget::updateDataDefinedLineWidth()
{
  QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
  QgsProperty dd( mWidthDDBtn->toProperty() );

  spinWidth->setEnabled( !mWidthDDBtn->isActive() );

  QgsProperty symbolDD( lineSymbol->dataDefinedWidth() );

  if ( // shall we remove datadefined expressions for layers ?
    ( !symbolDD && !dd )
    // shall we set the "en masse" expression for properties ?
    || dd )
  {
    lineSymbol->setDataDefinedWidth( dd );
    emit changed();
  }
}

void QgsSymbolsListWidget::updateAssistantSymbol()
{
  mAssistantSymbol.reset( mSymbol->clone() );
  if ( mSymbol->type() == QgsSymbol::Marker )
    mSizeDDBtn->setSymbol( mAssistantSymbol );
  else if ( mSymbol->type() == QgsSymbol::Line && mLayer )
    mWidthDDBtn->setSymbol( mAssistantSymbol );
}

void QgsSymbolsListWidget::symbolAddedToStyle( const QString &name, QgsSymbol *symbol )
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
  mStyle->saveSymbol( name, mSymbol->clone(), false, QStringList() );
  populateSymbolView();
}

void QgsSymbolsListWidget::saveSymbol()
{
  QgsStyleSaveDialog saveDlg( this );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  // check if there is no symbol with same name
  if ( mStyle->symbolNames().contains( saveDlg.name() ) )
  {
    int res = QMessageBox::warning( this, tr( "Save symbol" ),
                                    tr( "Symbol with name '%1' already exists. Overwrite?" )
                                    .arg( saveDlg.name() ),
                                    QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    mStyle->removeSymbol( saveDlg.name() );
  }

  QStringList symbolTags = saveDlg.tags().split( ',' );

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( saveDlg.name(), mSymbol->clone() );

  // make sure the symbol is stored
  mStyle->saveSymbol( saveDlg.name(), mSymbol->clone(), saveDlg.isFavorite(), symbolTags );
}

void QgsSymbolsListWidget::mSymbolUnitWidget_changed()
{
  if ( mSymbol )
  {

    mSymbol->setOutputUnit( mSymbolUnitWidget->unit() );
    mSymbol->setMapUnitScale( mSymbolUnitWidget->getMapUnitScale() );

    emit changed();
  }
}

void QgsSymbolsListWidget::opacityChanged( double opacity )
{
  if ( mSymbol )
  {
    mSymbol->setOpacity( opacity );
    emit changed();
  }
}

void QgsSymbolsListWidget::updateSymbolColor()
{
  btnColor->blockSignals( true );
  btnColor->setColor( mSymbol->color() );
  btnColor->blockSignals( false );
}

QgsExpressionContext QgsSymbolsListWidget::createExpressionContext() const
{
  if ( mContext.expressionContext() )
    return QgsExpressionContext( *mContext.expressionContext() );

  //otherwise create a default symbol context
  QgsExpressionContext expContext( mContext.globalProjectAtlasMapLayerScopes( layer() ) );

  // additional scopes
  Q_FOREACH ( const QgsExpressionContextScope &scope, mContext.additionalExpressionContextScopes() )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR
                                      << QgsExpressionContext::EXPR_GEOMETRY_PART_COUNT << QgsExpressionContext::EXPR_GEOMETRY_PART_NUM
                                      << QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT << QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM
                                      << QgsExpressionContext::EXPR_CLUSTER_COLOR << QgsExpressionContext::EXPR_CLUSTER_SIZE );

  return expContext;
}

void QgsSymbolsListWidget::updateSymbolInfo()
{
  updateSymbolColor();

  Q_FOREACH ( QgsPropertyOverrideButton *button, findChildren< QgsPropertyOverrideButton * >() )
  {
    button->registerExpressionContextGenerator( this );
  }

  if ( mSymbol->type() == QgsSymbol::Marker )
  {
    QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( mSymbol );
    spinSize->setValue( markerSymbol->size() );
    spinAngle->setValue( markerSymbol->angle() );

    if ( mLayer )
    {
      QgsProperty ddSize( markerSymbol->dataDefinedSize() );
      mSizeDDBtn->init( QgsSymbolLayer::PropertySize, ddSize, QgsSymbolLayer::propertyDefinitions(), mLayer, true );
      spinSize->setEnabled( !mSizeDDBtn->isActive() );
      QgsProperty ddAngle( markerSymbol->dataDefinedAngle() );
      mRotationDDBtn->init( QgsSymbolLayer::PropertyAngle, ddAngle, QgsSymbolLayer::propertyDefinitions(), mLayer, true );
      spinAngle->setEnabled( !mRotationDDBtn->isActive() );
    }
    else
    {
      mSizeDDBtn->setEnabled( false );
      mRotationDDBtn->setEnabled( false );
    }
  }
  else if ( mSymbol->type() == QgsSymbol::Line )
  {
    QgsLineSymbol *lineSymbol = static_cast<QgsLineSymbol *>( mSymbol );
    spinWidth->setValue( lineSymbol->width() );

    if ( mLayer )
    {
      QgsProperty dd( lineSymbol->dataDefinedWidth() );
      mWidthDDBtn->init( QgsSymbolLayer::PropertyStrokeWidth, dd, QgsSymbolLayer::propertyDefinitions(), mLayer, true );
      spinWidth->setEnabled( !mWidthDDBtn->isActive() );
    }
    else
    {
      mWidthDDBtn->setEnabled( false );
    }
  }

  mSymbolUnitWidget->blockSignals( true );
  mSymbolUnitWidget->setUnit( mSymbol->outputUnit() );
  mSymbolUnitWidget->setMapUnitScale( mSymbol->mapUnitScale() );
  mSymbolUnitWidget->blockSignals( false );

  mOpacityWidget->setOpacity( mSymbol->opacity() );

  if ( mSymbol->type() == QgsSymbol::Line || mSymbol->type() == QgsSymbol::Fill )
  {
    //add clip features option for line or fill symbols
    btnAdvanced->menu()->addAction( mClipFeaturesAction );
  }
  else
  {
    btnAdvanced->menu()->removeAction( mClipFeaturesAction );
  }
  btnAdvanced->setVisible( mAdvancedMenu || !btnAdvanced->menu()->isEmpty() );

  mClipFeaturesAction->blockSignals( true );
  mClipFeaturesAction->setChecked( mSymbol->clipFeaturesToExtent() );
  mClipFeaturesAction->blockSignals( false );
}

void QgsSymbolsListWidget::setSymbolFromStyle( const QModelIndex &index )
{
  QString symbolName = index.data( Qt::UserRole ).toString();
  lblSymbolName->setText( symbolName );
  // get new instance of symbol from style
  QgsSymbol *s = mStyle->symbol( symbolName );
  // remove all symbol layers from original symbolgroupsCombo
  while ( mSymbol->symbolLayerCount() )
    mSymbol->deleteSymbolLayer( 0 );
  // move all symbol layers to our symbol
  while ( s->symbolLayerCount() )
  {
    QgsSymbolLayer *sl = s->takeSymbolLayer( 0 );
    mSymbol->appendSymbolLayer( sl );
  }
  mSymbol->setOpacity( s->opacity() );

  // delete the temporary symbol
  delete s;

  updateSymbolInfo();
  emit changed();
}

void QgsSymbolsListWidget::groupsCombo_currentIndexChanged( int index )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/symbolsListGroupsIndex" ), index );

  populateSymbolView();
}
