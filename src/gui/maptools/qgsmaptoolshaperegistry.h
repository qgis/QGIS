/***************************************************************************
                         qgsmaptoolshaperegistry.h
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPEREGISTRY_H
#define QGSMAPTOOLSHAPEREGISTRY_H

#define SIP_NO_FILE

#include "qgsabstractrelationeditorwidget.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgis_gui.h"

class QgsMapToolShapeMetadata;
class QgsMapToolCapture;

/**
 * \ingroup gui
 * \brief Keeps track of the registered shape map tools
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsMapToolShapeRegistry
{
    Q_GADGET
  public:

    /**
     * Constructor
     */
    QgsMapToolShapeRegistry();

    ~QgsMapToolShapeRegistry();

    /**
     * Adds a new shape map tool
     */
    void addMapTool( QgsMapToolShapeMetadata *mapTool SIP_TRANSFER );

    /**
     * Removes a registered map tool at the given \a id
     * The tool will be deleted.
     */
    void removeMapTool( const QString &id );

    //! Returns the list of map tools
    QList<QgsMapToolShapeMetadata *> mapToolMetadatas() const {return mMapTools;}

    //! Returns the map tool metadata for the given \a id
    QgsMapToolShapeMetadata *mapToolMetadata( const QString &id ) const;

    /**
     * Constructs the map tool at the given \a id for the given \a parentTool
     * Caller takes ownership of the returned tool.
     */
    QgsMapToolShapeAbstract *mapTool( const QString &id, QgsMapToolCapture *parentTool ) const SIP_FACTORY;

  private:

    QList<QgsMapToolShapeMetadata *> mMapTools;

};

/**
 * \ingroup gui
 * \brief QgsMapToolShapeMetadata is a base class for shape map tools metadata to be used in QgsMapToolShapeRegistry
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsMapToolShapeMetadata
{
  public:
    //! Constructor
    QgsMapToolShapeMetadata() = default;

    virtual ~QgsMapToolShapeMetadata() = default;

    //! Unique ID for the shape map tool
    virtual QString id() const = 0;

    //! Translated readable name
    virtual QString name() const = 0;

    //! Icon to be displayed in the toolbar
    virtual QIcon icon() const = 0;

    //! Returns the shape category of the tool
    virtual QgsMapToolShapeAbstract::ShapeCategory category() const = 0;

    /**
     * Creates the shape map tool for the given \a parentTool
     * Caller takes ownership of the returned object.
     */
    virtual QgsMapToolShapeAbstract *factory( QgsMapToolCapture *parentlTool ) const SIP_FACTORY = 0;
};


#endif // QGSMAPTOOLSHAPEREGISTRY_H
