/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposereffect.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerEffect : QGraphicsEffect
{

%TypeHeaderCode
#include "qgscomposereffect.h"
%End
  public:

    QgsComposerEffect();
%Docstring
 Constructor for QgsComposerEffect.
%End

    void setCompositionMode( QPainter::CompositionMode compositionMode );

  protected:
    virtual void draw( QPainter *painter );
%Docstring
Called whenever source needs to be drawn
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposereffect.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
