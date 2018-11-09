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
#include "qgis.h"

#include "qgsguiutils.h"
#include <QAbstractListModel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLayout>
#include <QStandardItemModel>
#include <QWidget>
#include <QThread>
#include <QElapsedTimer>
#include "qgis_gui.h"

class QCheckBox;
class QLayout;
class QLineEdit;
class QListView;
class QPushButton;
class QTreeView;


#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \ingroup gui
 * \class QgsSvgSelectorLoader
 * Recursively loads SVG images from a path in a background thread.
 * \since QGIS 2.18
 */
class GUI_EXPORT QgsSvgSelectorLoader : public QThread
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgSelectorLoader
     * \param parent parent object
     */
    QgsSvgSelectorLoader( QObject *parent = nullptr );

    ~QgsSvgSelectorLoader() override;

    /**
     * Starts the loader finding and generating previews for SVG images. foundSvgs() will be
     * emitted as the loader encounters SVG images.
     * \brief run
     */
    void run() override;

    /**
     * Cancels the current loading operation. Waits until the thread has finished operation
     * before returning.
     */
    virtual void stop();

    /**
     * Sets the root path containing SVG images to load. If no path is set, the default SVG
     * search paths will be used instead.
     */
    void setPath( const QString &path )
    {
      mPath = path;
    }

  signals:

    /**
     * Emitted when the loader has found a block of SVG images. This signal is emitted with blocks
     * of SVG images to prevent spamming any connected model.
     * \param svgs list of SVGs and preview images found.
     */
    void foundSvgs( QStringList svgs );

  private:

    QString mPath;
    bool mCanceled = false;
    QStringList mQueuedSvgs;

    QElapsedTimer mTimer;
    int mTimerThreshold = 0;
    QSet< QString > mTraversedPaths;

    void loadPath( const QString &path );
    void loadImages( const QString &path );

};

/**
 * \ingroup gui
 * \class QgsSvgGroupLoader
 * Recursively loads SVG paths in a background thread.
 * \since QGIS 2.18
 */
class GUI_EXPORT QgsSvgGroupLoader : public QThread
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgGroupLoader
     * \param parent parent object
     */
    QgsSvgGroupLoader( QObject *parent = nullptr );

    ~QgsSvgGroupLoader() override;

    /**
     * Starts the loader finding folders for SVG images.
     * \brief run
     */
    void run() override;

    /**
     * Cancels the current loading operation. Waits until the thread has finished operation
     * before returning.
     */
    virtual void stop();

    /**
     * Sets the root path containing child paths to find. If no path is set, the default SVG
     * search paths will be used instead.
     */
    void setParentPaths( const QStringList &parentPaths )
    {
      mParentPaths = parentPaths;
    }

  signals:

    /**
     * Emitted when the loader has found a block of SVG images. This signal is emitted with blocks
     * of SVG images to prevent spamming any connected model.
     * \param svgs list of SVGs and preview images found.
     */
    void foundPath( const QString &parentPath, const QString &path );

  private:

    QStringList mParentPaths;
    bool mCanceled = false;
    QSet< QString > mTraversedPaths;

    void loadGroup( const QString &parentPath );

};

///@endcond
#endif

/**
 * \ingroup gui
 * \class QgsSvgSelectorListModel
 * A model for displaying SVG files with a preview icon. Population of the model is performed in
 * a background thread to ensure that initial creation of the model is responsive and does
 * not block the GUI.
 */
class GUI_EXPORT QgsSvgSelectorListModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgSelectorListModel. All SVGs in folders from the application SVG
     * search paths will be shown.
     * \param parent parent object
     * \param iconSize desired size of SVG icons to create
     */
    QgsSvgSelectorListModel( QObject *parent SIP_TRANSFERTHIS, int iconSize = 30 );

    /**
     * Constructor for creating a model for SVG files in a specific path.
     * \param parent parent object
     * \param path initial path, which is recursively searched
     * \param iconSize desired size of SVG icons to create
     */
    QgsSvgSelectorListModel( QObject *parent SIP_TRANSFERTHIS, const QString &path, int iconSize = 30 );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

  protected:
    QStringList mSvgFiles;

  private:
    QPixmap createPreview( const QString &entry ) const;
    QgsSvgSelectorLoader *mSvgLoader = nullptr;

    int mIconSize = 30;

  private slots:

    /**
     * Called to add SVG files to the model.
     * \param svgs list of SVG files to add to model.
     */
    void addSvgs( const QStringList &svgs );

};


/**
 * \ingroup gui
 * \class QgsSvgSelectorGroupsModel
 * A model for displaying SVG search paths. Population of the model is performed in
 * a background thread to ensure that initial creation of the model is responsive and does
 * not block the GUI.
 */
class GUI_EXPORT QgsSvgSelectorGroupsModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    QgsSvgSelectorGroupsModel( QObject *parent SIP_TRANSFERTHIS );
    ~QgsSvgSelectorGroupsModel() override;

  private:
    QgsSvgGroupLoader *mLoader = nullptr;
    QHash< QString, QStandardItem * > mPathItemHash;

  private slots:

    void addPath( const QString &parentPath, const QString &path );
};

/**
 * \ingroup gui
 * \class QgsSvgSelectorWidget
 */
class GUI_EXPORT QgsSvgSelectorWidget : public QWidget, private Ui::WidgetSvgSelector
{
    Q_OBJECT

  public:

    //! Constructor for QgsSvgSelectorWidget
    QgsSvgSelectorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QString currentSvgPath() const;

  public slots:
    //! Accepts absolute paths
    void setSvgPath( const QString &svgPath );

  signals:
    void svgSelected( const QString &path );

  protected:
    void populateList();

  private slots:
    void populateIcons( const QModelIndex &idx );
    void svgSelectionChanged( const QModelIndex &idx );
    void updateCurrentSvgPath( const QString &svgPath );
    void svgSourceChanged( const QString &text );

  private:

    int mIconSize = 30;

    QString mCurrentSvgPath; //!< Always stored as absolute path

};

/**
 * \ingroup gui
 * \class QgsSvgSelectorDialog
 */
class GUI_EXPORT QgsSvgSelectorDialog : public QDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSvgSelectorDialog.
     */
    QgsSvgSelectorDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                          Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                          QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close | QDialogButtonBox::Ok,
                          Qt::Orientation orientation = Qt::Horizontal );
    ~QgsSvgSelectorDialog() override;

    //! Returns pointer to the embedded SVG selector widget
    QgsSvgSelectorWidget *svgSelector() { return mSvgSelector; }

  protected:
    QVBoxLayout *mLayout = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
    QgsSvgSelectorWidget *mSvgSelector = nullptr;
};

#endif // QGSSVGSELECTORWIDGET_H
