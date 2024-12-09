/***************************************************************************
    qgscodeeditorcolorscheme.h
     --------------------------------------
    Date                 : October 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCODEEDITORCOLORSCHEME_H
#define QGSCODEEDITORCOLORSCHEME_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QString>
#include <QMap>
#include <QColor>

/**
 * \ingroup gui
 * \brief Defines a color scheme for use in QgsCodeEditor widgets.
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsCodeEditorColorScheme
{
  public:
    /**
     * Color roles.
     */
    enum class ColorRole
    {
      Default,                //!< Default text color
      Keyword,                //!< Keyword color
      Class,                  //!< Class color
      Method,                 //!< Method color
      Decoration,             //!< Decoration color
      Number,                 //!< Number color
      Comment,                //!< Comment color
      CommentLine,            //!< Line comment color
      CommentBlock,           //!< Comment block color
      Background,             //!< Background color
      Cursor,                 //!< Cursor color
      CaretLine,              //!< Caret line color
      SingleQuote,            //!< Single quote color
      DoubleQuote,            //!< Double quote color
      TripleSingleQuote,      //!< Triple single quote color
      TripleDoubleQuote,      //!< Triple double quote color
      Operator,               //!< Operator color
      QuotedOperator,         //!< Quoted operator color
      Identifier,             //!< Identifier color
      QuotedIdentifier,       //!< Quoted identifier color
      Tag,                    //!< Tag color
      UnknownTag,             //!< Unknown tag
      MarginBackground,       //!< Margin background color
      MarginForeground,       //!< Margin foreground color
      SelectionBackground,    //!< Selection background color
      SelectionForeground,    //!< Selection foreground color
      MatchedBraceBackground, //!< Matched brace background color
      MatchedBraceForeground, //!< Matched brace foreground color
      Edge,                   //!< Edge color
      Fold,                   //!< Fold color
      Error,                  //!< Error color
      ErrorBackground,        //!< Error background color
      FoldIconForeground,     //!< Fold icon foreground color
      FoldIconHalo,           //!< Fold icon halo color
      IndentationGuide,       //!< Indentation guide line
      SearchMatchBackground,  //!< Background color for search matches \since QGIS 3.38
    };

    /**
     * Constructor for QgsCodeEditorColorScheme.
     *
     * The \a id argument must be set to a unique, non-translated identifier for the color scheme.
     *
     * The \a name argument must be set to a translated, user-visible descriptive name of the scheme.
     */
    QgsCodeEditorColorScheme( const QString &id = QString(), const QString &name = QString() );


    /**
     * Returns the ID of the color scheme, which is a unique, non-translated identifier for the scheme.
     */
    QString id() const { return mId; }

    /**
     * Returns the name of the color scheme, which is the translated, user-visible name of the scheme.
     */
    QString name() const { return mThemeName; }

    /**
     * Returns the color to use in the editor for the specified \a role.
     *
     * \see setColor()
     */
    QColor color( ColorRole role ) const;

    /**
     * Sets the \a color to use in the editor for the specified \a role.
     *
     * \see color()
     */
    void setColor( ColorRole role, const QColor &color );

    /**
     * Sets all \a colors for the scheme.
     *
     * \note Not available in Python bindings.
     */
    void setColors( const QMap<ColorRole, QColor> &colors ) SIP_SKIP;

  private:
    QString mId;
    QString mThemeName;
    QMap<ColorRole, QColor> mColors;
};

#endif // QGSCODEEDITORCOLORSCHEME_H
