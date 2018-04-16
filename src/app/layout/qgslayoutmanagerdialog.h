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

class QgsLayoutManagerModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    enum Role
    {
      LayoutRole = Qt::UserRole + 1,
    };

    explicit QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QgsMasterLayoutInterface *layoutFromIndex( const QModelIndex &index ) const;

  private slots:
    void layoutAboutToBeAdded( const QString &name );
    void layoutAboutToBeRemoved( const QString &name );
    void layoutAdded( const QString &name );
    void layoutRemoved( const QString &name );
    void layoutRenamed( QgsMasterLayoutInterface *layout, const QString &newName );
  private:
    QgsLayoutManager *mLayoutManager = nullptr;
};

class QgsLayoutManagerProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    explicit QgsLayoutManagerProxyModel( QObject *parent );

};

/**
 * A dialog that allows management of layouts within a project.
*/
class QgsLayoutManagerDialog: public QDialog, private Ui::QgsLayoutManagerBase
{
    Q_OBJECT
  public:
    QgsLayoutManagerDialog( QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );

    void addTemplates( const QMap<QString, QString> &templates );

  public slots:
    //! Raise, unminimize and activate this window
    void activate();

  private:

    /**
     * Returns the default templates (key: template name, value: absolute path to template file)
     * \param fromUser whether to return user templates from ~/.qgis/composer_templates
     */
    QMap<QString, QString> defaultTemplates( bool fromUser = false ) const;
    QMap<QString, QString> otherTemplates() const;

    QMap<QString, QString> templatesFromPath( const QString &path ) const;

    /**
     * Open local directory with user's system, creating it if not present
     */
    void openLocalDirectory( const QString &localDirPath );

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
