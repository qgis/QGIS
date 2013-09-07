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


QgsSymbolsListWidget::QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent ) : QWidget( parent )
{
  mSymbol = symbol;
  mStyle = style;

  setupUi( this );

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

  if ( parent )
  {
    if ( dynamic_cast<QgsStyleV2ManagerDialog*>( parent->parentWidget() ) )
    {
      btnStyle->setVisible( false );
    }
  }
  // Set the Style Menu under btnStyle
  QMenu *styleMenu = new QMenu( btnStyle );
  QAction *styleMgrAction = new QAction( tr( "Style Manager" ), styleMenu );
  styleMenu->addAction( styleMgrAction );
  QAction *saveStyle = new QAction( tr( "Save in symbol library..." ), styleMenu );
  styleMenu->addAction( saveStyle );
  connect( styleMgrAction, SIGNAL( triggered() ), this, SLOT( openStyleManager() ) );
  connect( saveStyle, SIGNAL( triggered() ), this, SLOT( addSymbolToStyle() ) );
  btnStyle->setMenu( styleMenu );

  lblSymbolName->setText( "" );
  populateSymbolView();

  if ( mSymbol )
  {
    // output unit
    mSymbolUnitComboBox->blockSignals( true );
    mSymbolUnitComboBox->setCurrentIndex( mSymbol->outputUnit() );
    mSymbolUnitComboBox->blockSignals( false );

    mTransparencySlider->blockSignals( true );
    double transparency = 1 - symbol->alpha();
    mTransparencySlider->setValue( transparency * 255 );
    displayTransparency( symbol->alpha() );
    mTransparencySlider->blockSignals( false );
  }

  // select correct page in stacked widget
  // there's a correspondence between symbol type number and page numbering => exploit it!
  stackedWidget->setCurrentIndex( symbol->type() );
  connect( btnColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setSymbolColor( const QColor& ) ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerAngle( double ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerSize( double ) ) );
  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( setLineWidth( double ) ) );

  // Live color updates are not undoable to child symbol layers
  btnColor->setAcceptLiveUpdates( false );
  btnColor->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  // Set symbol color in btnColor
  updateSymbolColor();
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

void QgsSymbolsListWidget::setMarkerSize( double size )
{
  QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( mSymbol );
  if ( markerSymbol->size() == size )
    return;
  markerSymbol->setSize( size );
  emit changed();
}

void QgsSymbolsListWidget::setLineWidth( double width )
{
  QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
  if ( lineSymbol->width() == width )
    return;
  lineSymbol->setWidth( width );
  emit changed();
}

void QgsSymbolsListWidget::addSymbolToStyle()
{
  bool ok;
  QString name = QInputDialog::getText( this, tr( "Symbol name" ),
                                        tr( "Please enter name for the symbol:" ) , QLineEdit::Normal, tr( "New symbol" ), &ok );
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

void QgsSymbolsListWidget::on_mSymbolUnitComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  if ( mSymbol )
  {
    mSymbol->setOutputUnit(( QgsSymbolV2::OutputUnit ) mSymbolUnitComboBox->currentIndex() );

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
  }
  else if ( mSymbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = static_cast<QgsLineSymbolV2*>( mSymbol );
    spinWidth->setValue( lineSymbol->width() );
  }
}

void QgsSymbolsListWidget::setSymbolFromStyle( const QModelIndex & index )
{
  QString symbolName = index.data( Qt::UserRole ).toString();
  lblSymbolName->setText( symbolName );
  // get new instance of symbol from style
  QgsSymbolV2* s = mStyle->symbol( symbolName );
  // remove all symbol layers from original symbol
  while ( mSymbol->symbolLayerCount() )
    mSymbol->deleteSymbolLayer( 0 );
  // move all symbol layers to our symbol
  while ( s->symbolLayerCount() )
  {
    QgsSymbolLayerV2* sl = s->takeSymbolLayer( 0 );
    mSymbol->appendSymbolLayer( sl );
  }
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
