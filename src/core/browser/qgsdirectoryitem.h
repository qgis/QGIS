/***************************************************************************
                             qgsddirectoryitem.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDIRECTORYITEM_H
#define QGSDIRECTORYITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsdatacollectionitem.h"
#include <QDateTime>
#include <QTreeWidget>

class QFileSystemWatcher;
class QMouseEvent;

/**
 * \ingroup core
 * \brief A directory: contains subdirectories and layers
*/
class CORE_EXPORT QgsDirectoryItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDirectoryItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. This is usually
     * the directory name, but in certain cases may differ for special directories (e.g. "Home").
     * If a non-directory-name text is used, it should be a translated string when appropriate.
     *
     * The \a path argument specifies the directory path in the file system (e.g. "/home/gsherman/stuff"). A valid
     * directory path must be specified.
     */
    QgsDirectoryItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path );


    // TODO QGIS 4.0 -- rename "name" to "title" or "text" or something more descriptive, and "path" to something
    // else to clarify the role of dirPath vs path

    /**
     * Constructor for QgsDirectoryItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. This is usually
     * the directory name, but in certain cases may differ for special directories (e.g. "Home").
     * If a non-directory-name text is used, it should be a translated string when appropriate.
     *
     * The \a dirPath argument specifies the directory path in the file system (e.g. "/home/gsherman/stuff"). A valid
     * directory path must be specified.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form, but is usually
     * the same as \a dirPath or \a dirPath with a prefix, e.g. "favorites:/home/gsherman/Downloads"). QgsDirectoryItem
     * items pointing to different \a dirPaths should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDirectoryItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &dirPath, const QString &path, const QString &providerKey = QString() );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsDirectoryItem: %1 - %2>" ).arg( sipCpp->dirPath(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif
    void setState( Qgis::BrowserItemState state ) override;

    QVector<QgsDataItem *> createChildren() override;

    /**
     * Returns the full path to the directory the item represents.
     */
    QString dirPath() const { return mDirPath; }

    bool equal( const QgsDataItem *other ) override;
    QIcon icon() override;

    /**
     * Returns the directory's icon color.
     *
     * An invalid color will be returned if the default icon color is used.
     *
     * \see setIconColor()
     * \since QGIS 3.20
     */
    QColor iconColor() const;

    /**
     * Sets the directory's icon \a color.
     *
     * Setting an invalid color will cause the default icon color to be used.
     *
     * This is a transient property, and will not permanently alter the directory's colors
     * in future QGIS sessions. Use setCustomColor() to permanently set the directory's color.
     *
     * \see iconColor()
     * \since QGIS 3.20
     */
    void setIconColor( const QColor &color );

    /**
     * Sets a custom icon \a color to use for the items for the corresponding \a directory path.
     *
     * If \a color is an invalid color then the default icon color will be used.
     *
     * \since QGIS 3.20
     */
    static void setCustomColor( const QString &directory, const QColor &color );

    Q_DECL_DEPRECATED QWidget *paramWidget() override SIP_FACTORY SIP_DEPRECATED;
    bool hasDragEnabled() const override { return true; }
    QgsMimeDataUtils::UriList mimeUris() const override;

    //! Check if the given path is hidden from the browser model
    static bool hiddenPath( const QString &path );

    /**
     * Returns the monitoring setting for a directory \a path.
     *
     * This method returns the monitoring setting for \a path only. If no explicit monitoring setting
     * is in place for the path then Qgis::BrowserDirectoryMonitoring::Default is returned.
     *
     * This method does not consider the monitoring setting of parent directories.
     *
     * \since QGIS 3.20
     */
    static Qgis::BrowserDirectoryMonitoring monitoringForPath( const QString &path );

    /**
     * Returns TRUE if a directory \a path should be monitored by default.
     *
     * In the absence of any other settings this will dictate whether the directory is monitored. This method
     * does not consider an explicit monitoring setting set for the path, which can be determined by
     * calling monitoringForPath().
     *
     * All parent directories will be checked to determine whether they have monitoring
     * manually enabled or disabled. As soon as a parent directory is found which has monitoring
     * manually enabled or disabled then the corresponding value will be returned.
     *
     * If no explicit setting is in place for a parent directory, then a check will be made to determine
     * whether the path resides on a known slow drive. If so, monitoring is disabled by default and
     * FALSE will be returned. Otherwise paths are monitored by default and the function will return TRUE.
     *
     * \see isMonitored()
     * \see setMonitoring()
     * \since QGIS 3.20
     */
    static bool pathShouldByMonitoredByDefault( const QString &path );

    /**
     * Returns TRUE if the directory is currently being monitored for changes and the item auto-refreshed
     * when these occur.
     *
     * \since QGIS 3.20
     */
    bool isMonitored() const { return mMonitored; }

    /**
     * Returns the monitoring setting for this directory item.
     *
     * \see setMonitoring()
     * \since QGIS 3.20
     */
    Qgis::BrowserDirectoryMonitoring monitoring() const;

    /**
     * Sets the \a monitoring setting for this directory.
     *
     * This is a persistent setting, which is saved in QSettings.
     *
     * \see monitoring()
     * \since QGIS 3.20
     */
    void setMonitoring( Qgis::BrowserDirectoryMonitoring monitoring );

    /**
     * Re-evaluate whether the directory item should be monitored for changes.
     *
     * Should be called whenever the parent directory item's monitoring is overridden.
     *
     * \since QGIS 3.20
     */
    void reevaluateMonitoring() SIP_SKIP;

  public slots:
    void childrenCreated() override;
    void directoryChanged();

  protected:

    QString mDirPath;

  private:

    void init( const QString &dirName );

    void createOrDestroyFileSystemWatcher();

    Qgis::BrowserDirectoryMonitoring mMonitoring = Qgis::BrowserDirectoryMonitoring::Default;
    bool mMonitored = true;
    QFileSystemWatcher *mFileSystemWatcher = nullptr;
    bool mRefreshLater = false;
    QDateTime mLastScan;
    QColor mIconColor;

    bool mIsDir = false;
    bool mIsSymLink = false;

    friend class TestQgsDataItem;
};

// ---------

// TODO: move to qgis_gui for QGIS 4

/**
 * \ingroup core
 * \class QgsDirectoryParamWidget
 *
 * \brief Browser parameter widget implementation for directory items.
 */
class CORE_EXPORT QgsDirectoryParamWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    QgsDirectoryParamWidget( const QString &path, QWidget *parent SIP_TRANSFERTHIS = nullptr );

  protected:
    void mousePressEvent( QMouseEvent *event ) override;

  public slots:
    void showHideColumn();
};


#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief A directory item showing the current project directory.
 * \note Not available in Python bindings.
*/
class CORE_EXPORT QgsProjectHomeItem : public QgsDirectoryItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectHomeItem.
     */
    QgsProjectHomeItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path );

    QIcon icon() override;
    QVariant sortKey() const override;

};

#endif

#endif // QGSDATAITEM_H


