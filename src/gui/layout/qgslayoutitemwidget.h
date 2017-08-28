/***************************************************************************
                             qgslayoutitemwidget.h
                             ------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTITEMWIDGET_H
#define QGSLAYOUTITEMWIDGET_H

#include "qgis_gui.h"
#include "qgslayoutobject.h"
#include "qgspanelwidget.h"
#include <QObject>
#include <QPointer>


class QgsPropertyOverrideButton;

// NOTE - the inheritance here is tricky, as we need to avoid the multiple inheritance
// diamond problem and the ideal base object (QgsLayoutConfigObject) MUST be a QObject
// because of its slots.

// So here we go:
// QgsLayoutItemWidget is just a QWidget which is embedded inside specific item property
// widgets and contains common settings like position and rotation of the items. While the
// actual individual item type widgets MUST be QgsPanelWidgets unfortunately QgsLayoutItemWidget
// CANNOT be a QgsPanelWidget and must instead be a generic QWidget (otherwise a QgsPanelWidget
// contains a child QgsPanelWidget, which breaks lots of assumptions made in QgsPanelWidget
// and related classes).
// So QgsLayoutItemWidget HAS a QgsLayoutConfigObject to handle these common tasks.
// Specific item property widgets (e.g., QgsLayoutMapWidget) should inherit from QgsLayoutItemBaseWidget
// (which is a QgsPanelWidget) and also HAS a QgsLayoutConfigObject, with protected methods
// which are just proxied through to the QgsComposerConfigObject.
// phew!
// long story short - don't change this without good reason. If you add a new item type, inherit
// from QgsLayoutItemBaseWidget and trust that everything else has been done for you.

/**
 * \class QgsLayoutConfigObject
 * \ingroup gui
 *
 * An object for property widgets for layout items. All layout config type widgets should contain
 * this object.
 *
 * If you are creating a new QgsLayoutItem configuration widget, you should instead
 * inherit from QgsLayoutItemBaseWidget (rather then directly working with QgsLayoutConfigObject).
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutConfigObject: public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsLayoutConfigObject, linked with the specified \a layoutObject.
     */
    QgsLayoutConfigObject( QWidget *parent SIP_TRANSFERTHIS, QgsLayoutObject *layoutObject );

    /**
     * Registers a data defined \a button, setting up its initial value, connections and description.
     * The corresponding property \a key must be specified.
     */
    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty key );

    /**
     * Updates a data defined button to reflect the item's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    /**
     * Returns the current layout context coverage layer (if set).
     */
    QgsVectorLayer *coverageLayer() const;

#if 0 //TODO
    //! Returns the atlas for the composition
    QgsAtlasComposition *atlasComposition() const;
#endif

  private slots:
    //! Must be called when a data defined button changes
    void updateDataDefinedProperty();

    //! Updates data defined buttons to reflect current state of layout (e.g., coverage layer)
    void updateDataDefinedButtons();

  private:

    QPointer< QgsLayoutObject > mLayoutObject;
};

/**
 * \class QgsLayoutItemBaseWidget
 * \ingroup gui
 *
 * A base class for property widgets for layout items. All layout item widgets should inherit from
 * this base class.
 *
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutItemBaseWidget: public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemBaseWidget, linked with the specified \a layoutObject.
     */
    QgsLayoutItemBaseWidget( QWidget *parent SIP_TRANSFERTHIS, QgsLayoutObject *layoutObject );

  protected:

    /**
     * Registers a data defined \a button, setting up its initial value, connections and description.
     * The corresponding property \a key must be specified.
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty property );

    /**
     * Updates a previously registered data defined button to reflect the item's current properties.
     */
    void updateDataDefinedButton( QgsPropertyOverrideButton *button );

    /**
     * Returns the current layout context coverage layer (if set).
     */
    QgsVectorLayer *coverageLayer() const;

#if 0 //TODO
    //! Returns the atlas for the composition
    QgsAtlasComposition *atlasComposition() const;
#endif

  private:

    QgsLayoutConfigObject *mConfigObject = nullptr;
};

#endif // QGSLAYOUTITEMWIDGET_H
