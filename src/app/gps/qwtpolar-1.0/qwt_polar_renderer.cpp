/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_renderer.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_layout.h"
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_dyngrid_layout.h>
#include <qwt_text_label.h>
#include <qwt_text.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qimagewriter.h>
#include <qfileinfo.h>
#include <qmath.h>
#ifndef QWT_NO_POLAR_POLAR_SVG
#ifdef QT_SVG_LIB
#include <qsvggenerator.h>
#endif
#endif


static inline double qwtDistance(
    const QPointF &p1, const QPointF &p2 )
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return qSqrt( dx * dx + dy * dy );
}

class QwtPolarRenderer::PrivateData
{
public:
    PrivateData():
        plot( NULL )
    {
    }

    QwtPolarPlot *plot;
};

/*!
  Constructor
  \param parent Parent object
 */
QwtPolarRenderer::QwtPolarRenderer( QObject *parent ):
    QObject( parent )
{
    d_data = new PrivateData;
}

//! Destructor
QwtPolarRenderer::~QwtPolarRenderer()
{
    delete d_data;
}

/*!
  Render a polar plot to a file

  The format of the document will be autodetected from the
  suffix of the filename.

  \param plot Plot widget
  \param fileName Path of the file, where the document will be stored
  \param sizeMM Size for the document in millimeters.
  \param resolution Resolution in dots per Inch (dpi)
*/
void QwtPolarRenderer::renderDocument( QwtPolarPlot *plot,
    const QString &fileName, const QSizeF &sizeMM, int resolution )
{
    renderDocument( plot, fileName,
        QFileInfo( fileName ).suffix(), sizeMM, resolution );
}

/*!
  Render a plot to a file

  Supported formats are:

  - pdf\n
  - ps\n
  - svg\n
  - all image formats supported by Qt, see QImageWriter::supportedImageFormats()

  \param plot Plot widget
  \param fileName Path of the file, where the document will be stored
  \param format Format for the document
  \param sizeMM Size for the document in millimeters.
  \param resolution Resolution in dots per Inch (dpi)

  \sa renderTo(), render(), QwtPainter::setRoundingAlignment()
*/
void QwtPolarRenderer::renderDocument( QwtPolarPlot *plot,
    const QString &fileName, const QString &format,
    const QSizeF &sizeMM, int resolution )
{
    if ( plot == NULL || sizeMM.isEmpty() || resolution <= 0 )
        return;

    QString title = plot->title().text();
    if ( title.isEmpty() )
        title = "Plot Document";

    const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;

    const QRectF documentRect( 0.0, 0.0, size.width(), size.height() );

    const QString fmt = format.toLower();
    if ( format == "pdf" || format == "ps" )
    {
        QPrinter printer;
        printer.setFullPage( true );
        printer.setPaperSize( sizeMM, QPrinter::Millimeter );
        printer.setDocName( title );
        printer.setOutputFileName( fileName );
        printer.setOutputFormat( ( format == "pdf" )
            ? QPrinter::PdfFormat : QPrinter::PostScriptFormat );
        printer.setResolution( resolution );

        QPainter painter( &printer );
        render( plot, &painter, documentRect );
    }
#ifndef QWT_NO_POLAR_SVG
#ifdef QT_SVG_LIB
#if QT_VERSION >= 0x040500
    else if ( format == "svg" )
    {
        QSvgGenerator generator;
        generator.setTitle( title );
        generator.setFileName( fileName );
        generator.setResolution( resolution );
        generator.setViewBox( documentRect );

        QPainter painter( &generator );
        render( plot, &painter, documentRect );
    }
#endif
#endif
#endif
    else
    {
        if ( QImageWriter::supportedImageFormats().indexOf(
            format.toLatin1() ) >= 0 )
        {
            const QRect imageRect = documentRect.toRect();
            const int dotsPerMeter = qRound( resolution * mmToInch * 1000.0 );

            QImage image( imageRect.size(), QImage::Format_ARGB32 );
            image.setDotsPerMeterX( dotsPerMeter );
            image.setDotsPerMeterY( dotsPerMeter );
            image.fill( QColor( Qt::white ).rgb() );

            QPainter painter( &image );
            render( plot, &painter, imageRect );
            painter.end();

            image.save( fileName, format.toLatin1() );
        }
    }
}

/*!
  \brief Render the plot to a \c QPaintDevice

  This function renders the contents of a QwtPolarPlot instance to
  \c QPaintDevice object. The target rectangle is derived from
  its device metrics.

  \param plot Plot to be rendered
  \param paintDevice device to paint on, f.e a QImage

  \sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
*/

void QwtPolarRenderer::renderTo(
    QwtPolarPlot *plot, QPaintDevice &paintDevice ) const
{
    int w = paintDevice.width();
    int h = paintDevice.height();

    QPainter p( &paintDevice );
    render( plot, &p, QRectF( 0, 0, w, h ) );
}


/*!
  \brief Render the plot to a QPrinter

  This function renders the contents of a QwtPolarPlot instance to
  \c QPaintDevice object. The size is derived from the printer
  metrics.

  \param plot Plot to be rendered
  \param printer Printer to paint on

  \sa renderDocument(), render(), QwtPainter::setRoundingAlignment()
*/

void QwtPolarRenderer::renderTo(
    QwtPolarPlot *plot, QPrinter &printer ) const
{
    int w = printer.width();
    int h = printer.height();

    QRectF rect( 0, 0, w, h );
    double aspect = rect.width() / rect.height();
    if ( ( aspect < 1.0 ) )
        rect.setHeight( aspect * rect.width() );

    QPainter p( &printer );
    render( plot, &p, rect );
}

#ifndef QWT_NO_POLAR_SVG
#ifdef QT_SVG_LIB
#if QT_VERSION >= 0x040500

/*!
  \brief Render the plot to a QSvgGenerator

  If the generator has a view box, the plot will be rendered into it.
  If it has no viewBox but a valid size the target coordinates
  will be (0, 0, generator.width(), generator.height()). Otherwise
  the target rectangle will be QRectF(0, 0, 800, 600);

  \param plot Plot to be rendered
  \param generator SVG generator
*/
void QwtPolarRenderer::renderTo(
    QwtPolarPlot *plot, QSvgGenerator &generator ) const
{
    QRectF rect = generator.viewBoxF();
    if ( rect.isEmpty() )
        rect.setRect( 0, 0, generator.width(), generator.height() );

    if ( rect.isEmpty() )
        rect.setRect( 0, 0, 800, 600 ); // something

    QPainter p( &generator );
    render( plot, &p, rect );
}
#endif
#endif
#endif

/*!
   \brief Render the plot to a given rectangle ( f.e QPrinter, QSvgRenderer )

   \param plot Plot widget to be rendered
   \param painter Painter
   \param plotRect Bounding rectangle for the plot
*/
void QwtPolarRenderer::render( QwtPolarPlot *plot,
    QPainter *painter, const QRectF &plotRect ) const
{
    if ( plot == NULL || painter == NULL || !painter->isActive() ||
        !plotRect.isValid() || plot->size().isNull() )
    {
        return;
    }

    d_data->plot = plot;

    /*
      The layout engine uses the same methods as they are used
      by the Qt layout system. Therefore we need to calculate the
      layout in screen coordinates and paint with a scaled painter.
     */
    QTransform transform;
    transform.scale(
        double( painter->device()->logicalDpiX() ) / plot->logicalDpiX(),
        double( painter->device()->logicalDpiY() ) / plot->logicalDpiY() );

    const QRectF layoutRect = transform.inverted().mapRect( plotRect );

    QwtPolarLayout *layout = plot->plotLayout();

    // All paint operations need to be scaled according to
    // the paint device metrics.

    QwtPolarLayout::Options layoutOptions =
        QwtPolarLayout::IgnoreScrollbars | QwtPolarLayout::IgnoreFrames;

    layout->activate( plot, layoutRect, layoutOptions );

    painter->save();
    painter->setWorldTransform( transform, true );

    painter->save();
    renderTitle( painter, layout->titleRect() );
    painter->restore();

    painter->save();
    renderLegend( painter, layout->legendRect() );
    painter->restore();

    const QRectF &canvasRect = layout->canvasRect();

    painter->save();
    painter->setClipRect( canvasRect );
    plot->drawCanvas( painter, canvasRect );
    painter->restore();

    painter->restore();

    layout->invalidate();

    d_data->plot = NULL;
}

/*!
  Render the title into a given rectangle.

  \param painter Painter
  \param rect Bounding rectangle
*/

void QwtPolarRenderer::renderTitle( QPainter *painter, const QRectF &rect ) const
{
    QwtTextLabel *title = d_data->plot->titleLabel();

    painter->setFont( title->font() );

    const QColor color = title->palette().color(
        QPalette::Active, QPalette::Text );

    painter->setPen( color );
    title->text().draw( painter, rect );
}

/*!
  Render the legend into a given rectangle.

  \param painter Painter
  \param rect Bounding rectangle
*/

void QwtPolarRenderer::renderLegend(
    QPainter *painter, const QRectF &rect ) const
{
    QwtLegend *legend = d_data->plot->legend();
    if ( legend == NULL || legend->isEmpty() )
        return;

    const QwtDynGridLayout *legendLayout = qobject_cast<QwtDynGridLayout *>(
        legend->contentsWidget()->layout() );
    if ( legendLayout == NULL )
        return;

    uint numCols = legendLayout->columnsForWidth( rect.width() );
    const QList<QRect> itemRects =
        legendLayout->layoutItems( rect.toRect(), numCols );

    int index = 0;

    for ( int i = 0; i < legendLayout->count(); i++ )
    {
        QLayoutItem *item = legendLayout->itemAt( i );
        QWidget *w = item->widget();
        if ( w )
        {
            painter->save();

            painter->setClipRect( itemRects[index] );
            renderLegendItem( painter, w, itemRects[index] );

            index++;
            painter->restore();
        }
    }

}

/*!
  Print the legend item into a given rectangle.

  \param painter Painter
  \param widget Widget representing a legend item
  \param rect Bounding rectangle

  \note When widget is not derived from QwtLegendItem renderLegendItem
        does nothing and needs to be overloaded
*/
void QwtPolarRenderer::renderLegendItem( QPainter *painter,
    const QWidget *widget, const QRectF &rect ) const
{
    const QwtLegendItem *item = qobject_cast<const QwtLegendItem *>( widget );
    if ( item )
    {
        const QSize sz = item->identifierSize();

        const QRectF identifierRect( rect.x() + item->margin(),
            rect.center().y() - 0.5 * sz.height(), sz.width(), sz.height() );

        QwtLegendItemManager *itemManger = d_data->plot->legend()->find( item );
        if ( itemManger )
        {
            painter->save();
            painter->setClipRect( identifierRect, Qt::IntersectClip );
            itemManger->drawLegendIdentifier( painter, identifierRect );
            painter->restore();
        }

        // Label

        QRectF titleRect = rect;
        titleRect.setX( identifierRect.right() + 2 * item->spacing() );

        painter->setFont( item->font() );
        item->text().draw( painter, titleRect );
    }
}

