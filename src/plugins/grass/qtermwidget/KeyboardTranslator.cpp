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
#include "KeyboardTranslator.h"

// System
#include <cctype>
#include <cstdio>

// Qt
#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QKeySequence>
#include <QDir>
#include <QtDebug>

#include "tools.h"

// KDE
//#include <KDebug>
//#include <KLocale>
//#include <KStandardDirs>

using namespace Konsole;


const QByteArray KeyboardTranslatorManager::defaultTranslatorText(
"keyboard \"Fallback Key Translator\"\n"
"key Tab : \"\\t\""
);

#ifdef Q_OS_MAC
// On Mac, Qt::ControlModifier means Cmd, and MetaModifier means Ctrl
const Qt::KeyboardModifier KeyboardTranslator::CTRL_MOD = Qt::MetaModifier;
#else
const Qt::KeyboardModifier KeyboardTranslator::CTRL_MOD = Qt::ControlModifier;
#endif

KeyboardTranslatorManager::KeyboardTranslatorManager()
    : _haveLoadedAll(false)
{
}
KeyboardTranslatorManager::~KeyboardTranslatorManager()
{
    qDeleteAll(_translators);
}
QString KeyboardTranslatorManager::findTranslatorPath(const QString& name)
{
    return QString(get_kb_layout_dir() + name + QLatin1String(".keytab"));
    //return KGlobal::dirs()->findResource("data","konsole/"+name+".keytab");
}

void KeyboardTranslatorManager::findTranslators()
{
    QDir dir(get_kb_layout_dir());
    QStringList filters;
    filters << QLatin1String("*.keytab");
    dir.setNameFilters(filters);
    QStringList list = dir.entryList(filters);
//    QStringList list = KGlobal::dirs()->findAllResources("data",
//                                                         "konsole/*.keytab",
//                                                        KStandardDirs::NoDuplicates);

    // add the name of each translator to the list and associated
    // the name with a null pointer to indicate that the translator
    // has not yet been loaded from disk
    QStringListIterator listIter(list);
    while (listIter.hasNext())
    {
        QString translatorPath = listIter.next();

        QString name = QFileInfo(translatorPath).baseName();

        if ( !_translators.contains(name) )
            _translators.insert(name,0);
    }

    _haveLoadedAll = true;
}

const KeyboardTranslator* KeyboardTranslatorManager::findTranslator(const QString& name)
{
    if ( name.isEmpty() )
        return defaultTranslator();

    if ( _translators.contains(name) && _translators[name] != 0 )
        return _translators[name];

    KeyboardTranslator* translator = loadTranslator(name);

    if ( translator != nullptr )
        _translators[name] = translator;
    else if ( !name.isEmpty() )
        qDebug() << "Unable to load translator" << name;

    return translator;
}

bool KeyboardTranslatorManager::saveTranslator(const KeyboardTranslator* translator)
{
qDebug() << "KeyboardTranslatorManager::saveTranslator" << "unimplemented";
Q_UNUSED(translator)
#if 0
    const QString path = KGlobal::dirs()->saveLocation("data","konsole/")+translator->name()
           +".keytab";

    //kDebug() << "Saving translator to" << path;

    QFile destination(path);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Unable to save keyboard translation:"
                   << destination.errorString();
        return false;
    }

    {
        KeyboardTranslatorWriter writer(&destination);
        writer.writeHeader(translator->description());

        QListIterator<KeyboardTranslator::Entry> iter(translator->entries());
        while ( iter.hasNext() )
            writer.writeEntry(iter.next());
    }

    destination.close();
#endif
    return true;
}

KeyboardTranslator* KeyboardTranslatorManager::loadTranslator(const QString& name)
{
    const QString& path = findTranslatorPath(name);

    QFile source(path);
    if (name.isEmpty() || !source.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    return loadTranslator(&source,name);
}

const KeyboardTranslator* KeyboardTranslatorManager::defaultTranslator()
{
    // Try to find the default.keytab file if it exists, otherwise
    // fall back to the hard-coded one
    const KeyboardTranslator* translator = findTranslator(QLatin1String("default"));
    if (!translator)
    {
        QBuffer textBuffer;
        textBuffer.setData(defaultTranslatorText);
        textBuffer.open(QIODevice::ReadOnly);
        translator = loadTranslator(&textBuffer,QLatin1String("fallback"));
    }
    return translator;
}

KeyboardTranslator* KeyboardTranslatorManager::loadTranslator(QIODevice* source,const QString& name)
{
    KeyboardTranslator* translator = new KeyboardTranslator(name);
    KeyboardTranslatorReader reader(source);
    translator->setDescription( reader.description() );
    while ( reader.hasNextEntry() )
        translator->addEntry(reader.nextEntry());

    source->close();

    if ( !reader.parseError() )
    {
        return translator;
    }
    else
    {
        delete translator;
        return nullptr;
    }
}

KeyboardTranslatorWriter::KeyboardTranslatorWriter(QIODevice* destination)
: _destination(destination)
{
    Q_ASSERT( destination && destination->isWritable() );

    _writer = new QTextStream(_destination);
}
KeyboardTranslatorWriter::~KeyboardTranslatorWriter()
{
    delete _writer;
}
void KeyboardTranslatorWriter::writeHeader( const QString& description )
{
    *_writer << "keyboard \"" << description << '\"' << '\n';
}
void KeyboardTranslatorWriter::writeEntry( const KeyboardTranslator::Entry& entry )
{
    QString result;
    if ( entry.command() != KeyboardTranslator::NoCommand )
        result = entry.resultToString();
    else
        result = QLatin1Char('\"') + entry.resultToString() + QLatin1Char('\"');

    *_writer << QLatin1String("key ") << entry.conditionToString() << QLatin1String(" : ") << result << QLatin1Char('\n');
}


// each line of the keyboard translation file is one of:
//
// - keyboard "name"
// - key KeySequence : "characters"
// - key KeySequence : CommandName
//
// KeySequence begins with the name of the key ( taken from the Qt::Key enum )
// and is followed by the keyboard modifiers and state flags ( with + or - in front
// of each modifier or flag to indicate whether it is required ).  All keyboard modifiers
// and flags are optional, if a particular modifier or state is not specified it is
// assumed not to be a part of the sequence.  The key sequence may contain whitespace
//
// eg:  "key Up+Shift : scrollLineUp"
//      "key Next-Shift : "\E[6~"
//
// (lines containing only whitespace are ignored, parseLine assumes that comments have
// already been removed)
//

KeyboardTranslatorReader::KeyboardTranslatorReader( QIODevice* source )
    : _source(source)
    , _hasNext(false)
{
   // read input until we find the description
   while ( _description.isEmpty() && !source->atEnd() )
   {
        QList<Token> tokens = tokenize( QString::fromUtf8(source->readLine()) );
        if ( !tokens.isEmpty() && tokens.first().type == Token::TitleKeyword )
            _description = tokens[1].text;
   }
   // read first entry (if any)
   readNext();
}
void KeyboardTranslatorReader::readNext()
{
    // find next entry
    while ( !_source->atEnd() )
    {
        const QList<Token>& tokens = tokenize( QString::fromUtf8(_source->readLine()) );
        if ( !tokens.isEmpty() && tokens.first().type == Token::KeyKeyword )
        {
            KeyboardTranslator::States flags = KeyboardTranslator::NoState;
            KeyboardTranslator::States flagMask = KeyboardTranslator::NoState;
            Qt::KeyboardModifiers modifiers = Qt::NoModifier;
            Qt::KeyboardModifiers modifierMask = Qt::NoModifier;

            int keyCode = Qt::Key_unknown;

            decodeSequence(tokens[1].text.toLower(),
                           keyCode,
                           modifiers,
                           modifierMask,
                           flags,
                           flagMask);

            KeyboardTranslator::Command command = KeyboardTranslator::NoCommand;
            QByteArray text;

            // get text or command
            if ( tokens[2].type == Token::OutputText )
            {
                text = tokens[2].text.toLocal8Bit();
            }
            else if ( tokens[2].type == Token::Command )
            {
                // identify command
                if (!parseAsCommand(tokens[2].text,command))
                    qDebug() << "Command" << tokens[2].text << "not understood.";
            }

            KeyboardTranslator::Entry newEntry;
            newEntry.setKeyCode( keyCode );
            newEntry.setState( flags );
            newEntry.setStateMask( flagMask );
            newEntry.setModifiers( modifiers );
            newEntry.setModifierMask( modifierMask );
            newEntry.setText( text );
            newEntry.setCommand( command );

            _nextEntry = newEntry;

            _hasNext = true;

            return;
        }
    }

    _hasNext = false;
}

bool KeyboardTranslatorReader::parseAsCommand(const QString& text,KeyboardTranslator::Command& command)
{
    if ( text.compare(QLatin1String("erase"),Qt::CaseInsensitive) == 0 )
        command = KeyboardTranslator::EraseCommand;
    else if ( text.compare(QLatin1String("scrollpageup"),Qt::CaseInsensitive) == 0 )
        command = KeyboardTranslator::ScrollPageUpCommand;
    else if ( text.compare(QLatin1String("scrollpagedown"),Qt::CaseInsensitive) == 0 )
        command = KeyboardTranslator::ScrollPageDownCommand;
    else if ( text.compare(QLatin1String("scrolllineup"),Qt::CaseInsensitive) == 0 )
        command = KeyboardTranslator::ScrollLineUpCommand;
    else if ( text.compare(QLatin1String("scrolllinedown"),Qt::CaseInsensitive) == 0 )
        command = KeyboardTranslator::ScrollLineDownCommand;
    else if ( text.compare(QLatin1String("scrolllock"),Qt::CaseInsensitive) == 0 )
        command = KeyboardTranslator::ScrollLockCommand;
    else if ( text.compare(QLatin1String("scrolluptotop"),Qt::CaseInsensitive) == 0)
        command = KeyboardTranslator::ScrollUpToTopCommand;
    else if ( text.compare(QLatin1String("scrolldowntobottom"),Qt::CaseInsensitive) == 0)
        command = KeyboardTranslator::ScrollDownToBottomCommand;
    else
        return false;

    return true;
}

bool KeyboardTranslatorReader::decodeSequence(const QString& text,
                                              int& keyCode,
                                              Qt::KeyboardModifiers& modifiers,
                                              Qt::KeyboardModifiers& modifierMask,
                                              KeyboardTranslator::States& flags,
                                              KeyboardTranslator::States& flagMask)
{
    bool isWanted = true;
    bool endOfItem = false;
    QString buffer;

    Qt::KeyboardModifiers tempModifiers = modifiers;
    Qt::KeyboardModifiers tempModifierMask = modifierMask;
    KeyboardTranslator::States tempFlags = flags;
    KeyboardTranslator::States tempFlagMask = flagMask;

    for ( int i = 0 ; i < text.count() ; i++ )
    {
        const QChar& ch = text[i];
        bool isFirstLetter = i == 0;
        bool isLastLetter = ( i == text.count()-1 );
        endOfItem = true;
        if ( ch.isLetterOrNumber() )
        {
            endOfItem = false;
            buffer.append(ch);
        } else if ( isFirstLetter )
        {
            buffer.append(ch);
        }

        if ( (endOfItem || isLastLetter) && !buffer.isEmpty() )
        {
            Qt::KeyboardModifier itemModifier = Qt::NoModifier;
            int itemKeyCode = 0;
            KeyboardTranslator::State itemFlag = KeyboardTranslator::NoState;

            if ( parseAsModifier(buffer,itemModifier) )
            {
                tempModifierMask |= itemModifier;

                if ( isWanted )
                    tempModifiers |= itemModifier;
            }
            else if ( parseAsStateFlag(buffer,itemFlag) )
            {
                tempFlagMask |= itemFlag;

                if ( isWanted )
                    tempFlags |= itemFlag;
            }
            else if ( parseAsKeyCode(buffer,itemKeyCode) )
                keyCode = itemKeyCode;
            else
                qDebug() << "Unable to parse key binding item:" << buffer;

            buffer.clear();
        }

        // check if this is a wanted / not-wanted flag and update the
        // state ready for the next item
        if ( ch == QLatin1Char('+') )
           isWanted = true;
        else if ( ch == QLatin1Char('-') )
           isWanted = false;
    }

    modifiers = tempModifiers;
    modifierMask = tempModifierMask;
    flags = tempFlags;
    flagMask = tempFlagMask;

    return true;
}

bool KeyboardTranslatorReader::parseAsModifier(const QString& item , Qt::KeyboardModifier& modifier)
{
    if ( item == QLatin1String("shift") )
        modifier = Qt::ShiftModifier;
    else if ( item == QLatin1String("ctrl") || item == QLatin1String("control") )
        modifier = Qt::ControlModifier;
    else if ( item == QLatin1String("alt") )
        modifier = Qt::AltModifier;
    else if ( item == QLatin1String("meta") )
        modifier = Qt::MetaModifier;
    else if ( item == QLatin1String("keypad") )
        modifier = Qt::KeypadModifier;
    else
        return false;

    return true;
}
bool KeyboardTranslatorReader::parseAsStateFlag(const QString& item , KeyboardTranslator::State& flag)
{
    if ( item == QLatin1String("appcukeys") || item == QLatin1String("appcursorkeys") )
        flag = KeyboardTranslator::CursorKeysState;
    else if ( item == QLatin1String("ansi") )
        flag = KeyboardTranslator::AnsiState;
    else if ( item == QLatin1String("newline") )
        flag = KeyboardTranslator::NewLineState;
    else if ( item == QLatin1String("appscreen") )
        flag = KeyboardTranslator::AlternateScreenState;
    else if ( item == QLatin1String("anymod") || item == QLatin1String("anymodifier") )
        flag = KeyboardTranslator::AnyModifierState;
    else if ( item == QLatin1String("appkeypad") )
        flag = KeyboardTranslator::ApplicationKeypadState;
    else
        return false;

    return true;
}
bool KeyboardTranslatorReader::parseAsKeyCode(const QString& item , int& keyCode)
{
    QKeySequence sequence = QKeySequence::fromString(item);
    if ( !sequence.isEmpty() )
    {
        keyCode = sequence[0];

        if ( sequence.count() > 1 )
        {
            qDebug() << "Unhandled key codes in sequence: " << item;
        }
    }
    // additional cases implemented for backwards compatibility with KDE 3
    else if ( item == QLatin1String("prior") )
        keyCode = Qt::Key_PageUp;
    else if ( item == QLatin1String("next") )
        keyCode = Qt::Key_PageDown;
    else
        return false;

    return true;
}

QString KeyboardTranslatorReader::description() const
{
    return _description;
}
bool KeyboardTranslatorReader::hasNextEntry() const
{
    return _hasNext;
}
KeyboardTranslator::Entry KeyboardTranslatorReader::createEntry( const QString& condition ,
                                                                 const QString& result )
{
    QString entryString = QString::fromLatin1("keyboard \"temporary\"\nkey ");
    entryString.append(condition);
    entryString.append(QLatin1String(" : "));

    // if 'result' is the name of a command then the entry result will be that command,
    // otherwise the result will be treated as a string to echo when the key sequence
    // specified by 'condition' is pressed
    KeyboardTranslator::Command command;
    if (parseAsCommand(result,command))
        entryString.append(result);
    else
        entryString.append(QLatin1Char('\"') + result + QLatin1Char('\"'));

    QByteArray array = entryString.toUtf8();
    QBuffer buffer(&array);
    buffer.open(QIODevice::ReadOnly);
    KeyboardTranslatorReader reader(&buffer);

    KeyboardTranslator::Entry entry;
    if ( reader.hasNextEntry() )
        entry = reader.nextEntry();

    return entry;
}

KeyboardTranslator::Entry KeyboardTranslatorReader::nextEntry()
{
    Q_ASSERT( _hasNext );
    KeyboardTranslator::Entry entry = _nextEntry;
    readNext();
    return entry;
}
bool KeyboardTranslatorReader::parseError()
{
    return false;
}
QList<KeyboardTranslatorReader::Token> KeyboardTranslatorReader::tokenize(const QString& line)
{
    QString text = line;

    // remove comments
    bool inQuotes = false;
    int commentPos = -1;
    for (int i=text.length()-1;i>=0;i--)
    {
        QChar ch = text[i];
        if (ch == QLatin1Char('\"'))
            inQuotes = !inQuotes;
        else if (ch == QLatin1Char('#') && !inQuotes)
            commentPos = i;
    }
    if (commentPos != -1)
        text.remove(commentPos,text.length());

    text = text.simplified();

    // title line: keyboard "title"
    static QRegExp title(QLatin1String("keyboard\\s+\"(.*)\""));
    // key line: key KeySequence : "output"
    // key line: key KeySequence : command
    static QRegExp key(QLatin1String("key\\s+([\\w\\+\\s\\-\\*\\.]+)\\s*:\\s*(\"(.*)\"|\\w+)"));

    QList<Token> list;
    if ( text.isEmpty() )
    {
        return list;
    }

    if ( title.exactMatch(text) )
    {
        Token titleToken = { Token::TitleKeyword , QString() };
        Token textToken = { Token::TitleText , title.capturedTexts().at(1) };

        list << titleToken << textToken;
    }
    else if  ( key.exactMatch(text) )
    {
        Token keyToken = { Token::KeyKeyword , QString() };
        Token sequenceToken = { Token::KeySequence , key.capturedTexts().value(1).remove(QLatin1Char(' ')) };

        list << keyToken << sequenceToken;

        if ( key.capturedTexts().at(3).isEmpty() )
        {
            // capturedTexts()[2] is a command
            Token commandToken = { Token::Command , key.capturedTexts().at(2) };
            list << commandToken;
        }
        else
        {
            // capturedTexts()[3] is the output string
           Token outputToken = { Token::OutputText , key.capturedTexts().at(3) };
           list << outputToken;
        }
    }
    else
    {
        qDebug() << "Line in keyboard translator file could not be understood:" << text;
    }

    return list;
}

QList<QString> KeyboardTranslatorManager::allTranslators()
{
    if ( !_haveLoadedAll )
    {
        findTranslators();
    }

    return _translators.keys();
}

KeyboardTranslator::Entry::Entry()
: _keyCode(0)
, _modifiers(Qt::NoModifier)
, _modifierMask(Qt::NoModifier)
, _state(NoState)
, _stateMask(NoState)
, _command(NoCommand)
{
}

bool KeyboardTranslator::Entry::operator==(const Entry& rhs) const
{
    return _keyCode == rhs._keyCode &&
           _modifiers == rhs._modifiers &&
           _modifierMask == rhs._modifierMask &&
           _state == rhs._state &&
           _stateMask == rhs._stateMask &&
           _command == rhs._command &&
           _text == rhs._text;
}

bool KeyboardTranslator::Entry::matches(int keyCode ,
                                        Qt::KeyboardModifiers modifiers,
                                        States testState) const
{
#ifdef Q_OS_MAC
    // On Mac, arrow keys are considered part of keypad. Ignore that.
    modifiers &= ~Qt::KeypadModifier;
#endif

    if ( _keyCode != keyCode )
        return false;

    if ( (modifiers & _modifierMask) != (_modifiers & _modifierMask) )
        return false;

    // if modifiers is non-zero, the 'any modifier' state is implicit
    if ( (modifiers & ~Qt::KeypadModifier) != 0 )
        testState |= AnyModifierState;

    if ( (testState & _stateMask) != (_state & _stateMask) )
        return false;

    // special handling for the 'Any Modifier' state, which checks for the presence of
    // any or no modifiers.  In this context, the 'keypad' modifier does not count.
    bool anyModifiersSet = modifiers != 0 && modifiers != Qt::KeypadModifier;
    bool wantAnyModifier = _state & KeyboardTranslator::AnyModifierState;
    if ( _stateMask & KeyboardTranslator::AnyModifierState )
    {
        if ( wantAnyModifier != anyModifiersSet )
           return false;
    }

    return true;
}
QByteArray KeyboardTranslator::Entry::escapedText(bool expandWildCards,Qt::KeyboardModifiers modifiers) const
{
    QByteArray result(text(expandWildCards,modifiers));

    for ( int i = 0 ; i < result.count() ; i++ )
    {
        char ch = result[i];
        char replacement = 0;

        switch ( ch )
        {
            case 27 : replacement = 'E'; break;
            case 8  : replacement = 'b'; break;
            case 12 : replacement = 'f'; break;
            case 9  : replacement = 't'; break;
            case 13 : replacement = 'r'; break;
            case 10 : replacement = 'n'; break;
            default:
                // any character which is not printable is replaced by an equivalent
                // \xhh escape sequence (where 'hh' are the corresponding hex digits)
                if ( !QChar(QLatin1Char(ch)).isPrint() )
                    replacement = 'x';
        }

        if ( replacement == 'x' )
        {
            result.replace(i,1,"\\x"+QByteArray(1,ch).toHex());
        } else if ( replacement != 0 )
        {
            result.remove(i,1);
            result.insert(i,'\\');
            result.insert(i+1,replacement);
        }
    }

    return result;
}
QByteArray KeyboardTranslator::Entry::unescape(const QByteArray& input) const
{
    QByteArray result(input);

    for ( int i = 0 ; i < result.count()-1 ; i++ )
    {

        QByteRef ch = result[i];
        if ( ch == '\\' )
        {
           char replacement[2] = {0,0};
           int charsToRemove = 2;
           bool escapedChar = true;

           switch ( result[i+1] )
           {
              case 'E' : replacement[0] = 27; break;
              case 'b' : replacement[0] = 8 ; break;
              case 'f' : replacement[0] = 12; break;
              case 't' : replacement[0] = 9 ; break;
              case 'r' : replacement[0] = 13; break;
              case 'n' : replacement[0] = 10; break;
              case 'x' :
                 {
                    // format is \xh or \xhh where 'h' is a hexadecimal
                    // digit from 0-9 or A-F which should be replaced
                    // with the corresponding character value
                    char hexDigits[3] = {0};

                    if ( (i < result.count()-2) && isxdigit(result[i+2]) )
                            hexDigits[0] = result[i+2];
                    if ( (i < result.count()-3) && isxdigit(result[i+3]) )
                            hexDigits[1] = result[i+3];

                    unsigned charValue = 0;
                    sscanf(hexDigits,"%x",&charValue);

                    replacement[0] = (char)charValue;
                    charsToRemove = 2 + strlen(hexDigits);
                  }
              break;
              default:
                  escapedChar = false;
           }

           if ( escapedChar )
               result.replace(i,charsToRemove,replacement);
        }
    }

    return result;
}

void KeyboardTranslator::Entry::insertModifier( QString& item , int modifier ) const
{
    if ( !(modifier & _modifierMask) )
        return;

    if ( modifier & _modifiers )
        item += QLatin1Char('+');
    else
        item += QLatin1Char('-');

    if ( modifier == Qt::ShiftModifier )
        item += QLatin1String("Shift");
    else if ( modifier == Qt::ControlModifier )
        item += QLatin1String("Ctrl");
    else if ( modifier == Qt::AltModifier )
        item += QLatin1String("Alt");
    else if ( modifier == Qt::MetaModifier )
        item += QLatin1String("Meta");
    else if ( modifier == Qt::KeypadModifier )
        item += QLatin1String("KeyPad");
}
void KeyboardTranslator::Entry::insertState( QString& item , int state ) const
{
    if ( !(state & _stateMask) )
        return;

    if ( state & _state )
        item += QLatin1Char('+') ;
    else
        item += QLatin1Char('-') ;

    if ( state == KeyboardTranslator::AlternateScreenState )
        item += QLatin1String("AppScreen");
    else if ( state == KeyboardTranslator::NewLineState )
        item += QLatin1String("NewLine");
    else if ( state == KeyboardTranslator::AnsiState )
        item += QLatin1String("Ansi");
    else if ( state == KeyboardTranslator::CursorKeysState )
        item += QLatin1String("AppCursorKeys");
    else if ( state == KeyboardTranslator::AnyModifierState )
        item += QLatin1String("AnyModifier");
    else if ( state == KeyboardTranslator::ApplicationKeypadState )
        item += QLatin1String("AppKeypad");
}
QString KeyboardTranslator::Entry::resultToString(bool expandWildCards,Qt::KeyboardModifiers modifiers) const
{
    if ( !_text.isEmpty() )
        return QString::fromLatin1(escapedText(expandWildCards,modifiers));
    else if ( _command == EraseCommand )
        return QLatin1String("Erase");
    else if ( _command == ScrollPageUpCommand )
        return QLatin1String("ScrollPageUp");
    else if ( _command == ScrollPageDownCommand )
        return QLatin1String("ScrollPageDown");
    else if ( _command == ScrollLineUpCommand )
        return QLatin1String("ScrollLineUp");
    else if ( _command == ScrollLineDownCommand )
        return QLatin1String("ScrollLineDown");
    else if ( _command == ScrollLockCommand )
        return QLatin1String("ScrollLock");
    else if (_command == ScrollUpToTopCommand)
        return QLatin1String("ScrollUpToTop");
    else if (_command == ScrollDownToBottomCommand)
        return QLatin1String("ScrollDownToBottom");

    return QString();
}
QString KeyboardTranslator::Entry::conditionToString() const
{
    QString result = QKeySequence(_keyCode).toString();

    insertModifier( result , Qt::ShiftModifier );
    insertModifier( result , Qt::ControlModifier );
    insertModifier( result , Qt::AltModifier );
    insertModifier( result , Qt::MetaModifier );
    insertModifier( result , Qt::KeypadModifier );

    insertState( result , KeyboardTranslator::AlternateScreenState );
    insertState( result , KeyboardTranslator::NewLineState );
    insertState( result , KeyboardTranslator::AnsiState );
    insertState( result , KeyboardTranslator::CursorKeysState );
    insertState( result , KeyboardTranslator::AnyModifierState );
    insertState( result , KeyboardTranslator::ApplicationKeypadState );

    return result;
}

KeyboardTranslator::KeyboardTranslator(const QString& name)
: _name(name)
{
}

void KeyboardTranslator::setDescription(const QString& description)
{
    _description = description;
}
QString KeyboardTranslator::description() const
{
    return _description;
}
void KeyboardTranslator::setName(const QString& name)
{
    _name = name;
}
QString KeyboardTranslator::name() const
{
    return _name;
}

QList<KeyboardTranslator::Entry> KeyboardTranslator::entries() const
{
    return _entries.values();
}

void KeyboardTranslator::addEntry(const Entry& entry)
{
    const int keyCode = entry.keyCode();
    _entries.insert(keyCode,entry);
}
void KeyboardTranslator::replaceEntry(const Entry& existing , const Entry& replacement)
{
    if ( !existing.isNull() )
        _entries.remove(existing.keyCode(),existing);
    _entries.insert(replacement.keyCode(),replacement);
}
void KeyboardTranslator::removeEntry(const Entry& entry)
{
    _entries.remove(entry.keyCode(),entry);
}
KeyboardTranslator::Entry KeyboardTranslator::findEntry(int keyCode, Qt::KeyboardModifiers modifiers, States state) const
{
    for (auto it = _entries.cbegin(), end = _entries.cend(); it != end; ++it)
    {
        if (it.key() == keyCode)
            if ( it.value().matches(keyCode,modifiers,state) )
                return *it;
    }
    return Entry(); // entry not found
}
void KeyboardTranslatorManager::addTranslator(KeyboardTranslator* translator)
{
    _translators.insert(translator->name(),translator);

    if ( !saveTranslator(translator) )
        qDebug() << "Unable to save translator" << translator->name()
                   << "to disk.";
}
bool KeyboardTranslatorManager::deleteTranslator(const QString& name)
{
    Q_ASSERT( _translators.contains(name) );

    // locate and delete
    QString path = findTranslatorPath(name);
    if ( QFile::remove(path) )
    {
        _translators.remove(name);
        return true;
    }
    else
    {
        qDebug() << "Failed to remove translator - " << path;
        return false;
    }
}
Q_GLOBAL_STATIC( KeyboardTranslatorManager , theKeyboardTranslatorManager )
KeyboardTranslatorManager* KeyboardTranslatorManager::instance()
{
    return theKeyboardTranslatorManager;
}
