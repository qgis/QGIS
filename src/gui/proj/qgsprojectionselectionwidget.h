/***************************************************************************
    qgsprojectionselectionwidget.h
     --------------------------------------
    Date                 : 05.01.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROJECTIONSELECTIONWIDGET_H
#define QGSPROJECTIONSELECTIONWIDGET_H

#include <QWidget>
#include "qgis_sip.h"
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>
#include <QPointer>
#include <QConcatenateTablesProxyModel>

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatereferencesystemmodel.h"
#include "qgis_gui.h"

class QgsMapLayer;
class QgsHighlightableComboBox;
class QgsCrsSelectionWidget;
class QLabel;
class QgsMapLayer;
class QgsRecentCoordinateReferenceSystemsProxyModel;
class CombinedCoordinateReferenceSystemsProxyModel;

/**
 * \class QgsProjectionSelectionWidget
 * \ingroup gui
 * \brief A widget for selecting a projection.
 */
class GUI_EXPORT QgsProjectionSelectionWidget : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Predefined CRS options shown in widget
     */
    enum CrsOption SIP_ENUM_BASETYPE( IntFlag )
    {
      Invalid = 1 << 0,    //!< Invalid option, since QGIS 3.36
      LayerCrs = 1 << 1,   //!< Optional layer CRS
      ProjectCrs = 1 << 2, //!< Current project CRS (if OTF reprojection enabled)
      CurrentCrs = 1 << 3, //!< Current user selected CRS
      DefaultCrs = 1 << 4, //!< Global default QGIS CRS
      RecentCrs = 1 << 5,  //!< Recently used CRS
      CrsNotSet = 1 << 6,  //!< Not set (hidden by default)
    };

    /**
     * Flags for predefined CRS options shown in widget.
     * \since QGIS 3.36
     */
    SIP_SKIP Q_DECLARE_FLAGS( CrsOptions, CrsOption )

      /**
     * Constructor for QgsProjectionSelectionWidget, with the specified \a parent widget.
     *
     * Since QGIS 3.36, the optional \a filter argument can be used to specify filters on the systems
     * shown in the widget. The default is to show all horizontal and compound CRS in order to match
     * the behavior of older QGIS releases. The \a filter can be altered to also include vertical CRS if desired.
     */
      explicit QgsProjectionSelectionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsCoordinateReferenceSystemProxyModel::Filters filters = QgsCoordinateReferenceSystemProxyModel::FilterHorizontal | QgsCoordinateReferenceSystemProxyModel::FilterCompound );

    /**
     * Returns the currently selected CRS for the widget
     * \returns current CRS
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets whether a predefined CRS option should be shown in the widget.
     * \param option CRS option to show/hide
     * \param visible whether the option should be shown
     * \see optionVisible()
     */
    void setOptionVisible( CrsOption option, bool visible );

    /**
     * Returns whether the specified CRS option is visible in the widget.
     * \see setOptionVisible()
     */
    bool optionVisible( CrsOption option ) const;

    /**
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setOptionVisible().
     */
    void setNotSetText( const QString &text );

    /**
     * Sets a \a message to show in the dialog. If an empty string is
     * passed, the message will be a generic
     * 'define the CRS for this layer'.
     */
    void setMessage( const QString &text );

    /**
     * Returns display text for the specified \a crs.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.8
     */
    static QString crsOptionText( const QgsCoordinateReferenceSystem &crs ) SIP_SKIP;

    /**
     * Returns TRUE if the widget will show a warning to users when they select a CRS which has
     * low accuracy.
     *
     * \see setShowAccuracyWarnings()
     * \since QGIS 3.20
     */
    bool showAccuracyWarnings() const;

    /**
     * Sets whether the widget will \a show warnings to users when they select a CRS which has
     * low accuracy.
     *
     * \see showAccuracyWarnings()
     * \since QGIS 3.20
     */
    void setShowAccuracyWarnings( bool show );

    /**
     * Sets the original source \a ensemble datum name.
     *
     * If set, CRS accuracy warnings will not be shown when the selected CRS in the widget has a matching
     * ensemble datum, regardless of the ensemble's accuracy.
     *
     * \see sourceEnsemble()
     * \since QGIS 3.20
     */
    void setSourceEnsemble( const QString &ensemble );

    /**
     * Returns the original source ensemble datum name.
     *
     * If set, CRS accuracy warnings will not be shown when the selected CRS in the widget has a matching
     * ensemble datum, regardless of the ensemble's accuracy.
     *
     * \see setSourceEnsemble()
     * \since QGIS 3.20
     */
    QString sourceEnsemble() const;

    /**
     * Sets the \a title for the CRS selector dialog window.
     * \see dialogTitle()
     * \since QGIS 3.24
     */
    void setDialogTitle( const QString &title );

    /**
     * Returns the title for the CRS selector dialog window.
     * \see setDialogTitle()
     * \since QGIS 3.24
     */
    QString dialogTitle() const;

    /**
     * Sets a filtered list of CRSes to show in the widget.
     *
     * \since QGIS 3.28
     */
    void setFilter( const QList<QgsCoordinateReferenceSystem> &crses );

    /**
     * Returns the filters set on the available CRS.
     *
     * \see setFilters()
     * \since QGIS 3.36
     */
    QgsCoordinateReferenceSystemProxyModel::Filters filters() const;

    /**
     * Sets \a filters for the available CRS.
     *
     * \see filters()
     * \since QGIS 3.36
     */
    void setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters );

  signals:

    /**
     * Emitted when the selected CRS is changed
     */
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

    /**
     * Emitted when the not set option is selected.
     */
    void cleared();

  public slots:

    /**
     * Sets the current CRS for the widget
     * \param crs new CRS
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the layer CRS for the widget. If set, this will be added as an option
     * to the preset CRSes shown in the widget.
     * \param crs layer CRS
     */
    void setLayerCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Opens the dialog for selecting a new CRS
     */
    void selectCrs();

  protected:
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private:
    CombinedCoordinateReferenceSystemsProxyModel *mModel = nullptr;

    QgsHighlightableComboBox *mCrsComboBox = nullptr;
    QToolButton *mButton = nullptr;

    QString mMessage;

    bool mShowAccuracyWarnings = false;
    QString mSourceEnsemble;

    QWidget *mWarningLabelContainer = nullptr;
    QLabel *mWarningLabel = nullptr;

    QPointer<QgsCrsSelectionWidget> mActivePanel;
    int mIgnorePanelSignals = 0;

    QString mDialogTitle;

    void updateTooltip();

    QgsMapLayer *mapLayerFromMimeData( const QMimeData *data ) const;

  private slots:

    void comboIndexChanged( int idx );
    void updateWarning();
};

SIP_SKIP Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProjectionSelectionWidget::CrsOptions );

///@cond PRIVATE

class StandardCoordinateReferenceSystemsModel : public QAbstractItemModel SIP_SKIP
{
    Q_OBJECT

  public:
    enum Role
    {
      // values copied from QgsRecentCoordinateReferenceSystemsModel
      RoleCrs = Qt::UserRole, //!< Coordinate reference system
      // new values
      RoleOption = RoleCrs + 100, //!< Option
    };

    StandardCoordinateReferenceSystemsModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;

    QgsCoordinateReferenceSystem crs( const QModelIndex &index ) const;
    QgsProjectionSelectionWidget::CrsOption optionForIndex( const QModelIndex &index ) const;
    QModelIndex indexForOption( QgsProjectionSelectionWidget::CrsOption option ) const;

    void setLayerCrs( const QgsCoordinateReferenceSystem &crs );
    void setCurrentCrs( const QgsCoordinateReferenceSystem &crs );
    void setNotSetText( const QString &text );
    QString notSetText() const { return mNotSetText; }
    QgsCoordinateReferenceSystem currentCrs() const { return mCurrentCrs; }

  private:
    QgsProjectionSelectionWidget::CrsOptions mOptions;
    QgsCoordinateReferenceSystem mCurrentCrs;
    QgsCoordinateReferenceSystem mProjectCrs;
    QgsCoordinateReferenceSystem mDefaultCrs;
    QgsCoordinateReferenceSystem mLayerCrs;

    QString mNotSetText;
};

class CombinedCoordinateReferenceSystemsModel : public QConcatenateTablesProxyModel SIP_SKIP
{
    Q_OBJECT

  public:
    CombinedCoordinateReferenceSystemsModel( QObject *parent );
    void setNotSetText( const QString &text );
    QString notSetText() const;
    QgsCoordinateReferenceSystem currentCrs() const;
    StandardCoordinateReferenceSystemsModel *standardModel() { return mStandardModel; }

  private:
    StandardCoordinateReferenceSystemsModel *mStandardModel = nullptr;
    QgsRecentCoordinateReferenceSystemsProxyModel *mRecentModel = nullptr;
};

class CombinedCoordinateReferenceSystemsProxyModel : public QSortFilterProxyModel SIP_SKIP
{
    Q_OBJECT

  public:
    CombinedCoordinateReferenceSystemsProxyModel( QObject *parent );
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    void setLayerCrs( const QgsCoordinateReferenceSystem &crs );
    void setCurrentCrs( const QgsCoordinateReferenceSystem &crs );
    void setFilters( QgsCoordinateReferenceSystemProxyModel::Filters filters );
    QgsCoordinateReferenceSystemProxyModel::Filters filters() const;

    void setFilteredCrs( const QList<QgsCoordinateReferenceSystem> &crses );
    QList<QgsCoordinateReferenceSystem> filteredCrs() const { return mFilteredCrs; }

    void setOption( QgsProjectionSelectionWidget::CrsOption option, bool enabled );
    CombinedCoordinateReferenceSystemsModel *combinedModel() const { return mModel; }

  private:
    CombinedCoordinateReferenceSystemsModel *mModel = nullptr;
    QgsProjectionSelectionWidget::CrsOptions mVisibleOptions;

    QList<QgsCoordinateReferenceSystem> mFilteredCrs;

    QgsCoordinateReferenceSystemProxyModel::Filters mFilters = QgsCoordinateReferenceSystemProxyModel::FilterHorizontal | QgsCoordinateReferenceSystemProxyModel::FilterCompound;
};

///@endcond PRIVATE

#endif // QGSPROJECTIONSELECTIONWIDGET_H
