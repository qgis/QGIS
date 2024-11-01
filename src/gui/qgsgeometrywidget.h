/***************************************************************************
   qgsgeometrywidget.h
    --------------------------------------
   Date                 : March 2015
   Copyright            : (C) 2015 Nyall Dawson
   Email                : nyall.dawson@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSGEOMETRYWIDGET_H
#define QGSGEOMETRYWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QList>
#include <QWidget>

#include "qgsreferencedgeometry.h"

class QLineEdit;
class QToolButton;
class QMenu;

/**
 * \ingroup gui
 * \class QgsGeometryWidget
 * \brief A widget for storing and interacting with a QgsGeometry object.
 *
 * This widget can be used in places where an dialog needs to expose a geometry
 * value to users, and allow them to safely interact with it (such as changing
 * the stored geometry value).
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsGeometryWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( QgsReferencedGeometry geometryValue READ geometryValue WRITE setGeometryValue NOTIFY geometryValueChanged )

  public:
    /**
     * Constructor for QgsGeometryWidget, with the specified \a parent widget.
     */
    explicit QgsGeometryWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the current \a geometry value for the widget.
     *
     * \warning This should not be confused with QWidget::setGeometry(), which
     * modifies the placement and size of the widget itself.
     *
     * \see geometryValue()
     * \see geometryValueChanged()
     */
    void setGeometryValue( const QgsReferencedGeometry &geometry );

    /**
     * Returns the current geometry value for the widget.
     *
     * \warning This should not be confused with QWidget::geometry(), which
     * returns the placement and size of the widget itself.
     *
     * \see setGeometryValue()
     * \see geometryValueChanged()
     */
    QgsReferencedGeometry geometryValue() const;

    /**
     * Sets the list of WKB geometry \a types which are permitted for the widget.
     *
     * \see acceptedWkbTypes()
     */
    void setAcceptedWkbTypes( const QList<Qgis::WkbType> &types );

    /**
     * Returns the list of WKB geometry types which are permitted for the widget.
     *
     * \see setAcceptedWkbTypes()
     */
    QList<Qgis::WkbType> acceptedWkbTypes() const;

    /**
     * Returns whether the widget is in a read-only state.
     *
     * \see setReadOnly()
     */
    bool isReadOnly() const;

  public slots:

    /**
     * Sets whether the widget should be in a read-only state.
     *
     * \see isReadOnly()
     */
    void setReadOnly( bool readOnly );

    /**
     * Clears the current geometry value stored in the widget.
     */
    void clearGeometry();

    /**
     * Copies the current geometry value to the clipboard, as a WKT string.
     *
     * \see copyAsGeoJson()
     */
    void copyAsWkt();

    /**
     * Copies the current geometry value to the clipboard, as a GeoJSON string.
     *
     * \see copyAsWkt()
     */
    void copyAsGeoJson();

  signals:

    /**
     * Emitted whenever the geometry value of the widget is changed.
     *
     * \see geometryValue()
     * \see setGeometryValue()
     */
    void geometryValueChanged( const QgsReferencedGeometry &value );

  private slots:

    /**
     * Pastes a geometry value into the widget.
     */
    void pasteTriggered();

  private:
    QLineEdit *mLineEdit = nullptr;
    QToolButton *mButton = nullptr;
    QMenu *mMenu = nullptr;
    QAction *mClearAction = nullptr;
    QAction *mCopyWktAction = nullptr;
    QAction *mCopyGeoJsonAction = nullptr;
    QAction *mPasteAction = nullptr;
    QgsReferencedGeometry mGeometry;
    QgsReferencedGeometry mPastedGeom;
    QList<Qgis::WkbType> mAcceptedTypes;
    bool mReadOnly = false;

    void fetchGeomFromClipboard();
    bool typeIsAcceptable( Qgis::WkbType type ) const;

  private slots:

    void prepareMenu();
    void updateLineEdit();
};

#endif // QGSGEOMETRYWIDGET_H
