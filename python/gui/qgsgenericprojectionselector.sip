
/**
 * \ingroup gui
 * A generic dialog to prompt the user for a Coordinate Reference System.
 *
 * Typically you will use this when you want to prompt the user for
 * a coordinate system identifier e.g. from a plugin you might do this
 * to get an epsg code:
 * \code
 * QgsGenericProjectionSelector mySelector( mQGisIface->mainWindow() );
 * mySelector.setSelectedEpsg( mEpsgId );
 * if ( mySelector.exec() )
 * {
 *   mEpsgId = mySelector.selectedEpsg();
 * }
 * \endcode
 *
 * If you wish to embed the projection selector into an existing dialog
 * the you probably want to look at QgsProjectionSelector rather.
 */
class QgsGenericProjectionSelector : QDialog //, private Ui::QgsGenericProjectionSelectorBase
{
%TypeHeaderCode
#include <qgsgenericprojectionselector.h>
%End

  public:
    /**
     * Constructor
     */
    QgsGenericProjectionSelector( QWidget *parent = 0,
                                  Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    //! Destructor
    ~QgsGenericProjectionSelector();

  public slots:
    /** If no parameter is passed, the message will be a generic
     * 'define the CRS for this layer'.
     */
    void setMessage( QString theMessage = "" );
    long selectedCrsId();
    QString selectedAuthId();

    void setSelectedCrsName( QString theName );
    void setSelectedCrsId( long theID );
    void setSelectedAuthId( QString authId );

    void on_mButtonBox_helpRequested();

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
     * \warning This function's behaviour is undefined if it is called after the dialog is shown.
     */
    void setOgcWmsCrsFilter( QSet<QString> crsFilter );
};
