/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscomposerruler.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsComposerRuler: QWidget
{
%Docstring
 A class to show paper scale and the current cursor position
%End

%TypeHeaderCode
#include "qgscomposerruler.h"
%End
  public:
    enum Direction
    {
      Horizontal,
      Vertical
    };

    QgsComposerRuler( QgsComposerRuler::Direction d );
    ~QgsComposerRuler();

    virtual QSize minimumSizeHint() const;


    void setSceneTransform( const QTransform &transform );
    void updateMarker( QPointF pos );

    void setComposition( QgsComposition *c );
    QgsComposition *composition();
%Docstring
 :rtype: QgsComposition
%End

    int rulerSize();
%Docstring
 :rtype: int
%End

  protected:
    virtual void paintEvent( QPaintEvent *event );

    virtual void mouseMoveEvent( QMouseEvent *event );

    virtual void mouseReleaseEvent( QMouseEvent *event );

    virtual void mousePressEvent( QMouseEvent *event );


  signals:
    void cursorPosChanged( QPointF );
%Docstring
Is emitted when mouse cursor coordinates change
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscomposerruler.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
