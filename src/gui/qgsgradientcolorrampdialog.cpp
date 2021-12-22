/***************************************************************************
    qgsgradientcolorrampdialog.cpp
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

#include "qgsgradientcolorrampdialog.h"

#include "qgscolorramp.h"
#include "qgsdialog.h"
#include "qgscolordialog.h"
#include "qgscptcityarchive.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QColorDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QPainter>
#include <QTableWidget>
#include <QTextEdit>

// QWT Charting widget
#include <qwt_global.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_layout.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_scale_div.h>
#include <qwt_scale_map.h>

QgsGradientColorRampDialog::QgsGradientColorRampDialog( const QgsGradientColorRamp &ramp, QWidget *parent )
  : QDialog( parent )
  , mRamp( ramp )
  , mCurrentPlotColorComponent( -1 )
  , mCurrentPlotMarkerIndex( 0 )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mStopColorSpec->addItem( tr( "RGB" ), static_cast< int >( QColor::Spec::Rgb ) );
  mStopColorSpec->addItem( tr( "HSV" ), static_cast< int >( QColor::Spec::Hsv ) );
  mStopColorSpec->addItem( tr( "HSL" ), static_cast< int >( QColor::Spec::Hsl ) );
  mStopColorSpec->setCurrentIndex( mStopColorSpec->findData( static_cast< int >( ramp.colorSpec() ) ) );

  mStopDirection->addItem( tr( "Clockwise" ), static_cast< int >( Qgis::AngularDirection::Clockwise ) );
  mStopDirection->addItem( tr( "Counterclockwise" ), static_cast< int >( Qgis::AngularDirection::CounterClockwise ) );
  mStopDirection->setCurrentIndex( mStopColorSpec->findData( static_cast< int >( ramp.direction() ) ) );

  mStopDirection->setEnabled( static_cast< QColor::Spec>( mStopColorSpec->currentData().toInt() ) != QColor::Spec::Rgb );

  connect( mStopColorSpec, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    mStopDirection->setEnabled( static_cast< QColor::Spec>( mStopColorSpec->currentData().toInt() ) != QColor::Spec::Rgb );

    if ( mBlockChanges )
      return;
    mStopEditor->setSelectedStopColorSpec( static_cast< QColor::Spec>( mStopColorSpec->currentData().toInt() ) );
  } );

  connect( mStopDirection, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( mBlockChanges )
      return;

    mStopEditor->setSelectedStopDirection( static_cast< Qgis::AngularDirection >( mStopDirection->currentData().toInt() ) );
  } );

  connect( cboType, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGradientColorRampDialog::cboType_currentIndexChanged );
  connect( btnInformation, &QPushButton::pressed, this, &QgsGradientColorRampDialog::btnInformation_pressed );
  connect( mPositionSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsGradientColorRampDialog::mPositionSpinBox_valueChanged );
  connect( mPlotHueCheckbox, &QCheckBox::toggled, this, &QgsGradientColorRampDialog::mPlotHueCheckbox_toggled );
  connect( mPlotLightnessCheckbox, &QCheckBox::toggled, this, &QgsGradientColorRampDialog::mPlotLightnessCheckbox_toggled );
  connect( mPlotSaturationCheckbox, &QCheckBox::toggled, this, &QgsGradientColorRampDialog::mPlotSaturationCheckbox_toggled );
  connect( mPlotAlphaCheckbox, &QCheckBox::toggled, this, &QgsGradientColorRampDialog::mPlotAlphaCheckbox_toggled );
#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  mPositionSpinBox->setShowClearButton( false );
  btnColor1->setAllowOpacity( true );
  btnColor1->setColorDialogTitle( tr( "Select Ramp Color" ) );
  btnColor1->setContext( QStringLiteral( "symbology" ) );
  btnColor1->setShowNoColor( true );
  btnColor1->setNoColorString( tr( "Transparent" ) );
  btnColor2->setAllowOpacity( true );
  btnColor2->setColorDialogTitle( tr( "Select Ramp Color" ) );
  btnColor2->setContext( QStringLiteral( "symbology" ) );
  btnColor2->setShowNoColor( true );
  btnColor2->setNoColorString( tr( "Transparent" ) );
  updateColorButtons();
  connect( btnColor1, &QgsColorButton::colorChanged, this, &QgsGradientColorRampDialog::setColor1 );
  connect( btnColor2, &QgsColorButton::colorChanged, this, &QgsGradientColorRampDialog::setColor2 );

  // fill type combobox
  cboType->blockSignals( true );
  cboType->addItem( tr( "Discrete" ) );
  cboType->addItem( tr( "Continuous" ) );
  if ( mRamp.isDiscrete() )
    cboType->setCurrentIndex( 0 );
  else
    cboType->setCurrentIndex( 1 );
  cboType->blockSignals( false );

  if ( mRamp.info().isEmpty() )
    btnInformation->setEnabled( false );

  mStopEditor->setGradientRamp( mRamp );
  connect( mStopEditor, &QgsGradientStopEditor::changed, this, &QgsGradientColorRampDialog::updateRampFromStopEditor );

  connect( mColorWidget, &QgsCompoundColorWidget::currentColorChanged, this, &QgsGradientColorRampDialog::colorWidgetChanged );
  connect( mDeleteStopButton, &QAbstractButton::clicked, mStopEditor, &QgsGradientStopEditor::deleteSelectedStop );

  // hide the ugly canvas frame
  mPlot->setFrameStyle( QFrame::NoFrame );
  QFrame *plotCanvasFrame = dynamic_cast<QFrame *>( mPlot->canvas() );
  if ( plotCanvasFrame )
    plotCanvasFrame->setFrameStyle( QFrame::NoFrame );

  mPlot->setAxisScale( QwtPlot::yLeft, 0.0, 1.0 );
  mPlot->enableAxis( QwtPlot::yLeft, false );

  // add a grid
  QwtPlotGrid *grid = new QwtPlotGrid();
  QwtScaleDiv gridDiv( 0.0, 1.0, QList<double>(), QList<double>(), QList<double>() << 0.2 << 0.4 << 0.6 << 0.8 );
  grid->setXDiv( gridDiv );
  grid->setYDiv( gridDiv );
  grid->setPen( QPen( QColor( 0, 0, 0, 50 ) ) );
  grid->attach( mPlot );

  mLightnessCurve = new QwtPlotCurve();
  mLightnessCurve->setTitle( tr( "Lightness" ) );
  mLightnessCurve->setPen( QPen( QColor( 70, 150, 255 ), 0.0 ) ),
                  mLightnessCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mLightnessCurve->attach( mPlot );

  mHueCurve = new QwtPlotCurve();
  mHueCurve->setTitle( tr( "Hue" ) );
  mHueCurve->setPen( QPen( QColor( 255, 215, 70 ), 0.0 ) ),
            mHueCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mHueCurve->attach( mPlot );

  mSaturationCurve = new QwtPlotCurve();
  mSaturationCurve->setTitle( tr( "Saturation" ) );
  mSaturationCurve->setPen( QPen( QColor( 255, 70, 150 ), 0.0 ) ),
                   mSaturationCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mSaturationCurve->attach( mPlot );

  mAlphaCurve = new QwtPlotCurve();
  mAlphaCurve->setTitle( tr( "Opacity" ) );
  mAlphaCurve->setPen( QPen( QColor( 50, 50, 50 ), 0.0 ) ),
              mAlphaCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mAlphaCurve->attach( mPlot );

  mPlotFilter = new QgsGradientPlotEventFilter( mPlot );
  connect( mPlotFilter, &QgsGradientPlotEventFilter::mousePress, this, &QgsGradientColorRampDialog::plotMousePress );
  connect( mPlotFilter, &QgsGradientPlotEventFilter::mouseRelease, this, &QgsGradientColorRampDialog::plotMouseRelease );
  connect( mPlotFilter, &QgsGradientPlotEventFilter::mouseMove, this, &QgsGradientColorRampDialog::plotMouseMove );

  QgsSettings settings;
  mPlotHueCheckbox->setChecked( settings.value( QStringLiteral( "GradientEditor/plotHue" ), false ).toBool() );
  mPlotLightnessCheckbox->setChecked( settings.value( QStringLiteral( "GradientEditor/plotLightness" ), true ).toBool() );
  mPlotSaturationCheckbox->setChecked( settings.value( QStringLiteral( "GradientEditor/plotSaturation" ), false ).toBool() );
  mPlotAlphaCheckbox->setChecked( settings.value( QStringLiteral( "GradientEditor/plotAlpha" ), false ).toBool() );

  mHueCurve->setVisible( mPlotHueCheckbox->isChecked() );
  mLightnessCurve->setVisible( mPlotLightnessCheckbox->isChecked() );
  mSaturationCurve->setVisible( mPlotSaturationCheckbox->isChecked() );
  mAlphaCurve->setVisible( mPlotAlphaCheckbox->isChecked() );

  connect( mStopEditor, &QgsGradientStopEditor::selectedStopChanged, this, &QgsGradientColorRampDialog::selectedStopChanged );
  mStopEditor->selectStop( 0 );

  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsGradientColorRampDialog::showHelp );
}

QgsGradientColorRampDialog::~QgsGradientColorRampDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "GradientEditor/plotHue" ), mPlotHueCheckbox->isChecked() );
  settings.setValue( QStringLiteral( "GradientEditor/plotLightness" ), mPlotLightnessCheckbox->isChecked() );
  settings.setValue( QStringLiteral( "GradientEditor/plotSaturation" ), mPlotSaturationCheckbox->isChecked() );
  settings.setValue( QStringLiteral( "GradientEditor/plotAlpha" ), mPlotAlphaCheckbox->isChecked() );

}

void QgsGradientColorRampDialog::setRamp( const QgsGradientColorRamp &ramp )
{
  mRamp = ramp;

  updateColorButtons();
  updateStopEditor();
  updatePlot();

  emit changed();
}

QDialogButtonBox *QgsGradientColorRampDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsGradientColorRampDialog::cboType_currentIndexChanged( int index )
{
  if ( ( index == 0 && mRamp.isDiscrete() ) ||
       ( index == 1 && !mRamp.isDiscrete() ) )
    return;
  mRamp.convertToDiscrete( index == 0 );
  updateColorButtons();
  updateStopEditor();
  updatePlot();

  emit changed();
}

void QgsGradientColorRampDialog::btnInformation_pressed()
{
  if ( mRamp.info().isEmpty() )
    return;

  QgsDialog *dlg = new QgsDialog( this );
  QLabel *label = nullptr;

  // information table
  QTableWidget *tableInfo = new QTableWidget( dlg );
  tableInfo->verticalHeader()->hide();
  tableInfo->horizontalHeader()->hide();
  tableInfo->setRowCount( mRamp.info().count() );
  tableInfo->setColumnCount( 2 );
  int i = 0;
  QgsStringMap rampInfo = mRamp.info();
  for ( QgsStringMap::const_iterator it = rampInfo.constBegin();
        it != rampInfo.constEnd(); ++it )
  {
    if ( it.key().startsWith( QLatin1String( "cpt-city" ) ) )
      continue;
    tableInfo->setItem( i, 0, new QTableWidgetItem( it.key() ) );
    tableInfo->setItem( i, 1, new QTableWidgetItem( it.value() ) );
    tableInfo->resizeRowToContents( i );
    i++;
  }
  tableInfo->resizeColumnToContents( 0 );
  tableInfo->horizontalHeader()->setStretchLastSection( true );
  tableInfo->setRowCount( i );
  tableInfo->setFixedHeight( tableInfo->rowHeight( 0 ) * i + 5 );
  dlg->layout()->addWidget( tableInfo );
  dlg->resize( 600, 250 );

  dlg->layout()->addSpacing( 5 );

  // gradient file
  QString gradientFile = mRamp.info().value( QStringLiteral( "cpt-city-gradient" ) );
  if ( ! gradientFile.isNull() )
  {
    QString fileName = gradientFile;
    fileName.replace( QLatin1String( "<cpt-city>" ), QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = gradientFile;
      fileName.replace( QLatin1String( "<cpt-city>" ), QLatin1String( "http://soliton.vm.bytemark.co.uk/pub/cpt-city" ) );
    }
    label = new QLabel( tr( "Gradient file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
  }

  // license file
  QString licenseFile = mRamp.info().value( QStringLiteral( "cpt-city-license" ) );
  if ( !licenseFile.isNull() )
  {
    QString fileName = licenseFile;
    fileName.replace( QLatin1String( "<cpt-city>" ), QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = licenseFile;
      fileName.replace( QLatin1String( "<cpt-city>" ), QLatin1String( "http://soliton.vm.bytemark.co.uk/pub/cpt-city" ) );
    }
    label = new QLabel( tr( "License file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
    if ( QFile::exists( fileName ) )
    {
      QTextEdit *textEdit = new QTextEdit( dlg );
      textEdit->setReadOnly( true );
      QFile file( fileName );
      if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        textEdit->setText( file.readAll() );
        file.close();
        dlg->layout()->addSpacing( 5 );
        dlg->layout()->addWidget( textEdit );
        dlg->resize( 600, 500 );
      }
    }
  }

  dlg->show(); //non modal
}

void QgsGradientColorRampDialog::updateColorButtons()
{
  btnColor1->blockSignals( true );
  btnColor1->setColor( mRamp.color1() );
  btnColor1->blockSignals( false );
  btnColor2->blockSignals( true );
  btnColor2->setColor( mRamp.color2() );
  btnColor2->blockSignals( false );
}

void QgsGradientColorRampDialog::updateStopEditor()
{
  mStopEditor->blockSignals( true );
  mStopEditor->setGradientRamp( mRamp );
  mStopEditor->blockSignals( false );
}

void QgsGradientColorRampDialog::selectedStopChanged( const QgsGradientStop &stop )
{
  mBlockChanges++;
  mColorWidget->blockSignals( true );
  mColorWidget->setColor( stop.color );
  mColorWidget->blockSignals( false );
  mPositionSpinBox->blockSignals( true );
  mPositionSpinBox->setValue( stop.offset * 100 );
  mPositionSpinBox->blockSignals( false );

  mStopColorSpec->setCurrentIndex( mStopColorSpec->findData( static_cast< int >( mStopEditor->selectedStop().colorSpec() ) ) );
  mStopDirection->setCurrentIndex( mStopDirection->findData( static_cast< int >( mStopEditor->selectedStop().direction() ) ) );
  mBlockChanges--;

  if ( ( stop.offset == 0 && stop.color == mRamp.color1() ) || ( stop.offset == 1.0 && stop.color == mRamp.color2() ) )
  {
    //first/last stop can't be repositioned
    mPositionSpinBox->setDisabled( true );
    mDeleteStopButton->setDisabled( true );
  }
  else
  {
    mPositionSpinBox->setDisabled( false );
    mDeleteStopButton->setDisabled( false );
  }

  // first stop cannot have color spec or direction set
  mStopColorSpec->setEnabled( !( stop.offset == 0 && stop.color == mRamp.color1() ) );
  mStopDirection->setEnabled( !( stop.offset == 0 && stop.color == mRamp.color1() ) && mStopEditor->selectedStop().colorSpec() != QColor::Rgb );

  updatePlot();
}

void QgsGradientColorRampDialog::colorWidgetChanged( const QColor &color )
{
  mStopEditor->setSelectedStopColor( color );
}

void QgsGradientColorRampDialog::mPositionSpinBox_valueChanged( double val )
{
  mStopEditor->setSelectedStopOffset( val / 100.0 );
}

void QgsGradientColorRampDialog::mPlotHueCheckbox_toggled( bool checked )
{
  mHueCurve->setVisible( checked );
  updatePlot();
}

void QgsGradientColorRampDialog::mPlotLightnessCheckbox_toggled( bool checked )
{
  mLightnessCurve->setVisible( checked );
  updatePlot();
}

void QgsGradientColorRampDialog::mPlotSaturationCheckbox_toggled( bool checked )
{
  mSaturationCurve->setVisible( checked );
  updatePlot();
}

void QgsGradientColorRampDialog::mPlotAlphaCheckbox_toggled( bool checked )
{
  mAlphaCurve->setVisible( checked );
  updatePlot();
}

void QgsGradientColorRampDialog::plotMousePress( QPointF point )
{
  //find closest part

  double minDist = 1;
  mCurrentPlotColorComponent = -1;
  mCurrentPlotMarkerIndex = -1;
  // first color

  for ( int i = 0; i < mRamp.count(); ++i )
  {
    QColor currentCol;
    double currentOff = 0.0;
    if ( i == 0 )
    {
      currentOff = 0.0;
      currentCol = mRamp.color1();
    }
    else if ( i == mRamp.count() - 1 )
    {
      currentOff = 1.0;
      currentCol = mRamp.color2();
    }
    else
    {
      currentOff = mRamp.stops().at( i - 1 ).offset;
      currentCol = mRamp.stops().at( i - 1 ).color;
    }

    double currentDist;
    if ( mPlotHueCheckbox->isChecked() )
    {
      currentDist = std::pow( point.x() - currentOff, 2.0 ) + std::pow( point.y() - currentCol.hslHueF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 0;
        mCurrentPlotMarkerIndex = i;
      }
    }
    if ( mPlotLightnessCheckbox->isChecked() )
    {
      currentDist = std::pow( point.x() - currentOff, 2.0 ) + std::pow( point.y() - currentCol.lightnessF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 1;
        mCurrentPlotMarkerIndex = i;
      }
    }
    if ( mPlotSaturationCheckbox->isChecked() )
    {
      currentDist = std::pow( point.x() - currentOff, 2.0 ) + std::pow( point.y() - currentCol.hslSaturationF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 2;
        mCurrentPlotMarkerIndex = i;
      }
    }
    if ( mPlotAlphaCheckbox->isChecked() )
    {
      currentDist = std::pow( point.x() - currentOff, 2.0 ) + std::pow( point.y() - currentCol.alphaF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 3;
        mCurrentPlotMarkerIndex = i;
      }
    }
  }

  // watch out - selected stop index may differ if stops in editor are out of order!!!
  if ( mCurrentPlotMarkerIndex >= 0 )
    mStopEditor->selectStop( mCurrentPlotMarkerIndex );
}

void QgsGradientColorRampDialog::plotMouseRelease( QPointF )
{
  mCurrentPlotColorComponent = -1;
}

void QgsGradientColorRampDialog::plotMouseMove( QPointF point )
{
  QColor newColor = mStopEditor->selectedStop().color;

  if ( mCurrentPlotColorComponent == 0 )
    newColor = QColor::fromHslF( std::clamp( point.y(), qreal( 0.0 ), qreal( 1.0 ) ), newColor.hslSaturationF(), newColor.lightnessF(), newColor.alphaF() );
  else if ( mCurrentPlotColorComponent == 1 )
    newColor = QColor::fromHslF( newColor.hslHueF(), newColor.hslSaturationF(), std::clamp( point.y(), qreal( 0.0 ), qreal( 1.0 ) ), newColor.alphaF() );
  else if ( mCurrentPlotColorComponent == 2 )
    newColor = QColor::fromHslF( newColor.hslHueF(), std::clamp( point.y(), qreal( 0.0 ), qreal( 1.0 ) ), newColor.lightnessF(), newColor.alphaF() );
  else if ( mCurrentPlotColorComponent == 3 )
    newColor = QColor::fromHslF( newColor.hslHueF(), newColor.hslSaturationF(), newColor.lightnessF(), std::clamp( point.y(), qreal( 0.0 ), qreal( 1.0 ) ) );

  mStopEditor->setSelectedStopDetails( newColor, std::clamp( point.x(), qreal( 0.0 ), qreal( 1.0 ) ) );
}

bool byX( QPointF p1, QPointF p2 )
{
  return p1.x() < p2.x();
}

void QgsGradientColorRampDialog::addPlotMarker( double x, double y, const QColor &color, bool isSelected )
{
  QColor borderColor = color.darker( 200 );
  borderColor.setAlpha( 255 );

  QColor brushColor = color;
  brushColor.setAlpha( 255 );

  QwtPlotMarker *marker = new QwtPlotMarker();
  marker->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,  QBrush( brushColor ), QPen( borderColor, isSelected ? 2 : 1 ), QSize( 8, 8 ) ) );
  marker->setValue( x, y );
  marker->attach( mPlot );
  marker->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mMarkers << marker;
}

void QgsGradientColorRampDialog::addMarkersForColor( double x, const QColor &color, bool isSelected )
{
  if ( mPlotHueCheckbox->isChecked() )
    addPlotMarker( x, color.hslHueF(), color, isSelected && mCurrentPlotColorComponent == 0 );
  if ( mPlotLightnessCheckbox->isChecked() )
    addPlotMarker( x, color.lightnessF(), color, isSelected && mCurrentPlotColorComponent == 1 );
  if ( mPlotSaturationCheckbox->isChecked() )
    addPlotMarker( x, color.hslSaturationF(), color, isSelected && mCurrentPlotColorComponent == 2 );
  if ( mPlotAlphaCheckbox->isChecked() )
    addPlotMarker( x, color.alphaF(), color, isSelected && mCurrentPlotColorComponent == 3 );
}

void QgsGradientColorRampDialog::updatePlot()
{
  // remove existing markers
  const auto constMMarkers = mMarkers;
  for ( QwtPlotMarker *marker : constMMarkers )
  {
    marker->detach();
    delete marker;
  }
  mMarkers.clear();

  QPolygonF lightnessPoints;
  QPolygonF huePoints;
  QPolygonF saturationPoints;
  QPolygonF alphaPoints;
  lightnessPoints << QPointF( 0.0, mRamp.color1().lightnessF() );
  huePoints << QPointF( 0.0, mRamp.color1().hslHueF() );
  saturationPoints << QPointF( 0.0, mRamp.color1().hslSaturationF() );
  alphaPoints << QPointF( 0.0, mRamp.color1().alphaF() );
  addMarkersForColor( 0, mRamp.color1(), mCurrentPlotMarkerIndex == 0 );

  int i = 1;
  const auto constStops = mRamp.stops();
  for ( const QgsGradientStop &stop : constStops )
  {
    lightnessPoints << QPointF( stop.offset, stop.color.lightnessF() );
    huePoints << QPointF( stop.offset, stop.color.hslHueF() );
    saturationPoints << QPointF( stop.offset, stop.color.hslSaturationF() );
    alphaPoints << QPointF( stop.offset, stop.color.alphaF() );

    addMarkersForColor( stop.offset, stop.color, mCurrentPlotMarkerIndex == i );
    i++;
  }

  //add extra intermediate points
  for ( double p = 0.001; p < 1.0; p += 0.001 )
  {
    QColor c = mRamp.color( p );
    lightnessPoints << QPointF( p, c.lightnessF() );
    huePoints << QPointF( p, c.hslHueF() );
    saturationPoints << QPointF( p, c.hslSaturationF() );
    alphaPoints << QPointF( p, c.alphaF() );
  }

  lightnessPoints << QPointF( 1.0, mRamp.color2().lightnessF() );
  huePoints << QPointF( 1.0, mRamp.color2().hslHueF() );
  saturationPoints << QPointF( 1.0, mRamp.color2().hslSaturationF() );
  alphaPoints << QPointF( 1.0, mRamp.color2().alphaF() );
  addMarkersForColor( 1.0, mRamp.color2(), mCurrentPlotMarkerIndex == i );

  std::sort( lightnessPoints.begin(), lightnessPoints.end(), byX );
  std::sort( huePoints.begin(), huePoints.end(), byX );
  std::sort( saturationPoints.begin(), saturationPoints.end(), byX );
  std::sort( alphaPoints.begin(), alphaPoints.end(), byX );

  mLightnessCurve->setSamples( lightnessPoints );
  mHueCurve->setSamples( huePoints );
  mSaturationCurve->setSamples( saturationPoints );
  mAlphaCurve->setSamples( alphaPoints );
  mPlot->replot();
}

void QgsGradientColorRampDialog::updateRampFromStopEditor()
{
  mRamp = mStopEditor->gradientRamp();

  mBlockChanges++;
  mPositionSpinBox->blockSignals( true );
  mPositionSpinBox->setValue( mStopEditor->selectedStop().offset * 100 );
  mPositionSpinBox->blockSignals( false );
  mColorWidget->blockSignals( true );
  mColorWidget->setColor( mStopEditor->selectedStop().color );
  mColorWidget->blockSignals( false );

  mStopColorSpec->setCurrentIndex( mStopColorSpec->findData( static_cast< int >( mStopEditor->selectedStop().colorSpec() ) ) );
  mStopDirection->setCurrentIndex( mStopDirection->findData( static_cast< int >( mStopEditor->selectedStop().direction() ) ) );
  mBlockChanges--;

  // first stop cannot have color spec or direction set
  mStopColorSpec->setEnabled( !( mStopEditor->selectedStop().offset == 0 && mStopEditor->selectedStop().color == mRamp.color1() ) );
  mStopDirection->setEnabled( !( mStopEditor->selectedStop().offset == 0 && mStopEditor->selectedStop().color == mRamp.color1() ) && mStopEditor->selectedStop().colorSpec() != QColor::Rgb );

  updateColorButtons();
  updatePlot();

  emit changed();
}

void QgsGradientColorRampDialog::setColor1( const QColor &color )
{
  mStopEditor->setColor1( color );
  updateColorButtons();
}

void QgsGradientColorRampDialog::setColor2( const QColor &color )
{
  mStopEditor->setColor2( color );
  updateColorButtons();
}

void QgsGradientColorRampDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/style_manager.html#setting-a-color-ramp" ) );
}


/// @cond PRIVATE

QgsGradientPlotEventFilter::QgsGradientPlotEventFilter( QwtPlot *plot )
  : QObject( plot )
  , mPlot( plot )
{
  mPlot->canvas()->installEventFilter( this );
}

bool QgsGradientPlotEventFilter::eventFilter( QObject *object, QEvent *event )
{
  if ( !mPlot->isEnabled() )
    return QObject::eventFilter( object, event );

  switch ( event->type() )
  {
    case QEvent::MouseButtonPress:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mousePress( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseMove:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->buttons() & Qt::LeftButton )
      {
        // only emit when button pressed
        emit mouseMove( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      const QMouseEvent *mouseEvent = static_cast<QMouseEvent * >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mouseRelease( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    default:
      break;
  }

  return QObject::eventFilter( object, event );
}

QPointF QgsGradientPlotEventFilter::mapPoint( QPointF point ) const
{
  if ( !mPlot )
    return QPointF();

  return QPointF( mPlot->canvasMap( QwtPlot::xBottom ).invTransform( point.x() ),
                  mPlot->canvasMap( QwtPlot::yLeft ).invTransform( point.y() ) );
}

///@endcond
