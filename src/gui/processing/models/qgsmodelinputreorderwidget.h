/***************************************************************************
                             qgsmodelinputreorderwidget.h
                             ----------------------------------
    Date                 : April 2020
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

#ifndef QGSMODELINPUTREORDERWIDGET_H
#define QGSMODELINPUTREORDERWIDGET_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsmodelinputreorderwidgetbase.h"
#include "processing/models/qgsprocessingmodelparameter.h"
#include <QDialog>

class QStandardItemModel;
class QgsProcessingModelAlgorithm;

///@cond PRIVATE

/**
 * A widget for reordering inputs for Processing models.
 * \ingroup gui
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelInputReorderWidget : public QWidget, private Ui::QgsModelInputReorderWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelInputReorderWidget.
     */
    QgsModelInputReorderWidget( QWidget *parent = nullptr );

    /**
     * Sets the source \a model from which to obtain the list of inputs.
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the ordered list of inputs (by name).
     */
    QStringList inputOrder() const;

  private:

    QgsProcessingModelAlgorithm *mModel;
    QList< QgsProcessingModelParameter > mParameters;
    QStandardItemModel *mItemModel = nullptr;
};


/**
 * A dialog for reordering inputs for Processing models.
 * \ingroup gui
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelInputReorderDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelInputReorderDialog.
     */
    QgsModelInputReorderDialog( QWidget *parent = nullptr );

    /**
     * Sets the source \a model from which to obtain the list of inputs.
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the ordered list of inputs (by name).
     */
    QStringList inputOrder() const;

  private:

    QgsModelInputReorderWidget *mWidget = nullptr;
};

///@endcond

#endif // QGSMODELINPUTREORDERWIDGET_H
