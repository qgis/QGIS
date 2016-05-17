/** Dialog allowing the user to choose the minimum and maximum scale of an object in map units */
class QgsMapUnitScaleDialog : QDialog
{
%TypeHeaderCode
#include <qgsunitselectionwidget.h>
%End

  public:
    QgsMapUnitScaleDialog( QWidget* parent /TransferThis/ = 0);

    /** Returns the map unit scale */
    QgsMapUnitScale getMapUnitScale() const;
    /** Sets the map unit scale */
    void setMapUnitScale( const QgsMapUnitScale& scale );

    /** Sets the map canvas associated with the dialog. This allows the dialog to retrieve the current
     * map scale from the canvas.
     * @param canvas map canvas
     * @note added in QGIS 2.12
     */
    void setMapCanvas( QgsMapCanvas* canvas );

};

/** Widget displaying a combobox allowing the user to choose between millimeter and map units
 *  If the user chooses map units,  a button appears allowing the specification of minimum and maximum scale */
class QgsUnitSelectionWidget : QWidget
{
%TypeHeaderCode
#include <qgsunitselectionwidget.h>
%End

  public:
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
    void setUnits( const QgsSymbolV2::OutputUnitList& units );

    /** Get the selected unit index */
    int getUnit() const;

    /** Returns the current predefined selected unit (if applicable).
     * @returns selected output unit, or QgsSymbolV2::Mixed if the widget was populated with custom unit types
     * @note added in QGIS 2.9
     */
    QgsSymbolV2::OutputUnit unit() const;

    /** Sets the selected unit index
     * @param unitIndex index of unit to set as current
     * @note available in Python bindings as setUnitIndex
     */
    void setUnit( int unitIndex ) /PyName=setUnitIndex/;

    /** Sets the selected unit
     * @param unit predefined unit to set as current
     */
    void setUnit( QgsSymbolV2::OutputUnit unit );

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

