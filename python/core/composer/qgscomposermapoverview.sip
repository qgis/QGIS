/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermapoverview.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerMapOverviewStack : QgsComposerMapItemStack
{
%Docstring
 A collection of overviews which are drawn above the map content in a
QgsComposerMap. The overview stack controls which overviews are drawn and the
order they are drawn in.

.. versionadded:: 2.5

.. seealso:: :py:class:`QgsComposerMapOverview`
%End

%TypeHeaderCode
#include "qgscomposermapoverview.h"
%End
  public:

    QgsComposerMapOverviewStack( QgsComposerMap *map );
%Docstring
Constructor for QgsComposerMapOverviewStack.

:param map: QgsComposerMap the overview stack is attached to
%End

    void addOverview( QgsComposerMapOverview *overview /Transfer/ );
%Docstring
Adds a new map overview to the stack and takes ownership of the overview.
The overview will be added to the end of the stack, and rendered
above any existing map overviews already present in the stack.

:param overview: QgsComposerMapOverview to add to the stack

.. note::

   after adding a overview to the stack, update()
should be called for the QgsComposerMap to prevent rendering artifacts

.. seealso:: :py:func:`removeOverview`
%End

    void removeOverview( const QString &overviewId );
%Docstring
Removes an overview from the stack and deletes the corresponding QgsComposerMapOverview

:param overviewId: id for the QgsComposerMapOverview to remove

.. note::

   after removing an overview from the stack, update()
should be called for the QgsComposerMap to prevent rendering artifacts

.. seealso:: :py:func:`addOverview`
%End

    void moveOverviewUp( const QString &overviewId );
%Docstring
Moves an overview up the stack, causing it to be rendered above other overviews

:param overviewId: id for the QgsComposerMapOverview to move up

.. note::

   after moving an overview within the stack, update() should be
called for the QgsComposerMap to redraw the map with the new overview stack order

.. seealso:: :py:func:`moveOverviewDown`
%End

    void moveOverviewDown( const QString &overviewId );
%Docstring
Moves an overview down the stack, causing it to be rendered below other overviews

:param overviewId: id for the QgsComposerMapOverview to move down

.. note::

   after moving an overview within the stack, update() should be
called for the QgsComposerMap to redraw the map with the new overview stack order

.. seealso:: :py:func:`moveOverviewUp`
%End

    const QgsComposerMapOverview *constOverview( const QString &overviewId ) const;
%Docstring
Returns a const reference to an overview within the stack

:param overviewId: id for the QgsComposerMapOverview to find

:return: const reference to overview, if found

.. seealso:: :py:func:`overview`
%End

    QgsComposerMapOverview *overview( const QString &overviewId ) const;
%Docstring
Returns a reference to an overview within the stack

:param overviewId: id for the QgsComposerMapOverview to find

:return: reference to overview if found

.. seealso:: :py:func:`constOverview`
%End

    QgsComposerMapOverview *overview( const int index ) const;
%Docstring
Returns a reference to an overview within the stack

:param index: overview position in the stack

:return: reference to overview if found

.. seealso:: :py:func:`constOverview`
%End

    QgsComposerMapOverview &operator[]( int idx );

    QList< QgsComposerMapOverview * > asList() const;
%Docstring
Returns a list of QgsComposerMapOverviews contained by the stack

:return: list of overviews
%End

    virtual bool readXml( const QDomElement &elem, const QDomDocument &doc );

%Docstring
Sets the overview stack's state from a DOM document

:param elem: is DOM node corresponding to a 'ComposerMap' tag
:param doc: DOM document

:return: true if read was successful

.. seealso:: :py:func:`writeXml`
%End

};

class QgsComposerMapOverview : QgsComposerMapItem
{
%Docstring
 An individual overview which is drawn above the map content in a
QgsComposerMap, and shows the extent of another QgsComposerMap.

.. versionadded:: 2.5

.. seealso:: :py:class:`QgsComposerMapOverviewStack`
%End

%TypeHeaderCode
#include "qgscomposermapoverview.h"
%End
  public:

    QgsComposerMapOverview( const QString &name, QgsComposerMap *map );
%Docstring
Constructor for QgsComposerMapOverview.

:param name: friendly display name for overview
:param map: QgsComposerMap the overview is attached to
%End

    ~QgsComposerMapOverview();

    virtual void draw( QPainter *painter );

%Docstring
Draws an overview

:param painter: destination QPainter
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores overview state in DOM element

:param elem: is DOM element corresponding to a 'ComposerMap' tag
:param doc: DOM document

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets overview state from a DOM document

:param itemElem: is DOM node corresponding to a 'ComposerMapOverview' tag
:param doc: is DOM document

.. seealso:: :py:func:`writeXml`
%End

    virtual bool usesAdvancedEffects() const;


    void setFrameMap( const int mapId );
%Docstring
Sets overview frame map.

:param mapId: source map id. -1 disables the overview frame

.. seealso:: :py:func:`frameMapId`
%End

    int frameMapId() const;
%Docstring
Returns id of source map.

:return: source map id, or -1 if no source map set
%End

    void setFrameSymbol( QgsFillSymbol *symbol /Transfer/ );
%Docstring
Sets the fill symbol used for drawing the overview extent.

:param symbol: fill symbol for overview

.. seealso:: :py:func:`frameSymbol`
%End

    QgsFillSymbol *frameSymbol();
%Docstring
Gets the fill symbol used for drawing the overview extent.

:return: fill symbol for overview

.. seealso:: :py:func:`setFrameSymbol`
%End


    QPainter::CompositionMode blendMode() const;
%Docstring
Retrieves the blending mode used for drawing the overview.

:return: blending mode for overview

.. seealso:: :py:func:`setBlendMode`
%End

    void setBlendMode( const QPainter::CompositionMode blendMode );
%Docstring
Sets the blending mode used for drawing the overview.

:param blendMode: blending mode for overview

.. seealso:: :py:func:`blendMode`
%End

    bool inverted() const;
%Docstring
Returns whether the overview frame is inverted, ie, whether the shaded area is drawn outside
the extent of the overview map.

:return: true if overview frame is inverted

.. seealso:: :py:func:`setInverted`
%End

    void setInverted( const bool inverted );
%Docstring
Sets whether the overview frame is inverted, ie, whether the shaded area is drawn outside
the extent of the overview map.

:param inverted: set to true if overview frame is to be inverted

.. seealso:: :py:func:`inverted`
%End

    bool centered() const;
%Docstring
Returns whether the extent of the map is forced to center on the overview

:return: true if map will be centered on overview

.. seealso:: :py:func:`setCentered`
%End

    void setCentered( const bool centered );
%Docstring
Sets whether the extent of the map is forced to center on the overview

:param centered: set to true if map will be centered on overview

.. seealso:: :py:func:`centered`
%End

    void connectSignals();
%Docstring
Reconnects signals for overview map, so that overview correctly follows changes to source
map's extent
%End

  public slots:

    void overviewExtentChanged();
%Docstring
Handles recentering of the map and redrawing of the map's overview
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermapoverview.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
