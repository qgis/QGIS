/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_renderer.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_layout.h"
#include "qwt_legend.h"
#include "qwt_dyngrid_layout.h"
#include "qwt_text_label.h"
#include "qwt_text.h"

#include <qpainter.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qfiledialog.h>
#include <qimagewriter.h>
#include <qfileinfo.h>
#include <qmath.h>

#ifndef QWT_NO_SVG
#ifdef QT_SVG_LIB
#define QWT_FORMAT_SVG 1
#endif
#endif

#ifndef QT_NO_PRINTER
#define QWT_FORMAT_PDF 1
#endif

#ifndef QT_NO_PDF

// QPdfWriter::setResolution() has been introduced with
// Qt 5.3. Guess it is o.k. to stay with QPrinter for older
// versions.

#if QT_VERSION >= 0x050300

#ifndef QWT_FORMAT_PDF
#define QWT_FORMAT_PDF 1
#endif

#define QWT_PDF_WRITER 1

#endif
#endif

#ifndef QT_NO_PRINTER
// postscript support has been dropped in Qt5
#if QT_VERSION < 0x050000
#define QWT_FORMAT_POSTSCRIPT 1
#endif
#endif

#if QWT_FORMAT_SVG
#include <qsvggenerator.h>
#endif

#if QWT_PDF_WRITER
#include <qpdfwriter.h>
#endif

static inline double qwtDistance(
    const QPointF& p1, const QPointF& p2 )
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return qSqrt( dx * dx + dy * dy );
}

class QwtPolarRenderer::PrivateData
{
  public:
    PrivateData()
        : plot( NULL )
    {
    }

    QwtPolarPlot* plot;
};

/*!
   Constructor
   \param parent Parent object
 */
QwtPolarRenderer::QwtPolarRenderer( QObject* parent )
    : QObject( parent )
{
    m_data = new PrivateData;
}

//! Destructor
QwtPolarRenderer::~QwtPolarRenderer()
{
    delete m_data;
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
void QwtPolarRenderer::renderDocument( QwtPolarPlot* plot,
    const QString& fileName, const QSizeF& sizeMM, int resolution )
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
void QwtPolarRenderer::renderDocument( QwtPolarPlot* plot,
    const QString& fileName, const QString& format,
    const QSizeF& sizeMM, int resolution )
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
    if ( format == "pdf" )
    {
#if QWT_FORMAT_PDF
#if QWT_PDF_WRITER
        QPdfWriter pdfWriter( fileName );
        pdfWriter.setPageSize( QPageSize( sizeMM, QPageSize::Millimeter ) );
        pdfWriter.setTitle( title );
        pdfWriter.setPageMargins( QMarginsF() );
        pdfWriter.setResolution( resolution );

        QPainter painter( &pdfWriter );
        render( plot, &painter, documentRect );

#else
        QPrinter printer;
        printer.setOutputFormat( QPrinter::PdfFormat );
        printer.setColorMode( QPrinter::Color );
        printer.setFullPage( true );
        printer.setPaperSize( sizeMM, QPrinter::Millimeter );
        printer.setDocName( title );
        printer.setOutputFileName( fileName );
        printer.setResolution( resolution );

        QPainter painter( &printer );
        render( plot, &painter, documentRect );
#endif
#endif
    }
    else if ( format == "ps" )
    {
#if QWT_FORMAT_POSTSCRIPT
        QPrinter printer;
        printer.setColorMode( QPrinter::Color );
        printer.setFullPage( true );
        printer.setPaperSize( sizeMM, QPrinter::Millimeter );
        printer.setDocName( title );
        printer.setOutputFileName( fileName );
        printer.setOutputFormat( QPrinter::PostScriptFormat );
        printer.setResolution( resolution );

        QPainter painter( &printer );
        render( plot, &painter, documentRect );
#endif
    }
    else if ( format == "svg" )
    {
#ifdef QWT_FORMAT_SVG
        QSvgGenerator generator;
        generator.setTitle( title );
        generator.setFileName( fileName );
        generator.setResolution( resolution );
        generator.setViewBox( documentRect );

        QPainter painter( &generator );
        render( plot, &painter, documentRect );
#endif
    }
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
    QwtPolarPlot* plot, QPaintDevice& paintDevice ) const
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

#ifndef QT_NO_PRINTER

void QwtPolarRenderer::renderTo(
    QwtPolarPlot* plot, QPrinter& printer ) const
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

#endif

#ifdef QWT_FORMAT_SVG

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
    QwtPolarPlot* plot, QSvgGenerator& generator ) const
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

/*!
   \brief Render the plot to a given rectangle ( f.e QPrinter, QSvgRenderer )

   \param plot Plot widget to be rendered
   \param painter Painter
   \param plotRect Bounding rectangle for the plot
 */
void QwtPolarRenderer::render( QwtPolarPlot* plot,
    QPainter* painter, const QRectF& plotRect ) const
{
    if ( plot == NULL || painter == NULL || !painter->isActive() ||
        !plotRect.isValid() || plot->size().isNull() )
    {
        return;
    }

    m_data->plot = plot;

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

    QwtPolarLayout* layout = plot->plotLayout();

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
    renderLegend( plot, painter, layout->legendRect() );
    painter->restore();

    const QRectF canvasRect = layout->canvasRect();

    painter->save();
    painter->setClipRect( canvasRect );
    plot->drawCanvas( painter, canvasRect );
    painter->restore();

    painter->restore();

    layout->invalidate();

    m_data->plot = NULL;
}

/*!
   Render the title into a given rectangle.

   \param painter Painter
   \param rect Bounding rectangle
 */

void QwtPolarRenderer::renderTitle( QPainter* painter, const QRectF& rect ) const
{
    QwtTextLabel* title = m_data->plot->titleLabel();

    painter->setFont( title->font() );

    const QColor color = title->palette().color(
        QPalette::Active, QPalette::Text );

    painter->setPen( color );
    title->text().draw( painter, rect );
}

/*!
   Render the legend into a given rectangle.

   \param plot Plot widget
   \param painter Painter
   \param rect Bounding rectangle
 */
void QwtPolarRenderer::renderLegend( const QwtPolarPlot* plot,
    QPainter* painter, const QRectF& rect ) const
{
    if ( plot->legend() )
        plot->legend()->renderLegend( painter, rect, true );
}

/*!
   \brief Execute a file dialog and render the plot to the selected file

   The document will be rendered in 85 dpi for a size 30x30 cm

   \param plot Plot widget
   \param documentName Default document name
   \param sizeMM Size for the document in millimeters.
   \param resolution Resolution in dots per Inch (dpi)

   \sa renderDocument()
 */
bool QwtPolarRenderer::exportTo( QwtPolarPlot* plot,
    const QString& documentName, const QSizeF& sizeMM, int resolution )
{
    if ( plot == NULL )
        return false;

    QString fileName = documentName;

    // What about translation

#ifndef QT_NO_FILEDIALOG
    const QList< QByteArray > imageFormats =
        QImageWriter::supportedImageFormats();

    QStringList filter;
#ifndef QT_NO_PRINTER
    filter += QString( "PDF " ) + tr( "Documents" ) + " (*.pdf)";
#endif
#ifndef QWT_NO_SVG
    filter += QString( "SVG " ) + tr( "Documents" ) + " (*.svg)";
#endif
#ifndef QT_NO_PRINTER
    filter += QString( "Postscript " ) + tr( "Documents" ) + " (*.ps)";
#endif

    if ( imageFormats.size() > 0 )
    {
        QString imageFilter( tr( "Images" ) );
        imageFilter += " (";
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(
        NULL, tr( "Export File Name" ), fileName,
        filter.join( ";;" ), NULL, QFileDialog::DontConfirmOverwrite );
#endif
    if ( fileName.isEmpty() )
        return false;

    renderDocument( plot, fileName, sizeMM, resolution );

    return true;
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_polar_renderer.cpp"
#endif
