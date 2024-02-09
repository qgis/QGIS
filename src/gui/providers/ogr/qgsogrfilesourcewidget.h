/***************************************************************************
    qgsogrfilesourcewidget.h
     --------------------------------------
    Date                 : January 2024
    Copyright            : (C) 2024 by Nyall Dawson
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
#ifndef QGSOGRFILESOURCEWIDGET_H
#define QGSOGRFILESOURCEWIDGET_H

#include "qgsprovidersourcewidget.h"
#include <QVariantMap>

class QgsFileWidget;

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrFileSourceWidget : public QgsProviderSourceWidget
{
    Q_OBJECT

  public:
    QgsOgrFileSourceWidget( QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

  private slots:

    void validate();

  private:
    QgsFileWidget *mFileWidget = nullptr;

    QVariantMap mSourceParts;
    bool mIsValid = false;
};

///@endcond
#endif // QGSOGRFILESOURCEWIDGET_H
