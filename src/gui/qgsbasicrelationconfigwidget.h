/***************************************************************************
                         qgsbasicrelationconfigwidget.h
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


#ifndef QGSBASICRELATIONCONFIGWIDGET_H
#define QGSBASICRELATIONCONFIGWIDGET_H


#include "ui_qgsrelationeditorconfigwidgetbase.h"
#include "qgsrelationconfigwidget.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsBasicRelationConfigWidget
 * Creates a new configuration widget for the basic relation widget
 * \since QGIS 3.18
 */
class QgsBasicRelationConfigWidget : public QgsRelationConfigWidget, private Ui::QgsRelationEditorConfigWidgetBase
{
  public:

    /**
     * Create a new configuration widget
     *
     * \param relation    The relation for which the configuration dialog will be created
     * \param parent      A parent widget
     */
    explicit QgsBasicRelationConfigWidget( const QgsRelation &relation, QWidget *parent SIP_TRANSFERTHIS );

    /**
     * \brief Create a configuration from the current GUI state
     *
     * \returns A widget configuration
     */
    QVariantMap config();

    /**
     * \brief Update the configuration widget to represent the given configuration.
     *
     * \param config The configuration which should be represented by this widget
     */
    void setConfig( const QVariantMap &config );

};

#endif // QGSBASICRELATIONCONFIGWIDGET_H
