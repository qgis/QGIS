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
      RecentCrs, //!< Recently used CRS
      CrsNotSet, //!< Not set (hidden by default)
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
     * @see optionVisible()
     */
    void setOptionVisible( const CrsOption option, const bool visible );


    /**
     * Returns whether the specified CRS option is visible in the widget.
     * @note added in QGIS 3.0
     * @see setOptionVisible()
     */
    bool optionVisible( CrsOption option ) const;

    /**
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setOptionVisible().
     * @note added in QGIS 3.0
     */
    void setNotSetText( const QString& text );

  signals:

    /** Emitted when the selected CRS is changed
     */
    void crsChanged( const QgsCoordinateReferenceSystem& );

    /**
     * Emitted when the not set option is selected.
     * @note added in QGIS 3.0
     */
    void cleared();

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
