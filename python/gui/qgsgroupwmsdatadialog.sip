class QgsGroupWMSDataDialog : QDialog
{
%TypeHeaderCode
#include <qgsgroupwmsdatadialog.h>
%End

  public:
    //! Constructor
    QgsGroupWMSDataDialog( QWidget *parent /TransferThis/ = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    //~QgsGroupWMSDataDialog();

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
