/***************************************************************************
    qgsowssourcewidget.h
     --------------------------------------
    Date                 : November 2021
    Copyright            : (C) 2021 by Samweli Mwakisambwe
    Email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOWSSOURCEWIDGET_H
#define QGSOWSSOURCEWIDGET_H

#define SIP_NO_FILE

#include "qgsprovidersourcewidget.h"
#include "qgsreadwritecontext.h"
#include "ui_qgsowssourcewidgetbase.h"
#include <QVariantMap>

/**
 * \ingroup gui
 * \brief This widget sets and updates OWS layers source URI.
 *
 * \since QGIS 3.26
 */

class GUI_EXPORT QgsOWSSourceWidget : public QgsProviderSourceWidget, private Ui::QgsOWSSourceWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsOWSSourceWidget with the specified \a provider key and \a parent widget.
     */
    QgsOWSSourceWidget( const QString &providerKey, QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

    /**
     * Sets the spatial extent in the widget extent box.
     *
     * \since QGIS 3.26
     */
    void setExtent( const QgsRectangle &extent );

    /**
     * Returns the spatial extent from the widget extent box.
     *
     * \since QGIS 3.26
     */
    QgsRectangle extent() const;


    void setMapCanvas( QgsMapCanvas *canvas ) override;


  private:

    QVariantMap mSourceParts;
    const QString mProviderKey;
};

#endif // QGSOWSSOURCEWIDGET_H
