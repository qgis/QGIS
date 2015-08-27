/***************************************************************************
    qgsgrassregion.h  -  Edit region
                             -------------------
    begin                : August, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsgrassregion.h"
#include "qgsgrassplugin.h"
#include "qgsgrass.h"

#include "qgisinterface.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmaptool.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRubberBand>
#include <QSettings>


/** Map tool which uses rubber band for changing grass region */
QgsGrassRegionEdit::QgsGrassRegionEdit( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
  QgsDebugMsg( "entered" );
  mDraw = false;
  mRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
  mSrcRubberBand = new QgsRubberBand( mCanvas, QGis::Polygon );
  mCrs = QgsGrass::crs( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
  QgsDebugMsg( "mCrs: " + mCrs.toWkt() );
  setTransform();
  connect( canvas, SIGNAL( destinationCrsChanged() ), this, SLOT( setTransform() ) );
}

QgsGrassRegionEdit::~QgsGrassRegionEdit()
{
  delete mRubberBand;
  delete mSrcRubberBand;
}

//! mouse pressed in map canvas
void QgsGrassRegionEdit::canvasPressEvent( QMouseEvent * event )
{
  QgsDebugMsg( "entered." );
  mDraw = true;
  mRubberBand->reset( true );
  mSrcRubberBand->reset( true );
  emit captureStarted();

  mStartPoint = toMapCoordinates( event->pos() );
  mEndPoint = mStartPoint;
  setRegion( mStartPoint, mEndPoint );
}

//! mouse movement in map canvas
void QgsGrassRegionEdit::canvasMoveEvent( QMouseEvent * event )
{
  if ( !mDraw )
    return;

  mEndPoint = toMapCoordinates( event->pos() );
  setRegion( mStartPoint, mEndPoint );
}

//! mouse button released
void QgsGrassRegionEdit::canvasReleaseEvent( QMouseEvent * event )
{
  if ( !mDraw )
    return;

  mEndPoint = toMapCoordinates( event->pos() );
  setRegion( mStartPoint, mEndPoint );
  mDraw = false;
  emit captureEnded();
}

//! called when map tool is about to get inactive
void QgsGrassRegionEdit::deactivate()
{
  mRubberBand->reset( true );
  mSrcRubberBand->reset( true );
  QgsMapTool::deactivate();
}

void QgsGrassRegionEdit::setRegion( const QgsPoint& ul, const QgsPoint& lr )
{
  mStartPoint = ul;
  mEndPoint = lr;
  calcSrcRegion();
  drawRegion( canvas(), mRubberBand, mSrcRectangle, &mCoordinateTransform, true );
  drawRegion( canvas(), mSrcRubberBand, QgsRectangle( mStartPoint, mEndPoint ), 0, true );
}

void QgsGrassRegionEdit::calcSrcRegion()
{
  mSrcRectangle.set( mStartPoint, mEndPoint );

  if ( mCanvas->hasCrsTransformEnabled() && mCrs.isValid() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    QgsCoordinateTransform coordinateTransform;
    coordinateTransform.setSourceCrs( mCanvas->mapSettings().destinationCrs() );
    coordinateTransform.setDestCRS( mCrs );
    mSrcRectangle = coordinateTransform.transformBoundingBox( mSrcRectangle );
  }
}

void QgsGrassRegionEdit::setTransform()
{
  if ( mCrs.isValid() && canvas()->mapSettings().destinationCrs().isValid() )
  {
    mCoordinateTransform.setSourceCrs( mCrs );
    mCoordinateTransform.setDestCRS( canvas()->mapSettings().destinationCrs() );
  }
}

void QgsGrassRegionEdit::transform( QgsMapCanvas *canvas, QVector<QgsPoint> &points, QgsCoordinateTransform *coordinateTransform, QgsCoordinateTransform::TransformDirection direction )
{
  /** Coordinate transform */
  if ( canvas->hasCrsTransformEnabled() )
  {
    //QgsDebugMsg ( "srcCrs = " +  coordinateTransform->sourceCrs().toWkt() );
    //QgsDebugMsg ( "destCrs = " +  coordinateTransform->destCRS().toWkt() );
    try
    {
      for ( int i = 0; i < points.size(); i++ )
      {
        points[i] = coordinateTransform->transform( points[i], direction );
      }
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsg( QString( "transformation failed: %1" ).arg( cse.what() ) );
    }
  }
}

void QgsGrassRegionEdit::drawRegion( QgsMapCanvas *canvas, QgsRubberBand* rubberBand, const QgsRectangle &rect, QgsCoordinateTransform * coordinateTransform, bool isPolygon )
{
  QVector<QgsPoint> points;
  points.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  points.append( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  points.append( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  points.append( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  if ( !isPolygon )
  {
    points.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  }

  if ( coordinateTransform )
  {
    transform( canvas, points, coordinateTransform );
  }
  rubberBand->reset( isPolygon );
  for ( int i = 0; i < points.size(); i++ )
  {
    bool update = false; // true to update canvas
    if ( i == points.size() - 1 )
      update = true;
    rubberBand->addPoint( points[i], update );
  }
  rubberBand->show();
}

QgsRectangle QgsGrassRegionEdit::getRegion()
{
  //return QgsRectangle( mStartPoint, mEndPoint );
  return mSrcRectangle;
}

void QgsGrassRegionEdit::setSrcRegion( const QgsRectangle &rect )
{
  mSrcRectangle = rect;
}

QgsGrassRegion::QgsGrassRegion( QgisInterface *iface,
                                QWidget * parent, Qt::WindowFlags f )
    : QWidget( parent, f )
    , QgsGrassRegionBase()
    , mRegionEdit( 0 )
{
  QgsDebugMsg( "QgsGrassRegion()" );

  setupUi( this );
  setAttribute( Qt::WA_DeleteOnClose );

  connect( mButtonBox, SIGNAL( clicked( QAbstractButton * ) ), SLOT( buttonClicked( QAbstractButton * ) ) );

  //mPlugin = plugin;
  mInterface = iface;
  mCanvas = mInterface->mapCanvas();
  mUpdatingGui = false;

  // Set input validators
  QDoubleValidator *dv = new QDoubleValidator( 0 );
  QIntValidator *iv = new QIntValidator( 0 );

  mNorth->setValidator( dv );
  mSouth->setValidator( dv );
  mEast->setValidator( dv );
  mWest->setValidator( dv );
  mNSRes->setValidator( dv );
  mEWRes->setValidator( dv );
  mRows->setValidator( iv );
  mCols->setValidator( iv );

  // Group radio buttons
  mRadioGroup = new QButtonGroup();
  mRadioGroup->addButton( mResRadio );
  mRadioGroup->addButton( mRowsColsRadio );
  mResRadio->setChecked( true );
  radioChanged();

  connect( mRadioGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( radioChanged() ) );

  // Connect entries
  connect( mNorth, SIGNAL( editingFinished() ), this, SLOT( northChanged() ) );
  connect( mSouth, SIGNAL( editingFinished() ), this, SLOT( southChanged() ) );
  connect( mEast, SIGNAL( editingFinished() ), this, SLOT( eastChanged() ) );
  connect( mWest, SIGNAL( editingFinished() ), this, SLOT( westChanged() ) );
  connect( mNSRes, SIGNAL( editingFinished() ), this, SLOT( NSResChanged() ) );
  connect( mEWRes, SIGNAL( editingFinished() ), this, SLOT( EWResChanged() ) );
  connect( mRows, SIGNAL( editingFinished() ), this, SLOT( rowsChanged() ) );
  connect( mCols, SIGNAL( editingFinished() ), this, SLOT( colsChanged() ) );

  connect( QgsGrass::instance(), SIGNAL( regionChanged() ), SLOT( reloadRegion() ) );
  connect( mCanvas, SIGNAL( mapToolSet( QgsMapTool * ) ), SLOT( canvasMapToolSet( QgsMapTool * ) ) );
}

QgsGrassRegion::~QgsGrassRegion()
{
  delete mRegionEdit;
}

QString QgsGrassRegion::formatExtent( double v )
{
  // format with precision approximately to meters
  // max length of degree of latitude on pole is 111694 m
  return qgsDoubleToString( v, mCrs.mapUnits() == QGis::Degrees ? 6 : 1 );
}

QString QgsGrassRegion::formatResolution( double v )
{
  return qgsDoubleToString( v, mCrs.mapUnits() == QGis::Degrees ? 10 : 4 );
}

void QgsGrassRegion::readRegion()
{
  // Read current region
  try
  {
    QgsGrass::region( &mWindow );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsGrass::warning( e );
    return;
  }
}

void QgsGrassRegion::refreshGui()
{
  if ( mUpdatingGui )
  {
    return;
  }

  mUpdatingGui = true;

  QgsDebugMsg( "entered." );

  mNorth->setText( formatExtent( mWindow.north ) );
  mSouth->setText( formatExtent( mWindow.south ) );
  mEast->setText( formatExtent( mWindow.east ) );
  mWest->setText( formatExtent( mWindow.west ) );
  mNSRes->setText( formatResolution( mWindow.ns_res ) );
  mEWRes->setText( formatResolution( mWindow.ew_res ) );
  mRows->setText( QString::number( mWindow.rows ) );
  mCols->setText( QString::number( mWindow.cols ) );

  displayRegion();
  mUpdatingGui = false;
}

void QgsGrassRegion::reloadRegion()
{
  readRegion();
  refreshGui();
}

void QgsGrassRegion::mapsetChanged()
{
  delete mRegionEdit;
  mRegionEdit = 0;
  if ( QgsGrass::activeMode() )
  {
    mRegionEdit = new QgsGrassRegionEdit( mCanvas );
    connect( mRegionEdit, SIGNAL( captureEnded() ), this, SLOT( onCaptureFinished() ) );

    mCrs = QgsGrass::crs( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
    reloadRegion();
  }
}

void QgsGrassRegion::northChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.north = mNorth->text().toDouble();
  if ( mWindow.north < mWindow.south )
    mWindow.north = mWindow.south;

  adjust();
  refreshGui();
}

void QgsGrassRegion::southChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.south = mSouth->text().toDouble();
  if ( mWindow.south > mWindow.north )
    mWindow.south = mWindow.north;

  adjust();
  refreshGui();
}

void QgsGrassRegion::eastChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.east = mEast->text().toDouble();
  if ( mWindow.east < mWindow.west )
    mWindow.east = mWindow.west;

  adjust();
  refreshGui();
}

void QgsGrassRegion::westChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.west = mWest->text().toDouble();
  if ( mWindow.west > mWindow.east )
    mWindow.west = mWindow.east;

  adjust();
  refreshGui();
}

void QgsGrassRegion::NSResChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.ns_res = mNSRes->text().toDouble();
  if ( mWindow.ns_res <= 0 )
    mWindow.ns_res = 1;

  adjust();
  refreshGui();
}

void QgsGrassRegion::EWResChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.ew_res = mEWRes->text().toDouble();
  if ( mWindow.ew_res <= 0 )
    mWindow.ew_res = 1;

  adjust();
  refreshGui();
}

void QgsGrassRegion::rowsChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.rows = mRows->text().toInt();
  if ( mWindow.rows < 1 )
    mWindow.rows = 1;

  adjust();
  refreshGui();
}

void QgsGrassRegion::colsChanged()
{
  if ( mUpdatingGui )
    return;

  mWindow.cols = mCols->text().toInt();
  if ( mWindow.cols < 1 )
    mWindow.cols = 1;

  adjust();
  refreshGui();
}

void QgsGrassRegion::adjust()
{
  int rc = 0;
  if ( mRowsColsRadio->isChecked() )
  {
    rc = 1;
  }
  G_adjust_Cell_head( &mWindow, rc, rc );
}

void QgsGrassRegion::radioChanged()
{
  QgsDebugMsg( "entered." );

  bool res = !mRowsColsRadio->isChecked();

  mEWResLabel->setEnabled( res );
  mEWRes->setEnabled( res );
  mNSResLabel->setEnabled( res );
  mNSRes->setEnabled( res );

  mColsLabel->setEnabled( !res );
  mCols->setEnabled( !res );
  mRowsLabel->setEnabled( !res );
  mRows->setEnabled( !res );
}

void QgsGrassRegion::onCaptureFinished()
{
  QgsDebugMsg( "entered." );
  if ( !mRegionEdit )
  {
    return;
  }
  QgsRectangle rect = mRegionEdit->getRegion();

  mWindow.west = rect.xMinimum();
  mWindow.east = rect.xMaximum();
  mWindow.south = rect.yMinimum();
  mWindow.north = rect.yMaximum();
  adjust();

  refreshGui();
}

void QgsGrassRegion::canvasMapToolSet( QgsMapTool *tool )
{
  QgsDebugMsg( "entered" );
  mDrawButton->setChecked( tool == mRegionEdit );
}

void QgsGrassRegion::displayRegion()
{
  if ( !mRegionEdit )
  {
    return;
  }
  QgsPoint ul( mWindow.west, mWindow.north );
  QgsPoint lr( mWindow.east, mWindow.south );

  mRegionEdit->setSrcRegion( QgsRectangle( ul, lr ) );
}

void QgsGrassRegion::on_mDrawButton_clicked()
{
  QgsDebugMsg( "entered" );
  mCanvas->setMapTool( mRegionEdit );
}

void QgsGrassRegion::buttonClicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    try
    {
      QgsGrass::instance()->writeRegion( &mWindow );
    }
    catch ( QgsGrass::Exception &e )
    {
      QgsGrass::warning( e );
      return;
    }
  }
  else if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ResetRole )
  {
    reloadRegion();
  }
  // Better to keep the tool selected until another tool is chosen?
  mCanvas->unsetMapTool( mRegionEdit );
}


