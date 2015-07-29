/***************************************************************************
                              qgscomposermanager.h
                             ------------------------
    begin                : September 11 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERMANAGER_H
#define QGSCOMPOSERMANAGER_H

#include <QItemDelegate>

#include "ui_qgscomposermanagerbase.h"

class QListWidgetItem;
class QgsComposer;

/** Delegate for a line edit for renaming a composer. Prevents entry of duplicate composer names.*/
class QgsComposerNameDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    QgsComposerNameDelegate( QObject *parent = 0 );

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override;

    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model,
                       const QModelIndex &index ) const override;

    void updateEditorGeometry( QWidget *editor,
                               const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};

/** A dialog that shows the existing composer instances. Lets the user add new
instances and change title of existing ones*/
class QgsComposerManager: public QDialog, private Ui::QgsComposerManagerBase
{
    Q_OBJECT
  public:
    QgsComposerManager( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsComposerManager();

  public slots:
    /** Raise, unminimize and activate this window */
    void activate();

  private:
    /** Stores the relation between items and composer pointers. A 0 pointer for the composer means that
      this composer needs to be created from a default template*/
    QMap<QListWidgetItem*, QgsComposer*> mItemComposerMap;

    /** Returns the default templates (key: template name, value: absolute path to template file)
     * @param fromUser whether to return user templates from ~/.qgis/composer_templates
     */
    QMap<QString, QString> defaultTemplates( bool fromUser = false ) const;

    /** Open local directory with user's system, creating it if not present
     */
    void openLocalDirectory( const QString& localDirPath );

    QString mDefaultTemplatesDir;
    QString mUserTemplatesDir;

#ifdef Q_OS_MAC
    void showEvent( QShowEvent *event );
    void changeEvent( QEvent * );

    QAction* mWindowAction;
#endif

  private slots:
    void on_mAddButton_clicked();
    /** Slot to track combobox to use specific template path */
    void on_mTemplate_currentIndexChanged( int indx );
    /** Slot to choose path to template */
    void on_mTemplatePathBtn_pressed();
    /** Slot to open default templates dir with user's system */
    void on_mTemplatesDefaultDirBtn_pressed();
    /** Slot to open user templates dir with user's system */
    void on_mTemplatesUserDirBtn_pressed();

    /** Refreshes the list of composers */
    void refreshComposers();

    void remove_clicked();
    void show_clicked();
    /** Duplicate composer */
    void duplicate_clicked();
    void rename_clicked();
    void on_mComposerListWidget_itemChanged( QListWidgetItem * item );
};

#endif // QGSCOMPOSERMANAGER_H
