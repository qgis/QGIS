/***************************************************************************
                          qgsvirtuallayertask.h  -  description
                             -------------------
    begin                : Jan 19, 2018
    copyright            : (C) 2017 by Paul Blottiere
    email                : blottiere.paul@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUALLAYERTASK_H
#define QGSVIRTUALLAYERTASK_H

#include "qgsvirtuallayerdefinition.h"
#include "qgstaskmanager.h"
#include "qgsvectorlayer.h"

/**
 * \ingroup core
 *
 * Initializes a virtual layer with postpone mode activated and reloads the
 * data in a separated thread.
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsVirtualLayerTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor.
     * \param definition The definition to use for initializing the virtual layer
     */
    QgsVirtualLayerTask( const QgsVirtualLayerDefinition &definition );

    /**
     * Returns the underlying virtual layer.
     */
    QgsVectorLayer *layer();

    /**
     * Returns the underlying virtual layer and ownership.
     */
    QgsVectorLayer *takeLayer();

    /**
     * Returns the virtual layer definition.
     */
    QgsVirtualLayerDefinition definition() const;

    /**
     * Reloads the data.
     * \returns TRUE if the virtual layer is valid, FALSE otherwise.
     */
    bool run() override;

    /**
     * Cancels the pending query and the parent task.
     */
    void cancel() override;

    /**
     * Returns the exception text or an empty string if no exceptions were raised
     * \since QGIS 3.4
     */
    QString exceptionText() const;

    /**
     * Sets the \a exceptionText
     * \since QGIS 3.4
     */
    void setExceptionText( const QString &exceptionText );

  private:
    QString mExceptionText;
    QgsVirtualLayerDefinition mDefinition;
    std::unique_ptr<QgsVectorLayer> mLayer;
};

#endif // QGSVECTORLAYERTASK_H
