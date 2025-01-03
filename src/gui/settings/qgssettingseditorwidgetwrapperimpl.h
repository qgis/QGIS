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

    bool setWidgetFromVariant( const QVariant &value ) const override
    {
      return setWidgetValue( mSetting->convertFromVariant( value ) );
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
    V *editor() const { return mEditor; }

    //! Returns the setting
    const T *setting() const { return mSetting; }

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
 * \brief This class is a factory of editor for string settings with a line edit
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsStringLineEditWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>
{
    Q_OBJECT
  public:
    //! Constructor of the factory
    QgsSettingsStringLineEditWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>( parent ) {}

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsStringLineEditWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>( editor ) { configureEditor( editor, setting, dynamicKeyPartList ); }


    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsStringLineEditWrapper( parent ); }

    QString id() const override;

    bool setSettingFromWidget() const override;

    QString valueFromWidget() const override;

    bool setWidgetValue( const QString &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for string settings with a combo box
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsSettingsStringComboBoxWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QComboBox, QString>
{
    Q_OBJECT
  public:
    //! Mode to determine if the value is hold in the combo box text or data
    enum class Mode : int
    {
      Text, //!< Value is defined as the text entry
      Data  //!< Value is defined as data entry with Qt::UserRole
    };

    //! Constructor of the factory
    QgsSettingsStringComboBoxWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QComboBox, QString>( parent ) {}

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsStringComboBoxWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QComboBox, QString>( editor ) { configureEditor( editor, setting, dynamicKeyPartList ); }

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsStringComboBoxWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, Mode mode, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QComboBox, QString>( editor ), mMode( mode ) { configureEditor( editor, setting, dynamicKeyPartList ); }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsStringComboBoxWrapper( parent ); }

    QString id() const override;

    bool setSettingFromWidget() const override;

    QString valueFromWidget() const override;

    bool setWidgetValue( const QString &value ) const override;

    void enableAutomaticUpdatePrivate() override;

  private:
    Mode mMode = Mode::Text;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for boolean settings with a checkbox
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsBoolCheckBoxWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>
{
    Q_OBJECT
  public:
    //! Constructor of the factory
    QgsSettingsBoolCheckBoxWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>( parent ) {}

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsBoolCheckBoxWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>( editor ) { configureEditor( editor, setting, dynamicKeyPartList ); }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsBoolCheckBoxWrapper( parent ); }

    QString id() const override;

    bool setSettingFromWidget() const override;

    bool valueFromWidget() const override;

    bool setWidgetValue( const bool &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};

/**
 * \ingroup gui
 * \brief This class is a factory of editor for integer settings with a spin box
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsIntegerSpinBoxWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>
{
    Q_OBJECT
  public:
    //! Constructor of the factory
    QgsSettingsIntegerSpinBoxWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>( parent ) {}

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsIntegerSpinBoxWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>( editor ) { configureEditor( editor, setting, dynamicKeyPartList ); }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsIntegerSpinBoxWrapper( parent ); }

    QString id() const override;

    bool setSettingFromWidget() const override;

    int valueFromWidget() const override;

    bool setWidgetValue( const int &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for double settings with a double spin box
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsDoubleSpinBoxWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>
{
    Q_OBJECT
  public:
    //! Constructor of the factory
    QgsSettingsDoubleSpinBoxWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>( parent ) {}

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsDoubleSpinBoxWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>( editor ) { configureEditor( editor, setting, dynamicKeyPartList ); }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsDoubleSpinBoxWrapper( parent ); }

    QString id() const override;

    bool setSettingFromWidget() const override;

    double valueFromWidget() const override;

    bool setWidgetValue( const double &value ) const override;

    void enableAutomaticUpdatePrivate() override;
};


/**
 * \ingroup gui
 * \brief This class is a factory of editor for color settings with a color button
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsColorButtonWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>
{
    Q_OBJECT
  public:
    //! Constructor of the factory
    QgsSettingsColorButtonWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>( parent ) {}

    //! Constructor of the wrapper for a given \a setting and its widget \a editor
    QgsSettingsColorButtonWrapper( QWidget *editor, const QgsSettingsEntryBase *setting, const QStringList &dynamicKeyPartList = QStringList() )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>( editor ) { configureEditor( editor, setting, dynamicKeyPartList ); }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsColorButtonWrapper( parent ); }

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

#if defined( _MSC_VER )
#ifndef SIP_RUN
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryBool, QCheckBox, bool>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryColor, QgsColorButton, QColor>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryDouble, QDoubleSpinBox, double>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryInteger, QSpinBox, int>;
template class GUI_EXPORT QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryString, QLineEdit, QString>;
#endif
#endif


#endif // QGSSETTINGSEDITORWIDGETWRAPPERIMPL_H
