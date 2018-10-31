/***************************************************************************
    qgsbrowserdockwidget_p.h

    Private classes for QgsBrowserDockWidget

    ---------------------
    begin                : May 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    real work done by    : (C) 2011 by Martin Dobias
    email                : a dot pasotti at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSBROWSERDOCKWIDGET_P_H
#define QGSBROWSERDOCKWIDGET_P_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//


#include "ui_qgsbrowserdockwidgetbase.h"
#include "ui_qgsbrowserlayerpropertiesbase.h"
#include "ui_qgsbrowserdirectorypropertiesbase.h"
#include "ui_qgsbrowserpropertiesdialogbase.h"

#include "qgsdataitem.h"
#include "qgsbrowsertreeview.h"
#include "qgsdockwidget.h"
#include <QSortFilterProxyModel>

class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;

#define SIP_NO_FILE

/**
 * Hack to show wrapped text without spaces
 */
class QgsBrowserPropertiesWrapLabel : public QTextEdit
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserPropertiesWrapLabel
      * \param text label text
      * \param parent parent widget
      */
    QgsBrowserPropertiesWrapLabel( const QString &text, QWidget *parent = nullptr );

  private slots:
    void adjustHeight( QSizeF size );
};

/**
 * The QgsBrowserPropertiesWidget base class
 */
class QgsBrowserPropertiesWidget : public QWidget
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserPropertiesWidget
      * \param parent parent widget
      */
    explicit QgsBrowserPropertiesWidget( QWidget *parent = nullptr );
    //! Factory method to create a new browser properties widget
    static QgsBrowserPropertiesWidget *createWidget( QgsDataItem *item, QWidget *parent = nullptr );
    //! Stub
    virtual void setItem( QgsDataItem *item ) { Q_UNUSED( item ) }
    //! Sets content widget, usually item paramWidget. Takes ownership.
    virtual void setWidget( QWidget *widget );

    /**
     * Sets whether the properties widget should display in condensed mode, ie, for display in a dock
     * widget rather than it's own separate dialog.
     * \param condensedMode set to true to enable condensed mode
     * \since QGIS 2.10
     */
    virtual void setCondensedMode( bool condensedMode ) { Q_UNUSED( condensedMode ); }
};

/**
 * The QgsBrowserLayerProperties class
 */
class QgsBrowserLayerProperties : public QgsBrowserPropertiesWidget, private Ui::QgsBrowserLayerPropertiesBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserLayerProperties
      * \param parent parent widget
      */
    explicit QgsBrowserLayerProperties( QWidget *parent = nullptr );
    //! Sets item
    void setItem( QgsDataItem *item ) override;

    /**
     * Sets whether the properties widget should display in condensed mode, ie, for display in a dock
     * widget rather than it's own separate dialog.
     * \param condensedMode set to true to enable condensed mode
     * \since QGIS 2.10
     */
    void setCondensedMode( bool condensedMode ) override;

  private slots:

    void urlClicked( const QUrl &url );

  private:
    std::unique_ptr<QgsMapLayer> mLayer;

};

/**
 * The QgsBrowserDirectoryProperties class
 */
class QgsBrowserDirectoryProperties : public QgsBrowserPropertiesWidget, private Ui::QgsBrowserDirectoryPropertiesBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserDirectoryProperties
      * \param parent parent widget
      */
    explicit QgsBrowserDirectoryProperties( QWidget *parent = nullptr );

    //! Create widget from the given item and add it
    void setItem( QgsDataItem *item ) override;
  private:
    QgsDirectoryParamWidget *mDirectoryWidget = nullptr;
    QgsBrowserPropertiesWrapLabel *mPathLabel = nullptr;
};

/**
 * The QgsBrowserPropertiesDialog class
 */
class QgsBrowserPropertiesDialog : public QDialog, private Ui::QgsBrowserPropertiesDialogBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserPropertiesDialog
      * \param settingsSection prefix for settings (from the object name)
      * \param parent parent widget
      */
    QgsBrowserPropertiesDialog( const QString &settingsSection, QWidget *parent = nullptr );
    ~QgsBrowserPropertiesDialog() override;

    //! Create dialog from the given item and add it
    void setItem( QgsDataItem *item );

  private:
    QgsBrowserPropertiesWidget *mPropertiesWidget = nullptr;
    QString mSettingsSection;
};


/**
 * Utility class for correct drag&drop handling.
 *
 * We want to allow user to drag layers to qgis window. At the same time we do not
 * accept drops of the items on our view - but if we ignore the drag enter action
 * then qgis application consumes the drag events and it is possible to drop the
 * items on the tree view although the drop is actually managed by qgis app.
 */
class QgsDockBrowserTreeView : public QgsBrowserTreeView
{
    Q_OBJECT

  public:

    /**
      * Constructor for QgsDockBrowserTreeView
      * \param parent parent widget
      */
    explicit QgsDockBrowserTreeView( QWidget *parent );
    //! Overrides drag enter event
    void dragEnterEvent( QDragEnterEvent *e ) override;
    //! Overrides drag move event
    void dragMoveEvent( QDragMoveEvent *e ) override;
    //! Overrides drag stop event
    void dropEvent( QDropEvent *e ) override;

  private:
    void setAction( QDropEvent *e );
};

/// @endcond

#endif // QGSBROWSERDOCKWIDGET_P_H
