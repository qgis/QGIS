/***************************************************************************
                         qgsludialog.h  -  description
                             -------------------
    begin                : September 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLUDIALOG_H
#define QGSLUDIALOG_H

#include "ui_qgsludialogbase.h"
#include "qgisgui.h"

/** \ingroup gui
 * \class QgsLUDialog
 */
class GUI_EXPORT QgsLUDialog: public QDialog, private Ui::QgsLUDialogBase
{
    Q_OBJECT
  public:
    QgsLUDialog( QWidget *parent = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsLUDialog();
    QString lowerValue() const;
    void setLowerValue( const QString& val );
    QString upperValue() const;
    void setUpperValue( const QString& val );
};

#endif
