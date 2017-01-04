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
#include "qgswkbtypes.h"

class QLineEdit;
class QToolButton;
class QMenu;

#ifdef SIP_RUN
// adapted from the qpymultimedia_qlist.sip file from the PyQt6 sources
% MappedType QList<QgsWkbTypes::Type>
{
  % TypeHeaderCode
#include <QList>
  % End

  % ConvertFromTypeCode
  PyObject *l = PyList_New( sipCpp->size() );

  if ( !l )
    return 0;

  for ( int i = 0; i < sipCpp->size(); ++i )
  {
    PyObject *eobj = sipConvertFromEnum( sipCpp->at( i ),
                                         sipType_QgsWkbTypes_Type );

    if ( !eobj )
    {
      Py_DECREF( l );

      return 0;
    }

    PyList_SetItem( l, i, eobj );
  }

  return l;
  % End

  % ConvertToTypeCode
  PyObject *iter = PyObject_GetIter( sipPy );

  if ( !sipIsErr )
  {
    PyErr_Clear();
    Py_XDECREF( iter );

    return ( iter
#if PY_MAJOR_VERSION < 3
             && !PyString_Check( sipPy )
#endif
             && !PyUnicode_Check( sipPy ) );
  }

  if ( !iter )
  {
    *sipIsErr = 1;

    return 0;
  }

  QList<QgsWkbTypes::Type> *ql = new QList<QgsWkbTypes::Type>;

  for ( Py_ssize_t i = 0; ; ++i )
  {
    PyErr_Clear();
    PyObject *itm = PyIter_Next( iter );

    if ( !itm )
    {
      if ( PyErr_Occurred() )
      {
        delete ql;
        Py_DECREF( iter );
        *sipIsErr = 1;

        return 0;
      }

      break;
    }

    int v = sipConvertToEnum( itm, sipType_QgsWkbTypes_Type );

    if ( PyErr_Occurred() )
    {
      PyErr_Format( PyExc_TypeError,
                    "index %zd has type '%s' but 'QgsWkbTypes.Type' is expected",
                    i, sipPyTypeName( Py_TYPE( itm ) ) );

      Py_DECREF( itm );
      delete ql;
      Py_DECREF( iter );
      *sipIsErr = 1;

      return 0;
    }

    ql->append( static_cast<QgsWkbTypes::Type>( v ) );

    Py_DECREF( itm );
  }

  Py_DECREF( iter );

  *sipCppPtr = ql;

  return sipGetState( sipTransferObj );
  % End
};
#endif


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
    void setAcceptedWkbTypes( const QList<QgsWkbTypes::Type> &types );

    /**
     * Returns the list of WKB geometry types which are permitted for the widget.
     *
     * \see setAcceptedWkbTypes()
     */
    QList<QgsWkbTypes::Type> acceptedWkbTypes() const;

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
    QList<QgsWkbTypes::Type> mAcceptedTypes;
    bool mReadOnly = false;

    void fetchGeomFromClipboard();
    bool typeIsAcceptable( QgsWkbTypes::Type type ) const;

  private slots:

    void prepareMenu();
    void updateLineEdit();
};

#endif // QGSGEOMETRYWIDGET_H
