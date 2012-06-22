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


QgsSymbolsListWidget::QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QWidget* parent ) : QWidget( parent )
{
  mSymbol = symbol;
  mStyle = style;

  setupUi( this );

  btnAdvanced->hide(); // advanced button is hidden by default

  QStandardItemModel* model = new QStandardItemModel( viewSymbols );
  viewSymbols->setModel( model );
  connect( viewSymbols, SIGNAL( clicked( const QModelIndex & ) ), this, SLOT( setSymbolFromStyle( const QModelIndex & ) ) );

  connect( btnStyleManager, SIGNAL( clicked() ), SLOT( openStyleManager() ) );
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
  connect( btnColor, SIGNAL( clicked() ), this, SLOT( setSymbolColor() ) );
  connect( spinAngle, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerAngle( double ) ) );
  connect( spinSize, SIGNAL( valueChanged( double ) ), this, SLOT( setMarkerSize( double ) ) );
  connect( spinWidth, SIGNAL( valueChanged( double ) ), this, SLOT( setLineWidth( double ) ) );

  connect( btnAddToStyle, SIGNAL( clicked() ), this, SLOT( addSymbolToStyle() ) );
  btnAddToStyle->setIcon( QIcon( QgsApplication::defaultThemePath() + "symbologyAdd.png" ) );

}


void QgsSymbolsListWidget::populateSymbolView()
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

  QStringList names = mStyle->symbolNames();
  for ( int i = 0; i < names.count(); i++ )
  {
    QgsSymbolV2* s = mStyle->symbol( names[i] );
    if ( s->type() != mSymbol->type() )
    {
      delete s;
      continue;
    }
    QStandardItem* item = new QStandardItem( names[i] );
    item->setData( names[i], Qt::UserRole ); //so we can show a label when it is clicked
    item->setText( "" ); //set the text to nothing and show in label when clicked rather
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
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

void QgsSymbolsListWidget::setSymbolColor()
{
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  // Qt bug: http://bugreports.qt.nokia.com/browse/QTBUG-14889
  // FIXME need to also check max QT_VERSION when Qt bug fixed
  QColor color = QColorDialog::getColor( mSymbol->color(), this, "", QColorDialog::DontUseNativeDialog );
#else
  QColor color = QColorDialog::getColor( mSymbol->color(), this );
#endif
  if ( !color.isValid() )
    return;

  mSymbol->setColor( color );
  updateSymbolColor();
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
  mStyle->save();

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
  btnColor->setColor( mSymbol->color() );
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

QMenu* QgsSymbolsListWidget::advancedMenu()
{
  if ( mAdvancedMenu == NULL )
  {
    mAdvancedMenu = new QMenu;
    btnAdvanced->setMenu( mAdvancedMenu );
    btnAdvanced->show();
  }
  return mAdvancedMenu;
}




