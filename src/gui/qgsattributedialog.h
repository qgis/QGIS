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

#include "qgsfeatureattribute.h"
#include <vector>

class QDialog;
class QgsFeature;

class QgsAttributeDialog: public QDialog, private Ui::QgsAttributeDialogBase
{
    Q_OBJECT

  public:
    QgsAttributeDialog(const std::vector<QgsFeatureAttribute>* attributes);

    ~QgsAttributeDialog();

    /** Returns the field value of a row */
    QString value(int row);

    /** Returns if the field value of a row was edited since this dialog opened */
    bool isDirty(int row);

    /** Opens an attribute dialog and queries the attributes for a given feature. The
     attribute values are set to the feature if the dialog is accepted.
     \retval true if accepted
     \retval false if canceled */
    static bool queryAttributes(QgsFeature& f);

    // Saves and restores the size and position from the last time
    // this dialog box was used.
    void savePositionAndColumnWidth();

    void restorePositionAndColumnWidth();

  public slots:
    //! Slot to be called when an attribute value is edited in the table.
    void setAttributeValueChanged(int row, int column);

  private:
    QString _settingsPath;

    std::vector<bool> mRowIsDirty;
};

#endif
