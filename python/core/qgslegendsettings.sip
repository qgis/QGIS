
/**
 * @brief The QgsLegendSettings class stores the appearance and layout settings
 * for legend drawing with QgsLegendRenderer. The content of the legend is given
 * in QgsLegendModel class.
 *
 * @note added in 2.6
 */
class QgsLegendSettings
{
%TypeHeaderCode
#include <qgslegendsettings.h>
%End

  public:
    QgsLegendSettings();

    void setTitle( const QString& t );
    QString title() const;

    /** Returns the alignment of the legend title
     * @returns Qt::AlignmentFlag for the legend title
     * @see setTitleAlignment
    */
    Qt::AlignmentFlag titleAlignment() const;
    /** Sets the alignment of the legend title
     * @param alignment Text alignment for drawing the legend title
     * @see titleAlignment
    */
    void setTitleAlignment( Qt::AlignmentFlag alignment );

    /** Returns reference to modifiable style */
    QgsComposerLegendStyle & rstyle( QgsComposerLegendStyle::Style s );
    /** Returns style */
    QgsComposerLegendStyle style( QgsComposerLegendStyle::Style s ) const;
    void setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style );

    double boxSpace() const;
    void setBoxSpace( double s );

    void setWrapChar( const QString& t );
    QString wrapChar() const;

    double columnSpace() const;
    void setColumnSpace( double s );

    int columnCount() const;
    void setColumnCount( int c );

    int splitLayer() const;
    void setSplitLayer( bool s );

    int equalColumnWidth() const;
    void setEqualColumnWidth( bool s );

    QColor fontColor() const;
    void setFontColor( const QColor& c );

    QSizeF symbolSize() const;
    void setSymbolSize( QSizeF s );

    QSizeF wmsLegendSize() const;
    void setWmsLegendSize( QSizeF s );

    double lineSpacing() const;
    void setLineSpacing( double s );

    double mmPerMapUnit() const;
    void setMmPerMapUnit( double mmPerMapUnit );

    bool useAdvancedEffects() const;
    void setUseAdvancedEffects( bool use );

    double mapScale() const;
    void setMapScale( double scale );

    int dpi() const;
    void setDpi( int dpi );

    // utility functions

    /** Splits a string using the wrap char taking into account handling empty
      wrap char which means no wrapping */
    QStringList splitStringForWrapping( QString stringToSplt ) const;

    /** Draws Text. Takes care about all the composer specific issues (calculation to pixel, scaling of font and painter
     to work around the Qt font bug)*/
    void drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const;

    /** Like the above, but with a rectangle for multiline text
     * @param p painter to use
     * @param rect rectangle to draw into
     * @param text text to draw
     * @param font font to use
     * @param halignment optional horizontal alignment
     * @param valignment optional vertical alignment
     * @param flags allows for passing Qt::TextFlags to control appearance of rendered text
    */
    void drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment = Qt::AlignLeft, Qt::AlignmentFlag valignment = Qt::AlignTop, int flags = Qt::TextWordWrap ) const;

    /** Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE */
    QFont scaledFontPixelSize( const QFont& font ) const;

    /** Calculates font to from point size to pixel size */
    double pixelFontSize( double pointSize ) const;

    /** Returns the font width in millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE */
    double textWidthMillimeters( const QFont& font, const QString& text ) const;

    /** Returns the font height of a character in millimeters */
    double fontHeightCharacterMM( const QFont& font, const QChar& c ) const;

    /** Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE */
    double fontAscentMillimeters( const QFont& font ) const;

    /** Returns the font descent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE */
    double fontDescentMillimeters( const QFont& font ) const;

};
