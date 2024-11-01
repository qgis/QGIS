/***************************************************************************
                            qgsprocessinghistoryprovider.h
                            --------------------------
    begin                : December 2021
    copyright            : (C) 2021 by Nyall Dawson
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
#ifndef QGSPROCESSINGHISTORYPROVIDER_H
#define QGSPROCESSINGHISTORYPROVIDER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include "qgshistoryprovider.h"

/**
 * History provider for operations performed through the Processing framework.
 *
 * \ingroup gui
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsProcessingHistoryProvider : public QgsAbstractHistoryProvider
{
    Q_OBJECT

  public:
    QgsProcessingHistoryProvider();

    QString id() const override;

    /**
     * Ports the old text log to the history framework.
     *
     * This should only be called once -- calling multiple times will result in duplicate log entries
     */
    void portOldLog();

    QgsHistoryEntryNode *createNodeForEntry( const QgsHistoryEntry &entry, const QgsHistoryWidgetContext &context ) override SIP_FACTORY;
    void updateNodeForEntry( QgsHistoryEntryNode *node, const QgsHistoryEntry &entry, const QgsHistoryWidgetContext &context ) override;

  signals:

    /**
     * Emitted when the provider needs to execute python \a commands in the Processing context.
     *
     * \since QGIS 3.32
     */
    void executePython( const QString &commands );

    /**
     * Emitted when the provider needs to create a Processing test with the given python \a command.
     *
     * \since QGIS 3.32
     */
    void createTest( const QString &command );

  private:
    //! Executes some python commands
    void emitExecute( const QString &commands );

    void emitCreateTest( const QString &command );

    //! Returns the path to the old log file
    QString oldLogPath() const;

    friend class ProcessingHistoryBaseNode;
};

#endif //QGSHISTORYPROVIDER_H
