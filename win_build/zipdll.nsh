;ZipDLL include file for NSIS
;Written by Tim Kosse (mailto:tim.kosse@gmx.de)
;some improvements by deguix

;Supported languages with their translators in alphabetical order:

;Arabic translation by asdfuae
;Brazilian Portuguese translation by "deguix"
;Chinese, Simplified translation by Kii Ali <kiiali@cpatch.org>
;Chinese, Traditional traslation by "matini" and Kii Ali <kiiali@cpatch.org>
;Croatian translation by "iostriz"
;Danish translation by Claus Futtrup
;French translation by "veekee"
;German translation by Tim Kosse
;Hungarian translation by Toth Laszlo
;Korean translation by Seongab Kim
;Lithuanian translation by Vytautas Krivickas
;Polish translation by Krzysztof Galuszka
;Russion translation by Sergey
;Spanish translation by "dark_boy"

!ifndef ZIPDLL_USED

!define ZIPDLL_USED

!macro ZIPDLL_EXTRACT SOURCE DESTINATION FILE

  !define "FILE_${FILE}"

  !ifndef FILE_<ALL>
    Push "${FILE}"
  !endif

  IfFileExists "${DESTINATION}" +2
    CreateDirectory "${DESTINATION}"

  Push "${DESTINATION}"

  IfFileExists "${SOURCE}" +2
    SetErrors

  Push "${SOURCE}"

  ;The strings that will be translated are (ready to copy,
  ;remove leading semicolons in your language block):

  !ifdef LANG_ENGLISH

    ;English is default language of ZipDLL, no need to push the untranslated strings

    ;StrCmp $LANGUAGE ${LANG_ENGLISH} 0 +1

      ;Push "  Error: %s"
      ;Push "Could not get file attributes."
      ;Push "Error: Could not get file attributes."
      ;Push "Could not extract %s"
      ;Push "  Error: Could not extract %s"

      ;!ifdef FILE_<ALL>
        ;Push "  Extract: %s"
        ;Push "  Extracting %d files and directories"
        ;Push "Extracting contents of %s to %s"
      ;!else
        ;Push "Specified file does not exist in archive."
        ;Push "Error: Specified file does not exist in archive."
        ;Push "Extracting the file %s from %s to %s"
      ;!endif

      ;Push "/TRANSLATE"

  !endif

  !ifdef LANG_HUNGARIAN

    StrCmp $LANGUAGE ${LANG_HUNGARIAN} 0 +10

      Push "  Hiba: %s"
      Push "Nem olvasható a fájl attribútumai."
      Push "Hiba: Nem olvasható a fájl attribútumai."
      Push "Nem sikerült kicsomagolni a(z) %s"
      Push "  Hiba: Nem sikerült kicsomagolni a(z) %s"

      !ifdef FILE_<ALL>
        Push "  Kicsomagolás: %s"
        Push "  %d fájl és mappa kicsomagolása"
        Push "%s tartalom kicsomagolása a %s helyre"
      !else
        Push "A megadott fájl nem található az arhívumban."
        Push "Hiba: A megadott fájl nem található az arhívumban."
        Push "%s fájl kcsomagolása a(z) %s fájlból a %s helyre"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_FRENCH

    StrCmp $LANGUAGE ${LANG_FRENCH} 0 +10

      Push "  Erreur : %s"
      Push "Impossible de récupérer les informations sur le fichier."
      Push "Erreur : Impossible de récupérer les informations sur le fichier."
      Push "Impossible de décompresser %s."
      Push "  Erreur : Impossible de décompresser %s."

      !ifdef FILE_<ALL>
        Push "  Décompression : %s"
        Push "  Décompression de %d fichiers et répertoires"
        Push "Décompression des données de %s vers %s"
      !else
        Push "Le fichier spécifié n'existe pas dans l'archive"
        Push "Erreur : Le fichier spécifié n'existe pas dans l'archive"
        Push "Décompression du fichier %s depuis %s vers %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_GERMAN

    StrCmp $LANGUAGE ${LANG_GERMAN} 0 +10

      Push "  Fehler: %s"
      Push "Dateiattribute konnten nicht ermittelt werden."
      Push "Fehler: Dateiattribute konnten nicht ermittelt werden."
      Push "%s konnte nicht dekomprimiert werden."
      Push "  Fehler: %s konnte nicht dekomprimiert werden."

      !ifdef FILE_<ALL>
        Push "  Dekomprimiere: %s"
        Push "  Dekomprimiere %d Dateien und Verzeichnisse"
        Push "Dekomprimiere Inhalt von %s nach %s"
      !else
        Push "Die angegebene Datei existiert nicht im Archiv"
        Push "Fehler: Die angegebene Datei existiert nicht im Archiv"
        Push "Dekomprimiere Datei %s von %s nach %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_SPANISH

    StrCmp $LANGUAGE ${LANG_SPANISH} 0 +10

      Push "  Error: %s"
      Push "No se obtuvieron atributos del archivo"
      Push "Error: No se obtuvieron atributos del archivo"
      Push "No se pudo extraer %s"
      Push "  Error: No se pudo extraer %s"

      !ifdef FILE_<ALL>
        Push "  Extraer: %s"
        Push "  Extrayendo %d archivos y directorios"
        Push "Extraer archivos de %s a %s"
      !else
        Push "Archivo especificado no existe en el ZIP"
        Push "Error: El archivo especificado no existe en el ZIP"
        Push "Extrayendo el archivo %s de %s a %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_PORTUGUESEBR

    StrCmp $LANGUAGE ${LANG_PORTUGUESEBR} 0 +10

      Push "  Erro: %s"
      Push "Não se pode ler os atributos do arquivo"
      Push "Error: Não se pode ler os atributos do arquivo"
      Push "Não se pode extrair %s"
      Push "  Erro: Não se pode extrair %s"

      !ifdef FILE_<ALL>
        Push "  Extraindo: %s"
        Push "  Extraindo %d arquivos e diretórios"
        Push "Extraindo arquivos de %s a %s"
      !else
        Push "O arquivo especificado não existe no ZIP"
        Push "Erro: O arquivo especificado não existe no ZIP"
        Push "Extraindo o arquivo %s de %s a %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_TRADCHINESE

  StrCmp $LANGUAGE ${LANG_TRADCHINESE} 0 +11

    Push "  ¿ù»~: %s"
    Push "µLªk¨ú±oÀÉ®×Äİ©Ê¡C"
    Push "¿ù»~: µLªk¨ú±oÀÉ®×Äİ©Ê¡C"
    Push "µLªk¸ÑÀ£ÁY %s"
    Push "  ¿ù»~¡GµLªk¸ÑÀ£ÁY %s"
    
    !ifdef FILE_<ALL>
      Push "  ¸ÑÀ£ÁY¡G%s"
      Push "  ¥¿¦b¸ÑÀ£ÁY %d ÀÉ®×»P¥Ø¿ı"
      Push "¥¿¦b¸ÑÀ£ÁY %s ªº¤º®e¨ì %s"
    !else
      Push "«ü©wªºÀÉ®×¨Ã¤£¦s¦b©óÀ£ÁY¥]¡C"
      Push "¿ù»~¡G«ü©wªºÀÉ®×¨Ã¤£¦s¦b©óÀ£ÁY¥]¡C"
      Push "¥¿¦b¸ÑÀ£ÁYÀÉ®× %s ¡A±q %s ¨ì %s"
    !endif
    
    Push "/TRANSLATE"

  !endif

  !ifdef LANG_SIMPCHINESE

  StrCmp $LANGUAGE ${LANG_SIMPCHINESE} 0 +11

    Push "  ´íÎó: %s"
    Push "ÎŞ·¨È¡µÃÎÄ¼şÊôĞÔ¡£"
    Push "´íÎó: ÎŞ·¨È¡µÃÎÄ¼şÊôĞÔ¡£"
    Push "ÎŞ·¨½âÑ¹Ëõ %s"
    Push "  ´íÎó£ºÎŞ·¨½âÑ¹Ëõ %s"
    
    !ifdef FILE_<ALL>
      Push "  ½âÑ¹Ëõ£º%s"
      Push "  ÕıÔÚ½âÑ¹Ëõ %d ÎÄ¼şÓëÄ¿Â¼"
      Push "ÕıÔÚ½âÑ¹Ëõ %s µÄÄÚÈİµ½ %s"
    !else
      Push "Ö¸¶¨µÄÎÄ¼ş²¢²»´æÔÚÓÚÑ¹Ëõ°ü¡£"
      Push "´íÎó£ºÖ¸¶¨µÄÎÄ¼ş²¢²»´æÔÚÓÚÑ¹Ëõ°ü¡£"
      Push "ÕıÔÚ½âÑ¹ËõÎÄ¼ş %s £¬´Ó %s µ½ %s"
    !endif
    
    Push "/TRANSLATE"

  !endif

  !ifdef LANG_LITHUANIAN

    StrCmp $LANGUAGE ${LANG_LITHUANIAN} 0 +10

      Push "  Klaida: %s"
      Push "Negaleta gauti bylos nuorodu."
      Push "Klaida: Negaleta gauti bylos nuorodu."
      Push "Negaleta ištraukti %s"
      Push "  Klaida: Negaleta ištraukti %s"

      !ifdef FILE_<ALL>
        Push "  Ištraukiam : %s"
        Push "  Ištraukiame %d bylas ir katalogus"
        Push "Ištraukiame viska is %s i %s"
      !else
        Push "Parinkta byla nesurasta šiame archyve."
        Push "Klaida: Parinkta byla nesurasta šiame archyve."
        Push "Ištraukiame byla %s iš %s i %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef "LANG_POLISH"

    strcmp $LANGUAGE ${LANG_POLISH} 0 +10

      Push "  B³¹d: %s"
      Push "Nie mo¿e pobraæ atrybutu pliku."
      Push "B³¹d: Nie mo¿e pobraæ atrybutu pliku."
      Push "Nie mo¿e rozpakowaæ %s."
      Push "  B³¹d: Nie mo¿e rozpakowaæ %s."

      !ifdef FILE_<ALL>
        Push "  Rozpakuj: %s"
        Push "  Rozpakowywanie %d plików i katalogów"
        Push "Rozpakowywanie zawartoœci %s do %s"
      !else
        Push "Plik nie istnieje w archiwum"
        Push "B³¹d: Plik nie istnieje w archiwum"
        Push "Rozpakowywanie pliku %s z %s do %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef "LANG_KOREAN"
    strcmp $LANGUAGE ${LANG_KOREAN} 0 +10
      Push "  ¿À·ù : %s"
      Push "È­ÀÏ ¼Ó¼ºÀ» ¾ò¾î¿Ã ¼ö ¾ø½À´Ï´Ù."
      Push "¿À·ù: È­ÀÏ ¼Ó¼ºÀ» ¾ò¾î¿Ã ¼ö ¾ø½À´Ï´Ù."
      Push "%sÀ»(¸¦) Ç® ¼ö ¾ø½À´Ï´Ù."
      Push "  ¿À·ù: %sÀ»(¸¦) Ç® ¼ö ¾ø½À´Ï´Ù."

      !ifdef FILE_<ALL>
        Push "  Ç®±â : %s"
        Push "  %d°³ÀÇ ÆÄÀÏ°ú Æú´õ¸¦ Çª´Â Áß"
        Push "%sÀÇ ³»¿ëÀ» %s¿¡ Çª´Â Áß"
      !else
        Push "ÁöÁ¤µÈ ÆÄÀÏÀÌ ¾ĞÃà ÆÄÀÏ ¾È¿¡ ¾ø½À´Ï´Ù."
        Push "¿À·ù: ÁöÁ¤µÈ ÆÄÀÏÀÌ ¾ĞÃà ÆÄÀÏ ¾È¿¡ ¾ø½À´Ï´Ù."
        Push "%s ÆÄÀÏÀ» %s¿¡¼­ %s·Î Çª´Â Áß"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef "LANG_RUSSIAN"

    strcmp $LANGUAGE ${LANG_RUSSIAN} 0 +10

      Push "  Îøèáêà: %s"
      Push "Íå ìîãó ïîëó÷èòü àòğèáóòû ôàéëà."
      Push "Îøèáêà: Íå ìîãó ïîëó÷èòü àòğèáóòû ôàéëà."
      Push "Íå ìîãó èçâëå÷ü %s"
      Push "  Îøèáêà: Íå ìîãó èçâëå÷ü %s"

      !ifdef LANG_<ALL>
        Push "  Èçâëåêàş : %s"
        Push "  Èçâëå÷åíèå %d ôàéëîâ è ïàïîê"
        Push "Ñïèñîê èçâëåêàåìûõ ôàéëîâ èç %s â %s"
      !else
        Push "Èçâëåêàåìûé ôàéë íå îáíàğóæåí â àğõèâå."
        Push "Îøèáêà: SÈçâëåêàåìûé ôàéë íå îáíàğóæåí â àğõèâå."
        Push "Èçâëå÷åíèå ôàéëà %s èç %s â %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_ARABIC

    StrCmp $LANGUAGE ${LANG_ARABIC} 0 +10

      Push "  ÎØÇÁ: %s"
      Push "áã íÍÕá Úáì ÎÕÇÆÕ Çáãáİ."
      Push "ÎØÇÁ: áã íÍÕá Úáì ÎÕÇÆÕ Çáãáİ."
      Push "áÇ íãßä ÇÓÊÎÑÇÌ %s"
      Push " ÎØÇÁ: áÇ íãßä ÇÓÊÎÑÇÌ %s"
  
      !ifdef FILE_<ALL>
        Push "  ÇÓÊÎÑÇÌ : %s"
        Push "  ÇÓÊÎÑÇÌ ãÌáÏÇÊ æ ãáİÇÊ %d"
        Push "ÇÓÊÎÑÇÌ ãÍÊæíÇÊ %s Åáì %s"
      !else
        Push "Çáãáİ ÛíÑ ãæÌæÏ İí ÇáÓÌá."
        Push "ÎØÇÁ: Çáãáİ ÛíÑ ãæÌæÏ İí ÇáÓÌá."
        Push "ÇÓÊÎÑÇÌ Çáãáİ %s ãä %s Åáì %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef LANG_DANISH

    StrCmp $LANGUAGE ${LANG_DANISH} 0 +10

      Push "  Fejl: %s"
      Push "Kunne ikke læse fil attributter."
      Push "Fejl: Kunne ikke læse fil attributter."
      Push "Kunne ikke udpakke %s"
      Push "  Fejl: Kunne ikke udpakke %s"

      !ifdef FILE_<ALL>
        Push "  Udpakker: %s"
        Push "  Udpakker %d filer og mapper"
        Push "Udpakker indhold fra %s til %s"
      !else
        Push "Specificeret fil eksisterer ikke i filarkivet"
        Push "Fejl: Specificeret fil eksisterer ikke i filarkivet"
        Push "Udpakker fil %s fra %s til %s"
      !endif

      Push "/TRANSLATE"

  !endif 

  !ifdef LANG_CROATIAN

    StrCmp $LANGUAGE ${LANG_CROATIAN} 0 +10

      Push "  Greška: %s"
      Push "Ne mogu dohvatiti atribute datoteke."
      Push "Greška: Ne mogu dohvatiti atribute datoteke."
      Push "Ne mogu ekstrahirati %s"
      Push "  Greška: Ne mogu ekstrahirati %s"

      !ifdef FILE_<ALL>
        Push "  Ekstrakcija: %s"
        Push "  Ekstrakcija %d datoteka i mapa"
        Push "Ekstrakcija sadraja %s u %s"
      !else
        Push "Traena datoteka ne postoji u arhivi."
        Push "Greška: Traena datoteka ne postoji u arhivi."
        Push "Ekstrakcija datoteke %s iz %s u %s"
      !endif

      Push "/TRANSLATE"

  !endif

  !ifdef FILE_<ALL>
  !echo "============== ZipDLL::extractall defined ===================="
    ZipDLL::extractall
  !else
  !echo "============== ZipDLL::extractfile defined ===================="
    ZipDLL::extractfile
  !endif

  !undef "FILE_${FILE}"

!macroend

!endif
