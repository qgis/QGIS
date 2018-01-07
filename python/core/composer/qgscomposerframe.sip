/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerframe.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerFrame: QgsComposerItem
{
%Docstring
Frame item for a composer multiframe item.
%End

%TypeHeaderCode
#include "qgscomposerframe.h"
%End
  public:
    QgsComposerFrame( QgsComposition *c /TransferThis/, QgsComposerMultiFrame *mf, qreal x, qreal y, qreal width, qreal height );

    void setContentSection( const QRectF &section );
%Docstring
Sets the visible part of the multiframe's content which is visible within
this frame (relative to the total multiframe extent in mm).

:param section: visible portion of content

.. seealso:: :py:func:`extent`
%End

    QgsComposerMultiFrame *multiFrame() const;
%Docstring
Returns the parent multiframe for the frame.

:return: parent multiframe
%End

    virtual QString displayName() const;


    virtual void setSceneRect( const QRectF &rectangle );


    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

    virtual void beginItemCommand( const QString &text );

    virtual void endItemCommand();

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

    virtual int type() const;

    QRectF extent() const;
%Docstring
Returns the visible portion of the multi frame's content which
is shown in this frame.

:return: extent of visible portion

.. versionadded:: 2.5

.. seealso:: :py:func:`setContentSection`
%End

    bool hidePageIfEmpty() const;
%Docstring
Returns whether the page should be hidden (ie, not included in composer exports) if this frame is empty

:return: true if page should be hidden if frame is empty

.. versionadded:: 2.5

.. seealso:: :py:func:`setHidePageIfEmpty`
%End

    void setHidePageIfEmpty( const bool hidePageIfEmpty );
%Docstring
Sets whether the page should be hidden (ie, not included in composer exports) if this frame is empty

:param hidePageIfEmpty: set to true if page should be hidden if frame is empty

.. versionadded:: 2.5

.. seealso:: :py:func:`hidePageIfEmpty`
%End

    bool hideBackgroundIfEmpty() const;
%Docstring
Returns whether the background and frame stroke should be hidden if this frame is empty

:return: true if background and stroke should be hidden if frame is empty

.. versionadded:: 2.5

.. seealso:: :py:func:`setHideBackgroundIfEmpty`
%End

    void setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty );
%Docstring
Sets whether the background and frame stroke should be hidden if this frame is empty

:param hideBackgroundIfEmpty: set to true if background and stroke should be hidden if frame is empty

.. versionadded:: 2.5

.. seealso:: :py:func:`hideBackgroundIfEmpty`
%End

    bool isEmpty() const;
%Docstring
Returns whether the frame is empty

:return: true if frame is empty

.. versionadded:: 2.5

.. seealso:: :py:func:`hidePageIfEmpty`
%End

    virtual QgsExpressionContext createExpressionContext() const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerframe.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
