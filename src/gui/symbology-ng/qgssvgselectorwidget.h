/***************************************************************************
    qgssvgselectorwidget.h - group and preview selector for SVG files
                             built off of work in qgssymbollayerwidget

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
#include <QThread>
#include <QElapsedTimer>

class QCheckBox;
class QLayout;
class QLineEdit;
class QListView;
class QPushButton;
class QTreeView;

///@cond PRIVATE

/** \ingroup gui
 * \class QgsSvgSelectorLoader
 * Recursively loads SVG images from a path in a background thread.
 * \note added in QGIS 2.18
 */
class GUI_EXPORT QgsSvgSelectorLoader : public QThread
{
    Q_OBJECT

  public:

    /** Constructor for QgsSvgSelectorLoader
     * @param parent parent object
     */
    QgsSvgSelectorLoader( QObject* parent = nullptr );

    ~QgsSvgSelectorLoader();

    /** Starts the loader finding and generating previews for SVG images. foundSvgs() will be
     * emitted as the loader encounters SVG images.
     * @brief run
     */
    virtual void run() override;

    /** Cancels the current loading operation. Waits until the thread has finished operation
     * before returning.
     */
    virtual void stop();

    /** Sets the root path containing SVG images to load. If no path is set, the default SVG
     * search paths will be used instead.
     */
    void setPath( const QString& path )
    {
      mPath = path;
    }

  signals:

    /** Emitted when the loader has found a block of SVG images. This signal is emitted with blocks
     * of SVG images to prevent spamming any connected model.
     * @param svgs list of SVGs and preview images found.
     */
    void foundSvgs( QStringList svgs );

  private:

    QString mPath;
    bool mCancelled;
    QStringList mQueuedSvgs;

    QElapsedTimer mTimer;
    int mTimerThreshold;
    QSet< QString > mTraversedPaths;

    void loadPath( const QString& path );
    void loadImages( const QString& path );

};

/** \ingroup gui
 * \class QgsSvgGroupLoader
 * Recursively loads SVG paths in a background thread.
 * \note added in QGIS 2.18
 */
class GUI_EXPORT QgsSvgGroupLoader : public QThread
{
    Q_OBJECT

  public:

    /** Constructor for QgsSvgGroupLoader
     * @param parent parent object
     */
    QgsSvgGroupLoader( QObject* parent = nullptr );

    ~QgsSvgGroupLoader();

    /** Starts the loader finding folders for SVG images.
     * @brief run
     */
    virtual void run() override;

    /** Cancels the current loading operation. Waits until the thread has finished operation
     * before returning.
     */
    virtual void stop();

    /** Sets the root path containing child paths to find. If no path is set, the default SVG
     * search paths will be used instead.
     */
    void setParentPaths( const QStringList& parentPaths )
    {
      mParentPaths = parentPaths;
    }

  signals:

    /** Emitted when the loader has found a block of SVG images. This signal is emitted with blocks
     * of SVG images to prevent spamming any connected model.
     * @param svgs list of SVGs and preview images found.
     */
    void foundPath( const QString& parentPath, const QString& path );

  private:

    QStringList mParentPaths;
    bool mCancelled;
    QSet< QString > mTraversedPaths;

    void loadGroup( const QString& parentPath );

};

///@endcond
///

/** \ingroup gui
 * \class QgsSvgSelectorListModel
 * A model for displaying SVG files with a preview icon. Population of the model is performed in
 * a background thread to ensure that initial creation of the model is responsive and does
 * not block the GUI.
 */
class GUI_EXPORT QgsSvgSelectorListModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    /** Constructor for QgsSvgSelectorListModel. All SVGs in folders from the application SVG
     * search paths will be shown.
     * @param parent parent object
     */
    QgsSvgSelectorListModel( QObject* parent );

    /** Constructor for creating a model for SVG files in a specific path.
     * @param parent parent object
     * @param path initial path, which is recursively searched
     */
    QgsSvgSelectorListModel( QObject* parent, const QString& path );

    int rowCount( const QModelIndex & parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const override;

  protected:
    QStringList mSvgFiles;

  private:
    QPixmap createPreview( const QString& entry ) const;
    QgsSvgSelectorLoader* mSvgLoader;

  private slots:

    /** Called to add SVG files to the model.
     * @param svgs list of SVG files to add to model.
     */
    void addSvgs( const QStringList& svgs );

};


/** \ingroup gui
 * \class QgsSvgSelectorGroupsModel
 * A model for displaying SVG search paths. Population of the model is performed in
 * a background thread to ensure that initial creation of the model is responsive and does
 * not block the GUI.
 */
class GUI_EXPORT QgsSvgSelectorGroupsModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    QgsSvgSelectorGroupsModel( QObject* parent );
    ~QgsSvgSelectorGroupsModel();

  private:
    QgsSvgGroupLoader* mLoader;
    QHash< QString, QStandardItem* > mPathItemHash;

  private slots:

    void addPath( const QString& parentPath, const QString& path );
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
    QgsSvgSelectorDialog( QWidget* parent = nullptr, Qt::WindowFlags fl = QgisGui::ModalDialogFlags,
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
