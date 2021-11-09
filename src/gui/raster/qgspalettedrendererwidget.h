/***************************************************************************
                         qgspalettedrendererwidget.h
                         ---------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPALETTEDRENDERERWIDGET_H
#define QGSPALETTEDRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "qgis_sip.h"
#include "qgspalettedrasterrenderer.h"
#include "qgscolorschemelist.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "ui_qgspalettedrendererwidgetbase.h"
#include "qgis_gui.h"

#include <QThread>
#include <QSortFilterProxyModel>

class QgsRasterLayer;
class QgsLocaleAwareNumericLineEditDelegate;

#ifndef SIP_RUN
/// @cond PRIVATE

/**
 * \class QgsPalettedRendererClassGatherer
* Calculated raster stats for paletted renderer in a thread
*/
class QgsPalettedRendererClassGatherer: public QThread
{
    Q_OBJECT

  public:
    QgsPalettedRendererClassGatherer( QgsRasterLayer *layer, int bandNumber, const QgsPalettedRasterRenderer::ClassData &existingClasses, QgsColorRamp *ramp = nullptr );

    void run() override;

    //! Informs the gatherer to immediately stop collecting values
    void stop()
    {
      // be cautious, in case gatherer stops naturally just as we are canceling it and mFeedback gets deleted
      mFeedbackMutex.lock();
      if ( mFeedback )
        mFeedback->cancel();
      mFeedbackMutex.unlock();

      mWasCanceled = true;
    }

    //! Returns TRUE if collection was canceled before completion
    bool wasCanceled() const { return mWasCanceled; }

    QgsPalettedRasterRenderer::ClassData classes() const { return mClasses; }

  signals:

    /**
     * Emitted when classes have been collected
     */
    void collectedClasses();

  signals:
    //! Internal routines can connect to this signal if they use event loop
    void canceled();

    void progressChanged( double progress );

  private:

    QgsRasterLayer *mLayer = nullptr;
    int mBandNumber;
    std::unique_ptr< QgsColorRamp > mRamp;
    QString mSubstring;
    QgsPalettedRasterRenderer::ClassData mClasses;
    QgsRasterBlockFeedback *mFeedback = nullptr;
    QMutex mFeedbackMutex;
    bool mWasCanceled;
};

class QgsPalettedRendererModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    enum Column
    {
      ValueColumn = 0,
      ColorColumn = 1,
      LabelColumn = 2,
    };

    QgsPalettedRendererModel( QObject *parent = nullptr );

    void setClassData( const QgsPalettedRasterRenderer::ClassData &data );

    QgsPalettedRasterRenderer::ClassData classData() const { return mData; }
    QgsPalettedRasterRenderer::Class classAtIndex( const QModelIndex &index ) const { return mData.at( index.row() ); }

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    QModelIndex addEntry( const QColor &color );

  public slots:

    void deleteAll();

  signals:

    void classesChanged();

  private:

    QgsPalettedRasterRenderer::ClassData mData;


};

class QgsPalettedRendererProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    QgsPalettedRendererProxyModel( QObject *parent = 0 )
      : QSortFilterProxyModel( parent )
    {
    }

    //! Return sorted class data
    QgsPalettedRasterRenderer::ClassData classData() const;

  protected:

    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override
    {
      const QModelIndex lv { left.model()->index( left.row(), static_cast<int>( QgsPalettedRendererModel::Column::ValueColumn ), left.parent() ) };
      const QModelIndex rv { right.model()->index( right.row(), static_cast<int>( QgsPalettedRendererModel::Column::ValueColumn ), right.parent() ) };
      const double leftData { sourceModel()->data( lv ).toDouble( ) };
      const double rightData { sourceModel()->data( rv ).toDouble( ) };
      return leftData < rightData;
    }

};

///@endcond PRIVATE
#endif

/**
 * \ingroup gui
 * \class QgsPalettedRendererWidget
 */
class GUI_EXPORT QgsPalettedRendererWidget: public QgsRasterRendererWidget, private Ui::QgsPalettedRendererWidgetBase
{
    Q_OBJECT

  public:

    QgsPalettedRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsPalettedRendererWidget() override;
    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsPalettedRendererWidget( layer, extent ); }

    QgsRasterRenderer *renderer() SIP_FACTORY override;

    /**
     * Sets the widget state from the specified renderer.
     */
    void setFromRenderer( const QgsRasterRenderer *r );

  private:

    QMenu *mContextMenu = nullptr;
    QMenu *mAdvancedMenu = nullptr;
    QAction *mLoadFromLayerAction = nullptr;
    QgsPalettedRendererModel *mModel = nullptr;
    QgsPalettedRendererProxyModel *mProxyModel = nullptr;

    //! Background class gatherer thread
    QgsPalettedRendererClassGatherer *mGatherer = nullptr;

    int mBand = -1;

    QgsLocaleAwareNumericLineEditDelegate *mValueDelegate = nullptr;

    void setSelectionColor( const QItemSelection &selection, const QColor &color );

  private slots:

    void deleteEntry();
    void addEntry();
    void changeColor();
    void changeOpacity();
    void changeLabel();
    void applyColorRamp();
    void loadColorTable();
    void saveColorTable();
    void classify();
    void loadFromLayer();
    void bandChanged( int band );

    void gatheredClasses();
    void gathererThreadFinished();
    void layerWillBeRemoved( QgsMapLayer *layer );

};

#endif // QGSPALETTEDRENDERERWIDGET_H
