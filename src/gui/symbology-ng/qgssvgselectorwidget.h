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

#include "qgisgui.h"

#include <QAbstractListModel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QStandardItemModel>
#include <QWidget>

class QCheckBox;
class QLayout;
class QLineEdit;
class QListView;
class QPushButton;
class QTreeView;

/** \ingroup gui
 * \class QgsSvgSelectorListModel
 */
class GUI_EXPORT QgsSvgSelectorListModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    QgsSvgSelectorListModel( QObject* parent );

    // Constructor to create model for icons in a specific path
    QgsSvgSelectorListModel( QObject* parent, const QString& path );

    int rowCount( const QModelIndex & parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const override;

  protected:
    QStringList mSvgFiles;
};

/** \ingroup gui
 * \class QgsSvgSelectorGroupsModel
 */
class GUI_EXPORT QgsSvgSelectorGroupsModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    QgsSvgSelectorGroupsModel( QObject* parent );

  private:
    void createTree( QStandardItem* &parentGroup );
};

/** \ingroup gui
 * \class QgsSvgSelectorWidget
 */
class GUI_EXPORT QgsSvgSelectorWidget : public QWidget, private Ui::WidgetSvgSelector
{
    Q_OBJECT

  public:
    QgsSvgSelectorWidget( QWidget* parent = nullptr );
    ~QgsSvgSelectorWidget();

    static QgsSvgSelectorWidget* create( QWidget* parent = nullptr ) { return new QgsSvgSelectorWidget( parent ); }

    QString currentSvgPath() const;
    QString currentSvgPathToName() const;

    QTreeView* groupsTreeView() { return mGroupsTreeView; }
    QListView* imagesListView() { return mImagesListView; }
    QLineEdit* filePathLineEdit() { return mFileLineEdit; }
    QPushButton* filePathButton() { return mFilePushButton; }
    QCheckBox* relativePathCheckbox() { return mRelativePathChkBx; }
    QLayout* selectorLayout() { return this->layout(); }

  public slots:
    /** Accepts absolute and relative paths */
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
    void updateLineEditFeedback( bool ok, const QString& tip = QString() );
    void on_mFileLineEdit_textChanged( const QString& text );

  private:
    QString mCurrentSvgPath; // always stored as absolute path
};

/** \ingroup gui
 * \class QgsSvgSelectorDialog
 */
class GUI_EXPORT QgsSvgSelectorDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsSvgSelectorDialog( QWidget* parent = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags,
                          const QDialogButtonBox::StandardButtons& buttons = QDialogButtonBox::Close | QDialogButtonBox::Ok,
                          Qt::Orientation orientation = Qt::Horizontal );
    ~QgsSvgSelectorDialog();

    //! Returns the central layout. Widgets added to it must have this dialog as parent
    QVBoxLayout* layout() { return mLayout; }

    //! Returns the button box
    QDialogButtonBox* buttonBox() { return mButtonBox; }

    //! Returns pointer to the embedded SVG selector widget
    QgsSvgSelectorWidget* svgSelector() { return mSvgSelector; }

  protected:
    QVBoxLayout* mLayout;
    QDialogButtonBox* mButtonBox;
    QgsSvgSelectorWidget* mSvgSelector;
};

#endif // QGSSVGSELECTORWIDGET_H
