/***************************************************************************
                         qgslocatorwidget.h
                         ------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCATORWIDGET_H
#define QGSLOCATORWIDGET_H

#include "qgis_gui.h"
#include "qgslocatorfilter.h"
#include "qgsfloatingwidget.h"
#include <QWidget>
#include <QAbstractListModel>
#include <QTreeView>
#include <QFocusEvent>
#include <QHeaderView>
#include <QTimer>
#include <QSortFilterProxyModel>

class QgsLocator;
class QgsFilterLineEdit;
class QgsLocatorModel;
class QgsLocatorResultsView;
class QgsMapCanvas;
class QgsLocatorProxyModel;

/**
 * \class QgsLocatorWidget
 * \ingroup gui
 * A special locator widget which allows searching for matching results from a QgsLocator
 * and presenting them to users for selection.
 * \see QgsLocator
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLocatorWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLocatorWidget.
     */
    QgsLocatorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a pointer to the locator utilized by this widget.
     */
    QgsLocator *locator();

    /**
     * Sets a map \a canvas to associate with the widget. This allows the
     * widget to customise the searches performed by its locator(), such
     * as prioritizing results which are near the current canvas extent.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  public slots:

    /**
     * Triggers the locator widget to focus, open and start searching for a specified \a string.
     */
    void search( const QString &string );

  protected:

    bool eventFilter( QObject *obj, QEvent *event ) override;

  private slots:

    void scheduleDelayedPopup();
    void performSearch();
    void showList();
    void searchFinished();
    void addResult( const QgsLocatorResult &result );

  private:

    QgsLocator *mLocator = nullptr;
    QgsFilterLineEdit *mLineEdit = nullptr;
    QgsLocatorModel *mLocatorModel = nullptr;
    QgsLocatorProxyModel *mProxyModel = nullptr;
    QgsFloatingWidget *mResultsContainer = nullptr;
    QgsLocatorResultsView *mResultsView = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

    QString mNextRequestedString;
    bool mHasQueuedRequest = false;
    QTimer mPopupTimer;

    void updateResults( const QString &text );
    void acceptCurrentEntry();
    QgsLocatorContext createContext();

};

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \class QgsLocatorModel
 * \ingroup gui
 * An abstract list model for displaying the results in a QgsLocatorWidget.
 * \since QGIS 3.0
 */
class QgsLocatorModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum Role
    {
      ResultDataRole = Qt::UserRole + 1, //!< QgsLocatorResult data
      ResultTypeRole,
      ResultFilterPriorityRole,
      ResultFilterNameRole,
    };

    /**
     * Constructor for QgsLocatorModel.
     */
    QgsLocatorModel( QObject *parent = nullptr );

    /**
     * Resets the model and clears all existing results.
     */
    void clear();

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

  public slots:

    /**
     * Adds a new \a result to the model.
     */
    void addResult( const QgsLocatorResult &result );

  private:

    struct Entry
    {
      QgsLocatorResult result;
      QString filterTitle;
      QgsLocatorFilter *filter = nullptr;
    };

    QList<Entry> mResults;
    QSet<QString> mFoundResultsFromFilterNames;
};

class QgsLocatorProxyModel : public QSortFilterProxyModel
{
  public:

    explicit QgsLocatorProxyModel( QObject *parent = nullptr );
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
};

/**
 * \class QgsLocatorResultsView
 * \ingroup gui
 * Custom QTreeView designed for showing the results in a QgsLocatorWidget.
 * \since QGIS 3.0
 */
class QgsLocatorResultsView : public QTreeView
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLocatorResultsView.
     */
    QgsLocatorResultsView( QWidget *parent = nullptr );

    /**
     * Recalculates the optimal size for the view.
     */
    void recalculateSize();

    /**
     * Selects the next result in the list, wrapping around for the last result.
     */
    void selectNextResult();

    /**
     * Selects the previous result in the list, wrapping around for the first result.
     */
    void selectPreviousResult();

};

///@endcond

#endif


#endif // QGSLOCATORWIDGET_H


