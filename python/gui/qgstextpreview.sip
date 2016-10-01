/** \class QgsTextPreview
 * \ingroup gui
 * A widget for previewing text formatting settings.
 *
 * QgsTextPreview provides a widget for previewing the appearance of text rendered
 * using QgsTextRenderer. The preview includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * In order to preview the exact appearance of text which uses sizes in map units,
 * the scale and map units must be set by calling setScale() and setMapUnits().
 *
 * @note Added in QGIS 3.0
 */

class QgsTextPreview : public QLabel
{
%TypeHeaderCode
  #include <qgstextpreview.h>
%End
  public:

    /** Constructor for QgsTextPreview
     * @param parent parent widget
     */
    QgsTextPreview( QWidget* parent = nullptr );

    void paintEvent( QPaintEvent* e );

    /** Sets the text format for previewing in the widget.
     * @param format text format
     * @see format()
     */
    void setFormat( const QgsTextFormat& format );

    /** Returns the text format used for previewing text in the widget.
     * @see setFormat()
     */
    QgsTextFormat format() const;

    /** Sets the scale to use for previewing format sizes in map units.
     * @param scale preview map scale
     * @see scale()
     * @see setMapUnits()
     */
    void setScale( double scale );

    /** Returns the scale used for previewing format sizes in map units.
     * @see setScale()
     * @see mapUnits()
     */
    double scale() const;

    /** Sets the map unit type for previewing format sizes in map units.
     * @param unit map units
     * @see mapUnits()
     * @see setScale()
     */
    void setMapUnits( QgsUnitTypes::DistanceUnit unit );

    /** Returns the map unit type used for previewing format sizes in map units.
     * @see setMapUnits()
     * @see scale()
     */
    QgsUnitTypes::DistanceUnit mapUnits() const;

};
