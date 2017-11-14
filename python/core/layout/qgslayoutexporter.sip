/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutexporter.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutExporter
{
%Docstring
 Handles rendering and exports of layouts to various formats.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutexporter.h"
%End
  public:

    QgsLayoutExporter( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutExporter, for the specified ``layout``.
%End

    void renderPage( QPainter *painter, int page );
%Docstring
 Renders a full page to a destination ``painter``.

 The ``page`` argument specifies the page number to render. Page numbers
 are 0 based, such that the first page in a layout is page 0.

.. seealso:: renderRect()
%End

    void renderRegion( QPainter *painter, const QRectF &region );
%Docstring
 Renders a ``region`` from the layout to a ``painter``. This method can be used
 to render sections of pages rather than full pages.

.. seealso:: renderPage()
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutexporter.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
