/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology-ng/qgsdatadefinedsizelegenddialog.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsDataDefinedSizeLegendDialog : QDialog
{
%Docstring
 Dialog for configuration of appearance of legend for marker symbols with data-defined size.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsdatadefinedsizelegenddialog.h"
%End
  public:
    explicit QgsDataDefinedSizeLegendDialog( const QgsDataDefinedSizeLegend *ddsLegend, QWidget *parent /TransferThis/ = 0 );
%Docstring
Creates the dialog and initializes the content to what is passed in the legend configuration (may be null)
%End
    ~QgsDataDefinedSizeLegendDialog();

    void setSourceSymbol( QgsMarkerSymbol *symbol /Transfer/ );
%Docstring
Use given symbol for preview. Takes ownership of the symbol. It should have data-defined size enabled + size scale transformer attached.
%End

    void setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale );
%Docstring
Setup map view details to make preview match the expected output
%End

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
 * src/gui/symbology-ng/qgsdatadefinedsizelegenddialog.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
