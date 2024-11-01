/***************************************************************************
                             qgsprocessingfeaturesourceoptionswidget.h
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

#ifndef QGSPROCESSINGFEATURESOURCEOPTIONSWIDGET_H
#define QGSPROCESSINGFEATURESOURCEOPTIONSWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingfeaturesourceoptionsbase.h"

#define SIP_NO_FILE

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Widget for configuring advanced settings for a feature source.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingFeatureSourceOptionsWidget : public QgsPanelWidget, private Ui::QgsProcessingFeatureSourceOptionsBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingFeatureSourceOptionsWidget, with the specified \a parent widget.
     */
    QgsProcessingFeatureSourceOptionsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets a layer associated with the widget.
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Sets the geometry check method to use, and whether the default method is overridden.
     *
     * \see isOverridingInvalidGeometryCheck()
     * \see geometryCheckMethod()
     */
    void setGeometryCheckMethod( bool isOverridden, Qgis::InvalidGeometryCheck check );

    /**
     * Sets the feature \a limit for the source.
     *
     * \see featureLimit()
     */
    void setFeatureLimit( int limit );

    /**
     * Sets the filter \a expression for the source.
     *
     * \see filterExpression()
     */
    void setFilterExpression( const QString &expression );

    /**
     * Returns the selected geometry check method. Also check isOverridingInvalidGeometryCheck() to verify
     * whether this method should be applied, or the default one used instead.
     *
     * \see isOverridingInvalidGeometryCheck()
     * \see setGeometryCheckMethod()
     */
    Qgis::InvalidGeometryCheck geometryCheckMethod() const;

    /**
     * Returns TRUE if the default geometry check method is being overridden.
     * \see geometryCheckMethod()
     * \see setGeometryCheckMethod()
     */
    bool isOverridingInvalidGeometryCheck() const;

    /**
     * Returns the feature limit set in the widget, or -1 if no limit is set.
     *
     * \see setFeatureLimit()
     */
    int featureLimit() const;

    /**
     * Returns the expression filter set in the widget, or an empty string if no filter is set.
     *
     * \see setFilterExpression()
     */
    QString filterExpression() const;
};

///@endcond

#endif // QGSPROCESSINGFEATURESOURCEOPTIONSWIDGET_H
