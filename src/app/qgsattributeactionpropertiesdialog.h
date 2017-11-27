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
#include "qgshelp.h"

#include <QDialog>

class QgsAttributeActionPropertiesDialog: public QDialog, private Ui::QgsAttributeActionPropertiesDialogBase, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsAttributeActionPropertiesDialog( QgsAction::ActionType type, const QString &description, const QString &shortTitle, const QString &iconPath, const QString &actionText, bool capture, const QSet<QString> &actionScopes, const QString &notificationMessage, QgsVectorLayer *layer, QWidget *parent = nullptr );

    QgsAttributeActionPropertiesDialog( QgsVectorLayer *layer, QWidget *parent = nullptr );

    QgsAction::ActionType type() const;

    QString description() const;

    QString shortTitle() const;

    QString iconPath() const;

    QString actionText() const;

    QSet<QString> actionScopes() const;

    QString notificationMessage() const;

    bool capture() const;

    virtual QgsExpressionContext createExpressionContext() const override;

  private slots:
    void browse();
    void insertExpressionOrField();
    void chooseIcon();
    void updateButtons();
    void showHelp();

  private:
    void init( const QSet<QString> &actionScopes );

    QgsVectorLayer *mLayer = nullptr;
    QList<QCheckBox *> mActionScopeCheckBoxes;
};

#endif // QGSATTRIBUTEACTIONPROPERTIESDIALOG_H
