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
      "vibe giss buffer 500m sui comuni…",
      "scarica OSM per Brescia centro…",
      "conta gli alberi nel verde urbano…",
    ],
    en: [
      "vibe giss a 500m buffer on municipalities…",
      "download OSM for downtown Brescia…",
      "count trees in urban green areas…",
    ],
  };

  const translations = {
    it: {
      "meta.title": "Strata — vibe gissing per le tue mappe",
      "meta.description":
        "Strata — inizia a fare vibe gissing. GIS agent-native: descrivi cosa vuoi sulla mappa, l'agente ispeziona i layer ed esegue PyQGIS. Fork di QGIS.", // #spellok

      "nav.problems": "Problemi",
      "nav.features": "Feature",
      "nav.demo": "Demo",
      "nav.setup": "Setup",
      "nav.download": "Download",
      "nav.downloadBtn": "Scarica",

      "hero.badge": "Agent-native GIS · Fork di QGIS",
      "hero.titleLine": "Inizia anche tu a fare",
      "hero.titleHighlight": "vibe gissing.",
      "hero.subtitle":
        "Descrivi cosa vuoi sulla mappa. L'agente ispeziona i layer, esegue PyQGIS e propone — tu approvi.",
      "hero.pillPlan": "Plan",
      "hero.pillAgent": "Agent",
      "hero.pillAsk": "Ask",
      "hero.pillTools": "19 tool GIS",
      "hero.ctaPrimary": "Scarica e inizia",
      "hero.ctaSecondary": "Guarda la demo",

      "vibe.title": "Cos'è il vibe gissing?",
      "vibe.step1.title": "Parla alla mappa",
      "vibe.step1.desc":
        "Descrivi l'intento in linguaggio naturale — buffer, download dati, analisi layer. Niente script da scrivere a mano.",
      "vibe.step2.title": "L'agente agisce",
      "vibe.step2.desc":
        "Plan, Agent o Ask: ispeziona il progetto, esegue PyQGIS e usa 19 tool GIS integrati nel workspace.",
      "vibe.step3.title": "Tu approvi",
      "vibe.step3.desc":
        "Review diff side-by-side prima di salvare. Nessuna modifica al progetto senza il tuo ok.",

      "mockup.mapLabel": "Mappa GIS",
      "mockup.chatTitle": "AI Assistant",
      "mockup.modePlan": "Plan",
      "mockup.modeAgent": "Agent",
      "mockup.modeAsk": "Ask",
      "mockup.userMsg": "Fai buffer 500m sui comuni e esporta GeoJSON",
      "mockup.aiMsg":
        "Ho ispezionato il layer comuni, eseguito il buffer e salvato output/buffer_500m.geojson. Vuoi rivedere la proposta?",
      "mockup.input": "vibe giss buffer 500m sui comuni…",

      "problems.label": "Problemi comuni",
      "problems.title": "Conosci questi ostacoli in QGIS?",
      "problems.subtitle":
        "Workflow GIS spesso richiedono script PyQGIS, ricerca forum e copia-incolla da chat generiche. Strata risolve il gap tra domanda e azione nel tuo progetto.",

      "problems.1.title": "PyQGIS difficile",
      "problems.1.desc":
        "Copiare snippet da ChatGPT senza contesto del progetto. Errori di sintassi, API sbagliate, ore perse.",
      "problems.1.fix": "L'agente esegue PyQGIS in-session con dialog di approvazione.",

      "problems.2.title": "Ispezione layer laboriosa",
      "problems.2.desc":
        "Capire campi, CRS e estensioni richiede script manuali o round-trip nella console Python.",
      "problems.2.fix": "Tool nativi describe_layer e list_project_layers.",

      "problems.3.title": "Dipendenze Python mancanti",
      "problems.3.desc":
        "geopandas, osmnx, pandas… spesso non installati nell'ambiente QGIS.",
      "problems.3.fix": "install_python_package con approvazione pip controllata.",

      "problems.4.title": "Scaricare dati è lento",
      "problems.4.desc":
        "Overpass, GeoJSON, GADM — fetch manuale, salvataggio, import layer.",
      "problems.4.fix": "download_file direttamente nel workspace con una sola approval.",

      "problems.5.title": "Contesto disperso",
      "problems.5.desc":
        "File di progetto, script, layer e attributi sparsi — l'AI generica non li vede.",
      "problems.5.fix": "RAG workspace, @ file mentions e snapshot layer nel prompt.",

      "problems.6.title": "Modifiche senza controllo",
      "problems.6.desc":
        "Script che sovrascrivono file o eseguono codice pericoloso senza review.",
      "problems.6.fix": "Review diff side-by-side + approval PyQGIS e pip.",

      "problems.7.title": "Plugin AI disconnessi",
      "problems.7.desc":
        "Assistenti esterni non conoscono i tuoi layer attivi né il Processing Toolbox.", // #spellok
      "problems.7.fix": "Integrazione nativa in Strata.",

      "solution.label": "Soluzione",
      "solution.title": "Chat laterale + Agente integrato",
      "solution.subtitle":
        "Il modo agent-native di lavorare in QGIS: descrivi, l'agente propone, tu approvi — senza uscire dall'app.",

      "solution.chat.title": "Pannello AI Assistant",
      "solution.chat.desc":
        "Dock widget a destra con streaming delle risposte, cronologia chat, attach file, tag @ per menzionare file del workspace e Review Proposals per accettare o rifiutare modifiche.",
      "solution.chat.f1": "Streaming in tempo reale",
      "solution.chat.f2": "History per workspace",
      "solution.chat.f3": "Review Proposals con diff",

      "solution.agent.title": "Tre modalità agente",
      "solution.agent.desc":
        "Stile Cursor: Plan per pianificare, Agent per eseguire e modificare, Ask per domande e review. Rules & Skills personalizzabili.",
      "solution.agent.f1": "Plan · Agent · Ask",
      "solution.agent.f2": "19 tool GIS integrati",
      "solution.agent.f3": "Rules & Skills (.strata/)",

      "demo.label": "Demo",
      "demo.title": "Vedilo in azione",
      "demo.subtitle":
        "Due modalità reali di Strata: l'agente esegue PyQGIS sul progetto, Plan propone un piano approvabile prima di toccare i layer.",
      "demo.carousel.label": "Esempi interattivi di Strata",
      "demo.tabs.label": "Scegli un esempio demo",
      "demo.prev": "Esempio precedente",
      "demo.next": "Esempio successivo",
      "demo.agent.tab": "Agent",
      "demo.agent.kicker": "Esecuzione controllata",
      "demo.agent.title": "Agent mode — esecuzione sul progetto",
      "demo.agent.desc":
        "Chiedi in linguaggio naturale (es. «crea maschera esterna»): l'assistant ispeziona i layer, esegue run_python e mostra il risultato sulla mappa.",
      "demo.agent.alt": "Strata in Agent mode con pannello AI accanto alla mappa GIS",
      "demo.plan.tab": "Plan",
      "demo.plan.kicker": "Piano prima dell'azione",
      "demo.plan.title": "Plan mode — piano prima dell'azione",
      "demo.plan.desc":
        "Descrivi un obiettivo complesso (es. censimento alberi): Strata propone step strutturati e pulsanti Accept plan / Reject / revise prima di modificare il progetto.",
      "demo.plan.alt": "Strata in Plan mode con piano approvabile prima delle modifiche",
      "demo.maps.tab": "Analisi mappe",
      "demo.maps.kicker": "Domande sui layer reali",
      "demo.maps.title": "Analisi mappe — verde urbano e alberi",
      "demo.maps.desc":
        "L'assistente legge i layer del progetto, unisce geometrie sovrapposte, calcola superfici, conta gli alberi e restituisce una risposta verificabile direttamente accanto alla mappa.",
      "demo.maps.alt": "Analisi Strata su mappa di Brescia con risposta AI su verde urbano e alberi",

      "speed.label": "Vibe gissing in pratica",
      "speed.title": "Da ore a minuti",
      "speed.subtitle": "Esempio reale: ispeziona 5 layer, buffer, export GeoJSON.",
      "speed.before": "Prima",
      "speed.beforeTime": "~45 min",
      "speed.beforeSteps":
        "Aprire console Python, scrivere script per ogni layer, cercare API su StackExchange, installare dipendenze, debuggare path, export manuale.",
      "speed.after": "Con Strata",
      "speed.afterTime": "~5 min",
      "speed.afterSteps":
        "Una richiesta in linguaggio naturale. L'agente ispeziona, propone codice, chiede approval, esegue e salva nel workspace.",

      "features.label": "Feature",
      "features.title": "Tutto ciò che serve, integrato",
      "features.subtitle":
        "Potenza di QGIS 4.1 più un assistente che conosce il tuo progetto.", // #spellok

      "features.1.title": "19 tool integrati",
      "features.1.desc":
        "read_file, search_files, describe_layer, propose_edit, run_python, pip install, download_file, reindex e altro.",

      "features.2.title": "Rules & Skills",
      "features.2.desc":
        "Regole inline e cartelle .strata/rules e .strata/skills per guidare l'agente come in Cursor.",

      "features.3.title": "RAG workspace",
      "features.3.desc":
        "Indicizza .py, .qgs, .geojson, .md con embeddings OpenAI. Chunk e vettori in SQLite locale.",

      "features.4.title": "Layer indexing opt-in",
      "features.4.desc":
        "Attributi e bounding box inviati a embeddings solo con consenso esplicito. Auto-reindex su edit layer.",

      "features.5.title": "Multi-provider",
      "features.5.desc":
        "OpenAI API key, Claude API/OAuth, Codex/ChatGPT OAuth device flow. Scegli il modello dal dropdown.",

      "features.6.title": "Privacy locale",
      "features.6.desc":
        "Chiavi in QgsSettings locali. Token OAuth cifrati nell'auth store QGIS. Nessun server intermedio.",

      "setup.label": "Setup",
      "setup.title": "Credenziali e indicizzazione",
      "setup.subtitle":
        "Configura l'AI al primo avvio e capisci come funziona il RAG sul tuo workspace.",

      "setup.creds.title": "Credenziali e login",
      "setup.creds.1":
        "Apri Strata → menu Visualizza → Pannelli → AI Assistant.",
      "setup.creds.2": "Clicca sull'icona ⚙ impostazioni in alto a destra del pannello.",
      "setup.creds.3.title": "Scegli il provider:",
      "setup.creds.3.openai":
        "OpenAI / Anthropic: incolla la API key (platform.openai.com o console.anthropic.com).",
      "setup.creds.3.codex":
        "Codex/ChatGPT: Get Codex device code → apri la pagina → Complete Codex login.",
      "setup.creds.3.claude":
        "Claude OAuth: attiva Use Claude OAuth → Login with Claude → incolla il codice.",
      "setup.creds.4":
        "Seleziona il modello dal dropdown (GPT-4o, Claude Sonnet, Codex GPT-5.4, …).",
      "setup.creds.5":
        "Opzionale: abilita Allow custom agent actions (tool use) — default OFF per sicurezza.",

      "setup.index.title": "Indicizzazione (RAG)",
      "setup.index.1":
        "Workspace RAG: indicizza automaticamente i file testo del workspace. Chunk e vettori salvati in SQLite locale. La ricerca semantica viene iniettata nel prompt come Retrieved context.",
      "setup.index.2":
        "Layer indexing: disabilitato di default. Con Enable layer indexing, attributi e bounding box vengono inviati a OpenAI text-embedding-3-small. Auto-reindex su add/remove/edit layer.",
      "setup.index.3":
        "Requisito: serve una API key OpenAI (o variabile OPENAI_API_KEY) per ricostruire gli embeddings.",
      "setup.index.4":
        "Stato visibile nel dialog ⚙: Indexed: N file chunks, M layer chunks (last sync: …).", // #spellok
      "setup.index.privacy":
        "I dati dei layer vengono inviati al provider embeddings solo se abiliti esplicitamente l'indexing.",

      "download.label": "Download",
      "download.title": "Scarica Strata",
      "download.subtitle":
        "Binari precompilati da GitHub Releases. macOS e Windows sono firmati; Linux AppImage richiede ancora verifica manuale.",
      "download.cta": "Vai alle Releases",
      "download.status.fallback": "Releases GitHub",
      "download.status.complete": "Ultima release completa: {version}",
      "download.macos.title": "macOS",
      "download.macos.sub": "Intel + Apple Silicon",
      "download.macos.file": "File .dmg",
      "download.macos.note":
        "Build universal firmata e notarizzata. Trascina Strata in Applicazioni e avvia.",
      "download.windows.title": "Windows",
      "download.windows.sub": "10/11 x64",
      "download.windows.file": "File -win64.exe",
      "download.windows.note":
        "Installer NSIS firmato Authenticode. Verifica il publisher; la reputazione SmartScreen può richiedere tempo.",
      "download.linux.title": "Linux",
      "download.linux.sub": "x86_64",
      "download.linux.file": "File .AppImage",
      "download.linux.note":
        "chmod +x Strata-*.AppImage && ./Strata-*.AppImage. Ubuntu 22.04+, glibc ≥ 2.35.",

      "footer.tagline": "Vibe gissing, nativamente in QGIS.", // #spellok
      "footer.disclaimer":
        "Strata is an independent, unofficial fork based on QGIS. It is not endorsed by or affiliated with QGIS.ORG or the QGIS project.",
      "footer.github": "GitHub",
      "footer.star": "Metti una stella",
      "footer.license": "Licenza GPLv2+",
      "footer.contact": "Contatto",
    },
    en: {
      "meta.title": "Strata — vibe gissing for your maps",
      "meta.description":
        "Strata — start vibe gissing. Agent-native GIS: describe what you want on the map, the agent inspects layers and runs PyQGIS. A QGIS fork.",

      "nav.problems": "Problems",
      "nav.features": "Features",
      "nav.demo": "Demo",
      "nav.setup": "Setup",
      "nav.download": "Download",
      "nav.downloadBtn": "Download",

      "hero.badge": "Agent-native GIS · QGIS fork",
      "hero.titleLine": "",
      "hero.titleHighlight": "Start vibe gissing your maps.",
      "hero.subtitle":
        "Describe what you want on the map. The agent inspects layers, runs PyQGIS, and proposes — you approve.",
      "hero.pillPlan": "Plan",
      "hero.pillAgent": "Agent",
      "hero.pillAsk": "Ask",
      "hero.pillTools": "19 GIS tools",
      "hero.ctaPrimary": "Download and start",
      "hero.ctaSecondary": "See it in action",

      "vibe.title": "What is vibe gissing?",
      "vibe.step1.title": "Talk to your map",
      "vibe.step1.desc":
        "Describe your intent in plain language — buffer, fetch data, layer analysis. No scripts to write by hand.",
      "vibe.step2.title": "The agent acts",
      "vibe.step2.desc":
        "Plan, Agent, or Ask: inspects your project, runs PyQGIS, and uses 19 built-in GIS tools in the workspace.",
      "vibe.step3.title": "You approve",
      "vibe.step3.desc":
        "Side-by-side diff review before saving. No project changes without your OK.",

      "mockup.mapLabel": "GIS map",
      "mockup.chatTitle": "AI Assistant",
      "mockup.modePlan": "Plan",
      "mockup.modeAgent": "Agent",
      "mockup.modeAsk": "Ask",
      "mockup.userMsg": "Buffer municipalities by 500m and export GeoJSON",
      "mockup.aiMsg":
        "I inspected the municipalities layer, ran the buffer, and saved output/buffer_500m.geojson. Review the proposal?",
      "mockup.input": "vibe giss a 500m buffer on municipalities…",

      "problems.label": "Common problems",
      "problems.title": "Sound familiar in QGIS?",
      "problems.subtitle":
        "GIS workflows often mean PyQGIS scripts, forum searches, and copy-paste from generic chatbots. Strata closes the gap between question and action in your project.",

      "problems.1.title": "PyQGIS is hard",
      "problems.1.desc":
        "Copying snippets from ChatGPT without project context. Syntax errors, wrong APIs, hours lost.",
      "problems.1.fix": "The agent runs PyQGIS in-session with an approval dialog.",

      "problems.2.title": "Layer inspection is tedious",
      "problems.2.desc":
        "Understanding fields, CRS, and extents means manual scripts or Python console round-trips.",
      "problems.2.fix": "Native describe_layer and list_project_layers tools.",

      "problems.3.title": "Missing Python deps",
      "problems.3.desc":
        "geopandas, osmnx, pandas… often not installed in the QGIS environment.",
      "problems.3.fix": "install_python_package with controlled pip approval.",

      "problems.4.title": "Fetching data is slow",
      "problems.4.desc":
        "Overpass, GeoJSON, GADM — manual fetch, save, import layer.",
      "problems.4.fix": "download_file straight into the workspace with one approval.",

      "problems.5.title": "Scattered context",
      "problems.5.desc":
        "Project files, scripts, layers, and attributes everywhere — generic AI can't see them.",
      "problems.5.fix": "Workspace RAG, @ file mentions, and layer snapshots in the prompt.",

      "problems.6.title": "Uncontrolled edits",
      "problems.6.desc":
        "Scripts overwriting files or running risky code without review.",
      "problems.6.fix": "Side-by-side diff review + PyQGIS and pip approval.",

      "problems.7.title": "Disconnected AI plugins",
      "problems.7.desc":
        "External assistants don't know your active layers or Processing Toolbox.",
      "problems.7.fix": "Native integration in Strata.",

      "solution.label": "Solution",
      "solution.title": "Side chat + integrated agent",
      "solution.subtitle":
        "The agent-native way to work in QGIS: describe, the agent proposes, you approve — without leaving the app.",

      "solution.chat.title": "AI Assistant panel",
      "solution.chat.desc":
        "Right-side dock with streaming replies, chat history, file attach, @ tags for workspace files, and Review Proposals to accept or reject changes.",
      "solution.chat.f1": "Real-time streaming",
      "solution.chat.f2": "Per-workspace history",
      "solution.chat.f3": "Review Proposals with diff",

      "solution.agent.title": "Three agent modes",
      "solution.agent.desc":
        "Cursor-style: Plan to strategize, Agent to execute and edit, Ask for Q&A and review. Customizable Rules & Skills.",
      "solution.agent.f1": "Plan · Agent · Ask",
      "solution.agent.f2": "19 built-in GIS tools",
      "solution.agent.f3": "Rules & Skills (.strata/)",

      "demo.label": "Demo",
      "demo.title": "See it in action",
      "demo.subtitle":
        "Two real Strata modes: the agent runs PyQGIS on your project; Plan proposes an approvable plan before touching layers.",
      "demo.carousel.label": "Interactive Strata examples",
      "demo.tabs.label": "Choose a demo example",
      "demo.prev": "Previous example",
      "demo.next": "Next example",
      "demo.agent.tab": "Agent",
      "demo.agent.kicker": "Controlled execution",
      "demo.agent.title": "Agent mode — execution on your project",
      "demo.agent.desc":
        "Ask in plain language (e.g. \"create external mask\"): the assistant inspects layers, runs run_python, and shows the result on the map.",
      "demo.agent.alt": "Strata in Agent mode with the AI panel next to the GIS map",
      "demo.plan.tab": "Plan",
      "demo.plan.kicker": "Plan before action",
      "demo.plan.title": "Plan mode — plan before action",
      "demo.plan.desc":
        "Describe a complex goal (e.g. tree inventory): Strata proposes structured steps and Accept plan / Reject / revise buttons before changing the project.",
      "demo.plan.alt": "Strata in Plan mode with an approvable plan before changes",
      "demo.maps.tab": "Map analysis",
      "demo.maps.kicker": "Questions over real layers",
      "demo.maps.title": "Map analysis — urban green and trees",
      "demo.maps.desc":
        "The assistant reads project layers, dissolves overlapping geometries, calculates surfaces, counts trees, and returns a verifiable answer right beside the map.",
      "demo.maps.alt": "Strata analysis over a Brescia map with an AI answer about urban green and trees",

      "speed.label": "Vibe gissing in practice",
      "speed.title": "From hours to minutes",
      "speed.subtitle": "Real example: inspect 5 layers, buffer, export GeoJSON.",
      "speed.before": "Before",
      "speed.beforeTime": "~45 min",
      "speed.beforeSteps":
        "Open Python console, write scripts per layer, search StackExchange for APIs, install deps, debug paths, manual export.",
      "speed.after": "With Strata",
      "speed.afterTime": "~5 min",
      "speed.afterSteps":
        "One natural-language request. The agent inspects, proposes code, asks approval, runs, and saves to the workspace.",

      "features.label": "Features",
      "features.title": "Everything built in",
      "features.subtitle":
        "Full QGIS 4.1 power plus an assistant that knows your project.",

      "features.1.title": "19 integrated tools",
      "features.1.desc":
        "read_file, search_files, describe_layer, propose_edit, run_python, pip install, download_file, reindex, and more.",

      "features.2.title": "Rules & Skills",
      "features.2.desc":
        "Inline rules and .strata/rules and .strata/skills folders to steer the agent like Cursor.",

      "features.3.title": "Workspace RAG",
      "features.3.desc":
        "Indexes .py, .qgs, .geojson, .md with OpenAI embeddings. Chunks and vectors stored locally in SQLite.",

      "features.4.title": "Opt-in layer indexing",
      "features.4.desc":
        "Attributes and bounding boxes sent to embeddings only with explicit consent. Auto-reindex on layer edits.",

      "features.5.title": "Multi-provider",
      "features.5.desc":
        "OpenAI API key, Claude API/OAuth, Codex/ChatGPT OAuth device flow. Pick your model from the dropdown.",

      "features.6.title": "Local privacy",
      "features.6.desc":
        "Keys in local QgsSettings. OAuth tokens encrypted in QGIS auth store. No intermediary server.",

      "setup.label": "Setup",
      "setup.title": "Credentials and indexing",
      "setup.subtitle":
        "Configure AI on first launch and understand how RAG works on your workspace.",

      "setup.creds.title": "Credentials and login",
      "setup.creds.1":
        "Open Strata → View → Panels → AI Assistant.",
      "setup.creds.2": "Click the ⚙ settings icon at the top-right of the panel.",
      "setup.creds.3.title": "Choose a provider:",
      "setup.creds.3.openai":
        "OpenAI / Anthropic: paste your API key (platform.openai.com or console.anthropic.com).",
      "setup.creds.3.codex":
        "Codex/ChatGPT: Get Codex device code → open the page → Complete Codex login.",
      "setup.creds.3.claude":
        "Claude OAuth: enable Use Claude OAuth → Login with Claude → paste the code.",
      "setup.creds.4":
        "Select a model from the dropdown (GPT-4o, Claude Sonnet, Codex GPT-5.4, …).",
      "setup.creds.5":
        "Optional: enable Allow custom agent actions (tool use) — OFF by default for safety.",

      "setup.index.title": "Indexing (RAG)",
      "setup.index.1":
        "Workspace RAG: automatically indexes text files in the workspace. Chunks and vectors stored in local SQLite. Semantic search is injected into the prompt as Retrieved context.",
      "setup.index.2":
        "Layer indexing: disabled by default. With Enable layer indexing, attributes and bounding boxes are sent to OpenAI text-embedding-3-small. Auto-reindex on add/remove/edit layer.",
      "setup.index.3":
        "Requirement: an OpenAI API key (or OPENAI_API_KEY env var) is needed to rebuild embeddings.",
      "setup.index.4":
        "Status visible in the ⚙ dialog: Indexed: N file chunks, M layer chunks (last sync: …).",
      "setup.index.privacy":
        "Layer data is sent to the embeddings provider only if you explicitly enable indexing.",

      "download.label": "Download",
      "download.title": "Download Strata",
      "download.subtitle":
        "Prebuilt binaries from GitHub Releases. macOS and Windows are signed; Linux AppImage still requires manual verification.",
      "download.cta": "Go to Releases",
      "download.status.fallback": "GitHub Releases",
      "download.status.complete": "Latest complete release: {version}",
      "download.macos.title": "macOS",
      "download.macos.sub": "Intel + Apple Silicon",
      "download.macos.file": ".dmg file",
      "download.macos.note":
        "Signed and notarized universal build. Drag Strata to Applications and launch.",
      "download.windows.title": "Windows",
      "download.windows.sub": "10/11 x64",
      "download.windows.file": "-win64.exe file",
      "download.windows.note":
        "Signed Authenticode NSIS installer. Verify the publisher; SmartScreen reputation can take time to warm up.",
      "download.linux.title": "Linux",
      "download.linux.sub": "x86_64",
      "download.linux.file": ".AppImage file",
      "download.linux.note":
        "chmod +x Strata-*.AppImage && ./Strata-*.AppImage. Ubuntu 22.04+, glibc ≥ 2.35.",

      "footer.tagline": "Vibe gissing, natively in QGIS.",
      "footer.disclaimer":
        "Strata is an independent, unofficial fork based on QGIS. It is not endorsed by or affiliated with QGIS.ORG or the QGIS project.",
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

  document.addEventListener("DOMContentLoaded", () => {
    applyTranslations(currentLang);
    initDownloadReleaseLinks();
    initDemoCarousel();
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
