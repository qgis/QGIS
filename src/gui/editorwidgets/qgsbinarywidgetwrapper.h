/***************************************************************************
    qgsbinarywidgetwrapper.h
     -----------------------
    Date                 : November 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBINARYWIDGETWRAPPER_H
#define QGSBINARYWIDGETWRAPPER_H

#include "qgseditorwidgetwrapper.h"
#include "qgis_gui.h"

class QLabel;
class QToolButton;

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsBinaryWidgetWrapper
 * Widget wrapper for binary (BLOB) fields.
 * \note not available in Python bindings
 * \since QGIS 3.6
 */

class GUI_EXPORT QgsBinaryWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    explicit QgsBinaryWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *editor = nullptr, QWidget *parent = nullptr );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;
    void showIndeterminateState() override;
    void setEnabled( bool enabled ) override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

  public slots:
    void setValue( const QVariant &value ) override;

  private slots:

    void saveContent();
    void setContent();
    void clear();

  private:
    QString defaultPath();

    QByteArray mValue;

    QLabel *mLabel = nullptr;
    QToolButton *mButton = nullptr;
    QAction *mSetAction = nullptr;
    QAction *mClearAction = nullptr;
    QAction *mSaveAction = nullptr;
};

#endif // QGSBINARYWIDGETWRAPPER_H
