/***************************************************************************
                             qgsprocessingmultipleselectiondialog.h
                             ----------------------------------
    Date                 : February 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMULTIPLESELECTIONDIALOG_H
#define QGSPROCESSINGMULTIPLESELECTIONDIALOG_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessingmultipleselectiondialogbase.h"
#include "qgsprocessingparameters.h"


class QStandardItemModel;
class QToolButton;
class QStandardItem;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Dialog for configuration of a matrix (fixed table) parameter.
 * \note Not stable API
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsProcessingMultipleSelectionDialog : public QDialog, private Ui::QgsProcessingMultipleSelectionDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingMultipleSelectionDialog.
     *
     * The \a availableOptions list specifies the list of standard known options for the parameter,
     * whilst the \a selectedOptions list specifies which options should be initially selected.
     *
     * The \a selectedOptions list may contain extra options which are not present in \a availableOptions,
     * in which case they will be also added as existing options within the dialog.
     */
    QgsProcessingMultipleSelectionDialog( const QVariantList &availableOptions = QVariantList(),
                                          const QVariantList &selectedOptions = QVariantList(),
                                          QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = nullptr );


    /**
     * Sets a callback function to use when encountering an invalid geometry and
     */
#ifndef SIP_RUN
    void setValueFormatter( const std::function< QString( const QVariant & )> &formatter );
#else
    void setValueFormatter( SIP_PYCALLABLE );
    % MethodCode

    Py_BEGIN_ALLOW_THREADS

    sipCpp->setValueFormatter( [a0]( const QVariant &v )->QString
    {
      QString res;
      SIP_BLOCK_THREADS
      PyObject *s = sipCallMethod( NULL, a0, "D", &v, sipType_QVariant, NULL );
      int state;
      int sipIsError = 0;
      QString *t1 = reinterpret_cast<QString *>( sipConvertToType( s, sipType_QString, 0, SIP_NOT_NONE, &state, &sipIsError ) );
      if ( sipIsError == 0 )
      {
        res = QString( *t1 );
      }
      sipReleaseType( t1, sipType_QString, state );
      SIP_UNBLOCK_THREADS
      return res;
    } );

    Py_END_ALLOW_THREADS
    % End
#endif


    /**
     * Returns the ordered list of selected options.
     */
    QVariantList selectedOptions() const;

  private slots:

    void selectAll( bool checked );
    void toggleSelection();

  private:
    std::function< QString( const QVariant & )> mValueFormatter;

    QPushButton *mButtonSelectAll = nullptr;
    QPushButton *mButtonClearSelection = nullptr;
    QPushButton *mButtonToggleSelection = nullptr;
    QStandardItemModel *mModel = nullptr;

    QList< QStandardItem * > currentItems();

    void populateList( const QVariantList &availableOptions, const QVariantList &selectedOptions );

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGMULTIPLESELECTIONDIALOG_H
