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
    virtual bool validateConfig() = 0;

    virtual QgsStringMap configMap() const = 0;

  signals:
    void validityChanged( bool valid );

  public slots:
    virtual void loadConfig( const QgsStringMap &configmap ) = 0;

    virtual void resetConfig() = 0;

    virtual void clearConfig() = 0;

  protected:
    explicit QgsAuthMethodEdit( QWidget *parent = 0 )
        : QWidget( parent )
    {}

    virtual ~QgsAuthMethodEdit() {}
};

#endif // QGSAUTHMETHODEDIT_H
