/***************************************************************************
  qgsvaliditycheckresultswidget.cpp
 ----------------------------------
 begin                : November 2018
 copyright            : (C) 2018 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALIDITYCHECKRESULTSWIDGET_H
#define QGSVALIDITYCHECKRESULTSWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgsvaliditycheckresultsbase.h"

#include "qgsabstractvaliditycheck.h"

#include <QWidget>
#include <QAbstractItemModel>

class QgsValidityCheckContext;

/**
 * \class QgsValidityCheckResultsModel
 * \ingroup gui
 * \brief A QAbstractItemModel subclass for displaying the results from a QgsAbstractValidityCheck.
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsValidityCheckResultsModel : public QAbstractItemModel
{
    Q_OBJECT
  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsValidityCheckResultsModel::Roles
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsValidityCheckResultsModel, Roles ) : int
    {
      Description SIP_MONKEYPATCH_COMPAT_NAME(DescriptionRole) = Qt::UserRole + 1, //!< Result detailed description
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsValidityCheckResultsModel, showing the specified list of checks \a results.
     */
    QgsValidityCheckResultsModel( const QList< QgsValidityCheckResult > &results, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
  private:

    QList< QgsValidityCheckResult > mResults;
};

/**
 * \class QgsValidityCheckResultsWidget
 * \ingroup gui
 * \brief A reusable widget which displays a summary of the results from a QgsAbstractValidityCheck (or checks).
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsValidityCheckResultsWidget : public QWidget, private Ui::QgsValidityCheckResultsBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsValidityCheckResultsWidget, with the specified \a parent widget.
     */
    QgsValidityCheckResultsWidget( QWidget *parent SIP_TRANSFERTHIS );

    /**
     * Sets a \a description label to show at the top of the widget, e.g. notifying users of
     * why they are being shown the warnings.
     */
    void setDescription( const QString &description );

    /**
     * Sets the list of check \a results to show in the dialog.
     */
    void setResults( const QList< QgsValidityCheckResult > &results );

    /**
     * Runs all registered validity checks of the given \a type, and if any warnings or critical
     * errors are encountered then displays them to users in a dialog.
     *
     * The \a context argument must specify the correct QgsValidityCheckContext subclass for the
     * given check \a type.
     *
     * The \a title argument is used as the dialog's title, and the \a description text will be shown
     * to users as an explanation of why the checks are being run.
     *
     * The \a parent argument can be used to give a parent widget for the created dialogs.
     *
     * If any critical errors are encountered by the checks, then users will not be allowed to click OK
     * on the dialog and proceed with the operation. The function will return FALSE.
     *
     * Returns TRUE if no warnings were encountered (and no dialog was shown to users), or if only
     * warnings were shown and the user clicked OK after being shown these warnings.
     *
     * This method is a blocking method, and runs all checks in the main thread.
     */
    static bool runChecks( int type, const QgsValidityCheckContext *context, const QString &title, const QString &description, QWidget *parent = nullptr );

  private slots:

    void selectionChanged( const QModelIndex &current, const QModelIndex &previous );

  private:

    QgsValidityCheckResultsModel *mResultsModel = nullptr;

};

#endif // QGSVALIDITYCHECKRESULTSWIDGET_H
