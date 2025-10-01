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

#ifndef PDFAPPLICATIONTRANSLATOR_H
#define PDFAPPLICATIONTRANSLATOR_H

#include "pdfglobal.h"

#include <QTranslator>

class QSettings;

namespace pdf
{

class PDF4QTLIBCORESHARED_EXPORT PDFApplicationTranslator
{
    Q_GADGET

public:
    explicit PDFApplicationTranslator();
    ~PDFApplicationTranslator();

    enum ELanguage
    {
        E_LANGUAGE_AUTOMATIC_SELECTION,
        E_LANGUAGE_ENGLISH,
        E_LANGUAGE_CZECH,
        E_LANGUAGE_GERMAN,
        E_LANGUAGE_KOREAN,
        E_LANGUAGE_SPANISH,
        E_LANGUAGE_CHINESE,
        E_LANGUAGE_FRENCH,
        E_LANGUAGE_TURKISH,
        E_LANGUAGE_RUSSIAN
    };

    Q_ENUM(ELanguage)

    void installTranslator();
    void uninstallTranslator();
    void loadSettings();
    void saveSettings();

    ELanguage getLanguage() const;
    void setLanguage(ELanguage newLanguage);

    static ELanguage loadSettings(QSettings& settings);
    static void saveSettings(QSettings& settings, ELanguage language);

private:
    static QString loadLanguageKeyFromSettings(QSettings& settings);

    QString loadLanguageKeyFromSettings();
    QString getLanguageFileName() const;

    QTranslator* m_translator = nullptr;
    ELanguage m_language = E_LANGUAGE_AUTOMATIC_SELECTION;
};

}   // namespace

#endif // PDFAPPLICATIONTRANSLATOR_H
