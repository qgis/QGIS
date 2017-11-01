/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterhistogramwidget.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsRasterHistogramWidget : QgsMapLayerConfigWidget
{
%Docstring
 Histogram widget
%End

%TypeHeaderCode
#include "qgsrasterhistogramwidget.h"
%End
  public:
    QgsRasterHistogramWidget( QgsRasterLayer *lyr, QWidget *parent /TransferThis/ = 0 );

    bool histoSaveAsImage( const QString &filename, int width = 600, int height = 600, int quality = -1 );
%Docstring
Save the histogram as an image to disk
 :rtype: bool
%End

    void setRendererWidget( const QString &name, QgsRasterRendererWidget *rendererWidget = 0 );
%Docstring
Set the renderer widget (or just its name if there is no widget)
%End

    void setActive( bool activeFlag );
%Docstring
Activate the histogram widget
%End

    bool computeHistogram( bool forceComputeFlag );
%Docstring
 Compute the histogram on demand.
 :rtype: bool
%End

    void histoAction( const QString &actionName, bool actionFlag = true );
%Docstring
Apply a histoActionTriggered() event.
%End

    void setSelectedBand( int index );
%Docstring
Apply a histoActionTriggered() event.
%End

  public slots:
    void refreshHistogram();
%Docstring
 slot executed when user wishes to refresh raster histogramwidget
%End

    virtual void apply();


};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/raster/qgsrasterhistogramwidget.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
