/***************************************************************************
                              qgsowsserver.h
                              --------------
  begin                : March 24, 2014
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSSERVER_H
#define QGSOWSSERVER_H

#include "qgsrequesthandler.h"
#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsaccesscontrol.h"
#endif

#include "qgsfield.h"
#include <QHash>

class QgsMapLayer;

class QgsOWSServer
{
  public:
    QgsOWSServer(
      const QString& configFilePath
      , const QMap<QString, QString>& parameters
      , QgsRequestHandler* rh
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , const QgsAccessControl* ac
#endif
    )
        : mParameters( parameters )
        , mRequestHandler( rh )
        , mConfigFilePath( configFilePath )
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        , mAccessControl( ac )
#endif
    {}
    virtual ~QgsOWSServer() {}

    virtual void executeRequest() = 0;

    /** Restores the original layer filters
     * @param filterMap the original layer filter
     */
    static void restoreLayerFilters( const QHash < QgsMapLayer*, QString >& filterMap );

    static QString pkSeparator() { return "@@"; }

  private:
    QgsOWSServer() {}

  protected:
    QMap<QString, QString> mParameters;
    QgsRequestHandler* mRequestHandler;
    QString mConfigFilePath;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    /** The access control helper */
    const QgsAccessControl* mAccessControl;

    /** Apply filter strings from the access control to the layers.
     * @param layer the concerned layer
     * @param originalLayerFilters the original layer filter
     *
     */
    void applyAccessControlLayerFilters( QgsMapLayer* layer, QHash<QgsMapLayer*, QString>& originalLayerFilters ) const;
#endif

    /** Creates gml id for feature. Returns primary key value or feature id in case there is no PK
     * @param f feature
     * @param pkAttributes list of attribute indices used as primary key
     * @return pk as string or feature id
    */
    static QString featureGmlId( const QgsFeature* f, const QgsAttributeList& pkAttributes );

};

/** RAII class to restore layer filters on destruction
 */
class QgsOWSServerFilterRestorer
{
  public:

    QgsOWSServerFilterRestorer()  {}

    //! Destructor. When object is destroyed all original layer filters will be restored.
    ~QgsOWSServerFilterRestorer()
    {
      QgsOWSServer::restoreLayerFilters( mOriginalLayerFilters );
    }

    /** Returns a reference to the object's hash of layers to original subsetString filters.
     * Original layer subsetString filters MUST be inserted into this hash before modifying them.
     */
    QHash<QgsMapLayer*, QString>& originalFilters() { return mOriginalLayerFilters; }

  private:
    QHash<QgsMapLayer*, QString> mOriginalLayerFilters;

    QgsOWSServerFilterRestorer( const QgsOWSServerFilterRestorer& rh );
    QgsOWSServerFilterRestorer& operator=( const QgsOWSServerFilterRestorer& rh );
};

#endif // QGSOWSSERVER_H
