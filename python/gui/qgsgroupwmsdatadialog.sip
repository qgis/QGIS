class QgsGroupWmsDataDialog : QDialog
{
%TypeHeaderCode
#include <qgsgroupwmsdatadialog.h>
%End

  public:
    //! Constructor
    QgsGroupWmsDataDialog( QWidget *parent /TransferThis/ = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    //~QgsGroupWmsDataDialog();

    //! return group WMS title
    QString groupTitle();

    //! return group WMS short name
    QString groupShortName();

    //! return group WMS abstract
    QString groupAbstract();


  public slots:
    //! set group WMS title
    void setGroupTitle( const QString& title );

    //! set group WMS short name
    void setGroupShortName( const QString& shortName );

    //! set group WMS abstract
    void setGroupAbstract( const QString& abstract );

};
