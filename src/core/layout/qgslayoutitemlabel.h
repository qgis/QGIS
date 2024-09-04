/***************************************************************************
                             qgslayoutitemlabel.h
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTITEMLABEL_H
#define QGSLAYOUTITEMLABEL_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgstextformat.h"
#include <QFont>
#include <QUrl>

class QgsVectorLayer;
class QgsFeature;
class QgsDistanceArea;

/**
 * \ingroup core
 * \brief A layout item subclass for text labels.
 */
class CORE_EXPORT QgsLayoutItemLabel: public QgsLayoutItem
{
    Q_OBJECT

  public:

    //! Label modes
    enum Mode
    {
      ModeFont, //!< Label displays text rendered using a single font
      ModeHtml, //!< Label displays rendered HTML content
    };

    /**
     * Constructor for QgsLayoutItemLabel, with the specified parent \a layout.
     */
    QgsLayoutItemLabel( QgsLayout *layout );

    /**
     * Returns a new label item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemLabel *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QIcon icon() const override;
    //Overridden to contain part of label's text
    QString displayName() const override;

    /**
     * Resizes the item so that the label's text fits to the item. Keeps the top left point stationary.
     * \see sizeForText()
     */
    void adjustSizeToText();

    /**
     * Returns the required item size (in layout units) for the label's text to fill the item.
     * \see adjustSizeToText()
     */
    QSizeF sizeForText() const;

    /**
     * Returns the label's preset text.
     * \see currentText()
     * \see setText()
     */
    QString text() const { return mText; }

    /**
     * Sets the label's preset \a text.
     * \see text()
     */
    void setText( const QString &text );

    /**
     * Returns the text as it appears on the label (with evaluated expressions
     * and other dynamic content).
     * \see text()
     */
    QString currentText() const;

    /**
     * Returns the label's current mode.
     * \see setMode()
     */
    Mode mode() const { return mMode; }

    /**
     * Sets the label's current \a mode, allowing the label
     * to switch between font based and HTML based rendering.
     * \see mode()
     */
    void setMode( Mode mode );

    /**
     * Returns the label's current font.
     * \see setFont()
     * \deprecated QGIS 3.40. Use textFormat() instead (since QGIS 3.24).
     */
    Q_DECL_DEPRECATED QFont font() const SIP_DEPRECATED;

    /**
     * Sets the label's current \a font.
     * \see font()
     * \deprecated QGIS 3.40. Use setTextFormat() instead (since QGIS 3.24).
     */
    Q_DECL_DEPRECATED void setFont( const QFont &font ) SIP_DEPRECATED;

    /**
     * Returns for the vertical alignment of the label.
     * \see setVAlign()
     * \see hAlign()
     */
    Qt::AlignmentFlag vAlign() const { return mVAlignment; }

    /**
     * Returns the horizontal alignment of the label.
     * \see vAlign()
     * \see setHAlign()
     */
    Qt::AlignmentFlag hAlign() const { return mHAlignment; }

    /**
     * Sets the horizontal \a alignment of the label.
     * \see hAlign()
     * \see setVAlign()
     */
    void setHAlign( Qt::AlignmentFlag alignment ) { mHAlignment = alignment; invalidateCache(); }

    /**
     * Sets for the vertical \a alignment of the label.
     * \see vAlign()
     * \see setHAlign()
     */
    void setVAlign( Qt::AlignmentFlag alignment ) { mVAlignment = alignment; invalidateCache(); }

    /**
     * Returns the horizontal margin between the edge of the frame and the label
     * contents, in layout units.
     * \see setMargin()
     * \see marginY()
     */
    double marginX() const { return mMarginX; }

    /**
     * Returns the vertical margin between the edge of the frame and the label
     * contents, in layout units.
     * \see setMargin()
     * \see marginX()
     */
    double marginY() const { return mMarginY; }

    /**
     * Sets the \a margin between the edge of the frame and the label contents.
     * This method sets both the horizontal and vertical margins to the same
     * value. The margins can be individually controlled using the setMarginX()
     * and setMarginY() methods.
     *
     * Margins are set using the current layout units.

     * \see setMarginX()
     * \see setMarginY()
     */
    void setMargin( double margin );

    /**
     * Sets the horizontal \a margin between the edge of the frame and the label
     * contents, in layout units.
     * \see setMargin()
     * \see setMarginY()
     */
    void setMarginX( double margin );

    /**
     * Sets the vertical \a margin between the edge of the frame and the label
     * contents, in layout units.
     * \see setMargin()
     * \see setMarginX()
     */
    void setMarginY( double margin );

    /**
     * Sets the label font \a color.
     * \see fontColor()
     * \deprecated QGIS 3.40. Use setTextFormat() instead (since QGIS 3.24).
     */
    Q_DECL_DEPRECATED void setFontColor( const QColor &color ) SIP_DEPRECATED { mFormat.setColor( color ); }

    /**
     * Returns the label font color.
     * \see setFontColor()
     * \deprecated QGIS 3.40. Use textFormat() instead (since QGIS 3.24).
     */
    Q_DECL_DEPRECATED QColor fontColor() const SIP_DEPRECATED { return mFormat.color(); }

    // In case of negative margins, the bounding rect may be larger than the
    // label's frame
    QRectF boundingRect() const override;
    void setFrameEnabled( bool drawFrame ) override;
    void setFrameStrokeWidth( QgsLayoutMeasurement strokeWidth ) override;

    /**
     * Returns the text format used for drawing text in the label.
     * \see setTextFormat()
     * \since QGIS 3.24
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the text \a format used for drawing text in the label.
     * \see textFormat()
     * \since QGIS 3.24
     */
    void setTextFormat( const QgsTextFormat &format );

  public slots:

    void refresh() override;

    /**
     * Converts the label's text() to a static string, by evaluating any expressions included in the text
     * and replacing them with their current values.
     *
     * \since QGIS 3.20
     */
    void convertToStaticText();

  protected:
    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private slots:

    void refreshExpressionContext();
    //! Updates the bounding rect of this item
    void updateBoundingRect();

  private:
    bool mFirstRender = true;

    // Text
    QString mText;

    Mode mMode = ModeFont;
    double mHtmlUnitsToLayoutUnits = 1.0;
    double htmlUnitsToLayoutUnits(); //calculate scale factor

    //! Helper function to calculate x/y shift for adjustSizeToText() depending on rotation, current size and alignment
    void itemShiftAdjustSize( double newWidth, double newHeight, double &xShift, double &yShift ) const;

    //! Called when the content is changed to handle HTML loading
    void contentChanged();

    QgsTextFormat mFormat;

    //! Horizontal margin between contents and frame (in mm)
    double mMarginX = 0.0;
    //! Vertical margin between contents and frame (in mm)
    double mMarginY = 0.0;

    //! Horizontal Alignment
    Qt::AlignmentFlag mHAlignment = Qt::AlignLeft;

    //! Vertical Alignment
    Qt::AlignmentFlag mVAlignment = Qt::AlignTop;

    //! Replaces replace '$CURRENT_DATE<(FORMAT)>' with the current date (e.g. $CURRENT_DATE(d 'June' yyyy)
    void replaceDateText( QString &text ) const;

    //! Creates the default font used when rendering labels in HTML mode
    QFont createDefaultFont() const;

    //! Creates an encoded stylesheet url using the current font and label appearance settings
    QUrl createStylesheetUrl() const;

    //! Creates a stylesheet string using the current font and label appearance settings
    QString createStylesheet() const;

    std::unique_ptr< QgsDistanceArea > mDistanceArea;

    QRectF mCurrentRectangle;

    friend class QgsLayoutItemHtml;
};

#endif //QGSLAYOUTITEMLABEL_H
