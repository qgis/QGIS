#!/usr/bin/perl -i.bak -p

s%www.qgis.org%qgis.org%g;

s%/en/commercial-support.html%/de/site/forusers/commercial_support.html%g;
s%/en/community/mailing-lists.html%/en/site/forusers/support.html#mailing-lists%g;
s%/en/community/qgis-case-studies.html%/en/site/about/case_studies/index.html%g;
s%/en/documentation/manuals.html%/en/docs/index.html%g;
s%/en/sponsorship.html%/en/site/getinvolved/governance/sponsorship/sponsorship.html%g;
s%/en/sponsorship/donors.html%/en/site/about/sponsorship.html#list-of-donors%g;
s%/en/sponsorship/sponsors.html%/en/site/about/sponsorship.html#list-of-sponsors%g;
s%/en/sponsorship/donors.html%en/site/about/sponsorship.html#list-of-donors%g;

s%/de/gemeinschaft/fallstudien.html%/de/site/about/case_studies/index.html%g;
s%/de/gemeinschaft/mailinglisten.html%/de/site/forusers/support.html#mailing-lists%g;
s%/de/kommerzieller-support.html%/de/site/forusers/commercial_support.html%g;
s%/de/sponsoring.html%/de/site/getinvolved/governance/sponsorship/sponsorship.html%g;

s%/fr/sponsorship.html%/fr/site/getinvolved/governance/sponsorship/sponsorship.html%g;

s%/wiki/GUI_Translation%/en/site/getinvolved/translate.html#howto-translate-gui%g;
s%/qgiswiki/DocumentationWritersCorner%/en/site/getinvolved/governance/organisation/governance.html#community-resources%g;
