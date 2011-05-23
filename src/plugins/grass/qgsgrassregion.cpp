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


/** map tool which uses rubber band for changing grass region */
QgsGrassRegionEdit::QgsGrassRegionEdit( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
{
  mDraw = false;
  mRubberBand = new QgsRubberBand( mCanvas, true );
  mSrcRubberBand = new QgsRubberBand( mCanvas, true );
  QString gisdbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();
  mCrs = QgsGrass::crs( gisdbase, location );
  QgsDebugMsg( "mCrs: " + mCrs.toWkt() );
  setTransform();
  connect( canvas->mapRenderer(), SIGNAL( destinationSrsChanged() ), this, SLOT( setTransform() ) );
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

  if ( mCanvas->mapRenderer()->hasCrsTransformEnabled() && mCrs.isValid() && mCanvas->mapRenderer()->destinationCrs().isValid() )
  {
    QgsCoordinateTransform coordinateTransform;
    coordinateTransform.setSourceCrs( mCanvas->mapRenderer()->destinationCrs() );
    coordinateTransform.setDestCRS( mCrs );
    mSrcRectangle = coordinateTransform.transformBoundingBox( mSrcRectangle );
  }
}

void QgsGrassRegionEdit::setTransform()
{
  if ( mCrs.isValid() && canvas()->mapRenderer()->destinationCrs().isValid() )
  {
    mCoordinateTransform.setSourceCrs( mCrs );
    mCoordinateTransform.setDestCRS( canvas()->mapRenderer()->destinationCrs() );
  }
}

void QgsGrassRegionEdit::transform( QgsMapCanvas *canvas, QVector<QgsPoint> &points, QgsCoordinateTransform *coordinateTransform, QgsCoordinateTransform::TransformDirection direction )
{
  QgsDebugMsg( "Entered" );
  /** Coordinate transform */
  if ( canvas->mapRenderer()->hasCrsTransformEnabled() )
  {
    //QgsDebugMsg ( "srcCrs = " +  coordinateTransform->sourceCrs().toWkt() );
    //QgsDebugMsg ( "destCrs = " +  coordinateTransform->destCRS().toWkt() );
    for ( int i = 0; i < points.size(); i++ )
    {
      points[i] = coordinateTransform->transform( points[i], direction );
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

QgsGrassRegion::QgsGrassRegion( QgsGrassPlugin *plugin,  QgisInterface *iface,
                                QWidget * parent, Qt::WFlags f )
    : QDialog( parent, f ), QgsGrassRegionBase( )
{
  QgsDebugMsg( "QgsGrassRegion()" );

  setupUi( this );
  setAttribute( Qt::WA_DeleteOnClose );

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  mPlugin = plugin;
  mInterface = iface;
  mCanvas = mInterface->mapCanvas();
  restorePosition();
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
  mRadioGroup->addButton( mCellResRadio );
  mRadioGroup->addButton( mRowsColsRadio );
  mCellResRadio->setChecked( true );
  radioChanged();

  connect( mRadioGroup, SIGNAL( buttonClicked( int ) ), this, SLOT( radioChanged() ) );

  // Set values to current region
  QString gisdbase = QgsGrass::getDefaultGisdbase();
  QString location = QgsGrass::getDefaultLocation();
  QString mapset   = QgsGrass::getDefaultMapset();

  if ( gisdbase.isEmpty() || location.isEmpty() || mapset.isEmpty() )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "GISDBASE, LOCATION_NAME or MAPSET is not set, cannot display current region." ) );
  }

  QgsGrass::setLocation( gisdbase, location );
  char *err = G__get_window( &mWindow, ( char * ) "", ( char * ) "WIND", mapset.toLatin1().data() );

  if ( err )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot read current region: %1" ).arg( QString::fromUtf8( err ) ) );
    return;
  }

  mRegionEdit = new QgsGrassRegionEdit( mCanvas );
  connect( mRegionEdit, SIGNAL( captureStarted() ), this, SLOT( hide() ) );
  connect( mRegionEdit, SIGNAL( captureEnded() ), this, SLOT( onCaptureFinished() ) );
  mCanvas->setMapTool( mRegionEdit );

  refreshGui();

  connect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );

  // Connect entries
  connect( mNorth, SIGNAL( editingFinished() ), this, SLOT( northChanged() ) );
  connect( mSouth, SIGNAL( editingFinished() ), this, SLOT( southChanged() ) );
  connect( mEast, SIGNAL( editingFinished() ), this, SLOT( eastChanged() ) );
  connect( mWest, SIGNAL( editingFinished() ), this, SLOT( westChanged() ) );
  connect( mNSRes, SIGNAL( editingFinished() ), this, SLOT( NSResChanged() ) );
  connect( mEWRes, SIGNAL( editingFinished() ), this, SLOT( EWResChanged() ) );
  connect( mRows, SIGNAL( editingFinished() ), this, SLOT( rowsChanged() ) );
  connect( mCols, SIGNAL( editingFinished() ), this, SLOT( colsChanged() ) );

  // Symbology
  QPen pen = mPlugin->regionPen();
  mColorButton->setColor( pen.color() );
  connect( mColorButton, SIGNAL( clicked() ), this, SLOT( changeColor() ) );

  mWidthSpinBox->setValue( pen.width() );
  connect( mWidthSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( changeWidth() ) );
}

void QgsGrassRegion::changeColor( void )
{
  QPen pen = mPlugin->regionPen();
  QColor color = QColorDialog::getColor( pen.color(), this );
  if ( color.isValid() )
  {
    mColorButton->setColor( color );

    pen.setColor( color );
    mPlugin->setRegionPen( pen );
  }
}

void QgsGrassRegion::changeWidth( void )
{
  QPen pen = mPlugin->regionPen();

  pen.setWidth( mWidthSpinBox->value() );
  mPlugin->setRegionPen( pen );
}

QString QgsGrassRegion::formatEdge( double v )
{
  // Not sure about formating
  if ( v > 999999 )
  {
    return  QString( "%1" ).arg( v, 0, 'f', 0 ); // to avoid e format for large numbers
  }
  return QString( "%1" ).arg( v, 0, 'g' );
}

void QgsGrassRegion::refreshGui()
{
  if ( mUpdatingGui )
    return;

  mUpdatingGui = true;

  QgsDebugMsg( "entered." );

  mNorth->setText( QString( "%1" ).arg( mWindow.north, 0, 'g', 15 ) );
  mSouth->setText( QString( "%1" ).arg( mWindow.south, 0, 'g', 15 ) );
  mEast->setText( QString( "%1" ).arg( mWindow.east, 0, 'g', 15 ) );
  mWest->setText( QString( "%1" ).arg( mWindow.west, 0, 'g', 15 ) );
  mNSRes->setText( QString( "%1" ).arg( mWindow.ns_res, 0, 'g' ) );
  mEWRes->setText( QString( "%1" ).arg( mWindow.ew_res, 0, 'g' ) );
  mRows->setText( QString( "%1" ).arg( mWindow.rows ) );
  mCols->setText( QString( "%1" ).arg( mWindow.cols ) );

  displayRegion();
  mUpdatingGui = false;
}

QgsGrassRegion::~QgsGrassRegion()
{
  delete mRegionEdit;
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

  if ( mRowsColsRadio->isChecked() )
  {
    mNSRes->setEnabled( false );
    mEWRes->setEnabled( false );
    mRows->setEnabled( true );
    mCols->setEnabled( true );
  }
  else
  {
    mNSRes->setEnabled( true );
    mEWRes->setEnabled( true );
    mRows->setEnabled( false );
    mCols->setEnabled( false );
  }
}

void QgsGrassRegion::onCaptureFinished()
{
  QgsDebugMsg( "entered." );
  QgsRectangle rect = mRegionEdit->getRegion();

  mWindow.west = rect.xMinimum();
  mWindow.east = rect.xMaximum();
  mWindow.south = rect.yMinimum();
  mWindow.north = rect.yMaximum();
  adjust();

  refreshGui();
  show();
}

void QgsGrassRegion::displayRegion()
{
  QgsPoint ul( mWindow.west, mWindow.north );
  QgsPoint lr( mWindow.east, mWindow.south );

  //mRegionEdit->setRegion( ul, lr );
  mRegionEdit->setSrcRegion( QgsRectangle( ul, lr ) );
}

void QgsGrassRegion::accept()
{
  // TODO: better repaint region
  QSettings settings;

  bool on = settings.value( "/GRASS/region/on", true ).toBool();

  if ( on )
  {
    mPlugin->switchRegion( false ); // delete
  }

  QgsGrass::setLocation( QgsGrass::getDefaultGisdbase(), QgsGrass::getDefaultLocation() );
  G__setenv(( char * ) "MAPSET", QgsGrass::getDefaultMapset().toLatin1().data() );

  if ( G_put_window( &mWindow ) == -1 )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot write region" ) );
    return;
  }

  if ( on )
  {
    mPlugin->switchRegion( on );  // draw new
  }

  saveWindowLocation();
  mCanvas->setMapTool( NULL );
  QDialog::accept();
}

void QgsGrassRegion::reject()
{
  saveWindowLocation();
  mCanvas->setMapTool( NULL );
  QDialog::reject();
}

void QgsGrassRegion::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/GRASS/windows/region/geometry" ).toByteArray() );
}

void QgsGrassRegion::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/GRASS/windows/region/geometry", saveGeometry() );
}
