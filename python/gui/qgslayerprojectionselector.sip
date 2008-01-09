
/**
 * \class QgsLayerProjectionSelector
 * \brief Set Projection system for a layer
 */

class QgsLayerProjectionSelector : QDialog //, private Ui::QgsLayerProjectionSelectorBase
{
%TypeHeaderCode
#include <qgslayerprojectionselector.h>
%End

  public:
    /**
     * Constructor
     */
    QgsLayerProjectionSelector(QWidget *parent = 0, 
                               Qt::WFlags fl = QgisGui::ModalDialogFlags);

    //! Destructor
    ~QgsLayerProjectionSelector();

 public slots:
      QString getCurrentProj4String();
      long getCurrentSRSID();
      long getCurrentEpsg();

      void setSelectedSRSName(QString theName);
      void setSelectedSRSID(long theID);

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
       * \note This function only deals with EPSG labels only at this time.
       *
       * \warning This function's behaviour is undefined if it is called after the dialog is shown.
       */
      void setOgcWmsCrsFilter(QSet<QString> crsFilter);


};

