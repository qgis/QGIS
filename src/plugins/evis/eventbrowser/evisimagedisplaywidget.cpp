/*
** File: evisimagedisplaywidget.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-13
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
#include "evisimagedisplaywidget.h"

#include "qgsapplication.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QUrl>
/**
* Constructor
* @param parent - Pointer the to parent QWidget for modality
* @param fl - Windown flags
*/
eVisImageDisplayWidget::eVisImageDisplayWidget( QWidget* parent, Qt::WindowFlags fl ) : QWidget( parent, fl )
{
  //Setup zoom buttons
  pbtnZoomIn = new QPushButton();
  pbtnZoomOut = new QPushButton();
  pbtnZoomFull = new QPushButton();
  pbtnZoomIn->setEnabled( false );
  pbtnZoomOut->setEnabled( false );
  pbtnZoomFull->setEnabled( false );
  QString myThemePath = QgsApplication::activeThemePath();
  pbtnZoomIn->setToolTip( tr( "Zoom in" ) );
  pbtnZoomIn->setWhatsThis( tr( "Zoom in to see more detail." ) );
  pbtnZoomOut->setToolTip( tr( "Zoom out" ) );
  pbtnZoomOut->setWhatsThis( tr( "Zoom out to see more area." ) );
  pbtnZoomFull->setToolTip( tr( "Zoom to full extent" ) );
  pbtnZoomFull->setWhatsThis( tr( "Zoom to display the entire image." ) );
  pbtnZoomIn->setIcon( QIcon( QPixmap( myThemePath + "/mActionZoomIn.svg" ) ) );
  pbtnZoomOut->setIcon( QIcon( QPixmap( myThemePath + "/mActionZoomOut.svg" ) ) );
  pbtnZoomFull->setIcon( QIcon( QPixmap( myThemePath + "/mActionZoomFullExtent.svg" ) ) );
  connect( pbtnZoomIn, SIGNAL( clicked() ), this, SLOT( on_pbtnZoomIn_clicked() ) );
  connect( pbtnZoomOut, SIGNAL( clicked() ), this, SLOT( on_pbtnZoomOut_clicked() ) );
  connect( pbtnZoomFull, SIGNAL( clicked() ), this, SLOT( on_pbtnZoomFull_clicked() ) );

  //Setup zoom button layout
  QWidget* myButtonBar = new QWidget();
  QHBoxLayout* myButtonBarLayout = new QHBoxLayout();
  myButtonBarLayout->addStretch();
  myButtonBarLayout->addWidget( pbtnZoomIn );
  myButtonBarLayout->addWidget( pbtnZoomOut );
  myButtonBarLayout->addWidget( pbtnZoomFull );
  myButtonBar->setLayout( myButtonBarLayout );

  //setup display area
  mDisplayArea = new QScrollArea();

  QVBoxLayout* myLayout = new QVBoxLayout;
  myLayout->addWidget( myButtonBar );
  myLayout->addWidget( mDisplayArea );
  setLayout( myLayout );

  //setup label to hold image
  mImageLabel = new QLabel();
  mImageLabel->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
  mImageLabel->setScaledContents( true );
  mDisplayArea->setWidget( mImageLabel );

  //setup image
  mImageLoaded = false;
  mImage = new QPixmap( mDisplayArea->size().width(), mDisplayArea->size().height() );
  mImage->fill();
  mImageLabel->setPixmap( *mImage );

  //setup http connection
  mHttpBuffer = new QBuffer();
#if QT_VERSION < 0x050000
  mHttpConnection = new QHttp();
#endif
  mHttpBuffer->open( QBuffer::ReadWrite );
// TODO
#if QT_VERSION < 0x050000
  connect( mHttpConnection, SIGNAL( requestFinished( int, bool ) ), this, SLOT( displayUrlImage( int, bool ) ) );
#endif

  //initialize remaining variables
  mScaleByHeight = false;
  mScaleByWidth = false;
  mCurrentZoomStep = 0;
  ZOOM_STEPS = 5;
}

eVisImageDisplayWidget::~eVisImageDisplayWidget()
{

  delete mImageLabel;
  delete mImage;
  delete mHttpBuffer;
#if QT_VERSION < 0x050000
  delete mHttpConnection;
#endif
  delete pbtnZoomIn;
  delete pbtnZoomOut;
  delete pbtnZoomFull;
  delete mDisplayArea;
}

void eVisImageDisplayWidget::resizeEvent( QResizeEvent *event )
{
  event->accept();
  setScalers();
  displayImage();
}

/**
* Public method called to display an image loaded locally from disk
* @param path - The path and filename of the image to load from disk
*/
void eVisImageDisplayWidget::displayImage( QString path )
{
  mImageLoaded = mImage->load( path, 0, Qt::AutoColor );
  setToolTip( path );

  mCurrentZoomStep = 0;
  pbtnZoomOut->setEnabled( false );
  pbtnZoomFull->setEnabled( false );
  if ( mImageLoaded )
  {
    pbtnZoomIn->setEnabled( true );
  }
  else
  {
    pbtnZoomIn->setEnabled( false );
  }

  setScalers();

  displayImage();
}

/**
* Private method which scales and actually displays the image in the widget
*/
void eVisImageDisplayWidget::displayImage()
{
  QSize mySize;
  if ( mImageLoaded )
  {
    //TODO: See about migrating these nasty scaling routines to use a QMatrix
    if ( mScaleByWidth )
    {
      mySize.setWidth( static_cast<int>( mImage->width() *( mScaleToFit + ( mScaleFactor * mCurrentZoomStep ) ) ) );
      mySize.setHeight( static_cast<int>(( double )mySize.width() * mImageSizeRatio ) );
    }
    else
    {
      mySize.setHeight( static_cast<int>( mImage->height() *( mScaleToFit + ( mScaleFactor * mCurrentZoomStep ) ) ) );
      mySize.setWidth( static_cast<int>(( double )mySize.height() * mImageSizeRatio ) );
    }
  }
  else
  {
    mySize.setWidth( mDisplayArea->size().width() );
    mySize.setHeight( mDisplayArea->size().height() );
    mImage->fill();
  }
  //the minus 4 is there to keep scroll bars from appearing at full extent view
  mImageLabel->resize( mySize.width() - 4, mySize.height() - 4 );
  mImageLabel->setPixmap( *mImage );
}

/**
* Public method called to display an image loaded from a url
* @param url - The url from which to load an image
*/
void eVisImageDisplayWidget::displayUrlImage( QString url )
{
  QUrl myUrl( url );
#if QT_VERSION < 0x050000
  mHttpConnection->setHost( myUrl.host() );
  mCurrentHttpImageRequestId = mHttpConnection->get( myUrl.path().replace( "\\", "/" ), mHttpBuffer );
#endif
}

/**
* Private method to set the scaling and zoom steps for each new image
*/
void eVisImageDisplayWidget::setScalers()
{
  if ( mImageLoaded )
  {
    double xRatio = ( double )mDisplayArea->size().width() / ( double )mImage->width();
    double yRatio = ( double )mDisplayArea->size().height() / ( double )mImage->height();
    if ( xRatio < yRatio )
    {
      mScaleByWidth = true;
      mScaleByHeight = false;
      mImageSizeRatio = ( double )mImage->height() / ( double )mImage->width();
      mScaleToFit = ( double )mDisplayArea->size().width() / ( double )mImage->width();
      mScaleFactor = ( 1.0 - mScaleToFit ) / ( double )ZOOM_STEPS;
    }
    else
    {
      mScaleByWidth = false;
      mScaleByHeight = true;
      mImageSizeRatio = ( double )mImage->width() / ( double )mImage->height();
      mScaleToFit = ( double )mDisplayArea->size().height() / ( double )mImage->height();
      mScaleFactor = ( 1.0 - mScaleToFit ) / ( double )ZOOM_STEPS;
    }
  }
}

/*
 *
 * Public and Private Slots
 *
 */
/**
* Slot called when a http request is complete
* @param requestId - The id of the http request
* @param error - Boolean denoting success of http request
*/
void eVisImageDisplayWidget::displayUrlImage( int requestId, bool error )
{
  //only process if no error and the request id matches the request id stored in displayUrlImage
  if ( !error && requestId == mCurrentHttpImageRequestId )
  {
    //reset to be beginning of the buffer
    mHttpBuffer->seek( 0 );
    //load the image data from the buffer
    mImageLoaded = mImage->loadFromData( mHttpBuffer->buffer() );

    mCurrentZoomStep = 0;
    pbtnZoomOut->setEnabled( false );
    pbtnZoomFull->setEnabled( false );
    if ( mImageLoaded )
    {
      pbtnZoomIn->setEnabled( true );
    }
    else
    {
      pbtnZoomIn->setEnabled( false );
    }
  }

  setScalers();

  displayImage();
}

/**
* Slot called when the pbtnZoomIn button is pressed
*/
void eVisImageDisplayWidget::on_pbtnZoomIn_clicked()
{
  if ( mCurrentZoomStep < ZOOM_STEPS )
  {
    pbtnZoomOut->setEnabled( true );
    pbtnZoomFull->setEnabled( true );
    mCurrentZoomStep++;
    displayImage();
  }

  if ( mCurrentZoomStep == ZOOM_STEPS )
  {
    pbtnZoomIn->setEnabled( false );
  }
}

/**
* Slot called when the pbtnZoomOut button is pressed
*/
void eVisImageDisplayWidget::on_pbtnZoomOut_clicked()
{
  if ( mCurrentZoomStep > 0 )
  {
    pbtnZoomIn->setEnabled( true );
    mCurrentZoomStep--;
    displayImage();
  }

  if ( mCurrentZoomStep == 0 )
  {
    pbtnZoomOut->setEnabled( false );
    pbtnZoomFull->setEnabled( false );
  }
}

/**
* Slot called when the pbtnZoomFull button is pressed
*/
void eVisImageDisplayWidget::on_pbtnZoomFull_clicked()
{
  pbtnZoomOut->setEnabled( false );
  pbtnZoomFull->setEnabled( false );
  pbtnZoomIn->setEnabled( true );
  mCurrentZoomStep = 0;
  displayImage();
}
