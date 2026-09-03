/***************************************************************************
                         qgsprocessingwidgetcontext.h
                         ---------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSPROCESSINGWIDGETCONTEXT_H
#define QGSPROCESSINGWIDGETCONTEXT_H

#include <memory>

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QPointer>

class QgsMapCanvas;
class QgsMessageBar;
class QgsBrowserGuiModel;
class QgsProject;
class QgsProcessingModelAlgorithm;
class QgsMapLayer;
class QgsProcessingContextGenerator;
class QgsModelDesignerDialog;

// TODO QGIS 5: rename to QgsProcessingWidgetContext

/**
 * \ingroup gui
 * \class QgsProcessingParameterWidgetContext
 * \brief Contains settings which reflect the context in which a Processing parameter widget is shown.
 *
 * For instance, the parent model algorithm, a linked map canvas, and other relevant information which allows the widget
 * to fine-tune its behavior.
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingParameterWidgetContext
{
  public:
    QgsProcessingParameterWidgetContext() = default;

    /**
     * Sets the map \a canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * \see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas associated with the widget.
     * \see setMapCanvas()
     */
    QgsMapCanvas *mapCanvas() const;

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the user.
     * \see messageBar()
     * \since QGIS 3.12
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget. This allows the widget to push feedback messages
     * to the user.
     * \see setMessageBar()
     * \since QGIS 3.12
     */
    QgsMessageBar *messageBar() const;

    /**
     * Sets the browser \a model associated with the widget. This will usually be the shared app instance of the browser model
     * \see browserModel()
     * \since QGIS 3.14
     */
    void setBrowserModel( QgsBrowserGuiModel *model );

    /**
     * Returns the browser model associated with the widget.
     * \see setBrowserModel()
     * \since QGIS 3.12
     */
    QgsBrowserGuiModel *browserModel() const;

    /**
     * Sets the \a project associated with the widget. This allows the widget to retrieve the map layers
     * and other properties from the correct project.
     * \see project()
     * \since QGIS 3.8
     */
    void setProject( QgsProject *project );

    /**
     * Returns the project associated with the widget.
     * \see setProject()
     */
    QgsProject *project() const;

    /**
     * Returns the model which the parameter widget is associated with.
     *
     * \see setModel()
     * \see modelChildAlgorithmId()
     */
    QgsProcessingModelAlgorithm *model() const;

    /**
     * Sets the \a model which the parameter widget is associated with.
     *
     * \see model()
     * \see setModelChildAlgorithmId()
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the child algorithm ID within the model which the parameter widget is associated with.
     *
     * \see setModelChildAlgorithmId()
     * \see model()
     */
    QString modelChildAlgorithmId() const;

    /**
     * Sets the child algorithm \a id within the model which the parameter widget is associated with.
     *
     * \see modelChildAlgorithmId()
     * \see setModel()
     */
    void setModelChildAlgorithmId( const QString &id );

    /**
     * Returns the current active layer.
     *
     * \see setActiveLayer()
     * \since QGIS 3.14
     */
    QgsMapLayer *activeLayer() const;

    /**
     * Sets the current active \a layer.
     *
     * \see activeLayer()
     * \since QGIS 3.14
     */
    void setActiveLayer( QgsMapLayer *layer );

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the widget when required.
     *
     * The \a generator must exist for the lifetime of the widget, ownership is not transferred.
     *
     * \see processingContextGenerator()
     *
     * \since QGIS 4.0
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

    /**
     * Returns the Processing context generator class that will be used to retrieve
     * a Processing context for the widget when required.
     *
     * \see registerProcessingContextGenerator()
     * \since QGIS 4.0
     */
    const QgsProcessingContextGenerator *processingContextGenerator() const;

    /**
     * Returns the associated model designer dialog, if applicable.
     *
     * \warning This method is not considered stable API
     *
     * \see setModelDesignerDialog()
     * \since QGIS 4.0
     */
    QgsModelDesignerDialog *modelDesignerDialog() const;

    /**
     * Sets the associated model designer \a dialog, if applicable.
     *
     * \warning This method is not considered stable API
     *
     * \see modelDesignerDialog()
     * \since QGIS 4.0
     */
    void setModelDesignerDialog( QgsModelDesignerDialog *dialog );

  private:
    QgsProcessingModelAlgorithm *mModel = nullptr;

    QString mModelChildAlgorithmId;

    QPointer< QgsMapCanvas > mMapCanvas;

    QPointer< QgsMessageBar > mMessageBar;

    QPointer< QgsProject > mProject;

    QPointer< QgsBrowserGuiModel > mBrowserModel;

    QPointer< QgsMapLayer > mActiveLayer;

    QgsProcessingContextGenerator *mProcessingContextGenerator = nullptr;

    QPointer< QgsModelDesignerDialog > mModelDialog;
};

/**
 * \class QgsProcessingWidgetContextGenerator
 * \brief An interface for objects which can create Processing widget contexts.
 *
 * \ingroup gui
 * \since QGIS 4.4
 */
class GUI_EXPORT QgsProcessingWidgetContextGenerator
{
  public:
    /**
   * This method needs to be reimplemented in all classes which implement this interface
   * and return a Processing widget context.
   */
    virtual QgsProcessingParameterWidgetContext createWidgetContext() = 0;

    virtual ~QgsProcessingWidgetContextGenerator() = default;
};


#endif // QGSPROCESSINGWIDGETCONTEXT_H
