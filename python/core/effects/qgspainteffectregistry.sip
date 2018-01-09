/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgspainteffectregistry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsPaintEffectWidget /External/;

class QgsPaintEffectAbstractMetadata
{
%Docstring
 Stores metadata about a paint effect class.

.. note::

   It's necessary to implement the createPaintEffect() function.
In C++ you can use the QgsPaintEffectMetadata convenience class to
simplify creation of the metadata.

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgspainteffectregistry.h"
%End
  public:

    QgsPaintEffectAbstractMetadata( const QString &name, const QString &visibleName );
%Docstring
Construct a new QgsPaintEffectAbstractMetadata

:param name: unique string representing paint effect class
:param visibleName: user visible name representing paint effect class
%End

    virtual ~QgsPaintEffectAbstractMetadata();

    QString name() const;
%Docstring
Returns the unique string representing the paint effect class

:return: unique string

.. seealso:: :py:func:`visibleName`
%End

    QString visibleName() const;
%Docstring
Returns the user visible string representing the paint effect class

:return: friendly user visible string

.. seealso:: :py:func:`name`
%End

    virtual QgsPaintEffect *createPaintEffect( const QgsStringMap &map ) = 0 /Factory/;
%Docstring
Create a paint effect of this class given an encoded map of properties.

:param map: properties string map

:return: new paint effect
%End

    virtual QgsPaintEffectWidget *createWidget() /Factory/;
%Docstring
Create configuration widget for paint effect of this class. Can return None
if there's no GUI for the paint effect class.

:return: configuration widget
%End

  protected:

};




class QgsPaintEffectRegistry
{
%Docstring
 Registry of available paint effects.

QgsPaintEffectRegistry is not usually directly created, but rather accessed through
:py:func:`QgsApplication.paintEffectRegistry()`

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgspainteffectregistry.h"
%End
  public:

    QgsPaintEffectRegistry();
    ~QgsPaintEffectRegistry();


    QgsPaintEffectAbstractMetadata *effectMetadata( const QString &name ) const;
%Docstring
Returns the metadata for a specific effect.

:param name: unique string name for paint effect class

:return: paint effect metadata if found, otherwise None
%End

    bool addEffectType( QgsPaintEffectAbstractMetadata *metadata /Transfer/ );
%Docstring
Registers a new effect type.

:param metadata: effect metadata. Ownership is transferred to the registry.

:return: true if add was successful.
%End

    QgsPaintEffect *createEffect( const QString &name, const QgsStringMap &properties = QgsStringMap() ) const /Factory/;
%Docstring
Creates a new paint effect given the effect name and properties map.

:param name: unique name representing paint effect class
:param properties: encoded string map of effect properties

:return: new paint effect of specified class, or None if matching
paint effect could not be created
%End

    QgsPaintEffect *createEffect( const QDomElement &element ) const /Factory/;
%Docstring
Creates a new paint effect given a DOM element storing paint effect
properties.

:param element: encoded DOM element of effect properties

:return: new paint effect, or None if matching
paint effect could not be created
%End

    QStringList effects() const;
%Docstring
Returns a list of known paint effects.

:return: list of paint effect names
%End

    static QgsPaintEffect *defaultStack() /Factory/;
%Docstring
Returns a new effect stack consisting of a sensible selection of default
effects. All effects except the standard draw source effect are disabled,
but are included so that they can be easily drawn just by enabling the effect.

:return: default effects stack

.. seealso:: :py:func:`isDefaultStack`
%End

    static bool isDefaultStack( QgsPaintEffect *effect );
%Docstring
Tests whether a paint effect matches the default effects stack.

:param effect: paint effect to test

:return: true if effect is default stack

.. versionadded:: 2.12

.. seealso:: :py:func:`defaultStack`
%End

  private:
    QgsPaintEffectRegistry( const QgsPaintEffectRegistry &rh );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/effects/qgspainteffectregistry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
