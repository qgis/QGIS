class QgsProjectionSelectionDialog : QDialog //, private Ui::QgsGenericProjectionSelectorBase
{
%TypeHeaderCode
#include <qgsprojectionselectiondialog.h>
%End

  public:
    /**
     * Constructor
     */
    QgsProjectionSelectionDialog( QWidget *parent /TransferThis/ = 0,
                                  const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );

    //! Destructor
    ~QgsProjectionSelectionDialog();

    QgsCoordinateReferenceSystem crs() const;

    void setMessage( const QString& message );

    void setShowNoProjection( bool show );

    bool showNoProjection() const;

  public slots:


    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /**
     * \brief filters this dialog by the given CRSs
     *
     * Sets this dialog to filter the available projections to those listed
     * by the given Coordinate Reference Systems.
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by.  This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     *
     * \warning This function's behavior is undefined if it is called after the dialog is shown.
     */
    void setOgcWmsCrsFilter( const QSet<QString>& crsFilter );
};
