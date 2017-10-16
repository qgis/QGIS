/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemgroup.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutItemGroup: QgsLayoutItem
{
%Docstring
 A container for grouping several QgsLayoutItems.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemgroup.h"
%End
  public:

    explicit QgsLayoutItemGroup( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemGroup, belonging to the specified ``layout``.
%End
    ~QgsLayoutItemGroup();

    virtual int type() const;

    virtual QString stringType() const;

    virtual QString displayName() const;


    static QgsLayoutItemGroup *create( QgsLayout *layout ) /Factory/;
%Docstring
 Returns a new group item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemGroup
%End

    void addItem( QgsLayoutItem *item /Transfer/ );
%Docstring
 Adds an ``item`` to the group. Ownership of the item
 is transferred to the group.
%End

    void removeItems();
%Docstring
 Removes all items from the group (but does not delete them).
 Items remain in the scene but are no longer grouped together
%End

    QList<QgsLayoutItem *> items() const;
%Docstring
 Returns a list of items contained by the group.
 :rtype: list of QgsLayoutItem
%End

    virtual void setVisibility( const bool visible );


    virtual void attemptMove( const QgsLayoutPoint &point, bool useReferencePoint = true, bool includesFrame = false );

    virtual void attemptResize( const QgsLayoutSize &size, bool includesFrame = false );


    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );


  protected:
    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual bool writePropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context );


};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemgroup.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
