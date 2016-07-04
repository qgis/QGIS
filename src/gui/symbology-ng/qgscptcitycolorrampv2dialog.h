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


#include "ui_qgscptcitycolorrampv2dialogbase.h"
#include <QDialog>

#include "qgscptcityarchive.h"

#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>

class QgsCptCityColorRampV2;
class TreeFilterProxyModel;
class ListFilterProxyModel;
class UngroupProxyModel;

/** \ingroup gui
 * \class QgsCptCityColorRampV2Dialog
 */
class GUI_EXPORT QgsCptCityColorRampV2Dialog : public QDialog, private Ui::QgsCptCityColorRampV2DialogBase
{
    Q_OBJECT

  public:
    QgsCptCityColorRampV2Dialog( QgsCptCityColorRampV2* ramp, QWidget* parent = nullptr );
    ~QgsCptCityColorRampV2Dialog();

    QString selectedName() const
    { return mRamp ? QFileInfo( mRamp->schemeName() ).baseName() + mRamp->variantName() : QString(); }

    bool saveAsGradientRamp() const;

  public slots:
    void populateVariants();

    void on_mTreeView_clicked( const QModelIndex & );
    void on_mListWidget_itemClicked( QListWidgetItem * item );
    void on_mListWidget_itemSelectionChanged();
    void on_tabBar_currentChanged( int index );
    void on_pbtnLicenseDetails_pressed();
    void on_cboVariantName_currentIndexChanged( int index );
    void onFinished();
    void on_buttonBox_helpRequested();
    /* void refresh(); */

  protected:

    void updatePreview( bool clear = false );
    void clearCopyingInfo();
    void updateCopyingInfo( const QMap< QString, QString >& copyingMap );
    void updateTreeView( QgsCptCityDataItem *item, bool resetRamp = true );
    void updateListWidget( QgsCptCityDataItem *item );
    bool eventFilter( QObject *obj, QEvent *event ) override;

    QgsCptCityColorRampV2* mRamp;
    QgsCptCityArchive* mArchive;
    QgsCptCityBrowserModel::ViewType mArchiveViewType;

    /* void refreshModel( const QModelIndex& index ); */
    bool updateRamp();
    void showAll();
    void setTreeModel( QgsCptCityBrowserModel* model );

    QgsCptCityBrowserModel* mModel;
    QgsCptCityBrowserModel* mAuthorsModel;
    QgsCptCityBrowserModel* mSelectionsModel;
    TreeFilterProxyModel* mTreeFilter;
    QVector<QgsCptCityColorRampItem*> mListRamps;

};

/// @cond PRIVATE

/** \ingroup gui
 * \class TreeFilterProxyModel
 */
class TreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    TreeFilterProxyModel( QObject *parent, QgsCptCityBrowserModel* model );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    // bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

  private:
    QgsCptCityBrowserModel* mModel;
};

///@endcond


#endif
