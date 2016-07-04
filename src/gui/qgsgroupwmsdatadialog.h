/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSGROUPWMSDATADIALOG_H
#define QGSGROUPWMSDATADIALOG_H

#include "ui_qgsgroupwmsdatadialogbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

#include "qgis.h"

/** \ingroup gui
 * \class QgsGroupWMSDataDialog
 */
class GUI_EXPORT QgsGroupWMSDataDialog: public QDialog, private Ui::QgsGroupWMSDataDialogBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGroupWMSDataDialog( QWidget *parent = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
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


  private:

    QString mGroupTitle;
    QString mGroupShortName;

};

#endif // QGSGROUPWMSDATADIALOG_H
