/***************************************************************************
    qgssensorthingssourcewidget.h
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORTHINGSSOURCEWIDGET_H
#define QGSSENSORTHINGSSOURCEWIDGET_H

#include "qgsprovidersourcewidget.h"
#include "qgis.h"
#include "ui_qgssensorthingssourcewidgetbase.h"
#include <QVariantMap>

class QgsFileWidget;

///@cond PRIVATE
#define SIP_NO_FILE

class QgsSensorThingsSourceWidget : public QgsProviderSourceWidget, protected Ui::QgsSensorThingsSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsSensorThingsSourceWidget( QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

  private slots:

    void entityTypeChanged();
    void validate();

  private:
    void rebuildGeometryTypes( Qgis::SensorThingsEntity type );
    void setCurrentGeometryTypeFromString( const QString &geometryType );

    QVariantMap mSourceParts;
    bool mIsValid = false;
};

///@endcond
#endif // QGSSENSORTHINGSSOURCEWIDGET_H
