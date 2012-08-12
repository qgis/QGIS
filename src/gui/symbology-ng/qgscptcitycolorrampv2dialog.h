/***************************************************************************
    qgscptcitycolorrampv2dialog.h
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCPTCITYCOLORRAMPV2DIALOG_H
#define QGSCPTCITYCOLORRAMPV2DIALOG_H

#include <QDialog>

#include "ui_qgscptcitycolorrampv2dialogbase.h"

class QgsCptCityColorRampV2;
class QgsCptCityCollection;
class QgsCptCityBrowserModel;

class GUI_EXPORT QgsCptCityColorRampV2Dialog : public QDialog, private Ui::QgsCptCityColorRampV2DialogBase
{
    Q_OBJECT

  public:
    QgsCptCityColorRampV2Dialog( QgsCptCityColorRampV2* ramp, QWidget* parent = NULL );

    QString selectedName() const { return lblSchemeName->text() + cboVariantName->currentText(); }

  public slots:
    void populateVariants( QString newVariant = QString() );

    void on_mBrowserView_clicked( const QModelIndex & );
    void on_tabBar_currentChanged( int index );
    void on_pbtnLicenseDetails_pressed();
    void on_cboVariantName_currentIndexChanged( int index );
    /* void refresh(); */

  protected:

    void updatePreview();
    void updateCopyingInfo( const QMap< QString, QString >& copyingMap );
    bool eventFilter( QObject *obj, QEvent *event );

    QgsCptCityColorRampV2* mRamp;
    QgsCptCityCollection* mCollection;
    QString mCollectionGroup;

    /* void refreshModel( const QModelIndex& index ); */
    /* void showEvent( QShowEvent * event ); */

    QgsCptCityBrowserModel* mModel;
    QgsCptCityBrowserModel* mAuthorsModel;
    QgsCptCityBrowserModel* mSelectionsModel;

};


#endif
