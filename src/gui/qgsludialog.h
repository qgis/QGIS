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

    /**
     * Returns the lower value.
     * \since QGIS 3.21
     */
    double lowerValueDouble() const;
    void setLowerValue( const QString &val );
    QString upperValue() const;

    /**
     * Returns the upper value.
     * \since QGIS 3.21
     */
    double upperValueDouble() const;
    void setUpperValue( const QString &val );

  private:

    void setDecimalPlaces( QgsDoubleSpinBox *widget, double value ) const;
};

#endif
