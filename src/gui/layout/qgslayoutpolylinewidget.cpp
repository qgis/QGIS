/***************************************************************************
                         qgslayoutpolylinewidget.cpp
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpolylinewidget.h"
#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"
#include "qgslayoutundostack.h"
#include "qgsvectorlayer.h"
#include "qgslinesymbol.h"

#include <QButtonGroup>
#include <QFileDialog>

QgsLayoutPolylineWidget::QgsLayoutPolylineWidget( QgsLayoutItemPolyline *polyline )
  : QgsLayoutItemBaseWidget( nullptr, polyline )
  , mPolyline( polyline )
{
  setupUi( this );
  setPanelTitle( tr( "Polyline Properties" ) );

  connect( mStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPolylineWidget::arrowStrokeWidthChanged );
  connect( mArrowHeadWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutPolylineWidget::arrowHeadWidthChanged );
  connect( mArrowHeadFillColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutPolylineWidget::arrowHeadFillColorChanged );
  connect( mArrowHeadStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutPolylineWidget::arrowHeadStrokeColorChanged );
  connect( mRadioStartArrow, &QRadioButton::toggled, this, &QgsLayoutPolylineWidget::startArrowHeadToggled );
  connect( mRadioStartNoMarker, &QRadioButton::toggled, this, &QgsLayoutPolylineWidget::startNoMarkerToggled );
  connect( mRadioStartSVG, &QRadioButton::toggled, this, &QgsLayoutPolylineWidget::startSvgMarkerToggled );
  connect( mRadioEndArrow, &QRadioButton::toggled, this, &QgsLayoutPolylineWidget::endArrowHeadToggled );
  connect( mRadioEndNoMarker, &QRadioButton::toggled, this, &QgsLayoutPolylineWidget::endNoMarkerToggled );
  connect( mRadioEndSvg, &QRadioButton::toggled, this, &QgsLayoutPolylineWidget::endSvgMarkerToggled );
  connect( mStartMarkerLineEdit, &QLineEdit::textChanged, this, &QgsLayoutPolylineWidget::mStartMarkerLineEdit_textChanged );
  connect( mEndMarkerLineEdit, &QLineEdit::textChanged, this, &QgsLayoutPolylineWidget::mEndMarkerLineEdit_textChanged );
  connect( mStartMarkerToolButton, &QToolButton::clicked, this, &QgsLayoutPolylineWidget::mStartMarkerToolButton_clicked );
  connect( mEndMarkerToolButton, &QToolButton::clicked, this, &QgsLayoutPolylineWidget::mEndMarkerToolButton_clicked );
  setPanelTitle( tr( "Arrow Properties" ) );
  QButtonGroup *startMarkerGroup = new QButtonGroup( this );
  startMarkerGroup->addButton( mRadioStartNoMarker );
  startMarkerGroup->addButton( mRadioStartArrow );
  startMarkerGroup->addButton( mRadioStartSVG );
  startMarkerGroup->setExclusive( true );
  QButtonGroup *endMarkerGroup = new QButtonGroup( this );
  endMarkerGroup->addButton( mRadioEndNoMarker );
  endMarkerGroup->addButton( mRadioEndArrow );
  endMarkerGroup->addButton( mRadioEndSvg );
  endMarkerGroup->setExclusive( true );

  //disable the svg related gui elements by default
  enableStartSvgInputElements( false );
  enableEndSvgInputElements( false );

  mArrowHeadStrokeColorButton->setColorDialogTitle( tr( "Select Arrow Head Stroke Color" ) );
  mArrowHeadStrokeColorButton->setAllowOpacity( true );
  mArrowHeadStrokeColorButton->setContext( QStringLiteral( "composer" ) );
  mArrowHeadStrokeColorButton->setNoColorString( tr( "Transparent Stroke" ) );
  mArrowHeadStrokeColorButton->setShowNoColor( true );
  mArrowHeadFillColorButton->setColorDialogTitle( tr( "Select Arrow Head Fill Color" ) );
  mArrowHeadFillColorButton->setAllowOpacity( true );
  mArrowHeadFillColorButton->setContext( QStringLiteral( "composer" ) );
  mArrowHeadFillColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mArrowHeadFillColorButton->setShowNoColor( true );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, polyline );
  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolSelectorWidget
  mItemPropertiesWidget->showBackgroundGroup( false );
  mItemPropertiesWidget->showFrameGroup( false );
  mainLayout->addWidget( mItemPropertiesWidget );

  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutPolylineWidget::symbolChanged );

  if ( mPolyline )
  {
    connect( mPolyline, &QgsLayoutObject::changed, this, &QgsLayoutPolylineWidget::setGuiElementValues );
    mLineStyleButton->registerExpressionContextGenerator( mPolyline );
  }
  setGuiElementValues();

  mLineStyleButton->registerExpressionContextGenerator( mPolyline );
  mLineStyleButton->setLayer( coverageLayer() );
  if ( mPolyline->layout() )
  {
    connect( &mPolyline->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mLineStyleButton, &QgsSymbolButton::setLayer );
  }
}

void QgsLayoutPolylineWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

bool QgsLayoutPolylineWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutPolyline )
    return false;

  if ( mPolyline )
  {
    disconnect( mPolyline, &QgsLayoutObject::changed, this, &QgsLayoutPolylineWidget::setGuiElementValues );
  }

  mPolyline = qobject_cast< QgsLayoutItemPolyline * >( item );
  mItemPropertiesWidget->setItem( mPolyline );

  if ( mPolyline )
  {
    connect( mPolyline, &QgsLayoutObject::changed, this, &QgsLayoutPolylineWidget::setGuiElementValues );
    mLineStyleButton->registerExpressionContextGenerator( mPolyline );
  }

  setGuiElementValues();

  return true;
}


void QgsLayoutPolylineWidget::setGuiElementValues()
{
  if ( !mPolyline )
    return;

  whileBlocking( mLineStyleButton )->setSymbol( mPolyline->symbol()->clone() );

  whileBlocking( mArrowHeadFillColorButton )->setColor( mPolyline->arrowHeadFillColor() );
  whileBlocking( mArrowHeadStrokeColorButton )->setColor( mPolyline->arrowHeadStrokeColor() );
  whileBlocking( mStrokeWidthSpinBox )->setValue( mPolyline->arrowHeadStrokeWidth() );
  whileBlocking( mArrowHeadWidthSpinBox )->setValue( mPolyline->arrowHeadWidth() );

  mRadioStartNoMarker->blockSignals( true );
  mRadioStartArrow->blockSignals( true );
  mRadioStartSVG->blockSignals( true );
  mRadioEndArrow->blockSignals( true );
  mRadioEndNoMarker->blockSignals( true );
  mRadioEndSvg->blockSignals( true );
  switch ( mPolyline->startMarker() )
  {
    case QgsLayoutItemPolyline::NoMarker:
      mRadioStartNoMarker->setChecked( true );
      break;
    case QgsLayoutItemPolyline::ArrowHead:
      mRadioStartArrow->setChecked( true );
      break;
    case QgsLayoutItemPolyline::SvgMarker:
      mRadioStartSVG->setChecked( true );
      enableStartSvgInputElements( true );
      break;
  }
  switch ( mPolyline->endMarker() )
  {
    case QgsLayoutItemPolyline::NoMarker:
      mRadioEndNoMarker->setChecked( true );
      break;
    case QgsLayoutItemPolyline::ArrowHead:
      mRadioEndArrow->setChecked( true );
      break;
    case QgsLayoutItemPolyline::SvgMarker:
      mRadioEndSvg->setChecked( true );
      enableEndSvgInputElements( true );
      break;
  }
  mRadioStartNoMarker->blockSignals( false );
  mRadioStartArrow->blockSignals( false );
  mRadioStartSVG->blockSignals( false );
  mRadioEndArrow->blockSignals( false );
  mRadioEndNoMarker->blockSignals( false );
  mRadioEndSvg->blockSignals( false );

  mStartMarkerLineEdit->setText( mPolyline->startSvgMarkerPath() );
  mEndMarkerLineEdit->setText( mPolyline->endSvgMarkerPath() );
}

void QgsLayoutPolylineWidget::symbolChanged()
{
  if ( !mPolyline )
    return;

  mPolyline->layout()->undoStack()->beginCommand( mPolyline, tr( "Change Shape Style" ), QgsLayoutItem::UndoShapeStyle );
  mPolyline->setSymbol( mLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  mPolyline->layout()->undoStack()->endCommand();
}

void QgsLayoutPolylineWidget::arrowStrokeWidthChanged( double d )
{
  if ( !mPolyline )
    return;

  mPolyline->beginCommand( tr( "Change Arrow Head" ), QgsLayoutItem::UndoArrowStrokeWidth );
  mPolyline->setArrowHeadStrokeWidth( d );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::arrowHeadWidthChanged( double d )
{
  if ( !mPolyline )
    return;

  mPolyline->beginCommand( tr( "Change Arrow Width" ), QgsLayoutItem::UndoArrowHeadWidth );
  mPolyline->setArrowHeadWidth( d );
  mPolyline->endCommand();
}


void QgsLayoutPolylineWidget::arrowHeadFillColorChanged( const QColor &newColor )
{
  if ( !mPolyline )
    return;

  mPolyline->beginCommand( tr( "Change Arrow Fill Color" ), QgsLayoutItem::UndoArrowHeadFillColor );
  mPolyline->setArrowHeadFillColor( newColor );
  mPolyline->endCommand();
}


void QgsLayoutPolylineWidget::arrowHeadStrokeColorChanged( const QColor &newColor )
{
  if ( !mPolyline )
    return;

  mPolyline->beginCommand( tr( "Change Arrow Stroke Color" ), QgsLayoutItem::UndoArrowHeadStrokeColor );
  mPolyline->setArrowHeadStrokeColor( newColor );
  mPolyline->endCommand();
}


void QgsLayoutPolylineWidget::startArrowHeadToggled( bool toggled )
{
  if ( !mPolyline || !toggled )
    return;

  mPolyline->beginCommand( tr( "Set Arrow Marker" ) );
  mPolyline->setStartMarker( QgsLayoutItemPolyline::ArrowHead );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::endArrowHeadToggled( bool toggled )
{
  if ( !mPolyline || !toggled )
    return;

  mPolyline->beginCommand( tr( "Set Arrow Marker" ) );
  mPolyline->setEndMarker( QgsLayoutItemPolyline::ArrowHead );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::startNoMarkerToggled( bool toggled )
{
  if ( !mPolyline || !toggled )
    return;

  mPolyline->beginCommand( tr( "Set Line Marker" ) );
  mPolyline->setStartMarker( QgsLayoutItemPolyline::NoMarker );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::endNoMarkerToggled( bool toggled )
{
  if ( !mPolyline || !toggled )
    return;

  mPolyline->beginCommand( tr( "Set Line Marker" ) );
  mPolyline->setEndMarker( QgsLayoutItemPolyline::NoMarker );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::startSvgMarkerToggled( bool toggled )
{
  enableStartSvgInputElements( toggled );
  if ( !mPolyline || !toggled )
    return;

  mPolyline->beginCommand( tr( "Set SVG Marker" ) );
  mPolyline->setStartMarker( QgsLayoutItemPolyline::SvgMarker );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::endSvgMarkerToggled( bool toggled )
{
  enableEndSvgInputElements( toggled );
  if ( !mPolyline || !toggled )
    return;

  mPolyline->beginCommand( tr( "Set SVG Marker" ) );
  mPolyline->setEndMarker( QgsLayoutItemPolyline::SvgMarker );
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::enableStartSvgInputElements( bool enable )
{
  mStartMarkerLineEdit->setEnabled( enable );
  mStartMarkerToolButton->setEnabled( enable );
}

void QgsLayoutPolylineWidget::enableEndSvgInputElements( bool enable )
{
  mEndMarkerLineEdit->setEnabled( enable );
  mEndMarkerToolButton->setEnabled( enable );
}

void QgsLayoutPolylineWidget::mStartMarkerLineEdit_textChanged( const QString &text )
{
  if ( !mPolyline )
    return;

  mPolyline->beginCommand( tr( "Change Start Marker File" ) );
  const QFileInfo fi( text );
  if ( fi.exists() && fi.isFile() )
  {
    mPolyline->setStartSvgMarkerPath( text );
  }
  else
  {
    mPolyline->setStartSvgMarkerPath( QString() );
  }
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::mEndMarkerLineEdit_textChanged( const QString &text )
{
  if ( !mPolyline )
    return;

  mPolyline->beginCommand( tr( "Change End Marker File" ) );
  const QFileInfo fi( text );
  if ( fi.exists() && fi.isFile() )
  {
    mPolyline->setEndSvgMarkerPath( text );
  }
  else
  {
    mPolyline->setEndSvgMarkerPath( QString() );
  }
  mPolyline->endCommand();
}

void QgsLayoutPolylineWidget::mStartMarkerToolButton_clicked()
{
  QgsSettings s;
  QString openDir;

  if ( !mStartMarkerLineEdit->text().isEmpty() )
  {
    const QFileInfo fi( mStartMarkerLineEdit->text() );
    openDir = fi.dir().absolutePath();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( QStringLiteral( "/UI/lastComposerMarkerDir" ), QDir::homePath() ).toString();
  }

  const QString svgFileName = QFileDialog::getOpenFileName( this, tr( "Start marker svg file" ), openDir );
  if ( !svgFileName.isNull() )
  {
    const QFileInfo fileInfo( svgFileName );
    s.setValue( QStringLiteral( "/UI/lastComposerMarkerDir" ), fileInfo.absolutePath() );
    mPolyline->beginCommand( tr( "Change Start Marker File" ) );
    mStartMarkerLineEdit->setText( svgFileName );
    mPolyline->endCommand();
  }
}

void QgsLayoutPolylineWidget::mEndMarkerToolButton_clicked()
{
  QgsSettings s;
  QString openDir;

  if ( !mEndMarkerLineEdit->text().isEmpty() )
  {
    const QFileInfo fi( mEndMarkerLineEdit->text() );
    openDir = fi.dir().absolutePath();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( QStringLiteral( "/UI/lastComposerMarkerDir" ), QDir::homePath() ).toString();
  }

  const QString svgFileName = QFileDialog::getOpenFileName( this, tr( "End marker svg file" ), openDir );
  if ( !svgFileName.isNull() )
  {
    const QFileInfo fileInfo( svgFileName );
    s.setValue( QStringLiteral( "/UI/lastComposerMarkerDir" ), fileInfo.absolutePath() );
    mPolyline->beginCommand( tr( "Change End Marker File" ) );
    mEndMarkerLineEdit->setText( svgFileName );
    mPolyline->endCommand();
  }
}
