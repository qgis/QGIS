/*
    This source file is part of Konsole, a terminal emulator.

    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "ColorScheme.h"
#include "tools.h"

// Qt
#include <QBrush>
#include <QFile>
#include <QFileInfo>
#include <QtDebug>
#include <QSettings>
#include <QDir>
#include <QRegularExpression>
#include <QRandomGenerator>


// KDE
//#include <KColorScheme>
//#include <KConfig>
//#include <KLocale>
//#include <KDebug>
//#include <KConfigGroup>
//#include <KStandardDirs>

using namespace Konsole;

const ColorEntry ColorScheme::defaultTable[TABLE_COLORS] =
 // The following are almost IBM standard color codes, with some slight
 // gamma correction for the dim colors to compensate for bright X screens.
 // It contains the 8 ansiterm/xterm colors in 2 intensities.
{
    ColorEntry( QColor(0x00,0x00,0x00), false), ColorEntry(
QColor(0xFF,0xFF,0xFF), true), // Dfore, Dback
    ColorEntry( QColor(0x00,0x00,0x00), false), ColorEntry(
QColor(0xB2,0x18,0x18), false), // Black, Red
    ColorEntry( QColor(0x18,0xB2,0x18), false), ColorEntry(
QColor(0xB2,0x68,0x18), false), // Green, Yellow
    ColorEntry( QColor(0x18,0x18,0xB2), false), ColorEntry(
QColor(0xB2,0x18,0xB2), false), // Blue, Magenta
    ColorEntry( QColor(0x18,0xB2,0xB2), false), ColorEntry(
QColor(0xB2,0xB2,0xB2), false), // Cyan, White
    // intensive
    ColorEntry( QColor(0x00,0x00,0x00), false), ColorEntry(
QColor(0xFF,0xFF,0xFF), true),
    ColorEntry( QColor(0x68,0x68,0x68), false), ColorEntry(
QColor(0xFF,0x54,0x54), false),
    ColorEntry( QColor(0x54,0xFF,0x54), false), ColorEntry(
QColor(0xFF,0xFF,0x54), false),
    ColorEntry( QColor(0x54,0x54,0xFF), false), ColorEntry(
QColor(0xFF,0x54,0xFF), false),
    ColorEntry( QColor(0x54,0xFF,0xFF), false), ColorEntry(
QColor(0xFF,0xFF,0xFF), false)
};

const char* const ColorScheme::colorNames[TABLE_COLORS] =
{
  "Foreground",
  "Background",
  "Color0",
  "Color1",
  "Color2",
  "Color3",
  "Color4",
  "Color5",
  "Color6",
  "Color7",
  "ForegroundIntense",
  "BackgroundIntense",
  "Color0Intense",
  "Color1Intense",
  "Color2Intense",
  "Color3Intense",
  "Color4Intense",
  "Color5Intense",
  "Color6Intense",
  "Color7Intense"
};
// dummy silently comment out the tr_NOOP
#define tr_NOOP
const char* const ColorScheme::translatedColorNames[TABLE_COLORS] =
{
    tr_NOOP("Foreground"),
    tr_NOOP("Background"),
    tr_NOOP("Color 1"),
    tr_NOOP("Color 2"),
    tr_NOOP("Color 3"),
    tr_NOOP("Color 4"),
    tr_NOOP("Color 5"),
    tr_NOOP("Color 6"),
    tr_NOOP("Color 7"),
    tr_NOOP("Color 8"),
    tr_NOOP("Foreground (Intense)"),
    tr_NOOP("Background (Intense)"),
    tr_NOOP("Color 1 (Intense)"),
    tr_NOOP("Color 2 (Intense)"),
    tr_NOOP("Color 3 (Intense)"),
    tr_NOOP("Color 4 (Intense)"),
    tr_NOOP("Color 5 (Intense)"),
    tr_NOOP("Color 6 (Intense)"),
    tr_NOOP("Color 7 (Intense)"),
    tr_NOOP("Color 8 (Intense)")
};

ColorScheme::ColorScheme()
{
    _table = nullptr;
    _randomTable = nullptr;
    _opacity = 1.0;
}
ColorScheme::ColorScheme(const ColorScheme& other)
      : _opacity(other._opacity)
       ,_table(nullptr)
       ,_randomTable(nullptr)
{
    setName(other.name());
    setDescription(other.description());

    if ( other._table != nullptr )
    {
        for ( int i = 0 ; i < TABLE_COLORS ; i++ )
            setColorTableEntry(i,other._table[i]);
    }

    if ( other._randomTable != nullptr )
    {
        for ( int i = 0 ; i < TABLE_COLORS ; i++ )
        {
            const RandomizationRange& range = other._randomTable[i];
            setRandomizationRange(i,range.hue,range.saturation,range.value);
        }
    }
}
ColorScheme::~ColorScheme()
{
    delete[] _table;
    delete[] _randomTable;
}

void ColorScheme::setDescription(const QString& description) { _description = description; }
QString ColorScheme::description() const { return _description; }

void ColorScheme::setName(const QString& name) { _name = name; }
QString ColorScheme::name() const { return _name; }

void ColorScheme::setColorTableEntry(int index , const ColorEntry& entry)
{
    Q_ASSERT( index >= 0 && index < TABLE_COLORS );

    if ( !_table )
    {
        _table = new ColorEntry[TABLE_COLORS];

        for (int i=0;i<TABLE_COLORS;i++)
            _table[i] = defaultTable[i];
    }

    _table[index] = entry;
}
ColorEntry ColorScheme::colorEntry(int index) const
{
    Q_ASSERT( index >= 0 && index < TABLE_COLORS );

    ColorEntry entry = colorTable()[index];

    if ( _randomTable != nullptr &&
        !_randomTable[index].isNull() )
    {
        const RandomizationRange& range = _randomTable[index];


        int hueDifference = range.hue ? QRandomGenerator::global()->bounded(range.hue) - range.hue/2 : 0;
        int saturationDifference = range.saturation ? QRandomGenerator::global()->bounded(range.saturation) - range.saturation/2 : 0;
        int valueDifference = range.value ? QRandomGenerator::global()->bounded(range.value) - range.value/2 : 0;

        QColor& color = entry.color;

        int newHue = qAbs( (color.hue() + hueDifference) % MAX_HUE );
        int newValue = qMin( qAbs(color.value() + valueDifference) , 255 );
        int newSaturation = qMin( qAbs(color.saturation() + saturationDifference) , 255 );

        color.setHsv(newHue,newSaturation,newValue);
    }

    return entry;
}
void ColorScheme::getColorTable(ColorEntry* table) const
{
    for ( int i = 0 ; i < TABLE_COLORS ; i++ )
        table[i] = colorEntry(i);
}
bool ColorScheme::randomizedBackgroundColor() const
{
    return _randomTable == nullptr ? false : !_randomTable[1].isNull();
}
void ColorScheme::setRandomizedBackgroundColor(bool randomize)
{
    // the hue of the background colour is allowed to be randomly
    // adjusted as much as possible.
    //
    // the value and saturation are left alone to maintain read-ability
    if ( randomize )
    {
        setRandomizationRange( 1 /* background color index */ , MAX_HUE , 255 , 0 );
    }
    else
    {
        if ( _randomTable )
            setRandomizationRange( 1 /* background color index */ , 0 , 0 , 0 );
    }
}

void ColorScheme::setRandomizationRange( int index , quint16 hue , quint8 saturation ,
                                         quint8 value )
{
    Q_ASSERT( hue <= MAX_HUE );
    Q_ASSERT( index >= 0 && index < TABLE_COLORS );

    if ( _randomTable == nullptr )
        _randomTable = new RandomizationRange[TABLE_COLORS];

    _randomTable[index].hue = hue;
    _randomTable[index].value = value;
    _randomTable[index].saturation = saturation;
}

const ColorEntry* ColorScheme::colorTable() const
{
    if ( _table )
        return _table;
    else
        return defaultTable;
}
QColor ColorScheme::foregroundColor() const
{
    return colorTable()[0].color;
}
QColor ColorScheme::backgroundColor() const
{
    return colorTable()[1].color;
}
bool ColorScheme::hasDarkBackground() const
{
    // value can range from 0 - 255, with larger values indicating higher brightness.
    // so 127 is in the middle, anything less is deemed 'dark'
    return backgroundColor().value() < 127;
}
void ColorScheme::setOpacity(qreal opacity) { _opacity = opacity; }
qreal ColorScheme::opacity() const { return _opacity; }

void ColorScheme::read(const QString & fileName)
{
    QSettings s(fileName, QSettings::IniFormat);
    s.beginGroup(QLatin1String("General"));

    _description = s.value(QLatin1String("Description"), QObject::tr("Un-named Color Scheme")).toString();
    _opacity = s.value(QLatin1String("Opacity"),qreal(1.0)).toDouble();
    s.endGroup();

    for (int i=0 ; i < TABLE_COLORS ; i++)
    {
        readColorEntry(&s, i);
    }
}
#if 0
// implemented upstream - user apps
void ColorScheme::read(KConfig& config)
{
    KConfigGroup configGroup = config.group("General");

    QString description = configGroup.readEntry("Description", QObject::tr("Un-named Color Scheme"));

    _description = tr(description.toUtf8());
    _opacity = configGroup.readEntry("Opacity",qreal(1.0));

    for (int i=0 ; i < TABLE_COLORS ; i++)
    {
        readColorEntry(config,i);
    }
}
void ColorScheme::write(KConfig& config) const
{
    KConfigGroup configGroup = config.group("General");

    configGroup.writeEntry("Description",_description);
    configGroup.writeEntry("Opacity",_opacity);

    for (int i=0 ; i < TABLE_COLORS ; i++)
    {
        RandomizationRange random = _randomTable != 0 ? _randomTable[i] : RandomizationRange();
        writeColorEntry(config,colorNameForIndex(i),colorTable()[i],random);
    }
}
#endif

QString ColorScheme::colorNameForIndex(int index)
{
    Q_ASSERT( index >= 0 && index < TABLE_COLORS );

    return QString::fromLatin1(colorNames[index]);
}
QString ColorScheme::translatedColorNameForIndex(int index)
{
    Q_ASSERT( index >= 0 && index < TABLE_COLORS );

    return QString::fromLatin1(translatedColorNames[index]);
}

void ColorScheme::readColorEntry(QSettings * s , int index)
{
    QString colorName = colorNameForIndex(index);

    s->beginGroup(colorName);

    ColorEntry entry;

    QVariant colorValue = s->value(QLatin1String("Color"));
    QString colorStr;
    int r, g, b;
    bool ok = false;
    // XXX: Undocumented(?) QSettings behavior: values with commas are parsed
    // as QStringList and others QString
    if (colorValue.type() == QVariant::StringList)
    {
        QStringList rgbList = colorValue.toStringList();
        colorStr = rgbList.join(QLatin1Char(','));
        if (rgbList.count() == 3)
        {
            bool parse_ok;

            ok = true;
            r = rgbList[0].toInt(&parse_ok);
            ok = ok && parse_ok && (r >= 0 && r <= 0xff);
            g = rgbList[1].toInt(&parse_ok);
            ok = ok && parse_ok && (g >= 0 && g <= 0xff);
            b = rgbList[2].toInt(&parse_ok);
            ok = ok && parse_ok && (b >= 0 && b <= 0xff);
        }
    }
    else
    {
        colorStr = colorValue.toString();
        QRegularExpression hexColorPattern(QLatin1String("^#[0-9a-f]{6}$"),
                                           QRegularExpression::CaseInsensitiveOption);
        if (hexColorPattern.match(colorStr).hasMatch())
        {
            // Parsing is always ok as already matched by the regexp
            r = colorStr.midRef(1, 2).toInt(nullptr, 16);
            g = colorStr.midRef(3, 2).toInt(nullptr, 16);
            b = colorStr.midRef(5, 2).toInt(nullptr, 16);
            ok = true;
        }
    }
    if (!ok)
    {
        qWarning().nospace() << "Invalid color value " << colorStr
                             << " for " << colorName << ". Fallback to black.";
        r = g = b = 0;
    }
    entry.color = QColor(r, g, b);

    entry.transparent = s->value(QLatin1String("Transparent"),false).toBool();

    // Deprecated key from KDE 4.0 which set 'Bold' to true to force
    // a color to be bold or false to use the current format
    //
    // TODO - Add a new tri-state key which allows for bold, normal or
    // current format
    if (s->contains(QLatin1String("Bold")))
        entry.fontWeight = s->value(QLatin1String("Bold"),false).toBool() ? ColorEntry::Bold :
                                                                 ColorEntry::UseCurrentFormat;

    quint16 hue = s->value(QLatin1String("MaxRandomHue"),0).toInt();
    quint8 value = s->value(QLatin1String("MaxRandomValue"),0).toInt();
    quint8 saturation = s->value(QLatin1String("MaxRandomSaturation"),0).toInt();

    setColorTableEntry( index , entry );

    if ( hue != 0 || value != 0 || saturation != 0 )
       setRandomizationRange( index , hue , saturation , value );

    s->endGroup();
}
#if 0
// implemented upstream - user apps
void ColorScheme::writeColorEntry(KConfig& config , const QString& colorName, const ColorEntry& entry , const RandomizationRange& random) const
{
    KConfigGroup configGroup(&config,colorName);

    configGroup.writeEntry("Color",entry.color);
    configGroup.writeEntry("Transparency",(bool)entry.transparent);
    if (entry.fontWeight != ColorEntry::UseCurrentFormat)
    {
        configGroup.writeEntry("Bold",entry.fontWeight == ColorEntry::Bold);
    }

    // record randomization if this color has randomization or
    // if one of the keys already exists
    if ( !random.isNull() || configGroup.hasKey("MaxRandomHue") )
    {
        configGroup.writeEntry("MaxRandomHue",static_cast<int>(random.hue));
        configGroup.writeEntry("MaxRandomValue",static_cast<int>(random.value));
        configGroup.writeEntry("MaxRandomSaturation",static_cast<int>(random.saturation));
    }
}
#endif

//
// Work In Progress - A color scheme for use on KDE setups for users
// with visual disabilities which means that they may have trouble
// reading text with the supplied color schemes.
//
// This color scheme uses only the 'safe' colors defined by the
// KColorScheme class.
//
// A complication this introduces is that each color provided by
// KColorScheme is defined as a 'background' or 'foreground' color.
// Only foreground colors are allowed to be used to render text and
// only background colors are allowed to be used for backgrounds.
//
// The ColorEntry and TerminalDisplay classes do not currently
// support this restriction.
//
// Requirements:
//  - A color scheme which uses only colors from the KColorScheme class
//  - Ability to restrict which colors the TerminalDisplay widget
//    uses as foreground and background color
//  - Make use of KGlobalSettings::allowDefaultBackgroundImages() as
//    a hint to determine whether this accessible color scheme should
//    be used by default.
//
//
// -- Robert Knight <robertknight@gmail.com> 21/07/2007
//
AccessibleColorScheme::AccessibleColorScheme()
    : ColorScheme()
{
#if 0
// It's not finished in konsole and it breaks Qt4 compilation as well
    // basic attributes
    setName("accessible");
    setDescription(QObject::tr("Accessible Color Scheme"));

    // setup colors
    const int ColorRoleCount = 8;

    const KColorScheme colorScheme(QPalette::Active);

    QBrush colors[ColorRoleCount] =
    {
        colorScheme.foreground( colorScheme.NormalText ),
        colorScheme.background( colorScheme.NormalBackground ),

        colorScheme.foreground( colorScheme.InactiveText ),
        colorScheme.foreground( colorScheme.ActiveText ),
        colorScheme.foreground( colorScheme.LinkText ),
        colorScheme.foreground( colorScheme.VisitedText ),
        colorScheme.foreground( colorScheme.NegativeText ),
        colorScheme.foreground( colorScheme.NeutralText )
    };

    for ( int i = 0 ; i < TABLE_COLORS ; i++ )
    {
        ColorEntry entry;
        entry.color = colors[ i % ColorRoleCount ].color();

        setColorTableEntry( i , entry );
    }
#endif
}

ColorSchemeManager::ColorSchemeManager()
    : _haveLoadedAll(false)
{
}
ColorSchemeManager::~ColorSchemeManager()
{
    QHashIterator<QString,const ColorScheme*> iter(_colorSchemes);
    while (iter.hasNext())
    {
        iter.next();
        delete iter.value();
    }
}
void ColorSchemeManager::loadAllColorSchemes()
{
    //qDebug() << "loadAllColorSchemes";
    int failed = 0;

    QList<QString> nativeColorSchemes = listColorSchemes();
    QListIterator<QString> nativeIter(nativeColorSchemes);
    while ( nativeIter.hasNext() )
    {
        if ( !loadColorScheme( nativeIter.next() ) )
            failed++;
    }

    /*if ( failed > 0 )
        qDebug() << "failed to load " << failed << " color schemes.";*/

    _haveLoadedAll = true;
}
QList<const ColorScheme*> ColorSchemeManager::allColorSchemes()
{
    if ( !_haveLoadedAll )
    {
        loadAllColorSchemes();
    }

    return _colorSchemes.values();
}
#if 0
void ColorSchemeManager::addColorScheme(ColorScheme* scheme)
{
    _colorSchemes.insert(scheme->name(),scheme);

    // save changes to disk
    QString path = KGlobal::dirs()->saveLocation("data","konsole/") + scheme->name() + ".colorscheme";
    KConfig config(path , KConfig::NoGlobals);

    scheme->write(config);
}
#endif

bool ColorSchemeManager::loadCustomColorScheme(const QString& path)
{
    if (path.endsWith(QLatin1String(".colorscheme")))
        return loadColorScheme(path);

    return false;
}

void ColorSchemeManager::addCustomColorSchemeDir(const QString& custom_dir)
{
    add_custom_color_scheme_dir(custom_dir);
}

bool ColorSchemeManager::loadColorScheme(const QString& filePath)
{
    if ( !filePath.endsWith(QLatin1String(".colorscheme")) || !QFile::exists(filePath) )
        return false;

    QFileInfo info(filePath);

    const QString& schemeName = info.baseName();

    ColorScheme* scheme = new ColorScheme();
    scheme->setName(schemeName);
    scheme->read(filePath);

    if (scheme->name().isEmpty())
    {
        //qDebug() << "Color scheme in" << filePath << "does not have a valid name and was not loaded.";
        delete scheme;
        return false;
    }

    if ( !_colorSchemes.contains(schemeName) )
    {
        _colorSchemes.insert(schemeName,scheme);
    }
    else
    {
        /*qDebug() << "color scheme with name" << schemeName << "has already been" <<
            "found, ignoring.";*/

        delete scheme;
    }

    return true;
}
QList<QString> ColorSchemeManager::listColorSchemes()
{
    QList<QString> ret;
    for (const QString &scheme_dir : get_color_schemes_dirs())
    {
        const QString dname(scheme_dir);
        QDir dir(dname);
        QStringList filters;
        filters << QLatin1String("*.colorscheme");
        dir.setNameFilters(filters);
        const QStringList list = dir.entryList(filters);
        for (const QString &i : list)
            ret << dname + QLatin1Char('/') + i;
    }
    return ret;
//    return KGlobal::dirs()->findAllResources("data",
//                                             "konsole/*.colorscheme",
//                                             KStandardDirs::NoDuplicates);
}
const ColorScheme ColorSchemeManager::_defaultColorScheme;
const ColorScheme* ColorSchemeManager::defaultColorScheme() const
{
    return &_defaultColorScheme;
}
bool ColorSchemeManager::deleteColorScheme(const QString& name)
{
    Q_ASSERT( _colorSchemes.contains(name) );

    // lookup the path and delete
    QString path = findColorSchemePath(name);
    if ( QFile::remove(path) )
    {
        _colorSchemes.remove(name);
        return true;
    }
    else
    {
        //qDebug() << "Failed to remove color scheme -" << path;
        return false;
    }
}
QString ColorSchemeManager::findColorSchemePath(const QString& name) const
{
//    QString path = KStandardDirs::locate("data","konsole/"+name+".colorscheme");
    const QStringList dirs = get_color_schemes_dirs();
    if ( dirs.isEmpty() )
        return QString();

    const QString dir = dirs.first();
    QString path(dir + QLatin1Char('/')+ name + QLatin1String(".colorscheme"));
    if ( !path.isEmpty() )
        return path;

    //path = KStandardDirs::locate("data","konsole/"+name+".schema");
    path = dir + QLatin1Char('/')+ name + QLatin1String(".schema");

    return path;
}
const ColorScheme* ColorSchemeManager::findColorScheme(const QString& name)
{
    if ( name.isEmpty() )
        return defaultColorScheme();

    if ( _colorSchemes.contains(name) )
        return _colorSchemes[name];
    else
    {
        // look for this color scheme
        QString path = findColorSchemePath(name);
        if ( !path.isEmpty() && loadColorScheme(path) )
        {
            return findColorScheme(name);
        }

        //qDebug() << "Could not find color scheme - " << name;

        return nullptr;
    }
}
Q_GLOBAL_STATIC(ColorSchemeManager, theColorSchemeManager)
ColorSchemeManager* ColorSchemeManager::instance()
{
    return theColorSchemeManager;
}
