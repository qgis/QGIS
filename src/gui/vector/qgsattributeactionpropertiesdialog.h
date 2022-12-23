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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsattributeactionpropertiesdialogbase.h"
#include "qgsexpressioncontextgenerator.h"

#include "qgshelp.h"
#include "qgis_gui.h"

#include <QDialog>

/**
 * \ingroup gui
 * \class QgsAttributeActionPropertiesDialog
 */
class GUI_EXPORT QgsAttributeActionPropertiesDialog: public QDialog, private Ui::QgsAttributeActionPropertiesDialogBase, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAttributeActionPropertiesDialog.
     */
    QgsAttributeActionPropertiesDialog( Qgis::AttributeActionType type, const QString &description, const QString &shortTitle, const QString &iconPath, const QString &actionText, bool capture, const QSet<QString> &actionScopes, const QString &notificationMessage, bool isEnabledOnlyWhenEditable, QgsVectorLayer *layer, QWidget *parent = nullptr );

    QgsAttributeActionPropertiesDialog( QgsVectorLayer *layer, QWidget *parent = nullptr );

    Qgis::AttributeActionType type() const;

    QString description() const;

    QString shortTitle() const;

    QString iconPath() const;

    QString actionText() const;

    QSet<QString> actionScopes() const;

    QString notificationMessage() const;

    bool isEnabledOnlyWhenEditable() const;

    bool capture() const;

    QgsExpressionContext createExpressionContext() const override;

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
