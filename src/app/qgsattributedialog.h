/***************************************************************************
                         qgsattributedialog.h  -  description
                             -------------------
    begin                : October 2004
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
/* $Id$ */
#ifndef QGSATTRIBUTEDIALOG_H
#define QGSATTRIBUTEDIALOG_H

#include "ui_qgsattributedialogbase.h"

#include "qgsfeature.h"
#include <vector>

class QDialog;
class QgsFeature;
class QLayout;
class QgsField;
class QgsVectorLayer;

class QgsAttributeDialog: public QDialog, private Ui::QgsAttributeDialogBase
{
    Q_OBJECT

  public:
    QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature * thepFeature );
    ~QgsAttributeDialog();

    /** Overloaded accept method which will write the feature field
     * values, then delegate to QDialog::accept()
     */
    void accept();
    /** Saves the size and position for the next time
     *  this dialog box was used.
     */
    void saveGeometry();
    /** Restores the size and position from the last time
     *  this dialog box was used.
     */
    void restoreGeometry();

    static QWidget *createEditor( QgsVectorLayer *vl, int idx, const QVariant &value );

  private:
    QString mSettingsPath;
    QList<QWidget *> mpWidgets;
    QgsVectorLayer *mLayer;
    QgsFeature *  mpFeature;

};

#endif
