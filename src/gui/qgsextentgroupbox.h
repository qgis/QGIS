/***************************************************************************
    qgsextentgroupbox.h
    ---------------------
    begin                : March 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXTENTGROUPBOX_H
#define QGSEXTENTGROUPBOX_H

#include "qgscollapsiblegroupbox.h"

#include "ui_qgsextentgroupboxwidget.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"

class QgsCoordinateReferenceSystem;

/** \ingroup gui
 * Collapsible group box for configuration of extent, typically for a save operation.
 *
 * Besides allowing the user to enter the extent manually, it comes with options to use
 * original extent or extent defined by the current view in map canvas.
 *
 * When using the widget, make sure to call setOriginalExtent(), setCurrentExtent() and setOutputCrs() during initialization.
 *
 * @note added in 2.4
 */
class GUI_EXPORT QgsExtentGroupBox : public QgsCollapsibleGroupBox, private Ui::QgsExtentGroupBoxWidget
{
    Q_OBJECT
    Q_PROPERTY( QString titleBase READ titleBase WRITE setTitleBase )

  public:
    explicit QgsExtentGroupBox( QWidget* parent = nullptr );

    enum ExtentState
    {
      OriginalExtent,  //!<  layer's extent
      CurrentExtent,   //!<  map canvas extent
      UserExtent,      //!<  extent manually entered/modified by the user
    };

    //! Setup original extent - should be called as part of initialization
    void setOriginalExtent( const QgsRectangle& originalExtent, const QgsCoordinateReferenceSystem& originalCrs );

    QgsRectangle originalExtent() const { return mOriginalExtent; }
    const QgsCoordinateReferenceSystem& originalCrs() const { return mOriginalCrs; }

    //! Setup current extent - should be called as part of initialization (or whenever current extent changes)
    void setCurrentExtent( const QgsRectangle& currentExtent, const QgsCoordinateReferenceSystem& currentCrs );

    QgsRectangle currentExtent() const { return mCurrentExtent; }
    const QgsCoordinateReferenceSystem& currentCrs() const { return mCurrentCrs; }

    //! Set the output CRS - may need to be used for transformation from original/current extent.
    //! Should be called as part of initialization and whenever the the output CRS is changed
    void setOutputCrs( const QgsCoordinateReferenceSystem& outputCrs );

    //! Get the resulting extent - in output CRS coordinates
    QgsRectangle outputExtent() const;

    ExtentState extentState() const { return mExtentState; }

    //! Set base part of title of the group box (will be appended with extent state)
    //! @note added in 2.12
    void setTitleBase( const QString& title );
    //! Set base part of title of the group box (will be appended with extent state)
    //! @note added in 2.12
    QString titleBase() const;

  public slots:
    //! set output extent to be the same as original extent (may be transformed to output CRS)
    void setOutputExtentFromOriginal();

    //! set output extent to be the same as current extent (may be transformed to output CRS)
    void setOutputExtentFromCurrent();

    //! set output extent to custom extent (may be transformed to outut CRS)
    void setOutputExtentFromUser( const QgsRectangle& extent, const QgsCoordinateReferenceSystem& crs );

  signals:
    //! emitted when extent is changed
    void extentChanged( const QgsRectangle& r );

  protected slots:

    void on_mXMinLineEdit_textEdited( const QString & ) { setOutputExtentFromLineEdit(); }
    void on_mXMaxLineEdit_textEdited( const QString & ) { setOutputExtentFromLineEdit(); }
    void on_mYMinLineEdit_textEdited( const QString & ) { setOutputExtentFromLineEdit(); }
    void on_mYMaxLineEdit_textEdited( const QString & ) { setOutputExtentFromLineEdit(); }

    void groupBoxClicked();

  protected:
    void setOutputExtent( const QgsRectangle& r, const QgsCoordinateReferenceSystem& srcCrs, ExtentState state );
    void setOutputExtentFromLineEdit();
    void updateTitle();

    //! Base part of the title used for the extent
    QString mTitleBase;

    ExtentState mExtentState;

    QgsCoordinateReferenceSystem mOutputCrs;

    QgsRectangle mCurrentExtent;
    QgsCoordinateReferenceSystem mCurrentCrs;

    QgsRectangle mOriginalExtent;
    QgsCoordinateReferenceSystem mOriginalCrs;
};

#endif // QGSEXTENTGROUPBOX_H
