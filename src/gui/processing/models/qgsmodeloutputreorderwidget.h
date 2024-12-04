/***************************************************************************
                             qgsmodeloutputreorderwidget.h
                             ----------------------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELOUTPUTREORDERWIDGET_H
#define QGSMODELOUTPUTREORDERWIDGET_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsmodeloutputreorderwidgetbase.h"
#include "qgsprocessingmodeloutput.h"
#include <QDialog>

class QStandardItemModel;
class QgsProcessingModelAlgorithm;

///@cond PRIVATE

/**
 * A widget for reordering outputs for Processing models.
 * \ingroup gui
 * \note Not stable API
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsModelOutputReorderWidget : public QWidget, private Ui::QgsModelOutputReorderWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelOutputReorderWidget.
     */
    QgsModelOutputReorderWidget( QWidget *parent = nullptr );

    /**
     * Sets the source \a model from which to obtain the list of outputs.
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the ordered list of outputs.
     */
    QStringList outputOrder() const;

    /**
     * Returns the destination group name for outputs.
     */
    QString outputGroup() const;

  private:
    QgsProcessingModelAlgorithm *mModel;
    QList<QgsProcessingModelOutput> mOutputs;
    QStandardItemModel *mItemModel = nullptr;
};


/**
 * A dialog for reordering outputs for Processing models.
 * \ingroup gui
 * \note Not stable API
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsModelOutputReorderDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsModelOutputReorderDialog.
     */
    QgsModelOutputReorderDialog( QWidget *parent = nullptr );

    /**
     * Sets the source \a model from which to obtain the list of outputs.
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the ordered list of outputs (by name).
     */
    QStringList outputOrder() const;

    /**
     * Returns the destination group name for outputs.
     */
    QString outputGroup() const;

  private:
    QgsModelOutputReorderWidget *mWidget = nullptr;
};

///@endcond

#endif // QGSMODELOUTPUTREORDERWIDGET_H
