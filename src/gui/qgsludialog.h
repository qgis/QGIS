/***************************************************************************
                         qgsludialog.h  -  description
                             -------------------
    begin                : September 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLUDIALOG_H
#define QGSLUDIALOG_H

#include "ui_qgsludialogbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
 * \class QgsLUDialog
 */
class GUI_EXPORT QgsLUDialog: public QDialog, private Ui::QgsLUDialogBase
{
    Q_OBJECT
  public:
    QgsLUDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    QString lowerValue() const;
    void setLowerValue( const QString &val );
    QString upperValue() const;
    void setUpperValue( const QString &val );
};

#endif
