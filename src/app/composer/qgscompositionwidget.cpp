/***************************************************************************
                              qgscompositionwidget.cpp
                             --------------------------
    begin                : June 11 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgis.h>
#include "qgscompositionwidget.h"
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposeritem.h"
#include "qgsstylev2.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssymbollayerv2utils.h"
#include "qgsexpressioncontext.h"
#include <QColorDialog>
#include <QWidget>
#include <QPrinter> //for screen resolution

QgsCompositionWidget::QgsCompositionWidget( QWidget* parent, QgsComposition* c ): QWidget( parent ), mComposition( c )
{
  setupUi( this );
  blockSignals( true );
  createPaperEntries();

  //unit
  mPaperUnitsComboBox->addItem( tr( "mm" ) );
  mPaperUnitsComboBox->addItem( tr( "inch" ) );

  //orientation
  mPaperOrientationComboBox->insertItem( 0, tr( "Landscape" ) );
  mPaperOrientationComboBox->insertItem( 1, tr( "Portrait" ) );
  mPaperOrientationComboBox->setCurrentIndex( 0 );

  //read with/height from composition and find suitable entries to display
  displayCompositionWidthHeight();

  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::compositionScope( mComposition ) );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 2 );

  connect( mVariableEditor, SIGNAL( scopeChanged() ), this, SLOT( variablesChanged() ) );

  if ( mComposition )
  {
    mNumPagesSpinBox->setValue( mComposition->numPages() );
    connect( mComposition, SIGNAL( nPagesChanged() ), this, SLOT( setNumberPages() ) );

    updatePageStyle();

    //read printout resolution from composition
    mResolutionSpinBox->setValue( mComposition->printResolution() );

    double topMargin = 0;
    double rightMargin = 0;
    double bottomMargin = 0;
    double leftMargin = 0;
    mComposition->resizeToContentsMargins( topMargin, rightMargin, bottomMargin, leftMargin );
    mTopMarginSpinBox->setValue( topMargin );
    mRightMarginSpinBox->setValue( rightMargin );
    mBottomMarginSpinBox->setValue( bottomMargin );
    mLeftMarginSpinBox->setValue( leftMargin );

    //print as raster
    mPrintAsRasterCheckBox->setChecked( mComposition->printAsRaster() );

    // world file generation
    mGenerateWorldFileCheckBox->setChecked( mComposition->generateWorldFile() );

    // populate the map list
    mWorldFileMapComboBox->setComposition( mComposition );
    mWorldFileMapComboBox->setItemType( QgsComposerItem::ComposerMap );
    mWorldFileMapComboBox->setItem( mComposition->worldFileMap() );

    mSnapToleranceSpinBox->setValue( mComposition->snapTolerance() );

    //snap grid
    mGridResolutionSpinBox->setValue( mComposition->snapGridResolution() );
    mOffsetXSpinBox->setValue( mComposition->snapGridOffsetX() );
    mOffsetYSpinBox->setValue( mComposition->snapGridOffsetY() );

    QgsAtlasComposition* atlas = &mComposition->atlasComposition();
    if ( atlas )
    {
      // repopulate data defined buttons if atlas layer changes
      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
               this, SLOT( populateDataDefinedButtons() ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( populateDataDefinedButtons() ) );
    }
  }

  connect( mTopMarginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( resizeMarginsChanged() ) );
  connect( mRightMarginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( resizeMarginsChanged() ) );
  connect( mBottomMarginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( resizeMarginsChanged() ) );
  connect( mLeftMarginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( resizeMarginsChanged() ) );

  connect( mPaperSizeDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperSizeDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperSizeDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mPaperSizeComboBox, SLOT( setDisabled( bool ) ) );
  connect( mPaperWidthDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperWidthDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperWidthDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mPaperWidthDoubleSpinBox, SLOT( setDisabled( bool ) ) );
  connect( mPaperHeightDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperHeightDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperHeightDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mPaperHeightDoubleSpinBox, SLOT( setDisabled( bool ) ) );
  connect( mNumPagesDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mNumPagesDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mNumPagesDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mNumPagesSpinBox, SLOT( setDisabled( bool ) ) );
  connect( mPaperOrientationDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperOrientationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mPaperOrientationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mPaperOrientationComboBox, SLOT( setDisabled( bool ) ) );

  connect( mWorldFileMapComboBox, SIGNAL( itemChanged( QgsComposerItem* ) ), this, SLOT( worldFileMapChanged( QgsComposerItem* ) ) );

  //initialize data defined buttons
  populateDataDefinedButtons();

  blockSignals( false );
}

QgsCompositionWidget::QgsCompositionWidget(): QWidget( nullptr ), mComposition( nullptr )
{
  setupUi( this );
}

QgsCompositionWidget::~QgsCompositionWidget()
{

}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsComposition* composition = ( const QgsComposition* ) context;
  if ( !composition )
  {
    return QgsExpressionContext();
  }

  QScopedPointer< QgsExpressionContext > expContext( composition->createExpressionContext() );
  return QgsExpressionContext( *expContext );
}

void QgsCompositionWidget::populateDataDefinedButtons()
{
  if ( !mComposition )
  {
    return;
  }

  QgsVectorLayer* vl = nullptr;
  QgsAtlasComposition* atlas = &mComposition->atlasComposition();

  if ( atlas && atlas->enabled() )
  {
    vl = atlas->coverageLayer();
  }

  Q_FOREACH ( QgsDataDefinedButton* button, findChildren< QgsDataDefinedButton* >() )
  {
    button->blockSignals( true );
    button->registerGetExpressionContextCallback( &_getExpressionContext, mComposition );
  }

  mPaperSizeDDBtn->init( vl, mComposition->dataDefinedProperty( QgsComposerObject::PresetPaperSize ),
                         QgsDataDefinedButton::String, QgsDataDefinedButton::paperSizeDesc() );
  mPaperWidthDDBtn->init( vl, mComposition->dataDefinedProperty( QgsComposerObject::PaperWidth ),
                          QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  mPaperHeightDDBtn->init( vl, mComposition->dataDefinedProperty( QgsComposerObject::PaperHeight ),
                           QgsDataDefinedButton::Double, QgsDataDefinedButton::doublePosDesc() );
  mNumPagesDDBtn->init( vl, mComposition->dataDefinedProperty( QgsComposerObject::NumPages ),
                        QgsDataDefinedButton::Int, QgsDataDefinedButton::intPosOneDesc() );
  mPaperOrientationDDBtn->init( vl, mComposition->dataDefinedProperty( QgsComposerObject::PaperOrientation ),
                                QgsDataDefinedButton::String, QgsDataDefinedButton::paperOrientationDesc() );

  //initial state of controls - disable related controls when dd buttons are active
  mPaperSizeComboBox->setEnabled( !mPaperSizeDDBtn->isActive() );

  Q_FOREACH ( QgsDataDefinedButton* button, findChildren< QgsDataDefinedButton* >() )
  {
    button->blockSignals( false );
  }
}

void QgsCompositionWidget::variablesChanged()
{
  QgsExpressionContextUtils::setCompositionVariables( mComposition, mVariableEditor->variablesInActiveScope() );
}

void QgsCompositionWidget::resizeMarginsChanged()
{
  if ( !mComposition )
    return;

  mComposition->setResizeToContentsMargins( mTopMarginSpinBox->value(),
      mRightMarginSpinBox->value(),
      mBottomMarginSpinBox->value(),
      mLeftMarginSpinBox->value() );
}

void QgsCompositionWidget::setDataDefinedProperty( const QgsDataDefinedButton* ddBtn, QgsComposerObject::DataDefinedProperty property )
{
  if ( !mComposition )
  {
    return;
  }

  const QMap< QString, QString >& map = ddBtn->definedProperty();
  mComposition->setDataDefinedProperty( property, map.value( "active" ).toInt(), map.value( "useexpr" ).toInt(), map.value( "expression" ), map.value( "field" ) );
}

QgsComposerObject::DataDefinedProperty QgsCompositionWidget::ddPropertyForWidget( QgsDataDefinedButton *widget )
{
  if ( widget == mPaperSizeDDBtn )
  {
    return QgsComposerObject::PresetPaperSize;
  }
  else if ( widget == mPaperWidthDDBtn )
  {
    return QgsComposerObject::PaperWidth;
  }
  else if ( widget == mPaperHeightDDBtn )
  {
    return QgsComposerObject::PaperHeight;
  }
  else if ( widget == mNumPagesDDBtn )
  {
    return QgsComposerObject::NumPages;
  }
  else if ( widget == mPaperOrientationDDBtn )
  {
    return QgsComposerObject::PaperOrientation;
  }

  return QgsComposerObject::NoProperty;
}

void QgsCompositionWidget::updateDataDefinedProperty()
{
  QgsDataDefinedButton* ddButton = dynamic_cast<QgsDataDefinedButton*>( sender() );
  if ( !ddButton || !mComposition )
  {
    return;
  }

  QgsComposerObject::DataDefinedProperty property = ddPropertyForWidget( ddButton );
  if ( property == QgsComposerObject::NoProperty )
  {
    return;
  }

  setDataDefinedProperty( ddButton, property );
  mComposition->refreshDataDefinedProperty( property );
}

void QgsCompositionWidget::createPaperEntries()
{
  QList<QgsCompositionPaper> formats;

  formats
  // ISO formats
  << QgsCompositionPaper( tr( "A5 (148x210 mm)" ), 148, 210 )
  << QgsCompositionPaper( tr( "A4 (210x297 mm)" ), 210, 297 )
  << QgsCompositionPaper( tr( "A3 (297x420 mm)" ), 297, 420 )
  << QgsCompositionPaper( tr( "A2 (420x594 mm)" ), 420, 594 )
  << QgsCompositionPaper( tr( "A1 (594x841 mm)" ), 594, 841 )
  << QgsCompositionPaper( tr( "A0 (841x1189 mm)" ), 841, 1189 )
  << QgsCompositionPaper( tr( "B5 (176 x 250 mm)" ), 176, 250 )
  << QgsCompositionPaper( tr( "B4 (250 x 353 mm)" ), 250, 353 )
  << QgsCompositionPaper( tr( "B3 (353 x 500 mm)" ), 353, 500 )
  << QgsCompositionPaper( tr( "B2 (500 x 707 mm)" ), 500, 707 )
  << QgsCompositionPaper( tr( "B1 (707 x 1000 mm)" ), 707, 1000 )
  << QgsCompositionPaper( tr( "B0 (1000 x 1414 mm)" ), 1000, 1414 )
  // North american formats
  << QgsCompositionPaper( tr( "Legal (8.5x14 in)" ), 215.9, 355.6 )
  << QgsCompositionPaper( tr( "ANSI A (Letter; 8.5x11 in)" ), 215.9, 279.4 )
  << QgsCompositionPaper( tr( "ANSI B (Tabloid; 11x17 in)" ), 279.4, 431.8 )
  << QgsCompositionPaper( tr( "ANSI C (17x22 in)" ), 431.8, 558.8 )
  << QgsCompositionPaper( tr( "ANSI D (22x34 in)" ), 558.8, 863.6 )
  << QgsCompositionPaper( tr( "ANSI E (34x44 in)" ), 863.6, 1117.6 )
  << QgsCompositionPaper( tr( "Arch A (9x12 in)" ), 228.6, 304.8 )
  << QgsCompositionPaper( tr( "Arch B (12x18 in)" ), 304.8, 457.2 )
  << QgsCompositionPaper( tr( "Arch C (18x24 in)" ), 457.2, 609.6 )
  << QgsCompositionPaper( tr( "Arch D (24x36 in)" ), 609.6, 914.4 )
  << QgsCompositionPaper( tr( "Arch E (36x48 in)" ), 914.4, 1219.2 )
  << QgsCompositionPaper( tr( "Arch E1 (30x42 in)" ), 762, 1066.8 )
  ;
  mPaperSizeComboBox->addItem( tr( "Custom" ) );

  for ( QList<QgsCompositionPaper>::const_iterator it = formats.begin(); it != formats.end(); ++it )
  {
    mPaperSizeComboBox->addItem( it->mName );
    mPaperMap.insert( it->mName, *it );
  }
}

void QgsCompositionWidget::on_mPaperSizeComboBox_currentIndexChanged( const QString& text )
{
  Q_UNUSED( text );

  if ( mPaperSizeComboBox->currentText() == tr( "Custom" ) )
  {
    mPaperWidthDoubleSpinBox->setEnabled( true );
    mPaperHeightDoubleSpinBox->setEnabled( true );
    mPaperUnitsComboBox->setEnabled( true );
  }
  else
  {
    mPaperWidthDoubleSpinBox->setEnabled( false );
    mPaperHeightDoubleSpinBox->setEnabled( false );
    mPaperUnitsComboBox->setEnabled( false );
  }
  applyCurrentPaperSettings();
}

void QgsCompositionWidget::on_mPaperOrientationComboBox_currentIndexChanged( const QString& text )
{
  Q_UNUSED( text );

  if ( mPaperSizeComboBox->currentText() == tr( "Custom" ) )
  {
    adjustOrientation();
    applyWidthHeight();
  }
  else
  {
    adjustOrientation();
    applyCurrentPaperSettings();
  }
}

void QgsCompositionWidget::on_mPaperUnitsComboBox_currentIndexChanged( const QString& text )
{
  Q_UNUSED( text );

  double width = size( mPaperWidthDoubleSpinBox );
  double height = size( mPaperHeightDoubleSpinBox );

  if ( mPaperUnitsComboBox->currentIndex() == 0 )
  {
    // mm, value was inch
    width *= 25.4;
    height *= 25.4;
  }
  else
  {
    // inch, values was mm,
    width /= 25.4;
    height /= 25.4;
  }

  setSize( mPaperWidthDoubleSpinBox, width );
  setSize( mPaperHeightDoubleSpinBox, height );

  if ( mPaperSizeComboBox->currentText() == tr( "Custom" ) )
  {
    adjustOrientation();
    applyWidthHeight();
  }
  else
  {
    adjustOrientation();
    applyCurrentPaperSettings();
  }
}

void QgsCompositionWidget::adjustOrientation()
{
  double width = size( mPaperWidthDoubleSpinBox );
  double height = size( mPaperHeightDoubleSpinBox );

  if ( width < 0 || height < 0 )
  {
    return;
  }

  if ( height > width ) //change values such that width > height
  {
    double tmp = width;
    width = height;
    height = tmp;
  }

  bool lineEditsEnabled = mPaperWidthDoubleSpinBox->isEnabled();

  mPaperWidthDoubleSpinBox->setEnabled( true );
  mPaperHeightDoubleSpinBox->setEnabled( true );
  if ( mPaperOrientationComboBox->currentText() == tr( "Landscape" ) )
  {
    setSize( mPaperWidthDoubleSpinBox, width );
    setSize( mPaperHeightDoubleSpinBox, height );
  }
  else
  {
    setSize( mPaperWidthDoubleSpinBox, height );
    setSize( mPaperHeightDoubleSpinBox, width );
  }

  mPaperWidthDoubleSpinBox->setEnabled( lineEditsEnabled );
  mPaperHeightDoubleSpinBox->setEnabled( lineEditsEnabled );

  emit pageOrientationChanged( mPaperOrientationComboBox->currentText() );
}

void QgsCompositionWidget::setSize( QDoubleSpinBox *spin, double v )
{
  if ( mPaperUnitsComboBox->currentIndex() == 0 )
  {
    // mm
    spin->setValue( v );
  }
  else
  {
    // inch (show width in inch)
    spin->setValue( v / 25.4 );
  }
}

double QgsCompositionWidget::size( QDoubleSpinBox *spin )
{
  double size = spin->value();

  if ( mPaperUnitsComboBox->currentIndex() == 0 )
  {
    // mm
    return size;
  }
  else
  {
    // inch return in mm
    return size * 25.4;
  }
}

void QgsCompositionWidget::applyCurrentPaperSettings()
{
  if ( mComposition )
  {
    //find entry in mPaper map to set width and height
    QMap<QString, QgsCompositionPaper>::const_iterator it = mPaperMap.constFind( mPaperSizeComboBox->currentText() );
    if ( it == mPaperMap.constEnd() )
    {
      return;
    }

    mPaperWidthDoubleSpinBox->setEnabled( true );
    mPaperHeightDoubleSpinBox->setEnabled( true );
    setSize( mPaperWidthDoubleSpinBox, it->mWidth );
    setSize( mPaperHeightDoubleSpinBox, it->mHeight );
    mPaperWidthDoubleSpinBox->setEnabled( false );
    mPaperHeightDoubleSpinBox->setEnabled( false );

    adjustOrientation();
    applyWidthHeight();
  }
}

void QgsCompositionWidget::applyWidthHeight()
{
  double width = size( mPaperWidthDoubleSpinBox );
  double height = size( mPaperHeightDoubleSpinBox );

  if ( width < 0 || height < 0 )
    return;

  mComposition->setPaperSize( width, height );
}

void QgsCompositionWidget::on_mPaperWidthDoubleSpinBox_editingFinished()
{
  applyWidthHeight();
}

void QgsCompositionWidget::on_mPaperHeightDoubleSpinBox_editingFinished()
{
  applyWidthHeight();
}

void QgsCompositionWidget::on_mNumPagesSpinBox_valueChanged( int value )
{
  if ( !mComposition )
  {
    return;
  }
  mComposition->setNumPages( value );
}

void QgsCompositionWidget::displayCompositionWidthHeight()
{
  if ( !mComposition )
  {
    return;
  }

  double paperWidth = mComposition->paperWidth();
  setSize( mPaperWidthDoubleSpinBox, paperWidth );

  double paperHeight = mComposition->paperHeight();
  setSize( mPaperHeightDoubleSpinBox, paperHeight );

  //set orientation
  mPaperOrientationComboBox->blockSignals( true );
  if ( paperWidth > paperHeight )
  {
    mPaperOrientationComboBox->setCurrentIndex( mPaperOrientationComboBox->findText( tr( "Landscape" ) ) );
  }
  else
  {
    mPaperOrientationComboBox->setCurrentIndex( mPaperOrientationComboBox->findText( tr( "Portrait" ) ) );
  }
  mPaperOrientationComboBox->blockSignals( false );

  //set paper name
  bool found = false;
  QMap<QString, QgsCompositionPaper>::const_iterator paper_it = mPaperMap.constBegin();
  for ( ; paper_it != mPaperMap.constEnd(); ++paper_it )
  {
    QgsCompositionPaper currentPaper = paper_it.value();

    //consider width and height values may be exchanged
    if (( qgsDoubleNear( currentPaper.mWidth, paperWidth ) && qgsDoubleNear( currentPaper.mHeight, paperHeight ) )
        || ( qgsDoubleNear( currentPaper.mWidth, paperHeight ) && qgsDoubleNear( currentPaper.mHeight, paperWidth ) ) )
    {
      mPaperSizeComboBox->setCurrentIndex( mPaperSizeComboBox->findText( paper_it.key() ) );
      found = true;
      break;
    }
  }

  if ( !found )
  {
    //custom
    mPaperSizeComboBox->setCurrentIndex( 0 );
  }
  else
  {
    mPaperWidthDoubleSpinBox->setEnabled( false );
    mPaperHeightDoubleSpinBox->setEnabled( false );
    mPaperUnitsComboBox->setEnabled( false );
  }
}

void QgsCompositionWidget::on_mPageStyleButton_clicked()
{
  if ( !mComposition )
  {
    return;
  }

  QgsVectorLayer* coverageLayer = nullptr;
  // use the atlas coverage layer, if any
  if ( mComposition->atlasComposition().enabled() )
  {
    coverageLayer = mComposition->atlasComposition().coverageLayer();
  }

  QgsFillSymbolV2* newSymbol = mComposition->pageStyleSymbol()->clone();
  if ( !newSymbol )
  {
    newSymbol = new QgsFillSymbolV2();
  }
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), coverageLayer, this );
  d.setExpressionContext( mComposition->createExpressionContext() );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposition->setPageStyleSymbol( newSymbol );
    updatePageStyle();
  }
  delete newSymbol;
}

void QgsCompositionWidget::on_mResizePageButton_clicked()
{
  if ( !mComposition )
  {
    return;
  }

  mComposition->resizePageToContents( mTopMarginSpinBox->value(),
                                      mRightMarginSpinBox->value(),
                                      mBottomMarginSpinBox->value(),
                                      mLeftMarginSpinBox->value() );
}

void QgsCompositionWidget::updatePageStyle()
{
  if ( mComposition )
  {
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mComposition->pageStyleSymbol(), mPageStyleButton->iconSize() );
    mPageStyleButton->setIcon( icon );
  }
}

void QgsCompositionWidget::setPrintAsRasterCheckBox( bool state )
{
  mPrintAsRasterCheckBox->blockSignals( true );
  mPrintAsRasterCheckBox->setChecked( state );
  mPrintAsRasterCheckBox->blockSignals( false );
}

void QgsCompositionWidget::setNumberPages()
{
  if ( !mComposition )
  {
    return;
  }

  mNumPagesSpinBox->blockSignals( true );
  mNumPagesSpinBox->setValue( mComposition->numPages() );
  mNumPagesSpinBox->blockSignals( false );
}

void QgsCompositionWidget::displaySnapingSettings()
{
  if ( !mComposition )
  {
    return;
  }

  mGridResolutionSpinBox->setValue( mComposition->snapGridResolution() );
  mOffsetXSpinBox->setValue( mComposition->snapGridOffsetX() );
  mOffsetYSpinBox->setValue( mComposition->snapGridOffsetY() );
}

void QgsCompositionWidget::on_mResolutionSpinBox_valueChanged( const int value )
{
  mComposition->setPrintResolution( value );
}

void QgsCompositionWidget::on_mPrintAsRasterCheckBox_toggled( bool state )
{
  if ( !mComposition )
  {
    return;
  }

  mComposition->setPrintAsRaster( state );
}

void QgsCompositionWidget::on_mGenerateWorldFileCheckBox_toggled( bool state )
{
  if ( !mComposition )
  {
    return;
  }

  mComposition->setGenerateWorldFile( state );
}

void QgsCompositionWidget::worldFileMapChanged( QgsComposerItem* item )
{
  if ( !mComposition )
  {
    return;
  }

  QgsComposerMap* map = dynamic_cast< QgsComposerMap* >( item );
  mComposition->setWorldFileMap( map );
}

void QgsCompositionWidget::on_mGridResolutionSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    mComposition->setSnapGridResolution( d );
  }
}

void QgsCompositionWidget::on_mOffsetXSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    mComposition->setSnapGridOffsetX( d );
  }
}

void QgsCompositionWidget::on_mOffsetYSpinBox_valueChanged( double d )
{
  if ( mComposition )
  {
    mComposition->setSnapGridOffsetY( d );
  }
}

void QgsCompositionWidget::on_mSnapToleranceSpinBox_valueChanged( int tolerance )
{
  if ( mComposition )
  {
    mComposition->setSnapTolerance( tolerance );
  }
}

void QgsCompositionWidget::blockSignals( bool block )
{
  mPaperSizeComboBox->blockSignals( block );
  mPaperUnitsComboBox->blockSignals( block );
  mPaperWidthDoubleSpinBox->blockSignals( block );
  mPaperHeightDoubleSpinBox->blockSignals( block );
  mNumPagesSpinBox->blockSignals( block );
  mPaperOrientationComboBox->blockSignals( block );
  mPageStyleButton->blockSignals( block );
  mResolutionSpinBox->blockSignals( block );
  mPrintAsRasterCheckBox->blockSignals( block );
  mGridResolutionSpinBox->blockSignals( block );
  mOffsetXSpinBox->blockSignals( block );
  mOffsetYSpinBox->blockSignals( block );
  mSnapToleranceSpinBox->blockSignals( block );
  mGenerateWorldFileCheckBox->blockSignals( block );
  mWorldFileMapComboBox->blockSignals( block );
}

