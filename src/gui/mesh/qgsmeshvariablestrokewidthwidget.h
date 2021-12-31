/***************************************************************************
    qgsmeshvariablestrokewidthtwidget.h
    -------------------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHVARIABLESTROKEWIDTHWIDGET_H
#define QGSMESHVARIABLESTROKEWIDTHWIDGET_H

#include "qgis_gui.h"
#include "ui_qgsmeshvariablestrokewidthwidgetbase.h"
#include "qgspanelwidget.h"
#include "qgsmeshlayerrenderer.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsMeshVariableStrokeWidthButton
 *
 * \brief A widget push button that store variable stroke width and call a widget to set parameters
 */
class QgsMeshVariableStrokeWidthButton: public QPushButton
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMeshVariableStrokeWidthButton( QWidget *parent = nullptr );

    //! Returns the variable stroke width
    QgsInterpolatedLineWidth variableStrokeWidth() const;

    //! Sets the variable stroke width
    void setVariableStrokeWidth( const QgsInterpolatedLineWidth &variableStrokeWidth );

    //! Sets the default min/max values that will be reload if needed
    void setDefaultMinMaxValue( double minimum, double maximum );

  signals:
    void widgetChanged();

  private slots:
    void openWidget();

  private:
    void updateText();

    QgsInterpolatedLineWidth mVariableStrokeWidth;
    double mMinimumDefaultValue;
    double mMaximumDefaultValue;
};

/**
 * \ingroup gui
 * \class QgsMeshVariableStrokeWidthWidget
 *
 * \brief A widget to set parameters of variable stroke width
 */
class QgsMeshVariableStrokeWidthWidget: public QgsPanelWidget, public Ui::QgsMeshVariableStrokeWidthWidget
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMeshVariableStrokeWidthWidget( const QgsInterpolatedLineWidth &variableStrokeWidth,
                                      double defaultMinimumvalue,
                                      double defaultMaximumValue,
                                      QWidget *parent = nullptr );

    //! Sets the variable stroke width
    void setVariableStrokeWidth( const QgsInterpolatedLineWidth &variableStrokeWidth );

    //! Returns the variable stroke width
    QgsInterpolatedLineWidth variableStrokeWidth() const;

  private slots:
    void defaultMinMax();
  private:
    double mDefaultMinimumValue = 0;
    double mDefaultMaximumValue = 0;

    double lineEditValue( const QgsDoubleSpinBox *lineEdit ) const;
};

#endif // QGSMESHVARIABLESTROKEWIDTHWIDGET_H
