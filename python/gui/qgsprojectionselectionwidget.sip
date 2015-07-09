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

    /** Predefined CRS options shown in widget
     */
    enum CrsOption
    {
      LayerCrs, /*!< optional layer CRS */
      ProjectCrs, /*!< current project CRS (if OTF reprojection enabled) */
      CurrentCrs, /*!< current user selected CRS */
      DefaultCrs, /*!< global default QGIS CRS */
      RecentCrs /*!< recently used CRS */
    };

    explicit QgsProjectionSelectionWidget( QWidget *parent /TransferThis/ = 0 );

    /** Returns a pointer to the projection selector dialog used by the widget.
     * Can be used to modify how the projection selector dialog behaves.
     * @returns projection selector dialog
     */
    QgsGenericProjectionSelector* dialog();

    /** Returns the currently selected CRS for the widget
     * @returns current CRS
     */
    QgsCoordinateReferenceSystem crs() const;

    /** Sets whether a predefined CRS option should be shown in the widget.
     * @param option CRS option to show/hide
     * @param visible whether the option should be shown
     */
    void setOptionVisible( const CrsOption option, const bool visible );

  signals:

    /** Emitted when the selected CRS is changed
     */
    void crsChanged( QgsCoordinateReferenceSystem );

  public slots:

    /** Sets the current CRS for the widget
     * @param crs new CRS
     */
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /** Sets the layer CRS for the widget. If set, this will be added as an option
     * to the preset CRSes shown in the widget.
     * @param crs layer CRS
     */
    void setLayerCrs( const QgsCoordinateReferenceSystem& crs );

    /** Opens the dialog for selecting a new CRS
     */
    void selectCrs();

};
