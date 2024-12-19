/***************************************************************************
                            qgshistoryprovider.h
                            --------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSHISTORYPROVIDER_H
#define QGSHISTORYPROVIDER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QString>
#include <QVariantMap>

class QgsHistoryEntryNode;
class QgsHistoryEntry;
class QgsHistoryWidgetContext;

/**
 * Abstract base class for objects which track user history (i.e. operations performed through the GUI).
 *
 * QgsAbstractHistoryProvider subclasses are accessible through the QgsHistoryProviderRegistry class.
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsAbstractHistoryProvider : public QObject
{
    Q_OBJECT

  public:
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsProcessingHistoryProvider *>( sipCpp ) )
      sipType = sipType_QgsProcessingHistoryProvider;
    else
      sipType = nullptr;
    SIP_END
#endif

    virtual ~QgsAbstractHistoryProvider();

    /**
     * Returns the provider's unique id, which is used to associate existing history entries with the provider.
     */
    virtual QString id() const = 0;

    /**
     * Creates a new history node for the given \a entry.
     *
     * \since QGIS 3.32
     */
    virtual QgsHistoryEntryNode *createNodeForEntry( const QgsHistoryEntry &entry, const QgsHistoryWidgetContext &context ) SIP_FACTORY;

    /**
     * Updates an existing history \a node for the given \a entry.
     *
     * \since QGIS 3.32
     */
    virtual void updateNodeForEntry( QgsHistoryEntryNode *node, const QgsHistoryEntry &entry, const QgsHistoryWidgetContext &context );
};

#endif //QGSHISTORYPROVIDER_H
