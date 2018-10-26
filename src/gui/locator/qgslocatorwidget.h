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
#include <QTreeView>
#include <QFocusEvent>
#include <QHeaderView>
#include <QTimer>

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
     * widget to customize the searches performed by its locator(), such
     * as prioritizing results which are near the current canvas extent.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  public slots:

    /**
     * Triggers the locator widget to focus, open and start searching for a specified \a string.
     */
    void search( const QString &string );

    /**
     * Invalidates the current search results, e.g. as a result of changes to the locator
     * filter settings.
     */
    void invalidateResults();

  signals:

    /**
     * Emitted when the configure option is triggered in the widget.
     */
    void configTriggered();

  protected:

    bool eventFilter( QObject *obj, QEvent *event ) override;

  private slots:

    void scheduleDelayedPopup();
    void performSearch();
    void showList();
    void triggerSearchAndShowList();
    void searchFinished();
    void addResult( const QgsLocatorResult &result );
    void configMenuAboutToShow();

  private:

    QgsLocator *mLocator = nullptr;
    QgsFilterLineEdit *mLineEdit = nullptr;
    QgsLocatorModel *mLocatorModel = nullptr;
    QgsLocatorProxyModel *mProxyModel = nullptr;
    QgsFloatingWidget *mResultsContainer = nullptr;
    QgsLocatorResultsView *mResultsView = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QMenu *mMenu = nullptr;

    QString mNextRequestedString;
    bool mHasQueuedRequest = false;
    bool mHasSelectedResult = false;
    QTimer mPopupTimer;
    QTimer mFocusTimer;

    void updateResults( const QString &text );
    void acceptCurrentEntry();
    QgsLocatorContext createContext();

};

#ifndef SIP_RUN

///@cond PRIVATE

class QgsLocatorFilterFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsLocatorFilterFilter( QgsLocatorWidget *widget, QObject *parent = nullptr );

    QgsLocatorFilterFilter *clone() const override SIP_FACTORY;
    QgsLocatorFilter::Flags flags() const override;

    QString name() const override { return QStringLiteral( "filters" );}
    QString displayName() const override { return QString(); }
    Priority priority() const override { return static_cast< QgsLocatorFilter::Priority>( -1 ); /** shh, we cheat!**/ }
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

  private:

    QgsLocatorWidget *mLocator = nullptr;
};

/**
 * \class QgsLocatorResultsView
 * \ingroup gui
 * Custom QTreeView designed for showing the results in a QgsLocatorWidget.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLocatorResultsView : public QTreeView
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


