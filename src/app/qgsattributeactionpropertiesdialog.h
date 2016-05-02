/***************************************************************************
  qgsattributeactionpropertiesdialog.h - QgsAttributeActionPropertiesDialog

 ---------------------
 begin                : 18.4.2016
 copyright            : (C) 2016 by mku
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEACTIONPROPERTIESDIALOG_H
#define QGSATTRIBUTEACTIONPROPERTIESDIALOG_H

#include "ui_qgsattributeactionpropertiesdialogbase.h"

#include "qgsaction.h"

#include <QDialog>

class QgsAttributeActionPropertiesDialog: public QDialog, private Ui::QgsAttributeActionPropertiesDialogBase
{
    Q_OBJECT

  public:
    QgsAttributeActionPropertiesDialog( QgsAction::ActionType type, const QString& description, const QString& shortTitle, const QString& iconPath, const QString& actionText, bool capture, bool showInAttributeTable, QgsVectorLayer* layer, QWidget* parent = nullptr );

    QgsAttributeActionPropertiesDialog( QgsVectorLayer* layer, QWidget* parent = nullptr );

    QgsAction::ActionType type() const;

    QString description() const;

    QString shortTitle() const;

    QString iconPath() const;

    QString actionText() const;

    bool showInAttributeTable() const;

    bool capture() const;

  private slots:
    void browse();
    void insertExpressionOrField();
    void chooseIcon();
    void updateButtons();

  private:
    QgsVectorLayer* mLayer;
};

#endif // QGSATTRIBUTEACTIONPROPERTIESDIALOG_H
