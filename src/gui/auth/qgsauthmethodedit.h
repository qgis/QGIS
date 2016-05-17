/***************************************************************************
    qgsauthbasicedit.h
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHMETHODEDIT_H
#define QGSAUTHMETHODEDIT_H

#include <QWidget>

#include "qgis.h"
#include "qgsauthconfig.h"

/** \ingroup gui
 * Abstract base class for the edit widget of authentication method plugins
 */
class GUI_EXPORT QgsAuthMethodEdit : public QWidget
{
    Q_OBJECT

  public:
    /** Validate the configuration of subclasses */
    virtual bool validateConfig() = 0;

    /** The configuration key-vale map of subclasses */
    virtual QgsStringMap configMap() const = 0;

  signals:
    /** Emitted when the configuration validatity changes */
    void validityChanged( bool valid );

  public slots:
    /**
     * Load an existing config map into subclassed widget
     * @param configmap
     */
    virtual void loadConfig( const QgsStringMap &configmap ) = 0;

    /** Clear GUI controls in subclassed widget, optionally reloading any previously loaded config map */
    virtual void resetConfig() = 0;

    /** Clear GUI controls in subclassed widget */
    virtual void clearConfig() = 0;

  protected:
    /**
     * Construct widget to edit an authentication method configuration
     * @note Non-public since this is an abstract base class
     * @param parent Parent widget
     */
    explicit QgsAuthMethodEdit( QWidget *parent = nullptr )
        : QWidget( parent )
    {}

    virtual ~QgsAuthMethodEdit() {}
};

#endif // QGSAUTHMETHODEDIT_H
