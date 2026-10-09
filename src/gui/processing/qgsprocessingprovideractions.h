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
class QgsProcessingAlgorithm;

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

    /**
     * Returns the name of the algorithm associated with the action.
     *
     * \see setAlgorithmName()
     */
    QString algorithmName() const;

    /**
     * Sets the name of the algorithm associated with the action.
     *
     * \see algorithmName()
     */
    void setAlgorithmName( const QString &name );

    /**
     * Returns the ID of the provider associated with the action.
     *
     * \see setProviderId()
     */
    QString providerId() const;

    /**
     * Sets the ID of the provider associated with the action.
     *
     * \see providerId()
     */
    void setProviderId( const QString &id );

  private:
    QWidget *mParentWidget = nullptr;

    QString mAlgorithmName;
    QString mProviderId;
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


/**
 * \ingroup gui
 * \brief A custom action to show in the context menu for the Processing toolbox.
 *
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingToolboxContextAction
{
  public:
    //TODO QGIS 5.0 -- name should be mandatory
    /**
   * Constructor for QgsProcessingToolboxContextAction.
   */
    QgsProcessingToolboxContextAction( const QString &name = QString() );

    virtual ~QgsProcessingToolboxContextAction();

    /**
   * Returns the action's (translated, user visible) name.
   */
    QString actionName() const { return mName; }

    /**
   * Sets the action's (translated, user visible) name.
   * \deprecated QGIS 4.4. Use the constructor setter instead.
   */
    Q_DECL_DEPRECATED void setActionName( const QString &name ) SIP_DEPRECATED { mName = name; }

    /**
   * Returns the icon to use for the action.
   */
    virtual QIcon icon() const;

    /**
   * Returns TRUE if the action is enabled.
   *
   * \deprecated QGIS 4.4
   */
    Q_DECL_DEPRECATED virtual bool isEnabled() const SIP_DEPRECATED;

    /**
     * Returns TRUE if the action is compatible with the specified provider and algorithm.
     */
    virtual bool isCompatibleWithAlgorithm( const QString &providerId, const QString &algorithmName );

    /**
   * Returns the parent toolbox the action will be shown in.
   *
   * \deprecated QGIS 4.4
   */
    Q_DECL_DEPRECATED QgsDockWidget *getToolbox() SIP_DEPRECATED;

    /**
   * Sets the parent toolbox the action will be shown in.
   *
   * \deprecated QGIS 4.4
   */
    Q_DECL_DEPRECATED void setToolbox( QgsDockWidget *toolbox ) SIP_DEPRECATED;

#ifdef SIP_RUN
    SIP_PROPERTY( name = toolbox, get = getToolbox, set = setToolbox )
#endif

    /**
   * Returns the associated algorithm the action will be shown for.
   *
   * \deprecated QGIS 4.4
   */
    Q_DECL_DEPRECATED QgsProcessingAlgorithm *getItemData() SIP_DEPRECATED;

    /**
   * Sets the associated algorithm the action will be shown for.
   *
   * \deprecated QGIS 4.4
   */
    Q_DECL_DEPRECATED void setItemData( QgsProcessingAlgorithm *data ) SIP_DEPRECATED;

#ifdef SIP_RUN
    SIP_PROPERTY( name = itemData, get = getItemData, set = setItemData )
#endif

    /**
   * Sets the associated algorithm and toolbox the action will be shown for.
   *
   * \deprecated QGIS 4.4
   */
    Q_DECL_DEPRECATED void setData( QgsProcessingAlgorithm *data, QgsDockWidget *toolbox ) SIP_DEPRECATED;

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

    /**
     * Returns TRUE if the action is a separator action.
     *
     * \see setIsSeparator()
     */
    bool isSeparator() const;

    /**
     * Sets whether the action is a separator action.
     *
     * \see isSeparator()
     */
    void setIsSeparator( bool isSeparator );

#ifdef SIP_RUN
    SIP_PROPERTY( name = is_separator, get = isSeparator, set = setIsSeparator )
#endif

  private:
    QString mName;

    mutable bool mUseDeprecatedLoopBreak = false;

    QPointer< QgsDockWidget > mToolboxDockWidget;
    QgsProcessingAlgorithm *mAlgorithm = nullptr;

    bool mIsSeparator = false;
};


#endif // QGSPROCESSINGPROVIDERACTIONS_H
