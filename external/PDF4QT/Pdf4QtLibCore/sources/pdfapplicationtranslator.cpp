// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdfapplicationtranslator.h"

#include <QDir>
#include <QSettings>
#include <QMetaEnum>
#include <QCoreApplication>

namespace pdf
{

PDFApplicationTranslator::PDFApplicationTranslator()
{

}

PDFApplicationTranslator::~PDFApplicationTranslator()
{
    uninstallTranslator();
}

PDFApplicationTranslator::ELanguage PDFApplicationTranslator::getLanguage() const
{
    return m_language;
}

void PDFApplicationTranslator::installTranslator()
{
    QDir applicationDirectory(QCoreApplication::applicationDirPath());
    applicationDirectory.cd("translations");
    QString translationPath = applicationDirectory.absolutePath();

    Q_ASSERT(!m_translator);
    m_translator = new QTranslator();

    switch (m_language)
    {
        case E_LANGUAGE_AUTOMATIC_SELECTION:
        {
            if (m_translator->load(QLocale::system(), "PDF4QT", "_", translationPath))
            {
                QCoreApplication::installTranslator(m_translator);
            }
            else
            {
                delete m_translator;
                m_translator = nullptr;
            }
            break;
        }

        case E_LANGUAGE_ENGLISH:
        case E_LANGUAGE_CZECH:
        case E_LANGUAGE_GERMAN:
        case E_LANGUAGE_KOREAN:
        case E_LANGUAGE_SPANISH:
        case E_LANGUAGE_CHINESE_SIMPLIFIED:
        case E_LANGUAGE_CHINESE_TRADITIONAL:
        case E_LANGUAGE_FRENCH:
        case E_LANGUAGE_TURKISH:
        case E_LANGUAGE_RUSSIAN:
        {
            QString languageFileName = getLanguageFileName();

            if (m_translator->load(languageFileName, translationPath))
            {
                QCoreApplication::installTranslator(m_translator);
            }
            else
            {
                delete m_translator;
                m_translator = nullptr;
            }
            break;
        }

        default:
        {
            delete m_translator;
            m_translator = nullptr;

            Q_ASSERT(false);
            break;
        }
    }
}

void PDFApplicationTranslator::uninstallTranslator()
{
    if (m_translator)
    {
        QCoreApplication::removeTranslator(m_translator);

        delete m_translator;
        m_translator = nullptr;
    }
}

void PDFApplicationTranslator::loadSettings()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<ELanguage>();
    std::string languageKeyString = loadLanguageKeyFromSettings().toStdString();
    std::optional<quint64> value = metaEnum.keyToValue(languageKeyString.c_str());
    m_language = static_cast<ELanguage>(value.value_or(E_LANGUAGE_AUTOMATIC_SELECTION));
}

void PDFApplicationTranslator::saveSettings()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<ELanguage>();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("Language");
    settings.setValue("language", metaEnum.valueToKey(m_language));
    settings.endGroup();
}

void PDFApplicationTranslator::setLanguage(ELanguage newLanguage)
{
    m_language = newLanguage;
}

void PDFApplicationTranslator::saveSettings(QSettings& settings, ELanguage language)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<ELanguage>();

    settings.beginGroup("Language");
    settings.setValue("language", metaEnum.valueToKey(language));
    settings.endGroup();
}

PDFApplicationTranslator::ELanguage PDFApplicationTranslator::loadSettings(QSettings& settings)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<ELanguage>();
    std::string languageKeyString = loadLanguageKeyFromSettings(settings).toStdString();
    std::optional<quint64> value = metaEnum.keyToValue(languageKeyString.c_str());
    return static_cast<ELanguage>(value.value_or(E_LANGUAGE_AUTOMATIC_SELECTION));
}

QString PDFApplicationTranslator::loadLanguageKeyFromSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    return loadLanguageKeyFromSettings(settings);
}

QString PDFApplicationTranslator::loadLanguageKeyFromSettings(QSettings& settings)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<ELanguage>();

    settings.beginGroup("Language");
    QString languageKey = settings.value("language", metaEnum.valueToKey(E_LANGUAGE_AUTOMATIC_SELECTION)).toString();
    settings.endGroup();

    if (languageKey == QLatin1String("E_LANGUAGE_CHINESE"))
    {
        languageKey = QLatin1String("E_LANGUAGE_CHINESE_SIMPLIFIED");
    }

    return languageKey;
}

QString PDFApplicationTranslator::getLanguageFileName() const
{
    switch (m_language)
    {
        case E_LANGUAGE_ENGLISH:
            return QLatin1String("PDF4QT_en.qm");
        case E_LANGUAGE_CZECH:
            return QLatin1String("PDF4QT_cs.qm");
        case E_LANGUAGE_GERMAN:
            return QLatin1String("PDF4QT_de.qm");
        case E_LANGUAGE_KOREAN:
            return QLatin1String("PDF4QT_ko.qm");
        case E_LANGUAGE_SPANISH:
            return QLatin1String("PDF4QT_es.qm");
        case E_LANGUAGE_CHINESE_SIMPLIFIED:
            return QLatin1String("PDF4QT_zh_CN.qm");
        case E_LANGUAGE_CHINESE_TRADITIONAL:
            return QLatin1String("PDF4QT_zh_TW.qm");
        case E_LANGUAGE_FRENCH:
            return QLatin1String("PDF4QT_fr.qm");
        case E_LANGUAGE_TURKISH:
            return QLatin1String("PDF4QT_tr.qm");
        case E_LANGUAGE_RUSSIAN:
            return QLatin1String("PDF4QT_ru.qm");

        default:
            Q_ASSERT(false);
    }

    return QString();
}

}   // namespace
