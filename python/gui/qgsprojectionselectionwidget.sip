/**
 * \class QgsProjectionSelectionWidget
 * \ingroup gui
 * \brief A widget for selecting a projection.
 * \note added in QGIS 2.7
 */
class QgsProjectionSelectionWidget : QWidget
{
%TypeHeaderCode
#include <qgsprojectionselectionwidget.h>
%End

  public:

    explicit QgsProjectionSelectionWidget( QWidget *parent /TransferThis/ = 0 );

    /* Returns the currently selected CRS for the widget
     * @returns current CRS
     */
    QgsCoordinateReferenceSystem crs() const;

  signals:

    /* Emitted when the selected CRS is changed
     */
    void crsChanged( QgsCoordinateReferenceSystem );

  public slots:

    /* Sets the current CRS for the widget
     * @param crs new CRS
     */
    void setCrs( QgsCoordinateReferenceSystem crs );

    /* Opens the dialog for selecting a new CRS
     */
    void selectCrs();
};
