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
#include "qgssymbollevelsv2dialog.h"
#include "qgsexpressionbuilderdialog.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

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

  QgsDataDefinedWidthDialog dlg( symbolList, mLayer );

  if ( QMessageBox::Ok == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        if (( *symbolIt )->type() == QgsSymbolV2::Line )
          static_cast<QgsLineSymbolV2*>( *symbolIt )->setWidth( dlg.mSpinBox->value() );
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

  QgsDataDefinedSizeDialog dlg( symbolList, mLayer );

  if ( QMessageBox::Ok == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        if (( *symbolIt )->type() == QgsSymbolV2::Marker )
          static_cast<QgsMarkerSymbolV2*>( *symbolIt )->setSize( dlg.mSpinBox->value() );
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

  QgsDataDefinedRotationDialog dlg( symbolList, mLayer );

  if ( QMessageBox::Ok == dlg.exec() )
  {
    if ( !dlg.mDDBtn->isActive() )
    {
      QList<QgsSymbolV2*>::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        if (( *symbolIt )->type() == QgsSymbolV2::Marker )
          static_cast<QgsMarkerSymbolV2*>( *symbolIt )->setAngle( dlg.mSpinBox->value() );
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

#include "qgsfield.h"

QgsRendererV2DataDefinedMenus::QgsRendererV2DataDefinedMenus( QMenu* menu, QgsVectorLayer* layer, QString rotationField, QString sizeScaleField, QgsSymbolV2::ScaleMethod scaleMethod )
    : QObject( menu ), mLayer( layer )
{
  mRotationMenu = new QMenu( tr( "Rotation field" ) );
  mSizeScaleMenu = new QMenu( tr( "Size scale field" ) );

  mRotationAttributeActionGroup = new QActionGroup( mRotationMenu );
  mSizeAttributeActionGroup = new QActionGroup( mSizeScaleMenu );
  mSizeMethodActionGroup = new QActionGroup( mSizeScaleMenu );

  populateMenu( mRotationMenu, rotationField, mRotationAttributeActionGroup );
  populateMenu( mSizeScaleMenu, sizeScaleField, mSizeAttributeActionGroup );

  mSizeScaleMenu->addSeparator();

  QAction* aScaleByArea = new QAction( tr( "Scale area" ), mSizeMethodActionGroup );
  QAction* aScaleByDiameter = new QAction( tr( "Scale diameter" ), mSizeMethodActionGroup );

  aScaleByArea->setCheckable( true );
  aScaleByDiameter->setCheckable( true );

  if ( scaleMethod == QgsSymbolV2::ScaleDiameter )
  {
    aScaleByDiameter->setChecked( true );
  }
  else
  {
    aScaleByArea->setChecked( true );
  }

  mSizeScaleMenu->addActions( mSizeMethodActionGroup->actions() );

  //@todo cleanup the class since Rotation and SizeScale are now
  //defined using QgsDataDefinedButton
  //
  //menu->addMenu( mRotationMenu );
  //menu->addMenu( mSizeScaleMenu );

  connect( mSizeMethodActionGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( scaleMethodSelected( QAction* ) ) );
  connect( mRotationAttributeActionGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( rotationFieldSelected( QAction* ) ) );
  connect( mSizeAttributeActionGroup, SIGNAL( triggered( QAction* ) ), this, SLOT( sizeScaleFieldSelected( QAction* ) ) );
}

QgsRendererV2DataDefinedMenus::~QgsRendererV2DataDefinedMenus()
{
  delete mSizeMethodActionGroup;
  delete mSizeAttributeActionGroup;
  delete mRotationAttributeActionGroup;
  delete mRotationMenu;
  delete mSizeScaleMenu;
}

void QgsRendererV2DataDefinedMenus::populateMenu( QMenu* menu, QString fieldName, QActionGroup *actionGroup )
{
  QAction* aExpr = new QAction( tr( "- expression -" ), actionGroup );
  aExpr->setCheckable( true );
  menu->addAction( aExpr );
  menu->addSeparator();
  QAction* aNo = new QAction( tr( "- no field -" ), actionGroup );
  aNo->setCheckable( true );
  menu->addAction( aNo );
  menu->addSeparator();

  bool hasField = false;
  const QgsFields & flds = mLayer->pendingFields();
  for ( int idx = 0; idx < flds.count(); ++idx )
  {
    const QgsField& fld = flds[idx];
    if ( fld.type() == QVariant::Int || fld.type() == QVariant::Double )
    {
      QAction* a = new QAction( fld.name(), actionGroup );
      a->setCheckable( true );
      if ( fieldName == fld.name() )
      {
        a->setChecked( true );
        hasField = true;
      }
      menu->addAction( a );
    }
  }

  if ( !hasField )
  {
    if ( fieldName.isEmpty() )
    {
      aNo->setChecked( true );
    }
    else
    {
      aExpr->setChecked( true );
      aExpr->setText( tr( "- expression -" ) + fieldName );
    }
  }

}

void QgsRendererV2DataDefinedMenus::rotationFieldSelected( QAction* a )
{
  if ( a == NULL )
    return;

  QString fldName = a->text();
#if 0
  updateMenu( mRotationAttributeActionGroup, fldName );
#endif
  if ( fldName == tr( "- no field -" ) )
  {
    fldName = QString();
  }
  else if ( fldName.startsWith( tr( "- expression -" ) ) )
  {
    QString expr( fldName );
    expr.replace( 0, tr( "- expression -" ).length(), "" );
    QgsExpressionBuilderDialog dialog( mLayer, expr );
    if ( !dialog.exec() ) return;
    fldName = dialog.expressionText();
    Q_ASSERT( !QgsExpression( fldName ).hasParserError() );
    a->setText( tr( "- expression -" ) + fldName );
  }

  emit rotationFieldChanged( fldName );
}

void QgsRendererV2DataDefinedMenus::sizeScaleFieldSelected( QAction* a )
{
  if ( a == NULL )
    return;

  QString fldName = a->text();
#if 0
  updateMenu( mSizeAttributeActionGroup, fldName );
#endif
  if ( fldName == tr( "- no field -" ) )
  {
    fldName = QString();
  }
  else if ( fldName.startsWith( tr( "- expression -" ) ) )
  {
    QString expr( fldName );
    expr.replace( 0, tr( "- expression -" ).length(), "" );
    QgsExpressionBuilderDialog dialog( mLayer, expr );
    if ( !dialog.exec() ) return;
    fldName = dialog.expressionText();
    Q_ASSERT( !QgsExpression( fldName ).hasParserError() );
    a->setText( tr( "- expression -" ) + fldName );
  }

  emit sizeScaleFieldChanged( fldName );
}

void QgsRendererV2DataDefinedMenus::scaleMethodSelected( QAction* a )
{
  if ( a == NULL )
    return;

  if ( a->text() == tr( "Scale area" ) )
  {
    emit scaleMethodChanged( QgsSymbolV2::ScaleArea );
  }
  else if ( a->text() == tr( "Scale diameter" ) )
  {
    emit scaleMethodChanged( QgsSymbolV2::ScaleDiameter );
  }
}
#if 0 // MK: is there any reason for this?
void QgsRendererV2DataDefinedMenus::updateMenu( QActionGroup* actionGroup, QString fieldName )
{
  foreach ( QAction* a, actionGroup->actions() )
  {
    a->setChecked( a->text() == fieldName );
  }
}
#endif

QgsDataDefinedValueDialog::QgsDataDefinedValueDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer, const QString & label )
    : mSymbolList( symbolList )
    , mLayer( layer )
{
  setupUi( this );
  setWindowFlags( Qt::WindowStaysOnTopHint );
  mLabel->setText( label );
  connect( mDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( dataDefinedChanged() ) );
  connect( mDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( dataDefinedChanged() ) );

}

void QgsDataDefinedValueDialog::init( const QString & description )
{
  QgsDataDefined dd = symbolDataDefined();
  mDDBtn->init( mLayer, &dd, QgsDataDefinedButton::Double, description );
  mSpinBox->setValue( value( mSymbolList.back() ) );
  mSpinBox->setEnabled( !mDDBtn->isActive() );
}

QgsDataDefined QgsDataDefinedValueDialog::symbolDataDefined() const
{
  // check that all symbols share the same size expression
  QgsDataDefined dd = symbolDataDefined( mSymbolList.back() );
  foreach ( QgsSymbolV2 * it, mSymbolList )
  {
    if ( symbolDataDefined( it ) != dd ) return  QgsDataDefined();
  }
  return dd;
}

void QgsDataDefinedValueDialog::dataDefinedChanged()
{
  QgsDataDefined dd = mDDBtn->currentDataDefined();
  mSpinBox->setEnabled( !dd.isActive() );

  if ( // shall we remove datadefined expressions for layers ?
    ( symbolDataDefined().isActive() && !dd.isActive() )
    // shall we set the "en masse" expression for properties ?
    || dd.isActive() )
  {
    foreach ( QgsSymbolV2 * it, mSymbolList )
      setDataDefined( it, dd );
  }
}

QgsDataDefined QgsDataDefinedSizeDialog::symbolDataDefined( const QgsSymbolV2 *symbol ) const
{
  const QgsMarkerSymbolV2* marker = static_cast<const QgsMarkerSymbolV2*>( symbol );
  return marker->dataDefinedSize();
}

void QgsDataDefinedSizeDialog::setDataDefined( QgsSymbolV2* symbol, const QgsDataDefined& dd )
{
  static_cast<QgsMarkerSymbolV2*>( symbol )->setDataDefinedSize( dd );
}


QgsDataDefined QgsDataDefinedRotationDialog::symbolDataDefined( const QgsSymbolV2 *symbol ) const
{
  const QgsMarkerSymbolV2* marker = static_cast<const QgsMarkerSymbolV2*>( symbol );
  return marker->dataDefinedAngle();
}

void QgsDataDefinedRotationDialog::setDataDefined( QgsSymbolV2 *symbol, const QgsDataDefined &dd )
{
  static_cast<QgsMarkerSymbolV2*>( symbol )->setDataDefinedAngle( dd );
}


QgsDataDefined QgsDataDefinedWidthDialog::symbolDataDefined( const QgsSymbolV2 *symbol ) const
{
  const QgsLineSymbolV2* line = static_cast<const QgsLineSymbolV2*>( symbol );
  return line->dataDefinedWidth();
}

void QgsDataDefinedWidthDialog::setDataDefined( QgsSymbolV2 *symbol, const QgsDataDefined &dd )
{
  static_cast<QgsLineSymbolV2*>( symbol )->setDataDefinedWidth( dd );
}
