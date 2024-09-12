/***************************************************************************
  qgssettingseditorwidgetwrapperimpl.h
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSEDITORWIDGETWRAPPERIMPL_H
#define QGSSETTINGSEDITORWIDGETWRAPPERIMPL_H

#include <QColor>

#include "qgis_gui.h"
#include "qgssettingseditorwidgetwrapper.h"
#include "qgslogger.h"

#include "qgssettingsentryimpl.h"
#include "qgscolorbutton.h"
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>


//TODO variant map

class QgsColorButton;

/**
 * \ingroup gui
 * \brief This class is a base factory of editor for settings
 *
 * \since QGIS 3.32
 */
template<class T, class V, class U>
class QgsSettingsEditorWidgetWrapperTemplate : public QgsSettingsEditorWidgetWrapper
{
  public:
    //! Constructor
    QgsSettingsEditorWidgetWrapperTemplate( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapper( parent ) {}

    virtual QString id() const override = 0;

    virtual bool setWidgetFromSetting() const override
    {
      if ( mSetting )
        return setWidgetValue( mSetting->value( mDynamicKeyPartList ) );

      QgsDebugError( "editor is not configured" );
      return false;
    }

    virtual bool setSettingFromWidget() const override = 0;

    void setWidgetFromVariant( const QVariant &value ) const override
    {
      setWidgetValue( mSetting->convertFromVariant( value ) );
    }

    //! Sets the widget value
    virtual bool setWidgetValue( const U &value ) const = 0;

    QVariant variantValueFromWidget() const override
    {
      return QVariant::fromValue( valueFromWidget() );
    };

    //! Returns the widget value
    virtual U valueFromWidget() const = 0;

    //! Returns the editor
    V *editor() const {return mEditor;}

    //! Returns the setting
    const T *setting() const {return mSetting;}

    virtual QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override = 0;

  protected:
    virtual QWidget *createEditorPrivate( QWidget *parent = nullptr ) const override
    {
      V *editor = new V( parent );
      editor->setAutoFillBackground( true );
      return editor;
    }

    bool configureEditorPrivate( QWidget *editor, const QgsSettingsEntryBase *setting ) override
    {
      mSetting = static_cast<const T *>( setting );
      Q_ASSERT( mSetting );
      mEditor = qobject_cast<V *>( editor );
      if ( mEditor )
      {
        configureEditorPrivateImplementation();
        return true;
      }
      return false;
    }

    //! To be re-implemented to implemeent type specific configuration (e.g. opacity for colors)
    virtual void configureEditorPrivateImplementation() {}

    const T *mSetting = nullptr;
    V *mEditor = nullptr;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for string settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsStringEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSettingsStringEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>( parent ) {}

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override {return new QgsSettingsStringEditorWidgetWrapper( parent );}

    QString id() const override;

    bool setSettingFromWidget() const override;

    QString valueFromWidget() const override;

    bool setWidgetValue( const QString &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};

/**
 * \ingroup gui
 * \brief This class is a factory of editor for boolean settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsBoolEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSettingsBoolEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>( parent ) {}

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override {return new QgsSettingsBoolEditorWidgetWrapper( parent );}

    QString id() const override;

    bool setSettingFromWidget() const override;

    bool valueFromWidget() const override;

    bool setWidgetValue( const bool &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};

/**
 * \ingroup gui
 * \brief This class is a factory of editor for integer settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsIntegerEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSettingsIntegerEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>( parent ) {}

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override {return new QgsSettingsIntegerEditorWidgetWrapper( parent );}

    QString id() const override;

    bool setSettingFromWidget() const override;

    int valueFromWidget() const override;

    bool setWidgetValue( const int &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for double settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsDoubleEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSettingsDoubleEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>( parent ) {}

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override {return new QgsSettingsDoubleEditorWidgetWrapper( parent );}

    QString id() const override;

    bool setSettingFromWidget() const override;

    double valueFromWidget() const override;

    bool setWidgetValue( const double &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for color settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsColorEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>
{
    Q_OBJECT
  public:
    //! Constructor
    QgsSettingsColorEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>( parent ) {}

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override {return new QgsSettingsColorEditorWidgetWrapper( parent );}

    QString id() const override;

    bool setSettingFromWidget() const override;

    QColor valueFromWidget() const override;

    bool setWidgetValue( const QColor &value ) const override;

    void configureEditorPrivateImplementation() override;

    void enableAutomaticUpdatePrivate() override;
};


///**
// * \ingroup gui
// * \brief This class is a factory of editor for boolean settings
// *
// * \since QGIS 3.32
// */
//class GUI_EXPORT QgsSettingsStringListEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryStringList, QTableWidget, QStringList>
//{
//  public:
//    QgsSettingsStringListEditorWidgetWrapper()
//      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryStringList, QTableWidget, QStringList>() {}

//    QString id() const override;

//    bool setWidgetFromSetting() const override;

//    bool setSettingFromWidget() const override;

//    QStringList valueFromWidget() const override;
//};

#if defined(_MSC_VER)
#ifndef SIP_RUN
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>;
#endif
#endif



#endif // QGSSETTINGSEDITORWIDGETWRAPPERIMPL_H
