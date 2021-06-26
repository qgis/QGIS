/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   *
*  
*        *
*                                     *
*                                                                         *
***************************************************************************/

#ifndef QGSGROUPWMSDATADIALOG_H
#define QGSGROUPWMSDATADIALOG_H

#include "ui_qgsgroupwmsdatadialogbase.h"
#include "qgsguiutils.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsGroupWmsDataDialog
 */
class GUI_EXPORT QgsGroupWmsDataDialog: public QDialog, private Ui::QgsGroupWMSDataDialogBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGroupWmsDataDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    //~QgsGroupWMSDataDialog();

    //! Returns group WMS title
    QString groupTitle();

    //! Returns group WMS short name
    QString groupShortName();

    //! Returns group WMS abstract
    QString groupAbstract();


  public slots:
    //! Sets group WMS title
    void setGroupTitle( const QString &title );

    //! Sets group WMS short name
    void setGroupShortName( const QString &shortName );

    //! Sets group WMS abstract
    void setGroupAbstract( const QString &abstract );


  private:

    QString mGroupTitle;
    QString mGroupShortName;

};

#endif // QGSGROUPWMSDATADIALOG_H
