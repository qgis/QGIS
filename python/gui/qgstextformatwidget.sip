/** \class QgsTextFormatWidget
 * \ingroup gui
 * A widget for customising text formatting settings.
 *
 * QgsTextFormatWidget provides a widget for controlling the appearance of text rendered
 * using QgsTextRenderer. The preview includes all settings contained within
 * a QgsTextFormat, including shadow, background and buffer.
 *
 * Additionally, the widget can handle labeling settings due to the large overlap between
 * the text renderer settings and the labeling settings. This mode is possible by
 * subclassing QgsTextFormatWidget and calling the protected constructor with a mode
 * of Labeling.
 *
 * @note Added in QGIS 3.0
 */

class QgsTextFormatWidget : public QgsPanelWidget
{
%TypeHeaderCode
  #include <qgstextformatwidget.h>
%End

  public:

     /** Constructor for QgsTextFormatWidget.
     * @param format initial formatting settings to show in widget
     * @param mapCanvas associated map canvas
     * @param parent parent widget
     */
    QgsTextFormatWidget( const QgsTextFormat& format = QgsTextFormat(), QgsMapCanvas* mapCanvas = nullptr, QWidget* parent /TransferThis/ = nullptr );

    ~QgsTextFormatWidget();

    /** Returns the current formatting settings defined by the widget.
     */
    QgsTextFormat format() const;

  public slots:

    /** Sets whether the widget should be shown in a compact dock mode.
     * @param enabled set to true to show in dock mode.
     */
    void setDockMode( bool enabled );

  signals:

    //! Emitted when the text format defined by the widget changes
    void widgetChanged();

  protected:

    //! Widget mode
    enum Mode
    {
      Text, //!< Default mode, show text formatting settings only
      Labeling, //!< Show labeling settings in addition to text formatting settings
    };

    /** Constructor for QgsTextFormatWidget.
     * @param mapCanvas associated map canvas
     * @param parent parent widget
     * @param mode widget mode
     */
    QgsTextFormatWidget( QgsMapCanvas* mapCanvas, QWidget* parent /TransferThis/, Mode mode );

    /** Updates the widget's state to reflect the settings in a QgsTextFormat.
     * @param format source format
     */
    void updateWidgetForFormat( const QgsTextFormat& format );

    /** Sets the background color for the text preview widget.
     * @param color background color
     */
    void setPreviewBackground( const QColor& color );

    /** Controls whether data defined alignment buttons are enabled.
     * @param enable set to true to enable alignment controls
     */
    void enableDataDefinedAlignment( bool enable );

  protected slots:

    //! Updates line placement options to reflect current state of widget
    void updateLinePlacementOptions();

    //! Updates label placement options to reflect current state of widget
    void updatePlacementWidgets();
};
