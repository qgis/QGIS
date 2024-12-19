/***************************************************************************
                         qgslayoutmanagerdialog.h
                         -----------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTMANAGERDIALOG_H
#define QGSLAYOUTMANAGERDIALOG_H

#include <QItemDelegate>
#include <QSortFilterProxyModel>

#include "ui_qgslayoutmanagerbase.h"

class QListWidgetItem;
class QgsLayoutDesignerDialog;
class QgsMasterLayoutInterface;
class QgsLayoutManager;
class QgsLayoutManagerModel;
class QgsLayoutManagerProxyModel;

/**
 * A dialog that allows management of layouts within a project.
*/
class QgsLayoutManagerDialog: public QDialog, private Ui::QgsLayoutManagerBase
{
    Q_OBJECT
  public:
    QgsLayoutManagerDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    void addTemplates( const QMap<QString, QString> &templates );

  public slots:
    //! Raise, unminimize and activate this window
    void activate();

  private:

    /**
     * Returns the default templates (key: template name, value: absolute path to template file)
     * \param fromUser whether to return user templates from [profile folder]/composer_templates
     */
    QMap<QString, QString> defaultTemplates( bool fromUser = false ) const;
    QMap<QString, QString> otherTemplates() const;

    QMap<QString, QString> templatesFromPath( const QString &path ) const;

    /**
     * Opens local directory with user's system and tries to create it if not present
     */
    void openLocalDirectory( const QString &localDirPath );

    void updateTemplateButtonEnabledState();

    QString mDefaultTemplatesDir;
    QString mUserTemplatesDir;
    QPushButton *mCreateReportButton = nullptr;
    QgsLayoutManagerModel *mModel = nullptr;
    QgsLayoutManagerProxyModel *mProxyModel = nullptr;

#ifdef Q_OS_MAC
    void showEvent( QShowEvent *event );
    void changeEvent( QEvent * );

    QAction *mWindowAction = nullptr;
#endif

  private slots:
    //! Slot to update buttons state when selecting layouts
    void toggleButtons();
    void mAddButton_clicked();
    //! Slot to track combobox to use specific template path
    void mTemplate_currentIndexChanged( int indx );
    //! Slot to open default templates dir with user's system
    void mTemplatesDefaultDirBtn_pressed();
    //! Slot to open user templates dir with user's system
    void mTemplatesUserDirBtn_pressed();
    //! Slot to open help file
    void showHelp();

    void createReport();
    void removeClicked();
    void showClicked();
    //! Duplicate layout
    void duplicateClicked();
    void renameClicked();
    void itemDoubleClicked( const QModelIndex &index );
};

#endif // QGSLAYOUTMANAGERDIALOG_H
