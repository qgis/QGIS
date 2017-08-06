/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsdatadefinedsizelegendwidget.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsDataDefinedSizeLegendWidget : QgsPanelWidget
{
%Docstring
 Widget for configuration of appearance of legend for marker symbols with data-defined size.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsdatadefinedsizelegendwidget.h"
%End
  public:
    explicit QgsDataDefinedSizeLegendWidget( const QgsDataDefinedSizeLegend *ddsLegend, const QgsProperty &ddSize, QgsMarkerSymbol *overrideSymbol /Transfer/, QgsMapCanvas *canvas = 0, QWidget *parent /TransferThis/ = 0 );
%Docstring
when the symbol is given from outside rather than being set inside QgsDataDefinedSizeLegend.
%End
    ~QgsDataDefinedSizeLegendWidget();

    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const /Factory/;
%Docstring
Returns configuration as set up in the dialog (may be null). Ownership is passed to the caller.
 :rtype: QgsDataDefinedSizeLegend
%End

  signals:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsdatadefinedsizelegendwidget.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
