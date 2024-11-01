/***************************************************************************
    qgsabstractmaptoolhandler.h
    ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTMAPTOOLHANDLER_H
#define QGSABSTRACTMAPTOOLHANDLER_H

#include "qgis_gui.h"

class QgsMapLayer;
class QgsMapTool;
class QAction;

/**
 * \ingroup gui
 * \brief An abstract base class for map tool handlers which automatically handle all the necessary
 * logic for toggling the map tool and enabling/disabling the associated action
 * when the QGIS application is in a state permissible for the tool.
 *
 * Creating these handlers avoids a lot of complex setup code and manual connections
 * which are otherwise necessary to ensure that a map tool is correctly activated and
 * deactivated when the state of the QGIS application changes (e.g. when the active
 * layer is changed, when edit modes are toggled, when other map tools are switched
 * to, etc).
 *
 * - ### Example
 *
 * \code{.py}
 *   class MyMapTool(QgsMapTool):
 *      ...
 *
 *   class MyMapToolHandler(QgsAbstractMapToolHandler):
 *
 *      def __init__(self, tool, action):
 *          super().__init__(tool, action)
 *
 *      def isCompatibleWithLayer(self, layer, context):
 *          # this tool can only be activated when an editable vector layer is selected
 *          return isinstance(layer, QgsVectorLayer) and layer.isEditable()
 *
 *   my_tool = MyMapTool()
 *   my_action = QAction('My Map Tool')
 *
 *   my_handler = MyMapToolHandler(my_tool, my_action)
 *   iface.registerMapToolHandler(my_handler)
 * \endcode
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsAbstractMapToolHandler
{
  public:
    /**
     * Context of a QgsAbstractMapToolHandler call.
     *
     * \since QGIS 3.16
     */
    struct Context
    {
        //! Placeholder only
        bool dummy = false;
    };

    /**
     * Constructor for a map tool handler for the specified \a tool.
     *
     * The \a action argument must be set to the action associated with switching
     * to the tool.
     *
     * The ownership of neither \a tool nor \a action is transferred, and the caller
     * is responsible for ensuring that these objects exist for the lifetime of the
     * handler.
     *
     * \warning The handler will be responsible for creating the appropriate
     * connections between the \a action and the \a tool. These should NOT be
     * manually connected elsewhere!
     */
    QgsAbstractMapToolHandler( QgsMapTool *tool, QAction *action );

    virtual ~QgsAbstractMapToolHandler();

    /**
     * Returns the tool associated with this handler.
     */
    QgsMapTool *mapTool();

    /**
     * Returns the action associated with toggling the tool.
     */
    QAction *action();

    /**
     * Returns TRUE if the associated map tool is compatible with the specified \a layer.
     *
     * Additional information is available through the \a context argument.
    */
    virtual bool isCompatibleWithLayer( QgsMapLayer *layer, const QgsAbstractMapToolHandler::Context &context ) = 0;

    /**
     * Sets the \a layer to use for the tool.
     *
     * Called whenever a new layer should be associated with the tool, e.g. as a result of the
     * user selecting a different active layer.
     *
     * The default implementation does nothing.
     */
    virtual void setLayerForTool( QgsMapLayer *layer );

  private:
    QgsMapTool *mMapTool = nullptr;
    QAction *mAction = nullptr;
};

#endif // QGSABSTRACTMAPTOOLHANDLER_H
