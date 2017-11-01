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
class QgsComposition;
class QgsLayoutManager;

class QgsLayoutManagerModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    enum Role
    {
      CompositionRole = Qt::UserRole + 1,
    };

    explicit QgsLayoutManagerModel( QgsLayoutManager *manager, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QgsComposition *compositionFromIndex( const QModelIndex &index ) const;

  private slots:
    void compositionAboutToBeAdded( const QString &name );
    void compositionAboutToBeRemoved( const QString &name );
    void compositionAdded( const QString &name );
    void compositionRemoved( const QString &name );
    void compositionRenamed( QgsComposition *composition, const QString &newName );
  private:
    QgsLayoutManager *mLayoutManager = nullptr;
};

/**
 * A dialog that shows the existing composer instances. Lets the user add new
instances and change title of existing ones*/
class QgsComposerManager: public QDialog, private Ui::QgsComposerManagerBase
{
    Q_OBJECT
  public:
    QgsComposerManager( QWidget *parent = nullptr, Qt::WindowFlags f = 0 );
    ~QgsComposerManager();

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
    QPushButton *mShowButton = nullptr;
    QPushButton *mRemoveButton = nullptr;
    QPushButton *mRenameButton = nullptr;
    QPushButton *mDuplicateButton = nullptr;
    QgsLayoutManagerModel *mModel = nullptr;

#ifdef Q_OS_MAC
    void showEvent( QShowEvent *event );
    void changeEvent( QEvent * );

    QAction *mWindowAction = nullptr;
#endif

  private slots:
    //! Slot to update buttons state when selecting compositions
    void toggleButtons();
    void mAddButton_clicked();
    //! Slot to track combobox to use specific template path
    void mTemplate_currentIndexChanged( int indx );
    //! Slot to choose path to template
    void mTemplatePathBtn_pressed();
    //! Slot to open default templates dir with user's system
    void mTemplatesDefaultDirBtn_pressed();
    //! Slot to open user templates dir with user's system
    void mTemplatesUserDirBtn_pressed();

    void removeClicked();
    void showClicked();
    //! Duplicate composer
    void duplicateClicked();
    void renameClicked();
};

#endif // QGSCOMPOSERMANAGER_H
