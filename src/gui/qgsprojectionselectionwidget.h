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

#include "qgscoordinatereferencesystem.h"
#include "qgis_gui.h"

class QgsHighlightableComboBox;
class QgsCrsSelectionWidget;
class QLabel;

/**
 * \class QgsProjectionSelectionWidget
 * \ingroup gui
 * \brief A widget for selecting a projection.
 * \since QGIS 2.7
 */
class GUI_EXPORT QgsProjectionSelectionWidget : public QWidget
{
    Q_OBJECT
  public:

    /**
     * Predefined CRS options shown in widget
     */
    enum CrsOption
    {
      LayerCrs, //!< Optional layer CRS
      ProjectCrs, //!< Current project CRS (if OTF reprojection enabled)
      CurrentCrs, //!< Current user selected CRS
      DefaultCrs, //!< Global default QGIS CRS
      RecentCrs, //!< Recently used CRS
      CrsNotSet, //!< Not set (hidden by default)
    };

    //! Constructor for QgsProjectionSelectionWidget
    explicit QgsProjectionSelectionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

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
     * \since QGIS 3.0
     */
    bool optionVisible( CrsOption option ) const;

    /**
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setOptionVisible().
     * \since QGIS 3.0
     */
    void setNotSetText( const QString &text );

    /**
     * Sets a \a message to show in the dialog. If an empty string is
     * passed, the message will be a generic
     * 'define the CRS for this layer'.
     * \since QGIS 3.0
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
    void setFilter( const QList< QgsCoordinateReferenceSystem > &crses );

  signals:

    /**
     * Emitted when the selected CRS is changed
     */
    void crsChanged( const QgsCoordinateReferenceSystem & );

    /**
     * Emitted when the not set option is selected.
     * \since QGIS 3.0
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

    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsCoordinateReferenceSystem mProjectCrs;
    QgsCoordinateReferenceSystem mDefaultCrs;
    QgsHighlightableComboBox *mCrsComboBox = nullptr;
    QToolButton *mButton = nullptr;
    QString mNotSetText;
    QString mMessage;

    bool mShowAccuracyWarnings = false;
    QString mSourceEnsemble;

    QWidget *mWarningLabelContainer = nullptr;
    QLabel *mWarningLabel = nullptr;

    QPointer< QgsCrsSelectionWidget > mActivePanel;
    int mIgnorePanelSignals = 0;

    QString mDialogTitle;

    QList<QgsCoordinateReferenceSystem> mFilter;

    void addNotSetOption();
    void addProjectCrsOption();
    void addDefaultCrsOption();
    void addCurrentCrsOption();

    void addRecentCrs();
    bool crsIsShown( long srsid ) const;

    int firstRecentCrsIndex() const;
    void updateTooltip();

    QgsMapLayer *mapLayerFromMimeData( const QMimeData *data ) const;
    QgsCoordinateReferenceSystem crsAtIndex( int index ) const;

  private slots:

    void comboIndexChanged( int idx );
    void updateWarning();

};

#endif // QGSPROJECTIONSELECTIONWIDGET_H
