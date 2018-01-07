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

class QgsComposerManagerModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    enum Role
    {
      CompositionRole = Qt::UserRole + 1,
    };

    explicit QgsComposerManagerModel( QgsLayoutManager *manager, QObject *parent = nullptr );

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
    QgsComposerManager( QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsComposerManager() override;

  public slots:
    //! Raise, unminimize and activate this window
    void activate();

  private:

    QPushButton *mShowButton = nullptr;
    QgsComposerManagerModel *mModel = nullptr;

#ifdef Q_OS_MAC
    void showEvent( QShowEvent *event ) override;
    void changeEvent( QEvent * ) override;

    QAction *mWindowAction = nullptr;
#endif

  private slots:
    //! Slot to update buttons state when selecting compositions
    void toggleButtons();

    void showClicked();
};

#endif // QGSCOMPOSERMANAGER_H
