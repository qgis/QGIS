/*
** File: evisimagedisplaywidget.h
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
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/
/*  $Id$ */
#ifndef EVISIMAGEDISPLAYWIDGET_H
#define EVISIMAGEDISPLAYWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QScrollArea>
#include <QPushButton>
#include <QBuffer>
#include <QHttp>
#include <QResizeEvent>

/**
* \class eVisGenericEventBrowserGui
* \brief Generic viewer for browsing event
* The eVisImageDisplayWidget is a component of the eVisGenericEventBrowser. This widget provides
* the ability to display an image on the widget and basic zoom capabilities. This class was created
* so the same display features could be easily added to other widgets as needed.
*/
class eVisImageDisplayWidget : public QWidget
{

    Q_OBJECT

  public:
    /** \brief Constructor */
    eVisImageDisplayWidget( QWidget* parent = 0, Qt::WFlags fl = 0 );

    /** \brief Destructor */
    ~eVisImageDisplayWidget();

    /** \brief Load an image from disk and display */
    void displayImage( QString );

    /** \brief Load an image from a remote location using http and display */
    void displayUrlImage( QString );

    /*
     * There needs to be more logic around setting the zoom steps as you could change it mid display
     * and end up getting not being able to zoom in or out
     */
    /** \brief Accessor for ZOOM_STEPS */
    int getZoomSteps( ) { return ZOOM_STEPS; }

    /** \brief Mutator for ZOON_STEPS */
    void setZoomSteps( int steps ) { ZOOM_STEPS = steps; }

  protected:
    void resizeEvent( QResizeEvent *event );

  private:

    /** \brief Used to hold the http request to match the correct emits with the correct result */
    int mCurrentHttpImageRequestId;

    /** \brief CUrrent Zoom level */
    int mCurrentZoomStep;

    /** \brief widget to display the image in */
    QScrollArea* mDisplayArea;

    /** \brief Method that acually display the image in the widget */
    void displayImage( );

    /** \brief Pointer to the http buffer */
    QBuffer* mHttpBuffer;

    /** \brief Pointer to the http connection if needed */
    QHttp* mHttpConnection;

    /** \brief This is a point to the actual image being displayed */
    QPixmap* mImage;

    /** \brief Label to hold the image */
    QLabel* mImageLabel;

    /** \brief Flag to indicate the success of the last load request */
    bool mImageLoaded;

    /** \brief Ratio if height to width or width to height for the original image, which ever is smaller */
    double mImageSizeRatio;

    /** \brief Boolean to indicate which feature the mImageSizeRation corresponds to */
    bool mScaleByHeight;

    /** \brief Boolean to indicate which feature the mImageSizeRation corresponds to */
    bool mScaleByWidth;

    /** \brief The increment by which the image is scaled during each scaling event */
    double mScaleFactor;

    /** \brief The single factor by which the original image needs to be scaled to fit into current display area */
    double mScaleToFit;

    /** \brief Zoom in button */
    QPushButton* pbtnZoomIn;

    /** \brief Zoom out button */
    QPushButton* pbtnZoomOut;

    /** \brief Zoom to full extent button */
    QPushButton* pbtnZoomFull;

    /** \brief Method called to compute the various scaling parameters */
    void setScalers( );

    /** \brief The number of steps between the scale to fit image and full resolution */
    int ZOOM_STEPS;

  private slots:
    void on_pbtnZoomIn_clicked( );

    void on_pbtnZoomOut_clicked( );

    void on_pbtnZoomFull_clicked( );

    /** \brief Slot called when the http request is completed */
    void displayUrlImage( int, bool );
};
#endif
