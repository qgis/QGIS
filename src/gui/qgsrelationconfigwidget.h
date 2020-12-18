/***************************************************************************
                         qgsrelationconfigwidget.h
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONCONFIGBASEWIDGET_H
#define QGSRELATIONCONFIGBASEWIDGET_H

#include <QWidget>
#include "qgis_sip.h"
#include "qgis_gui.h"

#include "qgseditorwidgetwrapper.h"

class QgsRelation;
class QgsPropertyOverrideButton;

/**
 * \ingroup gui
 * This class should be subclassed for every configurable relation widget type.
 *
 * It implements the GUI configuration widget and transforms this to/from a configuration.
 *
 * It will only be instantiated by {\see QgsRelationWidgetFactory}
 * \since QGIS 3.18
 */

class GUI_EXPORT QgsRelationConfigWidget : public QWidget
{
    Q_OBJECT
  public:

    /**
     * Create a new configuration widget
     *
     * \param relation    The relation for which the configuration dialog will be created
     * \param parent      A parent widget
     */
    explicit QgsRelationConfigWidget( const QgsRelation &relation, QWidget *parent SIP_TRANSFERTHIS );

    /**
     * \brief Create a configuration from the current GUI state
     *
     * \returns A widget configuration
     */
    virtual QVariantMap config() = 0;

    /**
     * \brief Update the configuration widget to represent the given configuration.
     *
     * \param config The configuration which should be represented by this widget
     */
    virtual void setConfig( const QVariantMap &config ) = 0;

    /**
     * Returns the layer for which this configuration widget applies
     *
     * \returns The layer
     */
    QgsVectorLayer *layer();

    /**
     * Returns the relation for which this configuration widget applies
     *
     * \returns The relation
     */
    QgsRelation relation() const;


  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsRelation mRelation;
};

#endif // QGSRELATIONCONFIGBASEWIDGET_H
