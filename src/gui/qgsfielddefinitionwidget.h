/***************************************************************************
   qgsfielddefinitionwidget.h
    --------------------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSFIELDDEFINITIONWIDGET_H
#define QGSFIELDDEFINITIONWIDGET_H

#define SIP_NO_FILE

#include "ui_qgsfielddefinitionwidget.h"
#include "qgsguiutils.h"
#include "qgsapplication.h"

#include "qgis_gui.h"
#include "qgsfield.h"

#include <QRegExpValidator>
#include <QRegExp>

class QgsVectorLayer;

/**
* \ingroup gui
* A widget for setting up the definition of a new field in a vector layer. 
* \note unstable API (will likely change)
* \since QGIS 3.16
*/
class GUI_EXPORT QgsFieldDefinitionWidget : public QWidget, private Ui::QgsFieldDefinitionWidgetBase
{
    Q_OBJECT

  public:

    /**
     * List of advanced fields to be shown. If `Neither` is chosen, the whole advanced fields UI is hidden.
     */
    enum class AdvancedField
    {
      Comment = 1 << 1,
      Alias = 1 << 2,
      IsNullable = 1 << 3,
      All = Comment | Alias | IsNullable,
    };

    Q_DECLARE_FLAGS( AdvancedFields, AdvancedField )
    Q_FLAG( AdvancedFields )

    //! Constructor
    QgsFieldDefinitionWidget( AdvancedFields advancedFields = AdvancedFields(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Adds string field type to given field form with default settings
    static void addStringType( QgsFieldDefinitionWidget *fieldDefinitionWidget )
    {
      fieldDefinitionWidget->addType( QStringLiteral( "String" ), tr( "Text data" ), QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) ) );
    }

    //! Adds integer field type to given field form with default settings
    static void addIntegerType( QgsFieldDefinitionWidget *fieldDefinitionWidget )
    {
      fieldDefinitionWidget->addType( QStringLiteral( "Integer" ), tr( "Whole number (integer)" ), QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) ) );
    }

    //! Adds real field type to given field form with default settings
    static void addRealType( QgsFieldDefinitionWidget *fieldDefinitionWidget )
    {
      fieldDefinitionWidget->addType( QStringLiteral( "Real" ), tr( "Decimal number (real)" ), QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldFloat.svg" ) ), 15, 15 );
    }

    //! Adds date field type to given field form with default settings
    static void addDateType( QgsFieldDefinitionWidget *fieldDefinitionWidget )
    {
      fieldDefinitionWidget->addType( QStringLiteral( "Date" ), tr( "Date" ), QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDate.svg" ) ) );
    }

    /**
     * Returns the name of the field.
     */
    QString name() const;

    /**
     * Sets the name of the field.
     */
    void setName( const QString &name );

    /**
     * Returns the string name of the field type.
     */
    QString type() const;

    /**
     * Sets the string name of the field type. If no such type is present, does nothing.
     */
    bool setType( const QString &typeName );

    /**
     * Returns the field comment.
     */
    QString comment() const;

    /**
     * Sets the field comment.
     */
    void setComment( const QString &comment );

    /**
     * Returns whether the field is marked as nullable.
     */
    bool isNullable();

    /**
     * Sets whether the field can be nullable.
     */
    void setIsNullable( bool isNullable );

    /**
     * Returns the field alias.
     */
    QString alias() const;

    /**
     * Sets the field alias.
     */
    void setAlias( const QString &alias );

    /**
     * Returns the field length.
     */
    int length() const;

    /**
     * Sets the field length
     */
    void setLength( const int length );

    /**
     * Returns the field precision.
     */
    int precision() const;

    /**
     * Returns the field precision.
     */
    void setPrecision( const int precision );

    /**
     * Returns a list of string type names.
     */
    QStringList types() const;

    /**
     * Adds a new type \a typeName in the end of the types combobox. If \a length or \a precision are given as negative number, their UI fields are disabled for that type.
     */
    bool addType( const QString &typeName, const QString &typeDisplay, const QIcon &icon = QIcon(), int length = -1, int precision = -1 );

    /**
     * Inserts a new type \a typeName at given \a position in the types combobox. If \a length or \a precision are given as negative number, their UI fields are disabled for that type.
     */
    bool insertType( const int position, const QString &typeName, const QString &typeDisplay, const QIcon &icon = QIcon(), int length = -1, int precision = -1 );

    /**
     * Removes a type identified by its string \a typename from the list of available field data types.
     */
    bool removeType( const QString &typeName );

    /**
     * Checks whether the \a typeName is present as a field data type.
     */
    bool hasType( const QString &typeName ) const;

    /**
     * Returns the index of the given \a typeName in the list of available data types.
     */
    int typeIndex( const QString &typeName ) const;

    /**
     * Returns the current field name validator regular expresiion.
     */
    QRegExp nameRegExp() const;

    /**
     * Sets field name validator regular expresiion.
     */
    void setNameRegExp( QRegExp &nameRegExp );

    /**
     * Returns whether the field form is valid.
     */
    bool isValidForm() const;

    /**
     * Returns the current state of the form as a QgsField.
     */
    QgsField *asField() const;

    /**
     * Sets the current layer of the form. The layer is as a source to select existing field.
     */
    void setLayer( QgsVectorLayer *vl );

    /**
     * Returns the current layer.
     */
    QgsVectorLayer *layer() const;

    /**
     * Sets the current layer field.
     */
    void setLayerField( const QString &fieldName );

    /**
     * Returns the current existing layer field name.
     */
    QString layerField() const;

    /**
     * Checks whether the "use existing field" is active.
     */
    bool shouldUseExistingField() const;

    /**
     * Sets whether the "use exiting field" is active.
     */
    void setShouldUseExistingField( bool shouldUseExistingField );

  signals:

    /**
     * When any of the field form inputs is changed.
     */
    void changed();

    /**
     * When any of the text form inputs is has return button pressed.
     */
    void returnPressed();

  private slots:
    void mTypeCmb_currentIndexChanged( const int index );
    void mLengthSpinBox_valueChanged( const int value );

  private:
    QRegExpValidator *mRegExpValidator = new QRegExpValidator( QRegExp( QStringLiteral( "^(?!(test|def)$)[a-zA-Z_][a-zA-Z0-9_]{0,32}$" ) ), this );
};

#endif // QGSFIELDDEFINITIONWIDGET_H
