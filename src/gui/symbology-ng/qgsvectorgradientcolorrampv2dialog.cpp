/***************************************************************************
    qgsvectorgradientcolorrampv2dialog.cpp
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

#include "qgsvectorgradientcolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"
#include "qgsdialog.h"
#include "qgscolordialog.h"
#include "qgscptcityarchive.h"

#include <QColorDialog>
#include <QInputDialog>
#include <QPainter>
#include <QSettings>
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

QgsVectorGradientColorRampV2Dialog::QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent )
    : QDialog( parent )
    , mRamp( ramp )
    , mCurrentPlotColorComponent( -1 )
    , mCurrentPlotMarkerIndex( 0 )
{
  setupUi( this );
#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  mPositionSpinBox->setShowClearButton( false );
  btnColor1->setAllowAlpha( true );
  btnColor1->setColorDialogTitle( tr( "Select ramp color" ) );
  btnColor1->setContext( "symbology" );
  btnColor1->setShowNoColor( true );
  btnColor1->setNoColorString( tr( "Transparent" ) );
  btnColor2->setAllowAlpha( true );
  btnColor2->setColorDialogTitle( tr( "Select ramp color" ) );
  btnColor2->setContext( "symbology" );
  btnColor2->setShowNoColor( true );
  btnColor2->setNoColorString( tr( "Transparent" ) );
  updateColorButtons();
  connect( btnColor1, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor1( const QColor& ) ) );
  connect( btnColor2, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( setColor2( const QColor& ) ) );

  // fill type combobox
  cboType->blockSignals( true );
  cboType->addItem( tr( "Discrete" ) );
  cboType->addItem( tr( "Continuous" ) );
  if ( mRamp->isDiscrete() )
    cboType->setCurrentIndex( 0 );
  else
    cboType->setCurrentIndex( 1 );
  cboType->blockSignals( false );

  if ( mRamp->info().isEmpty() )
    btnInformation->setEnabled( false );

  mStopEditor->setGradientRamp( *mRamp );
  connect( mStopEditor, SIGNAL( changed() ), this, SLOT( updateRampFromStopEditor() ) );

  connect( mColorWidget, SIGNAL( currentColorChanged( QColor ) ), this, SLOT( colorWidgetChanged( QColor ) ) );
  connect( mDeleteStopButton, SIGNAL( clicked() ), mStopEditor, SLOT( deleteSelectedStop() ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/GradientEditor/geometry" ).toByteArray() );

  // hide the ugly canvas frame
  mPlot->setFrameStyle( QFrame::NoFrame );
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  QFrame* plotCanvasFrame = dynamic_cast<QFrame*>( mPlot->canvas() );
  if ( plotCanvasFrame )
    plotCanvasFrame->setFrameStyle( QFrame::NoFrame );
#else
  mPlot->canvas()->setFrameStyle( QFrame::NoFrame );
#endif

  mPlot->setAxisScale( QwtPlot::yLeft, 0.0, 1.0 );
  mPlot->enableAxis( QwtPlot::yLeft, false );

  mLightnessCurve = new QwtPlotCurve();
  mLightnessCurve->setTitle( "Lightness" );
  mLightnessCurve->setPen( QPen( QColor( 70, 150, 255 ), 0.0 ) ),
  mLightnessCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mLightnessCurve->attach( mPlot );

  mHueCurve = new QwtPlotCurve();
  mHueCurve->setTitle( "Hue" );
  mHueCurve->setPen( QPen( QColor( 255, 215, 70 ), 0.0 ) ),
  mHueCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mHueCurve->attach( mPlot );

  mSaturationCurve = new QwtPlotCurve();
  mSaturationCurve->setTitle( "Saturation" );
  mSaturationCurve->setPen( QPen( QColor( 255, 70, 150 ), 0.0 ) ),
  mSaturationCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mSaturationCurve->attach( mPlot );

  mAlphaCurve = new QwtPlotCurve();
  mAlphaCurve->setTitle( "Alpha" );
  mAlphaCurve->setPen( QPen( QColor( 50, 50, 50 ), 0.0 ) ),
  mAlphaCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mAlphaCurve->attach( mPlot );

  mPlotFilter = new QgsGradientPlotEventFilter( mPlot );
  connect( mPlotFilter, SIGNAL( mousePress( QPointF ) ), this, SLOT( plotMousePress( QPointF ) ) );
  connect( mPlotFilter, SIGNAL( mouseRelease( QPointF ) ), this, SLOT( plotMouseRelease( QPointF ) ) );
  connect( mPlotFilter, SIGNAL( mouseMove( QPointF ) ), this, SLOT( plotMouseMove( QPointF ) ) );

  mPlotHueCheckbox->setChecked( settings.value( "/GradientEditor/plotHue", false ).toBool() );
  mPlotLightnessCheckbox->setChecked( settings.value( "/GradientEditor/plotLightness", true ).toBool() );
  mPlotSaturationCheckbox->setChecked( settings.value( "/GradientEditor/plotSaturation", false ).toBool() );
  mPlotAlphaCheckbox->setChecked( settings.value( "/GradientEditor/plotAlpha", false ).toBool() );

  mHueCurve->setVisible( mPlotHueCheckbox->isChecked() );
  mLightnessCurve->setVisible( mPlotLightnessCheckbox->isChecked() );
  mSaturationCurve->setVisible( mPlotSaturationCheckbox->isChecked() );
  mAlphaCurve->setVisible( mPlotAlphaCheckbox->isChecked() );

  connect( mStopEditor, SIGNAL( selectedStopChanged( QgsGradientStop ) ), this, SLOT( selectedStopChanged( QgsGradientStop ) ) );
  mStopEditor->selectStop( 0 );
}

QgsVectorGradientColorRampV2Dialog::~QgsVectorGradientColorRampV2Dialog()
{
  QSettings settings;
  settings.setValue( "/Windows/GradientEditor/geometry", saveGeometry() );
  settings.setValue( "/GradientEditor/plotHue", mPlotHueCheckbox->isChecked() );
  settings.setValue( "/GradientEditor/plotLightness", mPlotLightnessCheckbox->isChecked() );
  settings.setValue( "/GradientEditor/plotSaturation", mPlotSaturationCheckbox->isChecked() );
  settings.setValue( "/GradientEditor/plotAlpha", mPlotAlphaCheckbox->isChecked() );

}

void QgsVectorGradientColorRampV2Dialog::on_cboType_currentIndexChanged( int index )
{
  if (( index == 0 && mRamp->isDiscrete() ) ||
      ( index == 1 && !mRamp->isDiscrete() ) )
    return;
  mRamp->convertToDiscrete( index == 0 );
  updateColorButtons();
  updateStopEditor();
  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::on_btnInformation_pressed()
{
  if ( mRamp->info().isEmpty() )
    return;

  QgsDialog *dlg = new QgsDialog( this );
  QLabel *label = nullptr;

  // information table
  QTableWidget *tableInfo = new QTableWidget( dlg );
  tableInfo->verticalHeader()->hide();
  tableInfo->horizontalHeader()->hide();
  tableInfo->setRowCount( mRamp->info().count() );
  tableInfo->setColumnCount( 2 );
  int i = 0;
  QgsStringMap rampInfo = mRamp->info();
  for ( QgsStringMap::const_iterator it = rampInfo.constBegin();
        it != rampInfo.constEnd(); ++it )
  {
    if ( it.key().startsWith( "cpt-city" ) )
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
  QString gradientFile = mRamp->info().value( "cpt-city-gradient" );
  if ( ! gradientFile.isNull() )
  {
    QString fileName = gradientFile;
    fileName.replace( "<cpt-city>", QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = gradientFile;
      fileName.replace( "<cpt-city>", "http://soliton.vm.bytemark.co.uk/pub/cpt-city" );
    }
    label = new QLabel( tr( "Gradient file : %1" ).arg( fileName ), dlg );
    label->setTextInteractionFlags( Qt::TextBrowserInteraction );
    dlg->layout()->addSpacing( 5 );
    dlg->layout()->addWidget( label );
  }

  // license file
  QString licenseFile = mRamp->info().value( "cpt-city-license" );
  if ( !licenseFile.isNull() )
  {
    QString fileName = licenseFile;
    fileName.replace( "<cpt-city>", QgsCptCityArchive::defaultBaseDir() );
    if ( ! QFile::exists( fileName ) )
    {
      fileName = licenseFile;
      fileName.replace( "<cpt-city>", "http://soliton.vm.bytemark.co.uk/pub/cpt-city" );
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

void QgsVectorGradientColorRampV2Dialog::updateColorButtons()
{
  btnColor1->blockSignals( true );
  btnColor1->setColor( mRamp->color1() );
  btnColor1->blockSignals( false );
  btnColor2->blockSignals( true );
  btnColor2->setColor( mRamp->color2() );
  btnColor2->blockSignals( false );
}

void QgsVectorGradientColorRampV2Dialog::updateStopEditor()
{
  mStopEditor->blockSignals( true );
  mStopEditor->setGradientRamp( *mRamp );
  mStopEditor->blockSignals( false );
}

void QgsVectorGradientColorRampV2Dialog::selectedStopChanged( const QgsGradientStop& stop )
{
  mColorWidget->blockSignals( true );
  mColorWidget->setColor( stop.color );
  mColorWidget->blockSignals( false );
  mPositionSpinBox->blockSignals( true );
  mPositionSpinBox->setValue( stop.offset * 100 );
  mPositionSpinBox->blockSignals( false );

  if (( stop.offset == 0 && stop.color == mRamp->color1() ) || ( stop.offset == 1.0 && stop.color == mRamp->color2() ) )
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

  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::colorWidgetChanged( const QColor &color )
{
  mStopEditor->setSelectedStopColor( color );
}

void QgsVectorGradientColorRampV2Dialog::on_mPositionSpinBox_valueChanged( double val )
{
  mStopEditor->setSelectedStopOffset( val / 100.0 );
}

void QgsVectorGradientColorRampV2Dialog::on_mPlotHueCheckbox_toggled( bool checked )
{
  mHueCurve->setVisible( checked );
  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::on_mPlotLightnessCheckbox_toggled( bool checked )
{
  mLightnessCurve->setVisible( checked );
  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::on_mPlotSaturationCheckbox_toggled( bool checked )
{
  mSaturationCurve->setVisible( checked );
  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::on_mPlotAlphaCheckbox_toggled( bool checked )
{
  mAlphaCurve->setVisible( checked );
  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::plotMousePress( QPointF point )
{
  //find closest part

  double minDist = 1;
  mCurrentPlotColorComponent = -1;
  mCurrentPlotMarkerIndex = -1;
  // first color

  for ( int i = 0; i < mRamp->count(); ++i )
  {
    QColor currentCol;
    double currentOff = 0.0;
    if ( i == 0 )
    {
      currentOff = 0.0;
      currentCol = mRamp->color1();
    }
    else if ( i == mRamp->count() - 1 )
    {
      currentOff = 1.0;
      currentCol = mRamp->color2();
    }
    else
    {
      currentOff = mRamp->stops().at( i - 1 ).offset;
      currentCol = mRamp->stops().at( i - 1 ).color;
    }

    double currentDist;
    if ( mPlotHueCheckbox->isChecked() )
    {
      currentDist = qPow( point.x() - currentOff, 2.0 ) + qPow( point.y() - currentCol.hslHueF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 0;
        mCurrentPlotMarkerIndex = i;
      }
    }
    if ( mPlotLightnessCheckbox->isChecked() )
    {
      currentDist = qPow( point.x() - currentOff, 2.0 ) + qPow( point.y() - currentCol.lightnessF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 1;
        mCurrentPlotMarkerIndex = i;
      }
    }
    if ( mPlotSaturationCheckbox->isChecked() )
    {
      currentDist = qPow( point.x() - currentOff, 2.0 ) + qPow( point.y() - currentCol.hslSaturationF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;
        mCurrentPlotColorComponent = 2;
        mCurrentPlotMarkerIndex = i;
      }
    }
    if ( mPlotAlphaCheckbox->isChecked() )
    {
      currentDist = qPow( point.x() - currentOff, 2.0 ) + qPow( point.y() - currentCol.alphaF(), 2.0 );
      if ( currentDist < minDist )
      {
        minDist = currentDist;;
        mCurrentPlotColorComponent = 3;
        mCurrentPlotMarkerIndex = i;
      }
    }
  }

  // watch out - selected stop index may differ if stops in editor are out of order!!!
  if ( mCurrentPlotMarkerIndex >= 0 )
    mStopEditor->selectStop( mCurrentPlotMarkerIndex );
}

void QgsVectorGradientColorRampV2Dialog::plotMouseRelease( QPointF )
{
  mCurrentPlotColorComponent = -1;
}

void QgsVectorGradientColorRampV2Dialog::plotMouseMove( QPointF point )
{
  QColor newColor = mStopEditor->selectedStop().color;

  if ( mCurrentPlotColorComponent == 0 )
    newColor = QColor::fromHslF( qBound( 0.0, point.y(), 1.0 ), newColor.hslSaturationF(), newColor.lightnessF(), newColor.alphaF() );
  else if ( mCurrentPlotColorComponent == 1 )
    newColor = QColor::fromHslF( newColor.hslHueF(), newColor.hslSaturationF(), qBound( 0.0, point.y(), 1.0 ), newColor.alphaF() );
  else if ( mCurrentPlotColorComponent == 2 )
    newColor = QColor::fromHslF( newColor.hslHueF(), qBound( 0.0, point.y(), 1.0 ), newColor.lightnessF(), newColor.alphaF() );
  else if ( mCurrentPlotColorComponent == 3 )
    newColor = QColor::fromHslF( newColor.hslHueF(), newColor.hslSaturationF(), newColor.lightnessF(), qBound( 0.0, point.y(), 1.0 ) );

  mStopEditor->setSelectedStopDetails( newColor, qBound( 0.0, point.x(), 1.0 ) );
}

bool byX( QPointF p1, QPointF p2 )
{
  return p1.x() < p2.x();
}

void QgsVectorGradientColorRampV2Dialog::addPlotMarker( double x, double y, const QColor& color, bool isSelected )
{
  QColor borderColor = color.darker( 200 );
  borderColor.setAlpha( 255 );

  QColor brushColor = color;
  brushColor.setAlpha( 255 );

  QwtPlotMarker *marker = new QwtPlotMarker();
#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  marker->setSymbol( new QwtSymbol( QwtSymbol::Ellipse,  QBrush( brushColor ), QPen( borderColor, isSelected ? 2 : 1 ), QSize( 10, 10 ) ) );
#else
  marker->setSymbol( QwtSymbol( QwtSymbol::Ellipse,  QBrush( brushColor ), QPen( borderColor, isSelected ? 2 : 1 ), QSize( 10, 10 ) ) );
#endif
  marker->setValue( x, y );
  marker->attach( mPlot );
  marker->setRenderHint( QwtPlotItem::RenderAntialiased, true );
  mMarkers << marker;
}

void QgsVectorGradientColorRampV2Dialog::addMarkersForColor( double x, const QColor& color, bool isSelected )
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

void QgsVectorGradientColorRampV2Dialog::updatePlot()
{
  // remove existing markers
  Q_FOREACH ( QwtPlotMarker* marker, mMarkers )
  {
    marker->detach();
    delete marker;
  }
  mMarkers.clear();

  QPolygonF lightnessPoints;
  QPolygonF huePoints;
  QPolygonF saturationPoints;
  QPolygonF alphaPoints;
  lightnessPoints << QPointF( 0.0, mRamp->color1().lightnessF() );
  huePoints << QPointF( 0.0, mRamp->color1().hslHueF() );
  saturationPoints << QPointF( 0.0, mRamp->color1().hslSaturationF() );
  alphaPoints << QPointF( 0.0, mRamp->color1().alphaF() );
  addMarkersForColor( 0, mRamp->color1(), mCurrentPlotMarkerIndex == 0 );

  int i = 1;
  Q_FOREACH ( const QgsGradientStop& stop, mRamp->stops() )
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
    QColor c = mRamp->color( p );
    lightnessPoints << QPointF( p, c.lightnessF() );
    huePoints << QPointF( p, c.hslHueF() );
    saturationPoints << QPointF( p, c.hslSaturationF() );
    alphaPoints << QPointF( p, c.alphaF() );
  }

  lightnessPoints << QPointF( 1.0, mRamp->color2().lightnessF() );
  huePoints << QPointF( 1.0, mRamp->color2().hslHueF() );
  saturationPoints << QPointF( 1.0, mRamp->color2().hslSaturationF() );
  alphaPoints << QPointF( 1.0, mRamp->color2().alphaF() );
  addMarkersForColor( 1.0, mRamp->color2(), mCurrentPlotMarkerIndex == i );

  qSort( lightnessPoints.begin(), lightnessPoints.end(), byX );
  qSort( huePoints.begin(), huePoints.end(), byX );
  qSort( saturationPoints.begin(), saturationPoints.end(), byX );
  qSort( alphaPoints.begin(), alphaPoints.end(), byX );

#if defined(QWT_VERSION) && QWT_VERSION>=0x060000
  mLightnessCurve->setSamples( lightnessPoints );
  mHueCurve->setSamples( huePoints );
  mSaturationCurve->setSamples( saturationPoints );
  mAlphaCurve->setSamples( alphaPoints );
#else
  mLightnessCurve->setData( lightnessPoints );
  mHueCurve->setData( huePoints );
  mSaturationCurve->setData( saturationPoints );
  mAlphaCurve->setData( alphaPoints );
#endif
  mPlot->replot();
}

void QgsVectorGradientColorRampV2Dialog::updateRampFromStopEditor()
{
  *mRamp = mStopEditor->gradientRamp();
  mPositionSpinBox->blockSignals( true );
  mPositionSpinBox->setValue( mStopEditor->selectedStop().offset * 100 );
  mPositionSpinBox->blockSignals( false );
  mColorWidget->blockSignals( true );
  mColorWidget->setColor( mStopEditor->selectedStop().color );
  mColorWidget->blockSignals( false );

  updateColorButtons();
  updatePlot();
}

void QgsVectorGradientColorRampV2Dialog::setColor1( const QColor& color )
{
  mStopEditor->setColor1( color );
  updateColorButtons();
}

void QgsVectorGradientColorRampV2Dialog::setColor2( const QColor& color )
{
  mStopEditor->setColor2( color );
  updateColorButtons();
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
      const QMouseEvent* mouseEvent = static_cast<QMouseEvent* >( event );
      if ( mouseEvent->button() == Qt::LeftButton )
      {
        emit mousePress( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseMove:
    {
      const QMouseEvent* mouseEvent = static_cast<QMouseEvent* >( event );
      if ( mouseEvent->buttons() & Qt::LeftButton )
      {
        // only emit when button pressed
        emit mouseMove( mapPoint( mouseEvent->pos() ) );
      }
      break;
    }
    case QEvent::MouseButtonRelease:
    {
      const QMouseEvent* mouseEvent = static_cast<QMouseEvent* >( event );
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
