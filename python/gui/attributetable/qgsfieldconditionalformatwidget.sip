
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
    void editStyle( int index, QgsConditionalStyle style );

    /** Resets the formatting options to their default state.
     */
    void reset();

  signals:

    /** Emitted when the conditional styling rules are updated.
     * @param fieldName name of field whose rules have been modified.
     */
    void rulesUpdated( const QString& fieldName );
};
