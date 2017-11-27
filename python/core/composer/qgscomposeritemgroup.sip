/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposeritemgroup.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerItemGroup: QgsComposerItem
{
%Docstring
 A container for grouping several QgsComposerItems
%End

%TypeHeaderCode
#include "qgscomposeritemgroup.h"
%End
  public:
    QgsComposerItemGroup( QgsComposition *c /TransferThis/ );
    ~QgsComposerItemGroup();

    virtual int type() const;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

    virtual void addItem( QgsComposerItem *item );

%Docstring
 Adds an item to the group. All the group members are deleted
if the group is deleted*
%End
    virtual void removeItems();

%Docstring
Removes the items but does not delete them
%End
    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0 );

%Docstring
Draw stroke and ev. selection handles
%End

    virtual void setSceneRect( const QRectF &rectangle );

%Docstring
 Sets this items bound in scene coordinates such that 1 item size units
corresponds to 1 scene size unit*
%End

    virtual void setVisibility( const bool visible );

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
 Stores state in Dom node
 \param elem is Dom element corresponding to 'Composer' tag
 \param doc is the Dom document
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
 Sets state from Dom document
 \param itemElem is Dom node corresponding to item tag
 \param doc is the Dom document
 :rtype: bool
%End

    QSet<QgsComposerItem *> items();
%Docstring
 :rtype: set of QgsComposerItem
%End

  signals:
    void childItemDeleted( QgsComposerItem *item );

  public slots:
    void itemDestroyed();

  protected:
    virtual void drawFrame( QPainter *p );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposeritemgroup.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
