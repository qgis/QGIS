(function () {
  const STORAGE_KEY = "strata_lang";
  const REPOSITORY_URL = "https://github.com/francemazzi/strata";
  const RELEASES_URL = `${REPOSITORY_URL}/releases`;
  const RELEASES_API_URL = "https://api.github.com/repos/francemazzi/strata/releases?per_page=20";
  const REQUIRED_RELEASE_ASSETS = {
    macos: /\.dmg$/i,
    windows: /-win64\.exe$/i,
    linux: /\.AppImage$/i,
  };

  const mockupPhrases = {
    it: [
      "buffer 500m sui comuni ed esporta GeoJSON…",
      "scarica OSM per Brescia centro…",
      "conta gli alberi nel verde urbano…",
    ],
    en: [
      "buffer municipalities by 500m and export GeoJSON…",
      "download OSM for downtown Brescia…",
      "count trees in urban green areas…",
    ],
  };

  const translations = {
    it: {
      "meta.title": "Strata — Agente AI per QGIS | Cloud pronto all'uso o Enterprise on-premise",
      "meta.description":
        "Strata: agente AI nativo in QGIS. Cloud gestito o on-premise con AI locale.",

      "nav.plans": "Piani",
      "nav.privacy": "Privacy",
      "nav.product": "Prodotto",
      "nav.features": "Feature",
      "nav.demo": "Demo",
      "nav.download": "Download",
      "nav.downloadBtn": "Inizia Subito",
      "nav.login": "Accedi",

      "hero.badge": "Agente AI per QGIS · Cloud o On-Premise",
      "hero.titleLine": "L'Agente AI per il tuo GIS.",
      "hero.titleHighlight": "Pronto all'uso, sicuro per l'azienda.",
      "hero.subtitle": "AI nativa in QGIS. Cloud gestito o deployment on-premise.",
      "hero.pillPlan": "Plan",
      "hero.pillAgent": "Agent",
      "hero.pillAsk": "Ask",
      "hero.pillTools": "19 tool GIS integrati",
      "hero.ctaPrimary": "Inizia Subito",
      "hero.ctaSecondary": "Contatta le vendite",

      "plans.label": "Piani e soluzioni",
      "plans.title": "Scegli come adottare Strata",
      "plans.pro.badge": "Pro",
      "plans.pro.title": "Pro — Chiavi in mano",
      "plans.pro.slogan": "Attiva e usa, zero configurazioni.",
      "plans.pro.desc": "Abbonamento mensile, niente API key da gestire.",
      "plans.team.badge": "Team",
      "plans.team.title": "Cloud Team — Collaborazione",
      "plans.team.slogan": "La conoscenza geografica del tuo team, centralizzata.",
      "plans.team.desc": "Contesto condiviso del team, indicizzato in Strata Cloud.",
      "plans.team.cta": "Accedi alla Strata Cloud →",
      "plans.enterprise.badge": "Ent.",
      "plans.enterprise.title": "Enterprise — Privacy totale & AI locale",
      "plans.enterprise.slogan": "I tuoi dati geografici non lasciano mai la tua rete.",
      "plans.enterprise.desc": "On-premise con LLM locali. Zero egress, pieno controllo.",

      "privacy.label": "Sicurezza e privacy",
      "privacy.title": "La tua mappa, i tuoi dati. Nessun compromesso sulla sicurezza.",
      "privacy.subtitle":
        "Per dati territoriali sensibili, tutto resta nella tua infrastruttura.",
      "privacy.1.title": "AI 100% offline",
      "privacy.1.desc":
        "LLM locali per analisi sul territorio, senza internet.",
      "privacy.2.title": "Isolamento dei dati",
      "privacy.2.desc":
        "Layer e documenti indicizzati su database cifrati interni.",
      "privacy.3.title": "Conformità normativa",
      "privacy.3.desc":
        "Per organizzazioni con dati sensibili non esportabili.",
      "privacy.closing": "Zero egress. Audit completo. Controllo totale.",

      "mockup.mapLabel": "Mappa GIS",
      "mockup.chatTitle": "AI Assistant",
      "mockup.modePlan": "Plan",
      "mockup.modeAgent": "Agent",
      "mockup.modeAsk": "Ask",
      "mockup.userMsg": "Fai buffer 500m sui comuni e esporta GeoJSON",
      "mockup.aiMsg":
        "Buffer eseguito. Salvato in output/buffer_500m.geojson. Approvi?",
      "mockup.input": "buffer 500m sui comuni ed esporta GeoJSON…",

      "problems.label": "Problemi comuni",
      "problems.title": "Conosci questi ostacoli in QGIS?",
      "problems.carousel.label": "Carousel problemi comuni",
      "problems.prev": "Problema precedente",
      "problems.next": "Problema successivo",

      "problems.1.title": "PyQGIS difficile",
      "problems.1.desc": "Snippet da ChatGPT senza contesto del progetto.",
      "problems.1.fix": "Esegue il codice con la tua approvazione.",

      "problems.2.title": "Ispezione layer laboriosa",
      "problems.2.desc": "Campi, CRS ed estensioni richiedono script manuali.",
      "problems.2.fix": "Legge layer e campi dal progetto.",

      "problems.3.title": "Dipendenze Python mancanti",
      "problems.3.desc": "geopandas, osmnx… spesso assenti in QGIS.",
      "problems.3.fix": "Installa le librerie mancanti con un clic.",

      "problems.4.title": "Scaricare dati è lento",
      "problems.4.desc": "Overpass, GeoJSON, GADM — fetch e import manuale.",
      "problems.4.fix": "Scarica e importa in un solo passaggio.",

      "problems.5.title": "Contesto disperso",
      "problems.5.desc": "Progetto, script e layer sparsi — l'AI generica non li vede.",
      "problems.5.fix": "Usa file e layer del progetto come contesto.",

      "problems.6.title": "Modifiche senza controllo",
      "problems.6.desc": "Script che sovrascrivono file senza review.",
      "problems.6.fix": "Ogni modifica va approvata prima.",

      "problems.7.title": "Plugin AI disconnessi",
      "problems.7.desc":
        "Assistenti esterni ignorano layer e Processing Toolbox.", // #spellok
      "problems.7.fix": "Assistente nativo in QGIS, conosce il progetto.",

      "solution.label": "Soluzione",
      "solution.title": "Chiedi, approva, fatto",
      "solution.subtitle":
        "Descrivi cosa ti serve. Strata propone, tu decidi — senza uscire da QGIS.",

      "solution.chat.title": "Chat accanto alla mappa",
      "solution.chat.desc":
        "Chiedi in italiano, allega file, vedi risposte accanto alla mappa.",
      "solution.chat.f1": "Risposte in tempo reale",
      "solution.chat.f2": "Cronologia salvata per progetto",

      "solution.agent.title": "Tre modalità, una sola app",
      "solution.agent.desc":
        "Pianifica, esegui o chiedi. Scegli il livello di autonomia.",
      "solution.agent.f1": "Pianifica · Esegui · Chiedi",
      "solution.agent.f2": "Azioni GIS integrate",

      "demo.label": "Demo",
      "demo.title": "Vedilo in azione",
      "demo.carousel.label": "Esempi interattivi di Strata",
      "demo.tabs.label": "Scegli un esempio demo",
      "demo.prev": "Esempio precedente",
      "demo.next": "Esempio successivo",
      "demo.agent.tab": "Agent",
      "demo.agent.kicker": "Esecuzione controllata",
      "demo.agent.title": "Esecuzione sul progetto",
      "demo.agent.desc":
        "Chiedi in naturale, ispeziona layer, esegue e mostra il risultato.",
      "demo.agent.alt": "Strata in Agent mode con pannello AI accanto alla mappa GIS",
      "demo.plan.tab": "Plan",
      "demo.plan.kicker": "Piano prima dell'azione",
      "demo.plan.title": "Piano strutturato",
      "demo.plan.desc":
        "Obiettivo complesso → piano da approvare prima di agire.",
      "demo.plan.alt": "Strata in Plan mode con piano approvabile prima delle modifiche",
      "demo.maps.tab": "Analisi mappe",
      "demo.maps.kicker": "Domande sui layer reali",
      "demo.maps.title": "Verde urbano e alberi",
      "demo.maps.desc":
        "Legge i layer, calcola metriche, risponde accanto alla mappa.",
      "demo.maps.alt": "Analisi Strata su mappa di Brescia con risposta AI su verde urbano e alberi",

      "speed.label": "Produttività GIS",
      "speed.title": "Da ore a minuti",
      "speed.subtitle": "Esempio reale: ispeziona 5 layer, buffer, export GeoJSON.",
      "speed.before": "Prima",
      "speed.beforeTime": "~45 min",
      "speed.beforeSteps":
        "Console Python, script per layer, API, dipendenze, export manuale.",
      "speed.after": "Con Strata",
      "speed.afterTime": "~5 min",
      "speed.afterSteps":
        "Una richiesta in linguaggio naturale. Approvi, esegue, salva.",

      "features.label": "Vantaggi",
      "features.title": "Tutto integrato, zero complessità",

      "features.1.title": "Fa il lavoro al posto tuo",
      "features.1.desc":
        "Buffer, export, analisi: chiedi in naturale, Strata esegue.",

      "features.2.title": "Si adatta al tuo modo di lavorare",
      "features.2.desc":
        "Regole e preferenze per risposte coerenti col team.",

      "features.3.title": "Conosce i tuoi file",
      "features.3.desc":
        "Script, progetti e documenti del workspace come contesto.",

      "features.4.title": "I layer solo quando vuoi",
      "features.4.desc":
        "Condividi layer e attributi solo quando decidi tu.",

      "features.5.title": "AI già inclusa",
      "features.5.desc":
        "Piano Pro: modelli AI pronti, zero gestione crediti.",

      "features.6.title": "I tuoi dati, le tue regole",
      "features.6.desc":
        "On-premise in locale. Nel cloud, tu decidi cosa condividere.",

      "start.label": "Come iniziare",
      "start.title": "Pronto in pochi minuti",
      "start.subtitle":
        "Nessuna configurazione complicata. Scarica, apri il tuo progetto QGIS e inizia subito.",
      "start.1.title": "Scarica Strata",
      "start.1.desc":
        "Mac, Windows o Linux. QGIS con AI già integrata.",
      "start.2.title": "Apri il tuo progetto",
      "start.2.desc":
        "Apri layer e mappe che usi ogni giorno.",
      "start.3.title": "Chiedi cosa ti serve",
      "start.3.desc":
        "Scrivi cosa vuoi fare. Strata propone, tu approvi.",
      "start.cta.download": "Scarica gratis",
      "start.cta.cloud": "Prova Strata Cloud",

      "download.label": "Download",
      "download.title": "Scarica Strata",
      "download.subtitle": "Binari da GitHub Releases.",
      "download.cta": "Vai alle Releases",
      "download.status.fallback": "Releases GitHub",
      "download.status.complete": "Ultima release completa: {version}",
      "download.macos.title": "macOS",
      "download.macos.sub": "Intel + Apple Silicon",
      "download.macos.file": "File .dmg",
      "download.macos.note": "DMG firmato. Trascina in Applicazioni.",
      "download.windows.title": "Windows",
      "download.windows.sub": "10/11 x64",
      "download.windows.file": "File -win64.exe",
      "download.windows.note": "Installer firmato. Verifica il publisher.",
      "download.linux.title": "Linux",
      "download.linux.sub": "x86_64",
      "download.linux.file": "File .AppImage",
      "download.linux.note": "AppImage eseguibile. Ubuntu 22.04+.",

      "team.label": "Team",
      "team.title": "Chi c'è dietro Strata",
      "team.francesco.role": "Co-founder, informatico",
      "team.valerio.role": "Co-founder, esperto GeoAI",
      "team.massimo.role": "Co-founder, esperto GIS",

      "call.label": "Contatti",
      "call.title": "Organizza call",
      "call.subtitle": "30 minuti per capire come Strata può aiutare il tuo team GIS.",
      "call.cta": "Prenota 30 minuti",

      "footer.tagline": "Agente AI nativo in QGIS. Cloud o on-premise.",
      "opensource.title": "Costruito su QGIS. Aperto, trasparente, senza lock-in.",
      "opensource.desc":
        "Basato su QGIS open-source. Compatibile con i tuoi flussi GIS, senza lock-in.",
      "opensource.closing": "Il GIS resta tuo. Il valore aggiunto è nel servizio.",
      "footer.disclaimer":
        "Strata is an independent commercial service based on QGIS. It is not endorsed by or affiliated with QGIS.ORG or the QGIS project.",
      "footer.github": "GitHub",
      "footer.star": "Metti una stella",
      "footer.license": "Licenza GPLv2+",
      "footer.contact": "Contatto",
    },
    en: {
      "meta.title": "Strata — AI Agent for QGIS | Managed Cloud or Enterprise On-Premise",
      "meta.description":
        "Strata: native AI agent in QGIS. Managed cloud or on-premise with local AI.",

      "nav.plans": "Plans",
      "nav.privacy": "Privacy",
      "nav.product": "Product",
      "nav.features": "Features",
      "nav.demo": "Demo",
      "nav.download": "Download",
      "nav.downloadBtn": "Get Started",
      "nav.login": "Log in",

      "hero.badge": "AI Agent for QGIS · Cloud or On-Premise",
      "hero.titleLine": "The AI agent for your GIS.",
      "hero.titleHighlight": "Ready to use, secure for enterprise.",
      "hero.subtitle": "Native AI in QGIS. Managed cloud or on-premise.",
      "hero.pillPlan": "Plan",
      "hero.pillAgent": "Agent",
      "hero.pillAsk": "Ask",
      "hero.pillTools": "19 integrated GIS tools",
      "hero.ctaPrimary": "Get Started",
      "hero.ctaSecondary": "Contact sales",

      "plans.label": "Plans & solutions",
      "plans.title": "Choose how to adopt Strata",
      "plans.pro.badge": "Pro",
      "plans.pro.title": "Pro — Turnkey",
      "plans.pro.slogan": "Activate and use, zero configuration.",
      "plans.pro.desc": "Monthly subscription, no API keys to manage.",
      "plans.team.badge": "Team",
      "plans.team.title": "Cloud Team — Collaboration",
      "plans.team.slogan": "Your team's geographic knowledge, centralized.",
      "plans.team.desc": "Shared team context, indexed in Strata Cloud.",
      "plans.team.cta": "Log in to Strata Cloud →",
      "plans.enterprise.badge": "Ent.",
      "plans.enterprise.title": "Enterprise — Total privacy & local AI",
      "plans.enterprise.slogan": "Your geographic data never leaves your network.",
      "plans.enterprise.desc": "On-premise with local LLMs. Zero egress, full control.",

      "privacy.label": "Security & privacy",
      "privacy.title": "Your map, your data. No compromise on security.",
      "privacy.subtitle":
        "For sensitive territorial data, everything stays in your infrastructure.",
      "privacy.1.title": "100% offline AI",
      "privacy.1.desc":
        "Local LLMs for territorial analysis, no internet required.",
      "privacy.2.title": "Data isolation",
      "privacy.2.desc":
        "Layers and documents indexed on encrypted internal databases.",
      "privacy.3.title": "Regulatory compliance",
      "privacy.3.desc":
        "For organizations with sensitive, non-exportable data.",
      "privacy.closing": "Zero egress. Full audit. Total control.",

      "mockup.mapLabel": "GIS map",
      "mockup.chatTitle": "AI Assistant",
      "mockup.modePlan": "Plan",
      "mockup.modeAgent": "Agent",
      "mockup.modeAsk": "Ask",
      "mockup.userMsg": "Buffer municipalities by 500m and export GeoJSON",
      "mockup.aiMsg":
        "Buffer done. Saved to output/buffer_500m.geojson. Approve?",
      "mockup.input": "buffer municipalities by 500m and export GeoJSON…",

      "problems.label": "Common problems",
      "problems.title": "Sound familiar in QGIS?",
      "problems.carousel.label": "Common problems carousel",
      "problems.prev": "Previous problem",
      "problems.next": "Next problem",

      "problems.1.title": "PyQGIS is hard",
      "problems.1.desc": "Snippets from ChatGPT without project context.",
      "problems.1.fix": "Runs the code with your approval.",

      "problems.2.title": "Layer inspection is tedious",
      "problems.2.desc": "Fields, CRS, and extents need manual scripts.",
      "problems.2.fix": "Reads layers and fields from your project.",

      "problems.3.title": "Missing Python deps",
      "problems.3.desc": "geopandas, osmnx… often missing in QGIS.",
      "problems.3.fix": "Install missing libraries in one click.",

      "problems.4.title": "Fetching data is slow",
      "problems.4.desc": "Overpass, GeoJSON, GADM — manual fetch and import.",
      "problems.4.fix": "Download and import in one step.",

      "problems.5.title": "Scattered context",
      "problems.5.desc": "Project, scripts, and layers everywhere — generic AI can't see them.",
      "problems.5.fix": "Uses project files and layers as context.",

      "problems.6.title": "Uncontrolled edits",
      "problems.6.desc": "Scripts overwriting files without review.",
      "problems.6.fix": "Every change must be approved first.",

      "problems.7.title": "Disconnected AI plugins",
      "problems.7.desc":
        "External assistants ignore layers and Processing Toolbox.",
      "problems.7.fix": "Native QGIS assistant that knows your project.",

      "solution.label": "Solution",
      "solution.title": "Ask, approve, done",
      "solution.subtitle":
        "Describe what you need. Strata proposes, you decide — without leaving QGIS.",

      "solution.chat.title": "Chat beside the map",
      "solution.chat.desc":
        "Ask in plain language, attach files, see answers beside the map.",
      "solution.chat.f1": "Real-time answers",
      "solution.chat.f2": "History saved per project",

      "solution.agent.title": "Three modes, one app",
      "solution.agent.desc":
        "Plan, run, or ask. Choose your level of autonomy.",
      "solution.agent.f1": "Plan · Run · Ask",
      "solution.agent.f2": "Built-in GIS actions",

      "demo.label": "Demo",
      "demo.title": "See it in action",
      "demo.carousel.label": "Interactive Strata examples",
      "demo.tabs.label": "Choose a demo example",
      "demo.prev": "Previous example",
      "demo.next": "Next example",
      "demo.agent.tab": "Agent",
      "demo.agent.kicker": "Controlled execution",
      "demo.agent.title": "Execution on your project",
      "demo.agent.desc":
        "Ask in plain language, inspect layers, run, and show the result.",
      "demo.agent.alt": "Strata in Agent mode with the AI panel next to the GIS map",
      "demo.plan.tab": "Plan",
      "demo.plan.kicker": "Plan before action",
      "demo.plan.title": "Structured plan",
      "demo.plan.desc":
        "Complex goal → plan to approve before acting.",
      "demo.plan.alt": "Strata in Plan mode with an approvable plan before changes",
      "demo.maps.tab": "Map analysis",
      "demo.maps.kicker": "Questions over real layers",
      "demo.maps.title": "Urban green and trees",
      "demo.maps.desc":
        "Reads layers, computes metrics, answers beside the map.",
      "demo.maps.alt": "Strata analysis over a Brescia map with an AI answer about urban green and trees",

      "speed.label": "GIS productivity",
      "speed.title": "From hours to minutes",
      "speed.subtitle": "Real example: inspect 5 layers, buffer, export GeoJSON.",
      "speed.before": "Before",
      "speed.beforeTime": "~45 min",
      "speed.beforeSteps":
        "Python console, scripts per layer, APIs, deps, manual export.",
      "speed.after": "With Strata",
      "speed.afterTime": "~5 min",
      "speed.afterSteps":
        "One natural-language request. You approve, it runs, saves.",

      "features.label": "Benefits",
      "features.title": "Everything built in, zero complexity",

      "features.1.title": "Does the work for you",
      "features.1.desc":
        "Buffer, export, analysis: ask in plain language, Strata runs it.",

      "features.2.title": "Fits how you work",
      "features.2.desc":
        "Rules and preferences for team-consistent answers.",

      "features.3.title": "Knows your files",
      "features.3.desc":
        "Scripts, projects, and workspace documents as context.",

      "features.4.title": "Layers only when you want",
      "features.4.desc":
        "Share layers and attributes only when you decide.",

      "features.5.title": "AI included",
      "features.5.desc":
        "Pro plan: AI models ready, zero credit management.",

      "features.6.title": "Your data, your rules",
      "features.6.desc":
        "On-premise locally. In the cloud, you choose what to share.",

      "start.label": "Get started",
      "start.title": "Ready in minutes",
      "start.subtitle":
        "No complicated setup. Download, open your QGIS project, and start right away.",
      "start.1.title": "Download Strata",
      "start.1.desc":
        "Mac, Windows, or Linux. QGIS with AI built in.",
      "start.2.title": "Open your project",
      "start.2.desc":
        "Open the layers and maps you use every day.",
      "start.3.title": "Ask for what you need",
      "start.3.desc":
        "Write what you want to do. Strata proposes, you approve.",
      "start.cta.download": "Download free",
      "start.cta.cloud": "Try Strata Cloud",

      "download.label": "Download",
      "download.title": "Download Strata",
      "download.subtitle": "Binaries from GitHub Releases.",
      "download.cta": "Go to Releases",
      "download.status.fallback": "GitHub Releases",
      "download.status.complete": "Latest complete release: {version}",
      "download.macos.title": "macOS",
      "download.macos.sub": "Intel + Apple Silicon",
      "download.macos.file": ".dmg file",
      "download.macos.note": "Signed DMG. Drag to Applications.",
      "download.windows.title": "Windows",
      "download.windows.sub": "10/11 x64",
      "download.windows.file": "-win64.exe file",
      "download.windows.note": "Signed installer. Verify the publisher.",
      "download.linux.title": "Linux",
      "download.linux.sub": "x86_64",
      "download.linux.file": ".AppImage file",
      "download.linux.note": "Runnable AppImage. Ubuntu 22.04+.",

      "team.label": "Team",
      "team.title": "The people behind Strata",
      "team.francesco.role": "Co-founder, software engineer",
      "team.valerio.role": "Co-founder, GeoAI specialist",
      "team.massimo.role": "Co-founder, GIS expert",

      "call.label": "Contact",
      "call.title": "Book a call",
      "call.subtitle": "30 minutes to see how Strata can help your GIS team.",
      "call.cta": "Book 30 minutes",

      "footer.tagline": "Native AI agent in QGIS. Cloud or on-premise.",
      "opensource.title": "Built on QGIS. Open, transparent, no lock-in.",
      "opensource.desc":
        "Built on open-source QGIS. Compatible with your GIS workflows, no lock-in.",
      "opensource.closing": "The GIS stays yours. The added value is in the service.",
      "footer.disclaimer":
        "Strata is an independent commercial service based on QGIS. It is not endorsed by or affiliated with QGIS.ORG or the QGIS project.",
      "footer.github": "GitHub",
      "footer.star": "Star on GitHub",
      "footer.license": "GPLv2+ License",
      "footer.contact": "Contact",
    },
  };

  function detectLanguage() {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored && translations[stored]) return stored;
    const browser = (navigator.language || "it").slice(0, 2).toLowerCase();
    return translations[browser] ? browser : "it";
  }

  let currentLang = detectLanguage();
  let latestCompleteRelease = null;
  let mockupTypingTimer = null;
  let mockupTypingState = null;

  function translate(key, replacements = {}) {
    let text = translations[currentLang]?.[key] || translations.it[key] || "";
    Object.entries(replacements).forEach(([name, value]) => {
      text = text.replaceAll(`{${name}}`, value);
    });
    return text;
  }

  function strataVersionFromTag(tagName) {
    const match = String(tagName || "").match(/^strata-v(\d+)\.(\d+)\.(\d+)$/);
    if (!match) return null;

    return {
      label: `${match[1]}.${match[2]}.${match[3]}`,
      parts: match.slice(1).map((value) => Number(value)),
    };
  }

  function normalizeCompleteRelease(release) {
    const version = strataVersionFromTag(release?.tag_name);
    if (!version || release.draft || release.prerelease) return null;

    const assets = Array.isArray(release.assets) ? release.assets : [];
    const requiredAssets = {};

    for (const [platform, matcher] of Object.entries(REQUIRED_RELEASE_ASSETS)) {
      const asset = assets.find((candidate) => {
        return matcher.test(candidate?.name || "") && candidate?.browser_download_url;
      });

      if (!asset) return null;
      requiredAssets[platform] = asset;
    }

    return {
      html_url: release.html_url || `${RELEASES_URL}/tag/${release.tag_name}`,
      published_at: release.published_at || "",
      tag_name: release.tag_name,
      version: version.label,
      versionParts: version.parts,
      requiredAssets,
    };
  }

  function compareCompleteReleases(a, b) {
    for (let index = 0; index < a.versionParts.length; index += 1) {
      if (a.versionParts[index] !== b.versionParts[index]) {
        return b.versionParts[index] - a.versionParts[index];
      }
    }

    return String(b.published_at).localeCompare(String(a.published_at));
  }

  function findLatestCompleteRelease(releases) {
    if (!Array.isArray(releases)) return null;

    return releases
      .map(normalizeCompleteRelease)
      .filter(Boolean)
      .sort(compareCompleteReleases)[0] || null;
  }

  function updateDownloadReleaseUi() {
    const releaseUrl = latestCompleteRelease?.html_url || RELEASES_URL;

    document.querySelectorAll("[data-release-link]").forEach((link) => {
      link.setAttribute("href", releaseUrl);
    });

    document.querySelectorAll("[data-download-asset]").forEach((link) => {
      const platform = link.getAttribute("data-download-asset");
      const asset = latestCompleteRelease?.requiredAssets?.[platform];
      link.setAttribute("href", asset?.browser_download_url || releaseUrl);
    });

    document.querySelectorAll("[data-download-file]").forEach((el) => {
      const platform = el.getAttribute("data-download-file");
      const asset = latestCompleteRelease?.requiredAssets?.[platform];
      if (asset?.name) {
        el.textContent = asset.name;
      }
    });

    const status = document.querySelector("[data-release-status]");
    if (status) {
      status.textContent = latestCompleteRelease
        ? translate("download.status.complete", { version: latestCompleteRelease.version })
        : translate("download.status.fallback");
    }
  }

  async function initDownloadReleaseLinks() {
    try {
      const response = await fetch(RELEASES_API_URL, {
        headers: { Accept: "application/vnd.github+json" },
      });

      if (!response.ok) {
        throw new Error(`GitHub releases request failed: ${response.status}`);
      }

      latestCompleteRelease = findLatestCompleteRelease(await response.json());
    } catch (_error) {
      latestCompleteRelease = null;
    }

    updateDownloadReleaseUi();
  }

  function applyTranslations(lang) {
    const dict = translations[lang];
    if (!dict) return;

    document.documentElement.lang = lang;

    document.querySelectorAll("[data-i18n]").forEach((el) => {
      const key = el.getAttribute("data-i18n");
      if (dict[key] !== undefined) {
        el.textContent = dict[key];
      }
    });

    document.querySelectorAll("[data-i18n-placeholder]").forEach((el) => {
      const key = el.getAttribute("data-i18n-placeholder");
      if (dict[key] !== undefined) {
        el.setAttribute("placeholder", dict[key]);
      }
    });

    document.querySelectorAll("[data-i18n-alt]").forEach((el) => {
      const key = el.getAttribute("data-i18n-alt");
      if (dict[key] !== undefined) {
        el.setAttribute("alt", dict[key]);
      }
    });

    document.querySelectorAll("[data-i18n-aria-label]").forEach((el) => {
      const key = el.getAttribute("data-i18n-aria-label");
      if (dict[key] !== undefined) {
        el.setAttribute("aria-label", dict[key]);
      }
    });

    const titleEl = document.querySelector("title[data-i18n]");
    if (titleEl && dict["meta.title"]) {
      titleEl.textContent = dict["meta.title"];
    }

    const metaDesc = document.querySelector('meta[name="description"]');
    if (metaDesc && dict["meta.description"]) {
      metaDesc.setAttribute("content", dict["meta.description"]);
    }

    document.querySelectorAll("[data-lang]").forEach((btn) => {
      btn.classList.toggle("active", btn.getAttribute("data-lang") === lang);
      btn.setAttribute("aria-pressed", btn.getAttribute("data-lang") === lang ? "true" : "false");
    });

    updateDownloadReleaseUi();
    restartMockupTyping();
  }

  function stopMockupTyping() {
    if (mockupTypingTimer) {
      clearTimeout(mockupTypingTimer);
      mockupTypingTimer = null;
    }
    mockupTypingState = null;
  }

  function restartMockupTyping() {
    stopMockupTyping();

    const target = document.querySelector("[data-mockup-typing]");
    if (!target) return;

    const phrases = mockupPhrases[currentLang] || mockupPhrases.it;
    const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;

    if (reducedMotion) {
      target.textContent = translate("mockup.input");
      return;
    }

    mockupTypingState = {
      phraseIndex: 0,
      charIndex: 0,
      deleting: false,
    };

    function tick() {
      if (!mockupTypingState) return;

      const phrase = phrases[mockupTypingState.phraseIndex];
      const { deleting } = mockupTypingState;

      if (!deleting) {
        mockupTypingState.charIndex += 1;
        target.textContent = phrase.slice(0, mockupTypingState.charIndex);

        if (mockupTypingState.charIndex >= phrase.length) {
          mockupTypingTimer = setTimeout(() => {
            mockupTypingState.deleting = true;
            tick();
          }, 1800);
          return;
        }

        mockupTypingTimer = setTimeout(tick, 42 + Math.random() * 28);
        return;
      }

      mockupTypingState.charIndex -= 1;
      target.textContent = phrase.slice(0, mockupTypingState.charIndex);

      if (mockupTypingState.charIndex <= 0) {
        mockupTypingState.deleting = false;
        mockupTypingState.phraseIndex = (mockupTypingState.phraseIndex + 1) % phrases.length;
        mockupTypingTimer = setTimeout(tick, 320);
        return;
      }

      mockupTypingTimer = setTimeout(tick, 22);
    }

    tick();
  }

  function initMockupTyping() {
    restartMockupTyping();

    const reducedMotionQuery = window.matchMedia("(prefers-reduced-motion: reduce)");
    if (typeof reducedMotionQuery.addEventListener === "function") {
      reducedMotionQuery.addEventListener("change", restartMockupTyping);
    } else if (typeof reducedMotionQuery.addListener === "function") {
      reducedMotionQuery.addListener(restartMockupTyping);
    }
  }

  function setLanguage(lang) {
    if (!translations[lang]) return;
    currentLang = lang;
    localStorage.setItem(STORAGE_KEY, lang);
    applyTranslations(lang);
  }

  function initDemoCarousel() {
    const carousel = document.querySelector(".demo-carousel");
    if (!carousel) return;

    const slides = Array.from(carousel.querySelectorAll("[data-demo-slide]"));
    const tabs = Array.from(carousel.querySelectorAll("[data-demo-target]"));
    const prev = carousel.querySelector("[data-demo-prev]");
    const next = carousel.querySelector("[data-demo-next]");
    const count = carousel.querySelector("[data-demo-count]");
    if (!slides.length || !tabs.length) return;

    let activeIndex = 0;

    function showSlide(index) {
      activeIndex = (index + slides.length) % slides.length;

      slides.forEach((slide, slideIndex) => {
        const isActive = slideIndex === activeIndex;
        slide.classList.toggle("active", isActive);
        slide.hidden = !isActive;
      });

      tabs.forEach((tab, tabIndex) => {
        const isActive = tabIndex === activeIndex;
        tab.classList.toggle("active", isActive);
        tab.setAttribute("aria-selected", isActive ? "true" : "false");
        tab.tabIndex = isActive ? 0 : -1;
      });

      if (count) {
        count.textContent = `${activeIndex + 1} / ${slides.length}`;
      }
    }

    tabs.forEach((tab) => {
      tab.addEventListener("click", () => {
        showSlide(Number(tab.getAttribute("data-demo-target")));
      });

      tab.addEventListener("keydown", (event) => {
        if (event.key === "ArrowRight") {
          event.preventDefault();
          showSlide(activeIndex + 1);
          tabs[activeIndex].focus();
        } else if (event.key === "ArrowLeft") {
          event.preventDefault();
          showSlide(activeIndex - 1);
          tabs[activeIndex].focus();
        }
      });
    });

    if (prev) {
      prev.addEventListener("click", () => showSlide(activeIndex - 1));
    }

    if (next) {
      next.addEventListener("click", () => showSlide(activeIndex + 1));
    }

    showSlide(activeIndex);
  }

  function initProblemsCarousel() {
    const carousel = document.querySelector(".problems-carousel");
    const track = carousel?.querySelector("[data-problems-track]");
    const dotsContainer = carousel?.querySelector("[data-problems-dots]");
    const prev = carousel?.querySelector("[data-problems-prev]");
    const next = carousel?.querySelector("[data-problems-next]");
    if (!carousel || !track || !dotsContainer) return;

    const cards = Array.from(track.querySelectorAll(".problem-card"));
    if (!cards.length) return;

    let activePage = 0;
    let cardsPerView = 1;
    const GAP_PX = 20;

    function getCardsPerView() {
      if (window.innerWidth < 640) return 1;
      if (window.innerWidth < 1024) return 2;
      return 3;
    }

    function getPageCount() {
      return Math.max(1, Math.ceil(cards.length / cardsPerView));
    }

    function updateDots() {
      dotsContainer.querySelectorAll(".problems-dot").forEach((dot, index) => {
        const isActive = index === activePage;
        dot.classList.toggle("active", isActive);
        dot.setAttribute("aria-selected", isActive ? "true" : "false");
        dot.tabIndex = isActive ? 0 : -1;
      });
    }

    function goToPage(page) {
      const pageCount = getPageCount();
      activePage = Math.max(0, Math.min(page, pageCount - 1));

      const cardWidth = cards[0].getBoundingClientRect().width;
      const offset = activePage * cardsPerView * (cardWidth + GAP_PX);
      track.style.transform = `translateX(-${offset}px)`;
      updateDots();
    }

    function buildDots() {
      dotsContainer.innerHTML = "";
      const pageCount = getPageCount();

      for (let index = 0; index < pageCount; index += 1) {
        const dot = document.createElement("button");
        dot.type = "button";
        dot.className = "problems-dot";
        dot.setAttribute("role", "tab");
        dot.setAttribute("aria-label", `${index + 1} / ${pageCount}`);
        dot.addEventListener("click", () => goToPage(index));
        dotsContainer.appendChild(dot);
      }
    }

    function refresh() {
      cardsPerView = getCardsPerView();
      buildDots();
      goToPage(Math.min(activePage, getPageCount() - 1));
    }

    if (prev) {
      prev.addEventListener("click", () => goToPage(activePage - 1));
    }

    if (next) {
      next.addEventListener("click", () => goToPage(activePage + 1));
    }

    let resizeTimer;
    window.addEventListener("resize", () => {
      window.clearTimeout(resizeTimer);
      resizeTimer = window.setTimeout(refresh, 150);
    });

    refresh();
  }

  document.addEventListener("DOMContentLoaded", () => {
    applyTranslations(currentLang);
    initDownloadReleaseLinks();
    initDemoCarousel();
    initProblemsCarousel();
    initMockupTyping();

    document.querySelectorAll("[data-lang]").forEach((btn) => {
      btn.addEventListener("click", () => setLanguage(btn.getAttribute("data-lang")));
    });

    const menuToggle = document.querySelector(".nav-toggle");
    const navLinks = document.querySelector(".nav-links");
    if (menuToggle && navLinks) {
      menuToggle.addEventListener("click", () => {
        const open = navLinks.classList.toggle("open");
        menuToggle.setAttribute("aria-expanded", open ? "true" : "false");
      });

      navLinks.querySelectorAll("a").forEach((link) => {
        link.addEventListener("click", () => {
          navLinks.classList.remove("open");
          menuToggle.setAttribute("aria-expanded", "false");
        });
      });
    }

    document.querySelectorAll('a[href^="#"]').forEach((anchor) => {
      anchor.addEventListener("click", (e) => {
        const id = anchor.getAttribute("href");
        if (id === "#") return;
        const target = document.querySelector(id);
        if (target) {
          e.preventDefault();
          target.scrollIntoView({ behavior: "smooth", block: "start" });
        }
      });
    });
  });

  window.STRATA_I18N = { setLanguage, RELEASES_URL, RELEASES_API_URL, findLatestCompleteRelease };
})();
