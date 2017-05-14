class QgsOptionalExpression
{
%Docstring

 An expression with an additional enabled flag.

 This can be used for configuration options where an expression can be enabled
 or disabled but when disabled it shouldn't lose it's information for the case
 it gets re-enabled later on and a user shouldn't be force to redo the configuration.

.. versionadded:: 2.18
%End

%TypeHeaderCode
#include "qgsoptionalexpression.h"
%End
  public:

    QgsOptionalExpression();
%Docstring
 Construct a default optional expression.
 It will be disabled and with an empty expression.
%End

    QgsOptionalExpression( const QgsExpression &expression );
%Docstring
 Construct an optional expression with the provided expression.
 It will be enabled.
%End

    QgsOptionalExpression( const QgsExpression &expression, bool enabled );
%Docstring
 Construct an optional expression with the provided expression and enabled state.
%End


    /**
     * Compare this QgsOptionalExpression to another one.
     *
     * This will compare the enabled flag and call the == operator
     * of the contained class.
     *
     * @note Added in QGIS 3.0
     */
    int operator== ( const QgsOptionalExpression &other ) const;
%MethodCode
      sipRes = *sipCpp == *a0;
%End        %End

    int __bool__() const;      +    QgsOptionalExpression( const QgsExpression &expression, bool enabled );
%MethodCode        +%Docstring
      sipRes = sipCpp->enabled();      + Construct an optional expression with the provided expression and enabled state.
%End        %End

    /**
     * Check if this optional is enabled
     *
     * @note Added in QGIS 3.0
     */
    bool enabled() const;

    /**
     * Set if this optional is enabled
     *
     * @note Added in QGIS 3.0
     */
    void setEnabled( bool enabled );

    /**
     * Access the payload data
     *
     * @note Added in QGIS 3.0
     */
    QgsExpression data() const;

    /**
     * Set the payload data
     *
     * @note Added in QGIS 3.0
     */
    void setData( const QgsExpression &data );

    void writeXml( QDomElement &element );
%Docstring
 Save the optional expression to the provided QDomElement.

 The caller is responsible to pass an empty QDomElement and to append it to
 a parent element.

.. versionadded:: 2.18
%End

    void readXml( const QDomElement &element );
%Docstring
 Read the optional expression from the provided QDomElement.

.. versionadded:: 2.18
%End
};
