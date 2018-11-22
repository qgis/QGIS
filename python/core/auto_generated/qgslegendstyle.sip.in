/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslegendstyle.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLegendStyle
{
%Docstring
Composer legend components style
%End

%TypeHeaderCode
#include "qgslegendstyle.h"
%End
  public:
    enum Style
    {
      Undefined,
      Hidden,
      Title,
      Group,
      Subgroup,
      Symbol,
      SymbolLabel
    };

    enum Side
    {
      Top,
      Bottom,
      Left,
      Right
    };

    QgsLegendStyle();

    QFont font() const;
%Docstring
The font for this style.
%End

    void setFont( const QFont &font );
%Docstring
The font for this style.
%End


    double margin( Side side );
    void setMargin( Side side, double margin );

    void setMargin( double margin );
%Docstring
Sets all margins
%End

    void writeXml( const QString &name, QDomElement &elem, QDomDocument &doc ) const;

    void readXml( const QDomElement &elem, const QDomDocument &doc );

    static QString styleName( Style s );
%Docstring
Gets name for style, used in project file
%End

    static Style styleFromName( const QString &styleName );
%Docstring
Gets style from name, used in project file
%End

    static QString styleLabel( Style s );
%Docstring
Gets style label, translated, used in UI
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslegendstyle.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
