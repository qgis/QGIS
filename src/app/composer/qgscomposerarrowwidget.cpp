/***************************************************************************
                         qgscomposerarrowwidget.cpp
                         --------------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerarrowwidget.h"
#include "qgscomposerarrow.h"
#include "qgscomposeritemwidget.h"
#include "qgssymbolselectordialog.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>

QgsComposerArrowWidget::QgsComposerArrowWidget( QgsComposerArrow *arrow ): QgsComposerItemBaseWidget( nullptr, arrow ), mArrow( arrow )
{
  setupUi( this );
  connect( mStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerArrowWidget::mStrokeWidthSpinBox_valueChanged );
  connect( mArrowHeadWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerArrowWidget::mArrowHeadWidthSpinBox_valueChanged );
  connect( mArrowHeadFillColorButton, &QgsColorButton::colorChanged, this, &QgsComposerArrowWidget::mArrowHeadFillColorButton_colorChanged );
  connect( mArrowHeadStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsComposerArrowWidget::mArrowHeadStrokeColorButton_colorChanged );
  connect( mDefaultMarkerRadioButton, &QRadioButton::toggled, this, &QgsComposerArrowWidget::mDefaultMarkerRadioButton_toggled );
  connect( mNoMarkerRadioButton, &QRadioButton::toggled, this, &QgsComposerArrowWidget::mNoMarkerRadioButton_toggled );
  connect( mSvgMarkerRadioButton, &QRadioButton::toggled, this, &QgsComposerArrowWidget::mSvgMarkerRadioButton_toggled );
  connect( mStartMarkerLineEdit, &QLineEdit::textChanged, this, &QgsComposerArrowWidget::mStartMarkerLineEdit_textChanged );
  connect( mEndMarkerLineEdit, &QLineEdit::textChanged, this, &QgsComposerArrowWidget::mEndMarkerLineEdit_textChanged );
  connect( mStartMarkerToolButton, &QToolButton::clicked, this, &QgsComposerArrowWidget::mStartMarkerToolButton_clicked );
  connect( mEndMarkerToolButton, &QToolButton::clicked, this, &QgsComposerArrowWidget::mEndMarkerToolButton_clicked );
  connect( mLineStyleButton, &QPushButton::clicked, this, &QgsComposerArrowWidget::mLineStyleButton_clicked );
  setPanelTitle( tr( "Arrow properties" ) );
  mRadioButtonGroup = new QButtonGroup( this );
  mRadioButtonGroup->addButton( mDefaultMarkerRadioButton );
  mRadioButtonGroup->addButton( mNoMarkerRadioButton );
  mRadioButtonGroup->addButton( mSvgMarkerRadioButton );
  mRadioButtonGroup->setExclusive( true );

  //disable the svg related gui elements by default
  mSvgMarkerRadioButton_toggled( false );

  //add widget for general composer item properties
  QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, mArrow );
  mainLayout->addWidget( itemPropertiesWidget );

  mArrowHeadStrokeColorButton->setColorDialogTitle( tr( "Select Arrow Head Stroke Color" ) );
  mArrowHeadStrokeColorButton->setAllowOpacity( true );
  mArrowHeadStrokeColorButton->setContext( QStringLiteral( "composer" ) );
  mArrowHeadStrokeColorButton->setNoColorString( tr( "Transparent stroke" ) );
  mArrowHeadStrokeColorButton->setShowNoColor( true );
  mArrowHeadFillColorButton->setColorDialogTitle( tr( "Select Arrow Head Fill Color" ) );
  mArrowHeadFillColorButton->setAllowOpacity( true );
  mArrowHeadFillColorButton->setContext( QStringLiteral( "composer" ) );
  mArrowHeadFillColorButton->setNoColorString( tr( "Transparent fill" ) );
  mArrowHeadFillColorButton->setShowNoColor( true );

  setGuiElementValues();

  if ( arrow )
  {
    connect( arrow, &QgsComposerObject::itemChanged, this, &QgsComposerArrowWidget::setGuiElementValues );
  }
}

void QgsComposerArrowWidget::mStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrow head stroke width" ), QgsComposerMergeCommand::ArrowStrokeWidth );
  mArrow->setArrowHeadStrokeWidth( d );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::mArrowHeadWidthSpinBox_valueChanged( double d )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrowhead width" ), QgsComposerMergeCommand::ArrowHeadWidth );
  mArrow->setArrowHeadWidth( d );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::mArrowHeadFillColorButton_colorChanged( const QColor &newColor )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrow head fill color" ), QgsComposerMergeCommand::ArrowHeadFillColor );
  mArrow->setArrowHeadFillColor( newColor );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::mArrowHeadStrokeColorButton_colorChanged( const QColor &newColor )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrow head stroke color" ), QgsComposerMergeCommand::ArrowHeadStrokeColor );
  mArrow->setArrowHeadStrokeColor( newColor );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::blockAllSignals( bool block )
{
  mLineStyleButton->blockSignals( block );
  mArrowHeadFillColorButton->blockSignals( block );
  mArrowHeadStrokeColorButton->blockSignals( block );
  mStrokeWidthSpinBox->blockSignals( block );
  mArrowHeadWidthSpinBox->blockSignals( block );
  mDefaultMarkerRadioButton->blockSignals( block );
  mNoMarkerRadioButton->blockSignals( block );
  mSvgMarkerRadioButton->blockSignals( block );
  mStartMarkerLineEdit->blockSignals( block );
  mStartMarkerToolButton->blockSignals( block );
  mEndMarkerLineEdit->blockSignals( block );
  mEndMarkerToolButton->blockSignals( block );
}

void QgsComposerArrowWidget::setGuiElementValues()
{
  if ( !mArrow )
  {
    return;
  }

  blockAllSignals( true );
  mArrowHeadFillColorButton->setColor( mArrow->arrowHeadFillColor() );
  mArrowHeadStrokeColorButton->setColor( mArrow->arrowHeadStrokeColor() );
  mStrokeWidthSpinBox->setValue( mArrow->arrowHeadStrokeWidth() );
  mArrowHeadWidthSpinBox->setValue( mArrow->arrowHeadWidth() );

  QgsComposerArrow::MarkerMode mode = mArrow->markerMode();
  if ( mode == QgsComposerArrow::DefaultMarker )
  {
    mDefaultMarkerRadioButton->setChecked( true );
  }
  else if ( mode == QgsComposerArrow::NoMarker )
  {
    mNoMarkerRadioButton->setChecked( true );
  }
  else //svg marker
  {
    mSvgMarkerRadioButton->setChecked( true );
    enableSvgInputElements( true );
  }
  mStartMarkerLineEdit->setText( mArrow->startMarker() );
  mEndMarkerLineEdit->setText( mArrow->endMarker() );

  updateLineSymbolMarker();

  blockAllSignals( false );
}

void QgsComposerArrowWidget::updateLineStyleFromWidget()
{
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( sender() );
  mArrow->setLineSymbol( dynamic_cast< QgsLineSymbol * >( w->symbol()->clone() ) );
  mArrow->update();
}

void QgsComposerArrowWidget::cleanUpLineStyleSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !w )
    return;

  delete w->symbol();
  updateLineSymbolMarker();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::enableSvgInputElements( bool enable )
{
  mStartMarkerLineEdit->setEnabled( enable );
  mStartMarkerToolButton->setEnabled( enable );
  mEndMarkerLineEdit->setEnabled( enable );
  mEndMarkerToolButton->setEnabled( enable );
}

void QgsComposerArrowWidget::mDefaultMarkerRadioButton_toggled( bool toggled )
{
  if ( mArrow && toggled )
  {
    mArrow->beginCommand( tr( "Arrow marker changed" ) );
    mArrow->setMarkerMode( QgsComposerArrow::DefaultMarker );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mNoMarkerRadioButton_toggled( bool toggled )
{
  if ( mArrow && toggled )
  {
    mArrow->beginCommand( tr( "Arrow marker changed" ) );
    mArrow->setMarkerMode( QgsComposerArrow::NoMarker );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mSvgMarkerRadioButton_toggled( bool toggled )
{
  enableSvgInputElements( toggled );
  if ( mArrow && toggled )
  {
    mArrow->beginCommand( tr( "Arrow marker changed" ) );
    mArrow->setMarkerMode( QgsComposerArrow::SVGMarker );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mStartMarkerLineEdit_textChanged( const QString &text )
{
  if ( mArrow )
  {
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    QFileInfo fi( text );
    if ( fi.exists() && fi.isFile() )
    {
      mArrow->setStartMarker( text );
    }
    else
    {
      mArrow->setStartMarker( QString() );
    }
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mEndMarkerLineEdit_textChanged( const QString &text )
{
  if ( mArrow )
  {
    mArrow->beginCommand( tr( "Arrow end marker" ) );
    QFileInfo fi( text );
    if ( fi.exists() && fi.isFile() )
    {
      mArrow->setEndMarker( text );
    }
    else
    {
      mArrow->setEndMarker( QString() );
    }
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mStartMarkerToolButton_clicked()
{
  QgsSettings s;
  QString openDir;

  if ( !mStartMarkerLineEdit->text().isEmpty() )
  {
    QFileInfo fi( mStartMarkerLineEdit->text() );
    openDir = fi.dir().absolutePath();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( QStringLiteral( "/UI/lastComposerMarkerDir" ), QDir::homePath() ).toString();
  }

  QString svgFileName = QFileDialog::getOpenFileName( this, tr( "Start marker svg file" ), openDir );
  if ( !svgFileName.isNull() )
  {
    QFileInfo fileInfo( svgFileName );
    s.setValue( QStringLiteral( "/UI/lastComposerMarkerDir" ), fileInfo.absolutePath() );
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    mStartMarkerLineEdit->setText( svgFileName );
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mEndMarkerToolButton_clicked()
{
  QgsSettings s;
  QString openDir;

  if ( !mEndMarkerLineEdit->text().isEmpty() )
  {
    QFileInfo fi( mEndMarkerLineEdit->text() );
    openDir = fi.dir().absolutePath();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( QStringLiteral( "/UI/lastComposerMarkerDir" ), QDir::homePath() ).toString();
  }

  QString svgFileName = QFileDialog::getOpenFileName( this, tr( "End marker svg file" ), openDir );
  if ( !svgFileName.isNull() )
  {
    QFileInfo fileInfo( svgFileName );
    s.setValue( QStringLiteral( "/UI/lastComposerMarkerDir" ), fileInfo.absolutePath() );
    mArrow->beginCommand( tr( "Arrow end marker" ) );
    mEndMarkerLineEdit->setText( svgFileName );
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::mLineStyleButton_clicked()
{
  if ( !mArrow )
  {
    return;
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();

  QgsLineSymbol *newSymbol = mArrow->lineSymbol()->clone();
  QgsExpressionContext context = mArrow->createExpressionContext();

  QgsSymbolSelectorWidget *d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, &QgsPanelWidget::widgetChanged, this, &QgsComposerArrowWidget::updateLineStyleFromWidget );
  connect( d, &QgsPanelWidget::panelAccepted, this, &QgsComposerArrowWidget::cleanUpLineStyleSelector );
  openPanel( d );
  mArrow->beginCommand( tr( "Arrow line style changed" ) );
}

void QgsComposerArrowWidget::updateLineSymbolMarker()
{
  if ( !mArrow )
  {
    return;
  }

  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( mArrow->lineSymbol(), mLineStyleButton->iconSize() );
  mLineStyleButton->setIcon( icon );
}
