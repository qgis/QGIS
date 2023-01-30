/***************************************************************************
    qgscodeeditorr.cpp - A R stats editor based on QScintilla
     --------------------------------------
    Date                 : October 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscodeeditorr.h"

#include <QWidget>
#include <QString>
#include <QFont>
#include <Qsci/qscilexerjson.h>


QgsCodeEditorR::QgsCodeEditorR( QWidget *parent, Mode mode )
  : QgsCodeEditor( parent,
                   QString(),
                   false,
                   false,
                   QgsCodeEditor::Flag::CodeFolding,
                   mode )
{
  if ( !parent )
  {
    setTitle( tr( "R Editor" ) );
  }
  QgsCodeEditorR::initializeLexer();
}

Qgis::ScriptLanguage QgsCodeEditorR::language() const
{
  return Qgis::ScriptLanguage::R;
}

void QgsCodeEditorR::initializeLexer()
{
  QgsQsciLexerR *lexer = new QgsQsciLexerR( this );

  QFont font = lexerFont();
  lexer->setDefaultFont( font );
  lexer->setFont( font, -1 );

  font.setItalic( true );
  lexer->setFont( font, QgsQsciLexerR::Comment );

  lexer->setDefaultColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ) );
  lexer->setDefaultPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ) );
  lexer->setPaper( lexerColor( QgsCodeEditorColorScheme::ColorRole::Background ), -1 );

  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Default ), QgsQsciLexerR::Default );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::CommentLine ), QgsQsciLexerR::Comment );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Keyword ), QgsQsciLexerR::Kword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Method ), QgsQsciLexerR::BaseKword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Class ), QgsQsciLexerR::OtherKword );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Number ), QgsQsciLexerR::Number );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QgsQsciLexerR::String );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ), QgsQsciLexerR::String2 );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Operator ), QgsQsciLexerR::Operator );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Identifier ), QgsQsciLexerR::Identifier );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QgsQsciLexerR::Infix );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::UnknownTag ), QgsQsciLexerR::InfixEOL );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Tag ), QgsQsciLexerR::Backticks );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QgsQsciLexerR::RawString );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::SingleQuote ), QgsQsciLexerR::RawString2 );
  lexer->setColor( lexerColor( QgsCodeEditorColorScheme::ColorRole::Decoration ), QgsQsciLexerR::EscapeSequence );

  setLexer( lexer );
  setLineNumbersVisible( true );
  runPostLexerConfigurationTasks();
}

/// @cond PRIVATE
QgsQsciLexerR::QgsQsciLexerR( QObject *parent )
  : QsciLexer( parent )
{

}

const char *QgsQsciLexerR::language() const
{
  return "r";
}

const char *QgsQsciLexerR::lexer() const
{
  return nullptr;
}

int QgsQsciLexerR::lexerId() const
{
  return QsciScintillaBase::SCLEX_R;
}

QString QgsQsciLexerR::description( int style ) const
{
  switch ( style )
  {
    case Default:
      return tr( "Default" );
    case Comment:
      return tr( "Comment" );
    case Kword:
      return tr( "Keyword" );
    case BaseKword:
      return tr( "Base Keyword" );
    case OtherKword:
      return tr( "Other Keyword" );
    case Number:
      return tr( "Number" );
    case String:
      return tr( "String" );
    case String2:
      return tr( "String 2" );
    case Operator:
      return tr( "Operator" );
    case Identifier:
      return tr( "Identifier" );
    case Infix:
      return tr( "Infix" );
    case InfixEOL:
      return tr( "Infix EOL" );
    case Backticks:
      return tr( "Backticks" );
    case RawString:
      return tr( "Raw String" );
    case RawString2:
      return tr( "Raw String 2" );
    case EscapeSequence:
      return tr( "Escape Sequence" );
  }
  return QString();
}

const char *QgsQsciLexerR::keywords( int set ) const
{
  switch ( set )
  {
    case 1:
      return "if else repeat while function for in next break TRUE FALSE NULL NA Inf NaN";

    case 2:
      return "abbreviate abline abs acf acos acosh addmargins aggregate agrep alarm alias alist all anova any aov aperm append apply approx "
             "approxfun apropos ar args arima array arrows asin asinh assign assocplot atan atanh attach attr attributes autoload autoloader "
             "ave axis backsolve barplot basename beta bindtextdomain binomial biplot bitmap bmp body box boxplot bquote break browser builtins "
             "bxp by bzfile c call cancor capabilities casefold cat category cbind ccf ceiling character charmatch chartr chol choose chull "
             "citation class close cm cmdscale codes coef coefficients col colnames colors colorspaces colours comment complex confint "  //#spellok
             "conflicts contour contrasts contributors convolve cophenetic coplot cor cos cosh cov covratio cpgram crossprod cummax cummin "
             "cumprod cumsum curve cut cutree cycle data dataentry date dbeta dbinom dcauchy dchisq de debug debugger decompose delay deltat "
             "demo dendrapply density deparse deriv det detach determinant deviance dexp df dfbeta dfbetas dffits dgamma dgeom dget dhyper "
             "diag diff diffinv difftime digamma dim dimnames dir dirname dist dlnorm dlogis dmultinom dnbinom dnorm dotchart double dpois "
             "dput drop dsignrank dt dump dunif duplicated dweibull dwilcox eapply ecdf edit effects eigen emacs embed end environment eval "
             "evalq example exists exp expression factanal factor factorial family fft fifo file filter find fitted fivenum fix floor flush "
             "for force formals format formula forwardsolve fourfoldplot frame frequency ftable function gamma gaussian gc gcinfo gctorture "
             "get getenv geterrmessage gettext gettextf getwd gl glm globalenv gray grep grey grid gsub gzcon gzfile hat hatvalues hcl "
             "hclust head heatmap help hist history hsv httpclient iconv iconvlist identical identify if ifelse image influence inherits "
             "integer integrate interaction interactive intersect invisible isoreg jitter jpeg julian kappa kernapply kernel kmeans knots "
             "kronecker ksmooth labels lag lapply layout lbeta lchoose lcm legend length letters levels lfactorial lgamma library licence "
             "license line lines list lm load loadhistory loadings local locator loess log logb logical loglin lowess ls lsfit machine mad "
             "mahalanobis makepredictcall manova mapply match matlines matplot matpoints matrix max mean median medpolish menu merge "
             "message methods mget min missing mode monthplot months mosaicplot mtext mvfft names napredict naprint naresid nargs nchar "
             "ncol next nextn ngettext nlevels nlm nls noquote nrow numeric objects offset open optim optimise optimize options order "
             "ordered outer pacf page pairlist pairs palette par parse paste pbeta pbinom pbirthday pcauchy pchisq pdf pentagamma person "
             "persp pexp pf pgamma pgeom phyper pi pico pictex pie piechart pipe plclust plnorm plogis plot pmatch pmax pmin pnbinom png "
             "pnorm points poisson poly polygon polym polyroot postscript power ppoints ppois ppr prcomp predict preplot pretty princomp "
             "print prmatrix prod profile profiler proj promax prompt provide psigamma psignrank pt ptukey punif pweibull pwilcox q qbeta "
             "qbinom qbirthday qcauchy qchisq qexp qf qgamma qgeom qhyper qlnorm qlogis qnbinom qnorm qpois qqline qqnorm qqplot qr "
             "qsignrank qt qtukey quantile quarters quasi quasibinomial quasipoisson quit qunif quote qweibull qwilcox rainbow range "
             "rank raw rbeta rbind rbinom rcauchy rchisq readline real recover rect reformulate regexpr relevel remove reorder rep repeat "
             "replace replicate replications require reshape resid residuals restart return rev rexp rf rgamma rgb rgeom rhyper rle rlnorm "
             "rlogis rm rmultinom rnbinom rnorm round row rownames rowsum rpois rsignrank rstandard rstudent rt rug runif runmed rweibull "
             "rwilcox sample sapply save savehistory scale scan screen screeplot sd search searchpaths seek segments seq sequence serialize "
             "setdiff setequal setwd shell sign signif sin single sinh sink smooth solve sort source spectrum spline splinefun split sprintf "
             "sqrt stack stars start stderr stdin stdout stem step stepfun stl stop stopifnot str strftime strheight stripchart strptime "
             "strsplit strtrim structure strwidth strwrap sub subset substitute substr substring sum summary sunflowerplot supsmu svd sweep "
             "switch symbols symnum system t table tabulate tail tan tanh tapply tempdir tempfile termplot terms tetragamma text time title "
             "toeplitz tolower topenv toupper trace traceback transform trigamma trunc truncate try ts tsdiag tsp typeof unclass undebug "
             "union unique uniroot unix unlink unlist unname unserialize unsplit unstack untrace unz update upgrade url var varimax vcov "
             "vector version vi vignette warning warnings weekdays weights which while window windows with write wsbrowser xedit xemacs "
             "xfig xinch xor xtabs xyinch yinch zapsmall";

    case 3:
      return "acme aids aircondit amis aml banking barchart barley beaver bigcity boot brambles breslow bs bwplot calcium cane capability "
             "cav censboot channing city claridge cloth cloud coal condense contourplot control corr darwin densityplot dogs dotplot ducks "
             "empinf envelope environmental ethanol fir frets gpar grav gravity grob hirose histogram islay knn larrows levelplot llines "
             "logit lpoints lsegments lset ltext lvqinit lvqtest manaus melanoma melanoma motor multiedit neuro nitrofen nodal ns nuclear "
             "oneway parallel paulsen poisons polar qq qqmath remission rfs saddle salinity shingle simplex singer somgrid splom stripplot "
             "survival tau tmd tsboot tuna unit urine viewport wireframe wool xyplot";
  }

  return nullptr;
}

///@endcond
