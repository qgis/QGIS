/***************************************************************************
    qgssvgselectorwidget.h - group and preview selector for SVG files
                             built off of work in qgssymbollayerv2widget

    ---------------------
    begin                : April 2, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSVGSELECTORWIDGET_H
#define QGSSVGSELECTORWIDGET_H

#include "ui_widget_svgselector.h"

#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QWidget>

class QLayout;
class QLineEdit;
class QListView;
class QPushButton;
class QTreeView;

class GUI_EXPORT QgsSvgSelectorListModel : public QAbstractListModel
{
  public:
    QgsSvgSelectorListModel( QObject* parent );

    // Constructor to create model for icons in a specific path
    QgsSvgSelectorListModel( QObject* parent, QString path );

    int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  protected:
    QStringList mSvgFiles;
};


class GUI_EXPORT QgsSvgSelectorGroupsModel : public QStandardItemModel
{
  public:
    QgsSvgSelectorGroupsModel( QObject* parent );

  private:
    void createTree( QStandardItem* &parentGroup );
};


class GUI_EXPORT QgsSvgSelectorWidget : public QWidget, private Ui::WidgetSvgSelector
{
    Q_OBJECT

  public:
    QgsSvgSelectorWidget( QWidget* parent = 0 );
    ~QgsSvgSelectorWidget();

    static QgsSvgSelectorWidget* create( QWidget* parent = 0 ) { return new QgsSvgSelectorWidget( parent ); }

    QString currentSvgPath() const;

    QTreeView* groupsTreeView() { return mGroupsTreeView; }
    QListView* imagesListView() { return mImagesListView; }
    QLineEdit* filePathLineEdit() { return mFileLineEdit; }
    QPushButton* filePathButton() { return mFilePushButton; }
    QLayout* selectorLayout() { return this->layout(); }

  public slots:
    void setSvgPath( const QString& svgPath );

  signals:
    void svgSelected( const QString& path );

  protected:
    void populateList();

  private slots:
    void populateIcons( const QModelIndex& idx );
    void svgSelectionChanged( const QModelIndex& idx );
    void updateCurrentSvgPath( const QString& svgPath );

    void on_mFilePushButton_clicked();
    void on_mFileLineEdit_textEdited( const QString& text );
    void on_mFileLineEdit_editingFinished();

  private:
    QString mCurrentSvgPath;
};

#endif // QGSSVGSELECTORWIDGET_H
