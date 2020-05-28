/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsgraduatedhistogramwidget.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGraduatedHistogramWidget : QgsHistogramWidget
{
%Docstring
Graphical histogram for displaying distribution of field values and
editing range breaks for a QgsGraduatedSymbolRenderer renderer.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgsgraduatedhistogramwidget.h"
%End
  public:

    QgsGraduatedHistogramWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
QgsGraduatedHistogramWidget constructor

:param parent: parent widget
%End

    void setRenderer( QgsGraduatedSymbolRenderer *renderer );
%Docstring
Sets the QgsGraduatedSymbolRenderer renderer associated with the histogram.
The histogram will fetch the ranges from the renderer before every refresh.

:param renderer: associated :py:class:`QgsGraduatedSymbolRenderer`
%End

  signals:

    void rangesModified( bool rangesAdded );
%Docstring
Emitted when the user modifies the graduated ranges using the histogram widget.

:param rangesAdded: ``True`` if the user has added ranges, ``False`` if the user has just
                    modified existing range breaks
%End

  protected:

    virtual void drawHistogram();


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsgraduatedhistogramwidget.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
