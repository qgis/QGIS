/***************************************************************************
    qgsfieldvalueslineedit.h
     -----------------------
    Date                 : 20-08-2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDVALUESLINEEDIT_H
#define QGSFIELDVALUESLINEEDIT_H

#include "qgsfilterlineedit.h"
#include "qgis_sip.h"
#include "qgsfeedback.h"

#include <QStringListModel>
#include <QTreeView>
#include <QFocusEvent>
#include <QHeaderView>
#include <QTimer>
#include <QThread>
#include <QMutex>

#include "qgis_gui.h"

class QgsFloatingWidget;
class QgsVectorLayer;


#ifndef SIP_RUN

// just internal guff - definitely not for exposing to public API!
///@cond PRIVATE

/**
 * \class QgsFieldValuesLineEditValuesGatherer
 * Collates unique values containing a matching substring in a thread.
 */
class QgsFieldValuesLineEditValuesGatherer: public QThread
{
    Q_OBJECT

  public:
    QgsFieldValuesLineEditValuesGatherer( QgsVectorLayer *layer, int attributeIndex )
      : mLayer( layer )
      , mAttributeIndex( attributeIndex )
      , mWasCanceled( false )
    {}

    /**
     * Sets the substring to find matching values containing
     */
    void setSubstring( const QString &string ) { mSubstring = string; }

    void run() override;

    //! Informs the gatherer to immediately stop collecting values
    void stop();

    //! Returns true if collection was canceled before completion
    bool wasCanceled() const { return mWasCanceled; }

  signals:

    /**
     * Emitted when values have been collected
     * \param values list of unique matching string values
     */
    void collectedValues( const QStringList &values );

  private:

    QgsVectorLayer *mLayer = nullptr;
    int mAttributeIndex;
    QString mSubstring;
    QStringList mValues;
    QgsFeedback *mFeedback = nullptr;
    QMutex mFeedbackMutex;
    bool mWasCanceled;
};

///@endcond

#endif

/**
 * \class QgsFieldValuesLineEdit
 * \ingroup gui
 * A line edit with an autocompleter which takes unique values from a vector layer's fields.
 * The autocompleter is populated from the vector layer in the background to ensure responsive
 * interaction with the widget.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsFieldValuesLineEdit: public QgsFilterLineEdit
{
    Q_OBJECT

    Q_PROPERTY( QgsVectorLayer *layer READ layer WRITE setLayer NOTIFY layerChanged )
    Q_PROPERTY( int attributeIndex READ attributeIndex WRITE setAttributeIndex NOTIFY attributeIndexChanged )

  public:

    /**
     * Constructor for QgsFieldValuesLineEdit
     * \param parent parent widget
     */
    QgsFieldValuesLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsFieldValuesLineEdit() override;

    /**
     * Sets the layer containing the field that values will be shown from.
     * \param layer vector layer
     * \see layer()
     * \see setAttributeIndex()
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Returns the layer containing the field that values will be shown from.
     * \see setLayer()
     * \see attributeIndex()
     */
    QgsVectorLayer *layer() const { return mLayer; }

    /**
     * Sets the attribute index for the field containing values to show in the widget.
     * \param index index of attribute
     * \see attributeIndex()
     * \see setLayer()
     */
    void setAttributeIndex( int index );

    /**
     * Returns the attribute index for the field containing values shown in the widget.
     * \see setAttributeIndex()
     * \see layer()
     */
    int attributeIndex() const { return mAttributeIndex; }

  signals:

    /**
     * Emitted when the layer associated with the widget changes.
     * \param layer vector layer
     */
    void layerChanged( QgsVectorLayer *layer );

    /**
     * Emitted when the field associated with the widget changes.
     * \param index new attribute index for field
     */
    void attributeIndexChanged( int index );

  private slots:

    /**
     * Requests that the autocompleter updates its completion list. The update will not occur immediately
     * but after a preset timeout to avoid multiple updates while a user is quickly typing.
     */
    void requestCompleterUpdate();

    /**
     * Updates the autocompleter list immediately. Calling
     * this will trigger a background request to the layer to fetch matching unique values.
     */
    void triggerCompleterUpdate();

    /**
     * Updates the values shown in the completer list.
     * \param values list of string values to show
     */
    void updateCompleter( const QStringList &values );

    /**
     * Called when the gatherer thread is complete, regardless of whether it finished collecting values.
     * Cleans up the gatherer thread and triggers a new background thread if the widget's text has changed
     * in the meantime.
     */
    void gathererThreadFinished();

  private:

    QgsVectorLayer *mLayer = nullptr;
    int mAttributeIndex = -1;

    //! Will be true when a background update of the completer values is occurring
    bool mUpdateRequested = false;

    //! Timer to prevent multiple updates of autocomplete list
    QTimer mShowPopupTimer;

    //! Background value gatherer thread
    QgsFieldValuesLineEditValuesGatherer *mGatherer = nullptr;

    //! Will be set to the latest completion text string which should be requested
    QString mRequestedCompletionText;

    //! Kicks off the gathering of completer text values for a specified substring
    void updateCompletionList( const QString &substring );

};


#endif //QGSFIELDVALUESLINEEDIT_H
