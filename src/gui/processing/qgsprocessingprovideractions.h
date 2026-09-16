/***************************************************************************
                             qgsprocessingprovideractions.h
                             ----------------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPROVIDERACTIONS_H
#define QGSPROCESSINGPROVIDERACTIONS_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QPointer>

class QgsDockWidget;

/**
 * \ingroup gui
 * \brief Encapsulates the context in which a Processing action is executed.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingActionContext
{
  public:
    /**
   * Returns the parent widget to use for dialogs constructed when triggering the action.
   *
   * \see setParentWidget()
   */
    QWidget *parentWidget() const;

    /**
   * Sets the parent \a widget to use for dialogs constructed when triggering the action.
   *
   * \see parentWidget()
   */
    void setParentWidget( QWidget *widget );

  private:
    QWidget *mParentWidget = nullptr;
};

/**
 * \ingroup gui
 * \brief A custom action to show in the Processing toolbox.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingToolboxAction
{
  public:
    //TODO QGIS 5.0 -- name and group should be mandatory
    /**
     * Constructor for QgsProcessingToolboxAction.
     */
    QgsProcessingToolboxAction( const QString &name = QString(), const QString &group = QString() );

    virtual ~QgsProcessingToolboxAction();

    /**
     * Returns the action's (translated, user visible) name.
     */
    QString actionName() const { return mName; }

    /**
     * Sets the action's (translated, user visible) name.
     *
     * \deprecated QGIS 4.4. Use the constructor setter instead.
     */
    Q_DECL_DEPRECATED void setActionName( const QString &name ) SIP_DEPRECATED { mName = name; }

    /**
     * Returns the action's (translated, user visible) group name.
     */
    QString groupName() const { return mGroup; }

    /**
     * Sets the action's (translated, user visible) group name.
     *
     * \deprecated QGIS 4.4. Use the constructor setter instead.
     */
    Q_DECL_DEPRECATED void setGroupName( const QString &group ) SIP_DEPRECATED { mGroup = group; }

#ifdef SIP_RUN
    SIP_PROPERTY( name = group, get = groupName, set = setGroupName )
#endif

    /**
     * Returns the icon to use for the action.
     */
    virtual QIcon icon() const;

    /**
     * Returns the icon to use for the action.
     *
     * \deprecated QGIS 4.4. Use icon() instead.
     */
    Q_DECL_DEPRECATED virtual QIcon getIcon() const SIP_DEPRECATED;

    /**
      * Sets the parent toolbox widget the action will be shown in.
      *
      * \deprecated QGIS 4.4
      */
    Q_DECL_DEPRECATED void setData( QgsDockWidget *widget ) SIP_DEPRECATED;

    /**
      * Returns the parent toolbox widget the action will be shown in.
      *
      * \deprecated QGIS 4.4
      */
    Q_DECL_DEPRECATED QgsDockWidget *data() const SIP_DEPRECATED;

#ifdef SIP_RUN
    SIP_PROPERTY( name = toolbox, get = data, set = setData )
#endif

    /**
      * Executes the action.
      *
      * \deprecated QGIS 4.4. Use trigger() instead.
      */
    Q_DECL_DEPRECATED virtual void execute() SIP_DEPRECATED;

    /**
      * Triggers the action.
      */
    virtual void trigger( const QgsProcessingActionContext &context );

  private:
    QString mName;
    QString mGroup;

    mutable bool mUseDeprecatedLoopBreak = false;

    QPointer< QgsDockWidget > mToolboxDockWidget;
};


#endif // QGSPROCESSINGPROVIDERACTIONS_H
