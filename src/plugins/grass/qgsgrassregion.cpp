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
#include "qgsmaptool.h"
#include "qgsexception.h"
#include "qgsmapmouseevent.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QMessageBox>
#include <QRubberBand>
#include <QSettings>
#include <QDoubleValidator>


//! Map tool which uses rubber band for changing grass region
QgsGrassRegionEdit::QgsGrassRegionEdit( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mDraw = false;
  mRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSrcRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
  QString error;
  mCrs = QgsGrass::crs( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation(), error );
  QgsDebugMsg( "mCrs: " + mCrs.toWkt() );
  setTransform();
  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsGrassRegionEdit::setTransform );
}

QgsGrassRegionEdit::~QgsGrassRegionEdit()
{
  delete mRubberBand;
  delete mSrcRubberBand;
}

//! mouse pressed in map canvas
void QgsGrassRegionEdit::canvasPressEvent( QgsMapMouseEvent *event )
{
  mDraw = true;
  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mSrcRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  emit captureStarted();

  mStartPoint = toMapCoordinates( event->pos() );
  mEndPoint = mStartPoint;
  setRegion( mStartPoint, mEndPoint );
}

//! mouse movement in map canvas
void QgsGrassRegionEdit::canvasMoveEvent( QgsMapMouseEvent *event )
{
  if ( !mDraw )
    return;

  mEndPoint = toMapCoordinates( event->pos() );
  setRegion( mStartPoint, mEndPoint );
}

//! mouse button released
void QgsGrassRegionEdit::canvasReleaseEvent( QgsMapMouseEvent *event )
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
  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mSrcRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  QgsMapTool::deactivate();
}

void QgsGrassRegionEdit::setRegion( const QgsPointXY &ul, const QgsPointXY &lr )
{
  mStartPoint = ul;
  mEndPoint = lr;
  calcSrcRegion();
  drawRegion( canvas(), mRubberBand, mSrcRectangle, mCoordinateTransform, true );
  drawRegion( canvas(), mSrcRubberBand, QgsRectangle( mStartPoint, mEndPoint ), QgsCoordinateTransform(), true );
}

void QgsGrassRegionEdit::calcSrcRegion()
{
  mSrcRectangle.set( mStartPoint, mEndPoint );

  if ( mCrs.isValid() && mCanvas->mapSettings().destinationCrs().isValid() )
  {
    QgsCoordinateTransform coordinateTransform;
    coordinateTransform.setSourceCrs( mCanvas->mapSettings().destinationCrs() );
    coordinateTransform.setDestinationCrs( mCrs );
    mSrcRectangle = coordinateTransform.transformBoundingBox( mSrcRectangle );
  }
}

void QgsGrassRegionEdit::setTransform()
{
  if ( mCrs.isValid() && canvas()->mapSettings().destinationCrs().isValid() )
  {
    mCoordinateTransform.setSourceCrs( mCrs );
    mCoordinateTransform.setDestinationCrs( canvas()->mapSettings().destinationCrs() );
  }
}

void QgsGrassRegionEdit::transform( QgsMapCanvas *, QVector<QgsPointXY> &points, const QgsCoordinateTransform &coordinateTransform, Qgis::TransformDirection direction )
{
  //! Coordinate transform
  //QgsDebugMsg ( "srcCrs = " +  coordinateTransform->sourceCrs().toWkt() );
  //QgsDebugMsg ( "destCrs = " +  coordinateTransform->destCRS().toWkt() );
  try
  {
    for ( int i = 0; i < points.size(); i++ )
    {
      points[i] = coordinateTransform.transform( points[i], direction );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsDebugMsg( QString( "transformation failed: %1" ).arg( cse.what() ) );
  }
}

void QgsGrassRegionEdit::drawRegion( QgsMapCanvas *canvas, QgsRubberBand *rubberBand, const QgsRectangle &rect, const QgsCoordinateTransform &coordinateTransform, bool isPolygon )
{
  QVector<QgsPointXY> points;
  points.append( QgsPointXY( rect.xMinimum(), rect.yMinimum() ) );
  points.append( QgsPointXY( rect.xMaximum(), rect.yMinimum() ) );
  points.append( QgsPointXY( rect.xMaximum(), rect.yMaximum() ) );
  points.append( QgsPointXY( rect.xMinimum(), rect.yMaximum() ) );
  if ( !isPolygon )
  {
    points.append( QgsPointXY( rect.xMinimum(), rect.yMinimum() ) );
  }

  if ( coordinateTransform.isValid() )
  {
    transform( canvas, points, coordinateTransform );
  }
  rubberBand->reset( isPolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
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
                                QWidget *parent, Qt::WindowFlags f )
  : QWidget( parent, f )
  , QgsGrassRegionBase()
  , mX( 0 )
  , mY( 0 )
  , mUpdatingGui( false )
{
  QgsDebugMsg( "QgsGrassRegion()" );
  QgsGrass::initRegion( &mWindow );

  setupUi( this );
  connect( mDrawButton, &QPushButton::clicked, this, &QgsGrassRegion::mDrawButton_clicked );
  setAttribute( Qt::WA_DeleteOnClose );

  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsGrassRegion::buttonClicked );

  //mPlugin = plugin;
  mInterface = iface;
  mCanvas = mInterface->mapCanvas();
  mUpdatingGui = false;

  // Set input validators
  QDoubleValidator *dv = new QDoubleValidator( nullptr );
  QIntValidator *iv = new QIntValidator( nullptr );

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

  connect( mRadioGroup, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ), this, &QgsGrassRegion::radioChanged );

  // Connect entries
  connect( mNorth, &QLineEdit::editingFinished, this, &QgsGrassRegion::northChanged );
  connect( mSouth, &QLineEdit::editingFinished, this, &QgsGrassRegion::southChanged );
  connect( mEast, &QLineEdit::editingFinished, this, &QgsGrassRegion::eastChanged );
  connect( mWest, &QLineEdit::editingFinished, this, &QgsGrassRegion::westChanged );
  connect( mNSRes, &QLineEdit::editingFinished, this, &QgsGrassRegion::NSResChanged );
  connect( mEWRes, &QLineEdit::editingFinished, this, &QgsGrassRegion::EWResChanged );
  connect( mRows, &QLineEdit::editingFinished, this, &QgsGrassRegion::rowsChanged );
  connect( mCols, &QLineEdit::editingFinished, this, &QgsGrassRegion::colsChanged );

  connect( QgsGrass::instance(), &QgsGrass::regionChanged, this, &QgsGrassRegion::reloadRegion );
  connect( mCanvas, &QgsMapCanvas::mapToolSet, this, &QgsGrassRegion::canvasMapToolSet );
}

QgsGrassRegion::~QgsGrassRegion()
{
  delete mRegionEdit;
}

QString QgsGrassRegion::formatExtent( double v )
{
  // format with precision approximately to meters
  // max length of degree of latitude on pole is 111694 m
  return qgsDoubleToString( v, mCrs.mapUnits() == QgsUnitTypes::DistanceDegrees ? 6 : 1 );
}

QString QgsGrassRegion::formatResolution( double v )
{
  return qgsDoubleToString( v, mCrs.mapUnits() == QgsUnitTypes::DistanceDegrees ? 10 : 4 );
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
  mRegionEdit = nullptr;
  if ( QgsGrass::activeMode() )
  {
    mRegionEdit = new QgsGrassRegionEdit( mCanvas );
    connect( mRegionEdit, &QgsGrassRegionEdit::captureEnded, this, &QgsGrassRegion::onCaptureFinished );

    QString error;
    mCrs = QgsGrass::crs( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation(), error );
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
  mButtonBox->button( QDialogButtonBox::Apply )->setDisabled( false );
  int rc = 0;
  if ( mRowsColsRadio->isChecked() )
  {
    rc = 1;
  }
  G_TRY
  {
    G_adjust_Cell_head( &mWindow, rc, rc );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsGrass::warning( e );
    mButtonBox->button( QDialogButtonBox::Apply )->setDisabled( true );
  }
}

void QgsGrassRegion::radioChanged()
{

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
  mDrawButton->setChecked( tool == mRegionEdit );
}

void QgsGrassRegion::displayRegion()
{
  if ( !mRegionEdit )
  {
    return;
  }
  QgsPointXY ul( mWindow.west, mWindow.north );
  QgsPointXY lr( mWindow.east, mWindow.south );

  mRegionEdit->setSrcRegion( QgsRectangle( ul, lr ) );
}

void QgsGrassRegion::mDrawButton_clicked()
{
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


