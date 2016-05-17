
/** \ingroup gui
 * \class QgsFieldConditionalFormatWidget
 * A widget for customising conditional formatting options.
 * \note added in QGIS 2.12
 */
class QgsFieldConditionalFormatWidget : QWidget
{
%TypeHeaderCode
#include <attributetable/qgsfieldconditionalformatwidget.h>
%End

  public:

    /** Constructor for QgsFieldConditionalFormatWidget.
     * @param parent parent widget
     */
    explicit QgsFieldConditionalFormatWidget( QWidget *parent /TransferThis/ = 0 );

    ~QgsFieldConditionalFormatWidget();

    /** Switches the widget to the rules page.
     */
    void viewRules();

    /** Sets the vector layer associated with the widget.
     * @param theLayer vector layer
     */
    void setLayer( QgsVectorLayer* theLayer );

    /** Switches the widget to the edit style mode for the specified style.
     * @param index index of conditional style to edit
     * @param style initial conditional styling options
     */
    void editStyle( int index, const QgsConditionalStyle& style );

    /**
     * @param style initial conditional styling options
     */
    void loadStyle( const QgsConditionalStyle& style );

    /** Resets the formatting options to their default state.
     */
    void reset();

    /**
     * @brief Set the presets that can be used for quick pick
     * @param styles A list of styles used as presets
     */
    void setPresets( const QList<QgsConditionalStyle>& styles );

    /**
     * @brief The default presets for the widget.  Normally set when the widget is
     * created however called setPresets will override the default styles.
     * @return List of default presets.
     */
    QList<QgsConditionalStyle> defaultPresets() const;

  signals:

    /** Emitted when the conditional styling rules are updated.
     * @param fieldName name of field whose rules have been modified.
     */
    void rulesUpdated( const QString& fieldName );
};
