/***************************************************************************
                             qgsmodelcomponentgraphicitem.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELCOMPONENTGRAPHICITEM_H
#define QGSMODELCOMPONENTGRAPHICITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsObject>

class QgsProcessingModelComponent;
class QgsProcessingModelParameter;
class QgsProcessingModelChildAlgorithm;
class QgsProcessingModelOutput;
class QgsProcessingModelAlgorithm;
class QgsModelDesignerFlatButtonGraphicItem;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Base class for graphic items representing model components in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelComponentGraphicItem : public QGraphicsObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelComponentGraphicItem for the specified \a component, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a component is transferred to the item.
     */
    QgsModelComponentGraphicItem( QgsProcessingModelComponent *component SIP_TRANSFER,
                                  QgsProcessingModelAlgorithm *model,
                                  QGraphicsItem *parent SIP_TRANSFERTHIS );

    /**
     * Returns the model component associated with this item.
     */
    QgsProcessingModelComponent *component();

    /**
     * Returns the model associated with this item.
     */
    QgsProcessingModelAlgorithm *model();

  signals:

    // TEMPORARY ONLY during refactoring

    /**
     * Emitted by the item to request a repaint of the parent model scene.
     */
    void requestModelRepaint();

    // TEMPORARY ONLY during refactoring

    /**
     * Emitted when the definition of the associated component is changed
     * by the item.
     */
    void changed();

  protected slots:

    /**
     * Called when the component should be edited.
     *
     * The default implementation does nothing.
     */
    virtual void editComponent() {}

    /**
     * Called when the component should be deleted.
     *
     * The default implementation does nothing.
     */
    virtual void deleteComponent() {}

  private:

    std::unique_ptr< QgsProcessingModelComponent > mComponent;
    QgsProcessingModelAlgorithm *mModel = nullptr;

    QgsModelDesignerFlatButtonGraphicItem *mEditButton = nullptr;
    QgsModelDesignerFlatButtonGraphicItem *mDeleteButton = nullptr;

    static constexpr double DEFAULT_BUTTON_WIDTH = 16;
    static constexpr double DEFAULT_BUTTON_HEIGHT = 16;
    QSizeF mButtonSize { DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT };

};

/**
 * \ingroup gui
 * \brief A graphic item representing a model parameter (input) in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelParameterGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelParameterGraphicItem for the specified \a parameter, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a parameter is transferred to the item.
     */
    QgsModelParameterGraphicItem( QgsProcessingModelParameter *parameter SIP_TRANSFER,
                                  QgsProcessingModelAlgorithm *model,
                                  QGraphicsItem *parent SIP_TRANSFERTHIS );

};

/**
 * \ingroup gui
 * \brief A graphic item representing a child algorithm in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelChildAlgorithmGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelChildAlgorithmGraphicItem for the specified \a child, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a child is transferred to the item.
     */
    QgsModelChildAlgorithmGraphicItem( QgsProcessingModelChildAlgorithm *child SIP_TRANSFER,
                                       QgsProcessingModelAlgorithm *model,
                                       QGraphicsItem *parent SIP_TRANSFERTHIS );

};


/**
 * \ingroup gui
 * \brief A graphic item representing a model output in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelOutputGraphicItem : public QgsModelComponentGraphicItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelOutputGraphicItem for the specified \a output, with the specified \a parent item.
     *
     * The \a model argument specifies the associated processing model. Ownership of \a model is not transferred, and
     * it must exist for the lifetime of this object.
     *
     * Ownership of \a output is transferred to the item.
     */
    QgsModelOutputGraphicItem( QgsProcessingModelOutput *output SIP_TRANSFER,
                               QgsProcessingModelAlgorithm *model,
                               QGraphicsItem *parent SIP_TRANSFERTHIS );

};
///@endcond

#endif // QGSMODELCOMPONENTGRAPHICITEM_H
