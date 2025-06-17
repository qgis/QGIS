/***************************************************************************
    qgsattributesformtreeviewindicator.h
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESFORMTREEVIEWINDICATOR_H
#define QGSATTRIBUTESFORMTREEVIEWINDICATOR_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"

#include <QIcon>
#include <QObject>


/**
 * \brief Indicator that can be used in an Attributes Form tree view to display icons next to field items.
 *
 * They add extra context to the field item, making it easier to get an overview of constraints and default values.
 *
 * \warning Not part of stable API and may change in future QGIS releases.
 * \ingroup gui
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsAttributesFormTreeViewIndicator : public QObject
{
    Q_OBJECT
  public:
    //! Constructs an indicator, optionally transferring ownership to a parent QObject.
    explicit QgsAttributesFormTreeViewIndicator( QObject *parent = nullptr );

    //! Indicator icon that will be displayed in the Attributes Form tree view.
    QIcon icon() const;
    //! Sets indicator icon that will be displayed in the Attributes Form tree view.
    void setIcon( const QIcon &icon );

    //! Returns tool tip text that will be shown when user hovers mouse over the indicator.
    QString toolTip() const;
    //! Sets tool tip text for the indicator.
    void setToolTip( const QString &tip );

  signals:
    /**
     * Emitted when the indicator changes state (e.g. icon).
     */
    void changed();

  private:
    QIcon mIcon;
    QString mToolTip;
};

#endif // QGSATTRIBUTESFORMTREEVIEWINDICATOR_H
