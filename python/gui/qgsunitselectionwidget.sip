/** \class QgsMapUnitScaleWidget
 * \ingroup gui
 * A widget which allows the user to choose the minimum and maximum scale of an object in map units
 * and millimeters. This widget is designed to allow users to edit the properties of a
 * QgsMapUnitScale object.
 * \note added in QGIS 3.0
 * \see QgsMapUnitScaleDialog
 * \see QgsUnitSelectionWidget
*/
class QgsMapUnitScaleWidget : QgsPanelWidget
{
%TypeHeaderCode
#include <qgsunitselectionwidget.h>
%End
  public:

    /** Constructor for QgsMapUnitScaleWidget.
     * @param parent parent widget
     */
    QgsMapUnitScaleWidget( QWidget* parent /TransferThis/ = nullptr );

    /** Returns a QgsMapUnitScale representing the settings shown in the
     * widget.
     * @see setMapUnitScale()
     * @see mapUnitScaleChanged()
     */
    QgsMapUnitScale mapUnitScale() const;

    /** Updates the widget to reflect the settings from the specified
     * QgsMapUnitScale object.
     * @param scale map unit scale to show in widget
     * @see mapUnitScale()
     * @see mapUnitScaleChanged()
     */
    void setMapUnitScale( const QgsMapUnitScale& scale );

    /** Sets the map canvas associated with the widget. This allows the
     * widget to retrieve the current map scale from the canvas.
     * @param canvas map canvas
     */
    void setMapCanvas( QgsMapCanvas* canvas );

  signals:

    /** Emitted when the settings in the widget are modified.
     * @param scale QgsMapUnitScale reflecting new settings from the widget
     */
    void mapUnitScaleChanged( const QgsMapUnitScale& scale );

};


/** \class QgsMapUnitScaleDialog
 * \ingroup gui
 * A dialog which allows the user to choose the minimum and maximum scale of an object in map units
 * and millimeters. This dialog is designed to allow users to edit the properties of a
 * QgsMapUnitScale object.
 * \see QgsMapUnitScaleWidget
 * \see QgsUnitSelectionWidget
*/
class QgsMapUnitScaleDialog : QDialog
{
%TypeHeaderCode
#include <qgsunitselectionwidget.h>
%End

  public:

    /** Constructor for QgsMapUnitScaleDialog.
     * @param parent parent widget
     */
    QgsMapUnitScaleDialog( QWidget* parent /TransferThis/ = 0);

    /** Returns a QgsMapUnitScale representing the settings shown in the
     * dialog.
     * @see setMapUnitScale()
     */
    QgsMapUnitScale getMapUnitScale() const;

    /** Updates the dialog to reflect the settings from the specified
     * QgsMapUnitScale object.
     * @param scale map unit scale to show in dialog
     * @see mapUnitScale()
     */
    void setMapUnitScale( const QgsMapUnitScale& scale );

    /** Sets the map canvas associated with the dialog. This allows the dialog to retrieve the current
     * map scale from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

};

/** \class QgsUnitSelectionWidget
 * \ingroup gui
 * A widget displaying a combobox allowing the user to choose between various display units,
 * such as millimeters or map unit. If the user chooses map units, a button appears allowing
 * adjustment of minimum and maximum scaling.
 * \see QgsMapUnitScaleWidget
 * \see QgsMapUnitScaleDialog
 */
class QgsUnitSelectionWidget : QWidget
{
%TypeHeaderCode
#include <qgsunitselectionwidget.h>
%End

  public:

    /** Constructor for QgsUnitSelectionWidget.
     * @param parent parent widget
     */
    QgsUnitSelectionWidget( QWidget* parent /TransferThis/ = 0 );

    /** Sets the units which the user can choose from in the combobox.
     * @param units list of strings for custom units to display in the widget
     * @param mapUnitIdx specifies which entry corresponds to the map units, or -1 if none
     */
    void setUnits( const QStringList& units, int mapUnitIdx );

    /** Sets the units which the user can choose from in the combobox. Clears any existing units.
     * @param units list of valid units
     * @note added in QGIS 2.9
     */
    void setUnits( const QgsUnitTypes::RenderUnitList& units );

    /** Get the selected unit index */
    int getUnit() const;

    /** Returns the current predefined selected unit (if applicable).
     * @returns selected output unit, or QgsUnitTypes::RenderUnknownUnit if the widget was populated with custom unit types
     * @note added in QGIS 2.9
     */
    QgsUnitTypes::RenderUnit unit() const;

    /** Sets the selected unit index
     * @param unitIndex index of unit to set as current
     * @note available in Python bindings as setUnitIndex
     */
    void setUnit( int unitIndex ) /PyName=setUnitIndex/;

    /** Sets the selected unit
     * @param unit predefined unit to set as current
     */
    void setUnit( QgsUnitTypes::RenderUnit unit );

    /** Returns the map unit scale */
    QgsMapUnitScale getMapUnitScale() const;

    /** Sets the map unit scale */
    void setMapUnitScale( const QgsMapUnitScale& scale );

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

  signals:
    void changed();

};

