/***************************************************************************
    qgsnative.h - abstracted interface to native system calls
                             -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNATIVE_H
#define QGSNATIVE_H

#include "qgis_native.h"
#include <QImage>
#include <QVariant>
#include <vector>
#include <QObject>

class QString;
class QWindow;

/**
 * \class QgsNative
 * \ingroup native
 * Base class for implementing methods for native system calls that
 * are implemented in subclasses to provide platform abstraction.
 * \since QGIS 3.0
 */
class NATIVE_EXPORT QgsNative : public QObject
{
    Q_OBJECT

  public:

    //! Native interface capabilities
    enum Capability
    {
      NativeDesktopNotifications = 1 << 1, //!< Native desktop notifications are supported. See showDesktopNotification().
      NativeFilePropertiesDialog = 1 << 2, //!< Platform can show a native "file" (or folder) properties dialog.
      NativeOpenTerminalAtPath = 1 << 3, //!< Platform can open a terminal (command line) at a specific path
    };
    Q_DECLARE_FLAGS( Capabilities, Capability )

    virtual ~QgsNative() = default;

    /**
     * Called on QGIS exit, allowing the native interface to gracefully
     * cleanup and exit.
     */
    virtual void cleanup();

    /**
     * Returns the native interface's capabilities.
     */
    virtual Capabilities capabilities() const;

    /**
     * Initializes the native interface, using the specified \a window.
     *
     * The \a applicationName, \a organizationName and \a version information
     * are used to initialize application-wide settings, depending on the platform.
     *
     * The default implementation does nothing.
     *
     * \since QGIS 3.4
     */
    virtual void initializeMainWindow( QWindow *window,
                                       const QString &applicationName,
                                       const QString &organizationName,
                                       const QString &version );

    /**
     * Brings the QGIS app to front. The default implementation does nothing.
     */
    virtual void currentAppActivateIgnoringOtherApps();

    /**
     * Opens the desktop file explorer at the folder containing \a path,
     * and (if possible) scrolls to and pre-selects the file at \a path itself.
     *
     * The default implementation just calls the QDesktopServices method to open the folder,
     * without selecting the specified file.
     *
     * \since QGIS 3.4
     */
    virtual void openFileExplorerAndSelectFile( const QString &path );

    /**
     * Opens the desktop explorer file (or folder) properties dialog, for the given \a path.
     *
     * The default implementation does nothing. Platforms which implement this interface should
     * return the QgsNative::NativeFilePropertiesDialog capability.
     *
     * \since QGIS 3.6
     */
    virtual void showFileProperties( const QString &path );

    /**
     * Shows the application progress report, using an "undefined" total
     * type progress (i.e. the platform's way of showing that a task
     * is occurring with an unknown progress).
     *
     * The default implementation does nothing.
     *
     * \see setApplicationProgress()
     * \see hideApplicationProgress()
     * \since QGIS 3.4
     */
    virtual void showUndefinedApplicationProgress();

    /**
     * Shows the application progress report, with the
     * specified \a progress (in percent).
     *
     * The default implementation does nothing.
     *
     * \see showUndefinedApplicationProgress()
     * \see hideApplicationProgress()
     * \since QGIS 3.4
     */
    virtual void setApplicationProgress( double progress );

    /**
     * Hides the application progress report.
     *
     * The default implementation does nothing.
     *
     * \see showUndefinedApplicationProgress()
     * \see setApplicationProgress()
     * \since QGIS 3.4
     */
    virtual void hideApplicationProgress();

    /**
     * Shows an application badge count.
     *
     * The default implementation does nothing.
     * \since QGIS 3.4
     */
    virtual void setApplicationBadgeCount( int count );

    /**
     * Returns TRUE if the operating system is set to utilize a "dark" theme.
     * \since QGIS 3.4
     */
    virtual bool hasDarkTheme();

    /**
     * Opens a terminal (command line) window at the given \a path.
     *
     * This method is only supported when the interface returns the NativeOpenTerminalAtPath flag for capabilities().
     *
     * Returns TRUE if terminal was successfully opened.
     */
    virtual bool openTerminalAtPath( const QString &path );

    /**
     * Notification settings, for use with showDesktopNotification().
     */
    struct NotificationSettings
    {
      NotificationSettings() {}

      //! Optional image to show in notification
      QImage image;

      //! Whether the notification should be transient (the meaning varies depending on platform)
      bool transient = true;

      //! Path to application icon in SVG format
      QString svgAppIconPath;

      //! Path to application icon in png format
      QString pngAppIconPath;

      //! Message ID, used to replace existing messages
      QVariant messageId;
    };

    /**
     * Result of sending a desktop notification, returned by showDesktopNotification().
     */
    struct NotificationResult
    {
      NotificationResult() {}

      //! True if notification was successfully sent.
      bool successful = false;

      //! Unique notification message ID, used by some platforms to identify the notification
      QVariant messageId;
    };

    /**
     * Shows a native desktop notification.
     *
     * The \a summary argument specifies a short title for the notification, and the \a body argument
     * specifies the complete notification message text.
     *
     * The \a settings argument allows fine-tuning of the notification, using a range of settings which
     * may or may not be respected on all platforms. It is recommended to set as many of these properties
     * as is applicable, and let the native implementation choose which to use based on the platform's
     * capabilities.
     *
     * This method is only supported when the interface returns the NativeDesktopNotifications flag for capabilities().
     *
     * Returns TRUE if notification was successfully sent.
     */
    virtual NotificationResult showDesktopNotification( const QString &summary, const QString &body, const NotificationSettings &settings = NotificationSettings() );

    /**
     * Contains properties of a recently used project.
     */
    struct RecentProjectProperties
    {
      //! Project name (will be project title if set, otherwise project filename)
      QString name;

      //! Project title, if set
      QString title;

      //! Project filename
      QString fileName;

      //! Full project path
      QString path;
    };

    /**
     * Called whenever the list of recently used projects is modified.
     *
     * The \a recentProjects list contains a list of recently used projects, with the
     * most recently used listed first.
     *
     * The default implementation does nothing.
     *
     * \since QGIS 3.4
     */
    virtual void onRecentProjectsChanged( const std::vector< RecentProjectProperties > &recentProjects );

  signals:

    /**
     * Emitted whenever a USB storage device has been inserted or removed.
     *
     * The \a path argument gives the file path to the device (if available).
     *
     * If \a inserted is TRUE then the device was inserted. If \a inserted is FALSE then
     * the device was removed.
     *
     * \since QGIS 3.4
     */
    void usbStorageNotification( const QString &path, bool inserted );

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsNative::Capabilities )

#endif // QGSNATIVE_H
