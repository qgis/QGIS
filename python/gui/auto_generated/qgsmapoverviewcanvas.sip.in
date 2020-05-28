/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmapoverviewcanvas.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMapOverviewCanvas : QWidget
{
%Docstring
A widget that displays an overview map.
%End

%TypeHeaderCode
#include "qgsmapoverviewcanvas.h"
%End
  public:
    QgsMapOverviewCanvas( QWidget *parent /TransferThis/ = 0, QgsMapCanvas *mapCanvas = 0 );

    void refresh();
%Docstring
renders overview and updates panning widget
%End

    void setBackgroundColor( const QColor &color );
%Docstring
changes background color
%End

    void setLayers( const QList<QgsMapLayer *> &layers );
%Docstring
updates layer set for overview
%End

    QList<QgsMapLayer *> layers() const;
%Docstring
Returns list of layers visible in the overview
%End

    void enableAntiAliasing( bool flag );

    void updateFullExtent();

  protected slots:
    void mapRenderingFinished();

    void layerRepaintRequested( bool deferred = false );
%Docstring
Triggered when a layer in the overview requests a repaint.
%End

  protected:

    void drawExtentRect();
%Docstring
used for overview canvas to reflect changed extent in main map canvas
%End

    void destinationCrsChanged();
%Docstring
Should be called when the canvas destination CRS is changed
%End

    void transformContextChanged();
%Docstring
Called when the canvas transform context is changed
%End

    virtual void paintEvent( QPaintEvent *pe );

%Docstring
Overridden paint event
%End

    virtual void showEvent( QShowEvent *e );

%Docstring
Overridden show event
%End

    virtual void resizeEvent( QResizeEvent *e );

%Docstring
Overridden resize event
%End

    virtual void mouseMoveEvent( QMouseEvent *e );

%Docstring
Overridden mouse move event
%End

    virtual void mousePressEvent( QMouseEvent *e );

%Docstring
Overridden mouse press event
%End

    virtual void mouseReleaseEvent( QMouseEvent *e );

%Docstring
Overridden mouse release event
%End

    void updatePanningWidget( QPoint pos );
%Docstring
called when panning to reflect mouse movement
%End






};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmapoverviewcanvas.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
