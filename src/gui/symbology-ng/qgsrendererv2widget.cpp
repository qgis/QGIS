/***************************************************************************
    qgsrendererv2widget.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrendererv2widget.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"
#include "qgscolordialog.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

#include "qgssymbollevelsv2dialog.h"
#include "qgsexpressionbuilderdialog.h"


QgsRendererV2Widget::QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style )
    : QWidget(), mLayer( layer ), mStyle( style )
{
  contextMenu = new QMenu( "Renderer Options " );

  mCopyAction = contextMenu->addAction( tr( "Copy" ), this, SLOT( copy() ) );
  mCopyAction->setShortcut( QKeySequence( QKeySequence::Copy ) );
  mPasteAction = contextMenu->addAction( tr( "Paste" ), this, SLOT( paste() ) );
  mPasteAction->setShortcut( QKeySequence( QKeySequence::Paste ) );

  contextMenu->addSeparator();
  contextMenu->addAction( tr( "Change color" ), this, SLOT( changeSymbolColor() ) );
  contextMenu->addAction( tr( "Change transparency" ), this, SLOT( changeSymbolTransparency() ) );
  contextMenu->addAction( tr( "Change output unit" ), this, SLOT( changeSymbolUnit() ) );

  if ( mLayer && mLayer->geometryType() == QGis::Line )
  {
    contextMenu->addAction( tr( "Change width" ), this, SLOT( changeSymbolWidth() ) );
  }
  else if ( mLayer && mLayer->geometryType() == QGis::Point )
  {
    contextMenu->addAction( tr( "Change size" ), this, SLOT( changeSymbolSize() ) );
    contextMenu->addAction( tr( "Change angle" ), this, SLOT( changeSymbolAngle() ) );
  }
}

void QgsRendererV2Widget::contextMenuViewCategories( const QPoint & )
{
  contextMenu->exec( QCursor::pos() );
}

void QgsRendererV2Widget::changeSymbolColor()
{
  QList<QgsSymbolV2*> symbolList = selectedSymbols();
  if ( symbolList.size() < 1 )
  {
    return;
  }

  QColor color = QgsColorDialogV2::getColor( symbolList.at( 0 )->color(), this, "Change Symbol Color", true );
  if ( color.isValid() )
  {
    QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
    for ( ; symbolIt != symbolList.end(); ++symbolIt )
    {
      ( *symbolIt )->setColor( color );
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolTransparency()
{
  QList<QgsSymbolV2*> symbolList = selectedSymbols();
  if ( symbolList.size() < 1 )
  {
    return;
  }

  bool ok;
  double oldTransparency = ( 1 - symbolList.at( 0 )->alpha() ) * 100; // convert to percents
  double transparency = QInputDialog::getDouble( this, tr( "Transparency" ), tr( "Change symbol transparency [%]" ), oldTransparency, 0.0, 100.0, 0, &ok );
  if ( ok )
  {
    QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
    for ( ; symbolIt != symbolList.end(); ++symbolIt )
    {
      ( *symbolIt )->setAlpha( 1 - transparency / 100 );
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolUnit()
{
  QList<QgsSymbolV2*> symbolList = selectedSymbols();
  if ( symbolList.size() < 1 )
  {
    return;
  }

  bool ok;
  int currentUnit = ( symbolList.at( 0 )->outputUnit() == QgsSymbolV2::MM ) ? 0 : 1;
  QString item = QInputDialog::getItem( this, tr( "Symbol unit" ), tr( "Select symbol unit" ), QStringList() << tr( "Millimeter" ) << tr( "Map unit" ), currentUnit, false, &ok );
  if ( ok )
  {
    QgsSymbolV2::OutputUnit unit = ( item.compare( tr( "Millimeter" ) ) == 0 ) ? QgsSymbolV2::MM : QgsSymbolV2::MapUnit;

    QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
    for ( ; symbolIt != symbolList.end(); ++symbolIt )
    {
      ( *symbolIt )->setOutputUnit( unit );
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolWidth()
{
  QList<QgsSymbolV2*> symbolList = selectedSymbols();
  if ( symbolList.size() < 1 )
  {
    return;
  }

  QgsEnMasseWidthDialog dlg( symbolList, mLayer );

  if ( QMessageBox::Ok == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        dynamic_cast<QgsLineSymbolV2*>( *symbolIt )->setWidth( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolSize()
{
  QList<QgsSymbolV2*> symbolList = selectedSymbols();
  if ( symbolList.size() < 1 )
  {
    return;
  }

  QgsEnMasseSizeDialog dlg( symbolList, mLayer );

  if ( QMessageBox::Ok == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        dynamic_cast<QgsMarkerSymbolV2*>( *symbolIt )->setSize( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::changeSymbolAngle()
{
  QList<QgsSymbolV2*> symbolList = selectedSymbols();
  if ( symbolList.size() < 1 )
  {
    return;
  }

  QgsEnMasseRotationDialog dlg( symbolList, mLayer );

  if ( QMessageBox::Ok == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        dynamic_cast<QgsMarkerSymbolV2*>( *symbolIt )->setAngle( dlg.mSpinBox->value() );
      }
    }
    refreshSymbolView();
  }
}

void QgsRendererV2Widget::showSymbolLevelsDialog( QgsFeatureRendererV2* r )
{
  QgsLegendSymbolList symbols = r->legendSymbolItems();

  QgsSymbolLevelsV2Dialog dlg( symbols, r->usingSymbolLevels(), this );

  if ( dlg.exec() )
  {
    r->setUsingSymbolLevels( dlg.usingLevels() );
  }
}


////////////

QgsEnMasseValueDialog::QgsEnMasseValueDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer, const QString & label )
    : mSymbolList( symbolList )
    , mLayer( layer )
{
  setupUi( this );
  mLabel->setText( label );
  connect( mDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( setSymbolExpression( const QString& ) ) );
  connect( mDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( setActiveSymbolExpression( bool ) ) );

}

void QgsEnMasseValueDialog::init( const QString & description )
{
  QgsDataDefined dd = symbolExpression();
  mDDBtn->init( mLayer, &dd, QgsDataDefinedButton::Double, description );
  mSpinBox->setValue( value( mSymbolList.back() ) );
  mSpinBox->setEnabled( !mDDBtn->isActive() );
}

QgsDataDefined QgsEnMasseValueDialog::symbolExpression() const
{
  // check that all symbols share the same size expression
  QgsDataDefined dd = expression( mSymbolList.back() );
for ( auto it : mSymbolList )
  {
    if ( expression( it ) != dd )
      return  QgsDataDefined();
  }
  return dd;
}

void QgsEnMasseValueDialog::setSymbolExpression( const QString & definition )
{
  if ( // shall we remove datadefined expressions for layers ?
    ( symbolExpression().isActive() && !definition.length() )
    // shall we set the "en masse" expression for properties ?
    || definition.length() )
  {
  for ( auto it : mSymbolList )
      setExpression( it, definition );
  }
}

void QgsEnMasseValueDialog::setActiveSymbolExpression( bool active )
{
  setSymbolExpression( active ? mDDBtn->currentDefinition() : "" );
  mSpinBox->setEnabled( !active );
}

