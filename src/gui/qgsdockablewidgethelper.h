/***************************************************************************
  qgsdockablewidgethelper.h
  --------------------------------------
  Date                 : January 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDOCKABLEWIDGETHELPER_H
#define QGSDOCKABLEWIDGETHELPER_H

#include "qgis_gui.h"

#include <QDialog>
#include <QToolButton>
#include <QMainWindow>
#include <QDomElement>
#include <QPointer>

#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgsgui.h"

#define SIP_NO_FILE

class QgsDockWidget;

///@cond PRIVATE

class GUI_EXPORT QgsNonRejectableDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit QgsNonRejectableDialog( QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );
    void reject() override;
};

/**
 * This class is responible of displaying a QWidget inside a top level window or a dock widget (only one of the 2 is alive at most).
 * The widget is not owned by this class and its ownership is passed to the owner window before this class's object is deleted or
 * another widget is set using setWidget() function
 *
 * \note Not available from Python bindings
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsDockableWidgetHelper : public QObject
{
    static inline QgsSettingsTreeNode *sTtreeDockConfigs = QgsGui::sTtreeWidgetGeometry->createNamedListNode( QStringLiteral( "docks" ) ) SIP_SKIP;

    static const QgsSettingsEntryBool *sSettingsIsDocked SIP_SKIP;
    static const QgsSettingsEntryVariant *sSettingsDockGeometry SIP_SKIP;
    static const QgsSettingsEntryVariant *sSettingsDialogGeometry SIP_SKIP;
    static const QgsSettingsEntryEnumFlag<Qt::DockWidgetArea> *sSettingsDockArea SIP_SKIP;

    Q_OBJECT
  public:
    enum class OpeningMode : int
    {
      RespectSetting, //! Respect the setting used
      ForceDocked,    //! Force the widget to be docked, despite its settings
      ForceDialog,    //! Force the widget to be shown in a dialog, despite its settings
    };

    enum class Option : int
    {
      RaiseTab = 1 << 1,        //!< Raise Tab
      PermanentWidget = 1 << 2, //!< The widget (either as a dock or window) cannot be destroyed and must be hidden instead
    };
    Q_ENUM( Option )
    Q_DECLARE_FLAGS( Options, Option )

    /**
     * Constructs an object that is responsible of making a docked widget or a window titled \a windowTitle that holds the \a widget
     * The ownership of \a widget is returned to \a ownerWindow once the object is destroyed.
     *
     * With a unique \a dockId, the status (docked, area and geometry) are saved in the settings and re-used on creation.
     * The default values of the settings can be overridden with \a defaultIsDocked and \a defaultDockArea.
     *
     * \since QGIS 3.42
     */
    QgsDockableWidgetHelper(
      const QString &windowTitle,
      QWidget *widget,
      QMainWindow *ownerWindow,
      const QString &dockId,
      const QStringList &tabifyWith = QStringList(),
      OpeningMode openingMode = OpeningMode::RespectSetting,
      bool defaultIsDocked = false,
      Qt::DockWidgetArea defaultDockArea = Qt::DockWidgetArea::RightDockWidgetArea,
      Options options = Options()
    );

    ~QgsDockableWidgetHelper();

    //! Returns if the widget is docked
    //! \since 3.42
    bool isDocked() const { return mIsDocked; }

    //! Reads the dimensions of both the dock widget and the top level window
    void writeXml( QDomElement &viewDom );
    void readXml( const QDomElement &viewDom );

    //! Sets the widget placed inside the dock widget or the window
    void setWidget( QWidget *widget );
    //! Return the widget placed inside the dock widget or the window
    QWidget *widget() { return mWidget; }

    //! Returns the dock widget if we are in docking mode and nullptr otherwise.
    QgsDockWidget *dockWidget();
    //! Returns the dialog window if we are in top level window mode and nullptr otherwise.
    QDialog *dialog();

    //! Sets the displayed title of the dialog and the dock widget
    void setWindowTitle( const QString &title );
    //! Returns the displayed title of the dialog and the dock widget
    QString windowTitle() const { return mWindowTitle; }

    //! Sets the object name of the dock widget
    void setDockObjectName( const QString &name );
    //! Returns the object name of the dock widget
    QString dockObjectName() const;

    /**
     * Returns TRUE if the widget is a visible dialog or a user-visible
     * dock widget.
     */
    bool isUserVisible() const;

    /**
     * Create a tool button for docking/undocking the widget
     * \note The ownership of the tool button is managed by the caller
     */
    QToolButton *createDockUndockToolButton();

    /**
     * Create an action for docking/undocking the widget, with the specified \a parent widget.
     */
    QAction *createDockUndockAction( const QString &title, QWidget *parent );

    bool eventFilter( QObject *watched, QEvent *event ) override;

    static std::function<void( Qt::DockWidgetArea, QDockWidget *, const QStringList &, bool )> sAddTabifiedDockWidgetFunction;
    static std::function<QString()> sAppStylesheetFunction;

    static QMainWindow *sOwnerWindow;

  signals:
    void closed();

    void dockModeToggled( bool docked );

    void visibilityChanged( bool isVisible );

  public slots:
    void toggleDockMode( bool docked );

    void setUserVisible( bool visible );

  private:
    void setupDockWidget( const QStringList &tabSiblings = QStringList() );

    bool mIsDocked = true;
    QWidget *mWidget = nullptr;

    QPointer<QDialog> mDialog;
    QRect mDialogGeometry;

    QPointer<QgsDockWidget> mDock;
    QRect mDockGeometry;
    bool mIsDockFloating = true;
    Qt::DockWidgetArea mDockArea = Qt::RightDockWidgetArea;

    QString mWindowTitle;
    QString mObjectName;
    QMainWindow *mOwnerWindow = nullptr;

    QStringList mTabifyWith;
    Options mOptions;

    // Unique identifier of dock
    QString mUuid;


    const QString mSettingKeyDockId;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDockableWidgetHelper::Options )


///@endcond

#endif // QGSDOCKABLEWIDGETHELPER_H
