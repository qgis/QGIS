/***************************************************************************
    qgscptcitycolorrampdialog.h
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

#ifndef QGSCPTCITYCOLORRAMPDIALOG_H
#define QGSCPTCITYCOLORRAMPDIALOG_H


#include "ui_qgscptcitycolorrampdialogbase.h"
#include "qgis_sip.h"
#include <QDialog>

#include "qgscptcityarchive.h"

#include <QAbstractProxyModel>
#include <QSortFilterProxyModel>
#include <QFileInfo>
#include "qgis_gui.h"

class QgsCptCityColorRamp;
class TreeFilterProxyModel;
class ListFilterProxyModel;
class UngroupProxyModel;

/**
 * \ingroup gui
 * \class QgsCptCityColorRampDialog
 * A dialog which allows users to modify the properties of a QgsCptCityColorRamp.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsCptCityColorRampDialog : public QDialog, private Ui::QgsCptCityColorRampDialogBase
{
    Q_OBJECT
    Q_PROPERTY( QgsCptCityColorRamp ramp READ ramp WRITE setRamp )

  public:

    /**
     * Constructor for QgsCptCityColorRampDialog.
     * \param ramp initial ramp to show in dialog
     * \param parent parent widget
     */
    QgsCptCityColorRampDialog( const QgsCptCityColorRamp &ramp, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a color ramp representing the current settings from the dialog.
     * \see setRamp()
     */
    QgsCptCityColorRamp ramp() const { return mRamp; }

    /**
     * Sets the color ramp to show in the dialog.
     * \param ramp color ramp
     * \see ramp()
     */
    void setRamp( const QgsCptCityColorRamp &ramp );

    /**
     * Returns the name of the ramp currently selected in the dialog.
     */
    QString selectedName() const
    {
      return QFileInfo( mRamp.schemeName() ).baseName() + mRamp.variantName();
    }

    /**
     * Returns true if the ramp should be converted to a QgsGradientColorRamp.
     */
    bool saveAsGradientRamp() const;

    bool eventFilter( QObject *obj, QEvent *event ) override;

  signals:

    //! Emitted when the dialog settings change
    void changed();

  private slots:
    void populateVariants();

    void mTreeView_clicked( const QModelIndex & );
    void mListWidget_itemClicked( QListWidgetItem *item );
    void mListWidget_itemSelectionChanged();
    void tabBar_currentChanged( int index );
    void pbtnLicenseDetails_pressed();
    void cboVariantName_currentIndexChanged( int index );
    void onFinished();
    void showHelp();
    /* void refresh(); */

  private:

    void updateUi();
    void updatePreview( bool clear = false );
    void clearCopyingInfo();
    void updateCopyingInfo( const QMap< QString, QString > &copyingMap );
    void updateTreeView( QgsCptCityDataItem *item, bool resetRamp = true );
    void updateListWidget( QgsCptCityDataItem *item );

    QgsCptCityColorRamp mRamp;
    QgsCptCityArchive *mArchive = nullptr;
    QgsCptCityBrowserModel::ViewType mArchiveViewType;

    /* void refreshModel( const QModelIndex& index ); */
    bool updateRamp();
    void showAll();
    void setTreeModel( QgsCptCityBrowserModel *model );

    QgsCptCityBrowserModel *mModel = nullptr;
    QgsCptCityBrowserModel *mAuthorsModel = nullptr;
    QgsCptCityBrowserModel *mSelectionsModel = nullptr;
    TreeFilterProxyModel *mTreeFilter = nullptr;
    QVector<QgsCptCityColorRampItem *> mListRamps;

};

#ifndef SIP_RUN
/// @cond PRIVATE

/**
 * \ingroup gui
 * \class TreeFilterProxyModel
 */
class TreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    TreeFilterProxyModel( QObject *parent, QgsCptCityBrowserModel *model );

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    // bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

  private:
    QgsCptCityBrowserModel *mModel = nullptr;
};

///@endcond
#endif

#endif
