/***************************************************************************
                             qgsprocessingparameterswidget.h
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

#ifndef QGSPROCESSINGPARAMETERSWIDGET_H
#define QGSPROCESSINGPARAMETERSWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingparameterswidgetbase.h"
#include <QWidget>
#include "qgsprocessingwidgetwrapper.h"

class QgsProcessingAlgorithm;
class QgsProcessingParameterDefinition;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A widget which allows users to select the value for the parameters for an algorithm.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingParametersWidget : public QgsPanelWidget, public QgsProcessingParametersGenerator, private Ui::QgsProcessingParametersWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingParametersWidget, for the specified \a algorithm.
     */
    QgsProcessingParametersWidget( const QgsProcessingAlgorithm *algorithm, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    const QgsProcessingAlgorithm *algorithm() const;

  protected:
    virtual void initWidgets();

    void addParameterWidget( const QgsProcessingParameterDefinition *parameter, QWidget *widget SIP_TRANSFER, int stretch = 0 );
    void addParameterLabel( const QgsProcessingParameterDefinition *parameter, QWidget *label SIP_TRANSFER );

    void addOutputLabel( QWidget *label SIP_TRANSFER );
    void addOutputWidget( QWidget *widget SIP_TRANSFER, int stretch = 0 );

    void addExtraWidget( QWidget *widget SIP_TRANSFER );

  private:
    const QgsProcessingAlgorithm *mAlgorithm = nullptr;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGPARAMETERSWIDGET_H
