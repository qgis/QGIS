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

#include "qgsprovidersourcewidget.h"
#include "qgsreadwritecontext.h"
#include "ui_qgsowssourcewidgetbase.h"
#include <QVariantMap>

class GUI_EXPORT QgsOWSSourceWidget : public QgsProviderSourceWidget, private Ui::QgsOWSSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsOWSSourceWidget( const QString &providerKey, QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

    void setTimes( const QStringList &times );
    QStringList times() const;

    void hideInputWidgets();

    void prepareExtent( QgsMapCanvas *mapCanvas );
    bool extentChecked();
    QgsRectangle outputExtent();

    void clearFormats();

    void clearCrs();

    void populateTimes();
    void clearTimes();
    QString selectedTimeText();

    void setCRSLabel( const QString &label );
    void setSelectedCRSLabel( const QString &label );

    void setChangeCRSButtonEnabled( bool enabled );

    void insertItemFormat( int index, const QString &label );
    void setFormatCurrentIndex( int index );
    void setFormatEnabled( bool enabled );
    int formatCurrentIndex();

    QVariant cacheData();

  signals:

    void changeCRSButtonClicked();

  private slots:

    //! Opens the Spatial Reference System dialog.
    void mChangeCRSButton_clicked();


  private:

    QVariantMap mSourceParts;
    const QString mProviderKey;
    QStringList mTimes;
    QgsMapCanvas *mMapCanvas = nullptr;
};

#endif // QGSOWSSOURCEWIDGET_H
