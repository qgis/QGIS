#include "qgsdesignerwidgets.h"
#include <qstringlist.h>
#include <qimage.h>
#include <qdragobject.h>
#include "qgslegend.h"
#include "qgsmapcanvas.h"
#include "qgslinestylewidget.h"
#include "qgsfillstylewidget.h"
#include "qgspointstylewidget.h"
#include "qgsvectorsymbologywidget.h"
static const char *legend_pixmap[] = {
    "22 22 8 1",
    "  c Gray100",
    ". c Gray97",
    "X c #4f504f",
    "o c #00007f",
    "O c Gray0",
    "+ c none",
    "@ c Gray0",
    "# c Gray0",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "+OOOOOOOOOOOOOOOOOOOO+",
    "OOXXXXXXXXXXXXXXXXXXOO",
    "OXX.                 O",
    "OX.   ooooooooo      O",
    "OX.  o         o    .O",
    "OX   o         o     O",
    "OX   o         o     O",
    "O    o         o     O",
    "OX   o      o  o     O",
    "OX   o       o o     O",
    "OX   o         o     O",
    "OX    ooooooooo  o   O",
    "OO..................OO",
    "+OOOOOOOOOOOOOOOOOOOO+",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++"
};

//shamelessly snarfed from qwt stuff - a very nice simple neat way to 
//accommodate building multiple widgets...Tim
namespace
{
    struct Entry
    {
        Entry() {}
        Entry( QString theClassname, QString theHeader, QString  thePixmap,
                QString theTooltip, QString theWhatsThisString):       
                classname(theClassname),
                header(theHeader),
                pixmap(thePixmap),
                tooltip(theTooltip),
                whatshis(theWhatsThisString)
        {}

        QString classname;
        QString header;
        QString pixmap;
        QString tooltip;
        QString whatshis;
    };

    QValueList<Entry> mEntriesVector;

    const Entry *entry(const QString& theString)
    {
        for ( uint i = 0; i < mEntriesVector.count(); i++ )
        {
            if (theString == mEntriesVector[i].classname)
                return &mEntriesVector[i];
        }
        return NULL;
    }
}

QgsDesignerWidgets::QgsDesignerWidgets()
{
   mEntriesVector.append(Entry("QgsLegend", "qgslegend.h",
        "", "A legend widget that shows layers associated with a mapcanvas.", "A legend widget that shows layers associated with a mapcanvas"));
   mEntriesVector.append(Entry("QgsMapCanvas", "qgsmapcanvas.h", 
        "", "A map canvas widget", "A map canvas is an interactive map that can be panned and zoomed."));
   mEntriesVector.append(Entry("QgsLineStyleWidget", "qgslinestylewidget.h",
        "", "A widget that lets you select a line style.", "A widget that lets you select a line style"));
   mEntriesVector.append(Entry("QgsFillStyleWidget", "qgsfillstylewidget.h",
        "", "A widget that lets you select a fill style.", "A widget that lets you select a fill style"));
   mEntriesVector.append(Entry("QgsPointStyleWidget", "qgspointstylewidget.h",
        "", "A widget that lets you select a point style.", "A widget that lets you select a point style"));
   mEntriesVector.append(Entry("QgsVectorSymbologyWidget", "QgsVectorSymbologyWidget",
        "", "A widget that lets you select vector symbology.", "A widget that lets you select vector symbology"));
}


QWidget* QgsDesignerWidgets::create(const QString &key, 
    QWidget* parent, const char* name)
{
    if ( key == "QgsLegend" )
        return new QgsLegend( parent, name );
    else if ( key == "QgsMapCanvas" )
        return new QgsMapCanvas ( parent, name );
    else if ( key == "QgsLineStyleWidget" )
        return new QgsLineStyleWidget ( parent, name );
    else if ( key == "QgsFillStyleWidget" )
        return new QgsFillStyleWidget ( parent, name );
    else if ( key == "QgsPointStyleWidget" )
        return new QgsPointStyleWidget ( parent, name );
    else if ( key == "QgsVectorSymbologyWidget" )
        return new QgsVectorSymbologyWidget( parent, name );
    return 0;
}


QStringList QgsDesignerWidgets::keys() const
{
    QStringList list;
    
    for (unsigned i = 0; i < mEntriesVector.count(); i++)
        list += mEntriesVector[i].classname;

    return list;
}

QString QgsDesignerWidgets::group( const QString& feature ) const
{
    if (entry(feature) != NULL )
        return QString("QGIS"); 
    return QString::null;
}

QIconSet QgsDesignerWidgets::iconSet( const QString& thePixmap) const
{
    QString pixmapKey("qwtwidget.png");
    if (entry(thePixmap) != NULL )
        pixmapKey = entry(thePixmap)->pixmap;

    const QMimeSource *ms =
        QMimeSourceFactory::defaultFactory()->data(pixmapKey);

    QPixmap pixmap;
    QImageDrag::decode(ms, pixmap);

    return QIconSet(pixmap);
}

QString QgsDesignerWidgets::includeFile( const QString& feature ) const
{
    if (entry(feature) != NULL)
        return entry(feature)->header;        
    return QString::null;
}

QString QgsDesignerWidgets::toolTip( const QString& feature ) const
{
    if (entry(feature) != NULL )
        return entry(feature)->tooltip;       
    return QString::null;
}

QString QgsDesignerWidgets::whatsThis( const QString& feature ) const
{
    if (entry(feature) != NULL)
        return entry(feature)->whatshis;      
    return QString::null;
}

bool QgsDesignerWidgets::isContainer( const QString& ) const
{
    return FALSE;
}

/*
The Q_EXPORT_PLUGIN macro.
    Q_EXPORT_PLUGIN( CustomWidgetPlugin )
    This macro identifies the module as a plugin -- all the other code simply 
    implements the relevant interface, i.e. wraps the classes you wish to make available.
    This macro must appear once in your plugin. It should be copied with the class name 
    changed to the name of your plugin's class. (See the Qt Plugin documentation for more 
    information on the plugin entry point.)
    Each widget you wrap in a widget plugin implementation becomes a class that the 
    plugin implementation offers. There is no limit to the number of classes that you 
    may include in an plugin implementation.
*/
Q_EXPORT_PLUGIN( QgsDesignerWidgets )
