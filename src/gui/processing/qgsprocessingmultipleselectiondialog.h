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
#include "qgsmimedatautils.h"
#include <QDialog>

class QStandardItemModel;
class QToolButton;
class QStandardItem;
class QgsProcessingModelChildParameterSource;
class QgsProcessingModelAlgorithm;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A panel widget for selection of multiple options from a fixed list of options.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingMultipleSelectionPanelWidget : public QgsPanelWidget, private Ui::QgsProcessingMultipleSelectionDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingMultipleSelectionPanelWidget.
     *
     * The \a availableOptions list specifies the list of standard known options for the parameter,
     * whilst the \a selectedOptions list specifies which options should be initially selected.
     *
     * The \a selectedOptions list may contain extra options which are not present in \a availableOptions,
     * in which case they will be also added as existing options within the dialog.
     */
    QgsProcessingMultipleSelectionPanelWidget( const QVariantList &availableOptions = QVariantList(), const QVariantList &selectedOptions = QVariantList(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets a callback function to use when encountering an invalid geometry and
     */
#ifndef SIP_RUN
    void setValueFormatter( const std::function<QString( const QVariant & )> &formatter );
#else
    void setValueFormatter( SIP_PYCALLABLE );
    //%MethodCode

    Py_BEGIN_ALLOW_THREADS

      sipCpp->setValueFormatter( [a0]( const QVariant &v ) -> QString {
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
    //%End
#endif


    /**
     * Returns the ordered list of selected options.
     */
    QVariantList selectedOptions() const;

    /**
     * Returns the widget's button box.
     */
    QDialogButtonBox *buttonBox() { return mButtonBox; }

  signals:

    /**
     * Emitted when the accept button is clicked.
     */
    void acceptClicked();

    /**
     * Emitted when the selection changes in the widget.
     */
    void selectionChanged();

  protected:
    /**
     * Adds a new option to the widget.
     */
    void addOption( const QVariant &value, const QString &title, bool selected, bool updateExistingTitle = false );

    //! Returns pointer to the list view
    QListView *listView() const { return mSelectionList; }

    //! Dialog list model
    QStandardItemModel *mModel = nullptr;
    //! Value formatter
    std::function<QString( const QVariant & )> mValueFormatter;

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private slots:

    void selectAll( bool checked );
    void toggleSelection();

  private:
    QPushButton *mButtonSelectAll = nullptr;
    QPushButton *mButtonClearSelection = nullptr;
    QPushButton *mButtonToggleSelection = nullptr;

    QList<QStandardItem *> currentItems();

    void populateList( const QVariantList &availableOptions, const QVariantList &selectedOptions );

    friend class TestProcessingGui;

    /**
     * Returns a map layer, compatible with the filters set for the combo box, from
     * the specified mime \a data (if possible!).
     */
    QList<int> existingMapLayerFromMimeData( const QMimeData *data ) const;
};


/**
 * \ingroup gui
 * \brief A dialog for selection of multiple options from a fixed list of options.
 * \note Not stable API
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsProcessingMultipleSelectionDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingMultipleSelectionPanelWidget.
     *
     * The \a availableOptions list specifies the list of standard known options for the parameter,
     * whilst the \a selectedOptions list specifies which options should be initially selected.
     *
     * The \a selectedOptions list may contain extra options which are not present in \a availableOptions,
     * in which case they will be also added as existing options within the dialog.
     */
    QgsProcessingMultipleSelectionDialog( const QVariantList &availableOptions = QVariantList(), const QVariantList &selectedOptions = QVariantList(), QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );


    /**
     * Sets a callback function to use when encountering an invalid geometry and
     */
#ifndef SIP_RUN
    void setValueFormatter( const std::function<QString( const QVariant & )> &formatter );
#else
    void setValueFormatter( SIP_PYCALLABLE );
    //%MethodCode

    Py_BEGIN_ALLOW_THREADS

      sipCpp->setValueFormatter( [a0]( const QVariant &v ) -> QString {
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
    //%End
#endif


    /**
     * Returns the ordered list of selected options.
     */
    QVariantList selectedOptions() const;

  private:
    QgsProcessingMultipleSelectionPanelWidget *mWidget = nullptr;
};


/**
 * \ingroup gui
 * \brief A panel widget for selection of multiple inputs from a fixed list of options.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingMultipleInputPanelWidget : public QgsProcessingMultipleSelectionPanelWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingMultipleInputPanelWidget.
     */
    QgsProcessingMultipleInputPanelWidget( const QgsProcessingParameterMultipleLayers *parameter, const QVariantList &selectedOptions, const QList<QgsProcessingModelChildParameterSource> &modelSources, QgsProcessingModelAlgorithm *model = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the project associated with the widget.
     */
    void setProject( QgsProject *project );

    /**
     * Returns a list of layer URIs compatible with the \a parameter, from mime data.
     *
     * \since QGIS 3.40
     */
    static QStringList compatibleUrisFromMimeData(
      const QgsProcessingParameterMultipleLayers *parameter,
      const QMimeData *data,
      const QgsMimeDataUtils::UriList &skipUrls
    ) SIP_SKIP;

  private slots:

    void addFiles();
    void addDirectory();

  protected:
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private:
    /**
     * Returns a map layer, compatible with the filters set for the combo box, from
     * the specified mime \a data (if possible!).
     */
    QList<int> existingMapLayerFromMimeData( const QMimeData *data, QgsMimeDataUtils::UriList &handledUrls ) const;
    void populateFromProject( QgsProject *project );

    const QgsProcessingParameterMultipleLayers *mParameter = nullptr;
};


/**
 * \ingroup gui
 * \brief A dialog for selection of multiple layer inputs.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingMultipleInputDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingMultipleInputDialog.
     *
     * The \a selectedOptions list may contain extra options which are not present in \a availableOptions,
     * in which case they will be also added as existing options within the dialog.
     */
    QgsProcessingMultipleInputDialog( const QgsProcessingParameterMultipleLayers *parameter, const QVariantList &selectedOptions, const QList<QgsProcessingModelChildParameterSource> &modelSources, QgsProcessingModelAlgorithm *model = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Returns the ordered list of selected options.
     */
    QVariantList selectedOptions() const;

    /**
     * Sets the project associated with the dialog.
     */
    void setProject( QgsProject *project );

  private:
    QgsProcessingMultipleInputPanelWidget *mWidget = nullptr;
};


///@endcond

#endif // QGSPROCESSINGMULTIPLESELECTIONDIALOG_H
