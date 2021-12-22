/***************************************************************************
    qgstablecell.h
    --------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTABLECELL_H
#define QGSTABLECELL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstextformat.h"
#include <QVariant>
#include <QColor>
#include <memory>

class QgsNumericFormat;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsTableCell
 * \brief Encapsulates the contents and formatting of a single table cell.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsTableCell
{

  public:

    /**
     * Constructor for QgsTableCell, with the specified \a content.
     */
    QgsTableCell( const QVariant &content = QVariant() );

    //! Copy constructor
    QgsTableCell( const QgsTableCell &other );

    ~QgsTableCell();

    QgsTableCell &operator=( const QgsTableCell &other );

    /**
     * Returns the cell's content.
     *
     * \see setContent()
     */
    QVariant content() const { return mContent; }

    /**
     * Sets the cell's \a content.
     *
     * \see content()
     */
    void setContent( const QVariant &content ) { mContent = content; }

    /**
     * Returns the cell's background color, or an invalid color if a default color should be used for the background.
     *
     * \see setBackgroundColor()
     */
    QColor backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the cell's background \a color.
     *
     * Set an invalid \a color if a default color should be used for the background.
     *
     * \see backgroundColor()
     */
    void setBackgroundColor( const QColor &color ) { mBackgroundColor = color; }

    /**
     * Returns the cell's foreground color, or an invalid color if a default color should be used for the foreground.
     *
     * \see setForegroundColor()
     */
    QColor foregroundColor() const { return mForegroundColor; }

    /**
     * Sets the cell's foreground \a color.
     *
     * Set an invalid \a color if a default color should be used for the foreground.
     *
     * \see foregroundColor()
     */
    void setForegroundColor( const QColor &color ) { mForegroundColor = color; }

    /**
     * Returns the cell's text format.
     *
     * \note The text format will only be used if hasTextFormat() returns TRUE.
     *
     * \see setTextFormat()
     * \since QGIS 3.16
     */
    QgsTextFormat textFormat() const { return mTextFormat; }

    /**
     * Sets the cell's text \a format.
     *
     * \note The text format will only be used if hasTextFormat() returns TRUE.
     *
     * \see textFormat()
     * \since QGIS 3.16
     */
    void setTextFormat( const QgsTextFormat &format ) { mTextFormat = format; }

    /**
     * Returns the numeric format used for numbers in the cell, or NULLPTR if no format is set.
     *
     * \see setNumericFormat()
     */
    const QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used for numbers in the cell, or NULLPTR if no specific format is set.
     *
     * Ownership of \a format is transferred to the cell.
     *
     * \see numericFormat()
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the horizontal alignment for text in the cell.
     *
     * \see setHorizontalAlignment()
     * \see verticalAlignment()
     * \since QGIS 3.16
     */
    Qt::Alignment horizontalAlignment() const;

    /**
     * Sets the horizontal \a alignment for text in the cell.
     *
     * \see horizontalAlignment()
     * \see setVerticalAlignment()
     * \since QGIS 3.16
     */
    void setHorizontalAlignment( Qt::Alignment alignment );

    /**
     * Returns the vertical alignment for text in the cell.
     *
     * \see setVerticalAlignment()
     * \see horizontalAlignment()
     * \since QGIS 3.16
     */
    Qt::Alignment verticalAlignment() const;

    /**
     * Sets the vertical \a alignment for text in the cell.
     *
     * \see verticalAlignment()
     * \see setHorizontalAlignment()
     * \since QGIS 3.16
     */
    void setVerticalAlignment( Qt::Alignment alignment );

    /**
     * Returns the properties of the cell.
     *
     * \see setProperties()
     */
    QVariantMap properties( const QgsReadWriteContext &context ) const;

    /**
     * Sets the \a properties for the cell.
     *
     * \see properties()
     */
    void setProperties( const QVariantMap &properties, const QgsReadWriteContext &context );


#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsTableCell: %1>" ).arg( sipCpp->content().toString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:

    QVariant mContent;
    QColor mBackgroundColor;
    QColor mForegroundColor;
    QgsTextFormat mTextFormat;
    std::unique_ptr< QgsNumericFormat > mFormat;

    Qt::Alignment mHAlign = Qt::AlignLeft;
    Qt::Alignment mVAlign = Qt::AlignVCenter;

};

/**
 * A row of table cells
 *
 * \since QGIS 3.12
 */
typedef QVector<QgsTableCell> QgsTableRow;

#ifndef SIP_RUN

/**
 * A set of table rows.
 *
 * \since QGIS 3.12
 */
typedef QVector<QgsTableRow> QgsTableContents;
#else

/**
 * A set of table rows.
 *
 * \since QGIS 3.12
 */
typedef QVector<QVector<QgsTableCell>> QgsTableContents;
#endif


#endif // QGSTABLECELL_H
