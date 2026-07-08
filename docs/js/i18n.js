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
        "Strata porta modelli linguistici avanzati dentro QGIS. Piano Pro chiavi in mano, Cloud Team per la collaborazione, Enterprise on-premise con AI locale.",

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
      "hero.wordmark": "Pro · Cloud Team · Enterprise",
      "hero.subtitle":
        "Porta la potenza dei modelli linguistici avanzati dentro QGIS. Scegli la comodità del nostro Cloud o la sicurezza totale di un'infrastruttura 100% locale e on-premise.",
      "hero.pillPlan": "Plan",
      "hero.pillAgent": "Agent",
      "hero.pillAsk": "Ask",
      "hero.pillTools": "19 tool GIS integrati",
      "hero.ctaPrimary": "Inizia Subito",
      "hero.ctaSecondary": "Contatta le vendite",

      "plans.label": "Piani e soluzioni",
      "plans.title": "Scegli come adottare Strata",
      "plans.subtitle":
        "Dalla attivazione immediata alla governance enterprise. Un unico agente AI, tre modalità di deployment.",
      "plans.pro.badge": "Pro",
      "plans.pro.title": "Pro — Chiavi in mano",
      "plans.pro.slogan": "Attiva e usa, zero configurazioni.",
      "plans.pro.desc":
        "Dimentica la gestione delle API key, i crediti OpenAI e i limiti di fatturazione. Con il piano Pro hai un assistente AI nativo in QGIS, pronto all'uso con un unico abbonamento mensile. Ci occupiamo noi di connetterti ai modelli più veloci e avanzati sul mercato.",
      "plans.team.badge": "Team",
      "plans.team.title": "Cloud Team — Collaborazione",
      "plans.team.slogan": "La conoscenza geografica del tuo team, centralizzata.",
      "plans.team.desc":
        "Condividi il contesto. Strata Cloud indicizza in modo sicuro i file di progetto, i layer e gli script del team. L'agente AI apprende dalle mappe della tua organizzazione per offrire risposte precise e coerenti a tutti i membri, semplificando la collaborazione su progetti GIS complessi.",
      "plans.team.cta": "Accedi alla Strata Cloud →",
      "plans.enterprise.badge": "Ent.",
      "plans.enterprise.title": "Enterprise — Privacy totale & AI locale",
      "plans.enterprise.slogan": "I tuoi dati geografici non lasciano mai la tua rete.",
      "plans.enterprise.desc":
        "Pensato per organizzazioni con severi requisiti di compliance, riservatezza e data governance. Strata Enterprise funziona interamente on-premise: il software GIS e l'agente AI, basato su modelli LLM open-source ottimizzati come Llama o Mistral, girano localmente sui server aziendali o su workstation dedicate. Nessun dato inviato a terze parti, massima sovranità sul dato e pieno controllo dell'infrastruttura.",

      "privacy.label": "Sicurezza e privacy",
      "privacy.title": "La tua mappa, i tuoi dati. Nessun compromesso sulla sicurezza.",
      "privacy.subtitle":
        "Per enti pubblici, utility e organizzazioni che gestiscono dati territoriali sensibili. Strata Enterprise mantiene mappe, layer e documenti GIS all'interno della tua infrastruttura.",
      "privacy.1.title": "AI 100% offline",
      "privacy.1.desc":
        "Supporto nativo per LLM locali. Esegui analisi complesse sul territorio sfruttando la potenza dell'hardware aziendale, senza connessione internet attiva.",
      "privacy.2.title": "Isolamento dei dati",
      "privacy.2.desc":
        "L'indicizzazione dei layer e dei documenti GIS avviene su database cifrati all'interno della tua infrastruttura.",
      "privacy.3.title": "Conformità normativa",
      "privacy.3.desc":
        "La soluzione ideale per Pubblica Amministrazione, utility, contractor militari e organizzazioni infrastrutturali che gestiscono dati sensibili non esportabili all'estero.",
      "privacy.closing":
        "Strata Enterprise: zero egress, audit completo, pieno controllo su modelli, policy e infrastruttura.",

      "mockup.mapLabel": "Mappa GIS",
      "mockup.chatTitle": "AI Assistant",
      "mockup.modePlan": "Plan",
      "mockup.modeAgent": "Agent",
      "mockup.modeAsk": "Ask",
      "mockup.userMsg": "Fai buffer 500m sui comuni e esporta GeoJSON",
      "mockup.aiMsg":
        "Ho ispezionato il layer comuni, eseguito il buffer e salvato output/buffer_500m.geojson. Vuoi rivedere la proposta?",
      "mockup.input": "buffer 500m sui comuni ed esporta GeoJSON…",

      "problems.label": "Problemi comuni",
      "problems.title": "Conosci questi ostacoli in QGIS?",
      "problems.subtitle":
        "Workflow GIS spesso richiedono script PyQGIS, ricerca forum e copia-incolla da chat generiche. Strata risolve il gap tra domanda e azione nel tuo progetto.",
      "problems.carousel.label": "Carousel problemi comuni",
      "problems.prev": "Problema precedente",
      "problems.next": "Problema successivo",

      "problems.1.title": "PyQGIS difficile",
      "problems.1.desc":
        "Copiare snippet da ChatGPT senza contesto del progetto. Errori di sintassi, API sbagliate, ore perse.",
      "problems.1.fix": "Strata esegue il codice per te, con la tua approvazione.",

      "problems.2.title": "Ispezione layer laboriosa",
      "problems.2.desc":
        "Capire campi, CRS e estensioni richiede script manuali o round-trip nella console Python.",
      "problems.2.fix": "Legge layer e campi direttamente dal tuo progetto.",

      "problems.3.title": "Dipendenze Python mancanti",
      "problems.3.desc":
        "geopandas, osmnx, pandas… spesso non installati nell'ambiente QGIS.",
      "problems.3.fix": "Installa le librerie mancanti con un clic, sempre sotto il tuo controllo.",

      "problems.4.title": "Scaricare dati è lento",
      "problems.4.desc":
        "Overpass, GeoJSON, GADM — fetch manuale, salvataggio, import layer.",
      "problems.4.fix": "Scarica e importa i dati in un solo passaggio.",

      "problems.5.title": "Contesto disperso",
      "problems.5.desc":
        "File di progetto, script, layer e attributi sparsi — l'AI generica non li vede.",
      "problems.5.fix": "Usa i file e i layer del progetto come contesto per le risposte.",

      "problems.6.title": "Modifiche senza controllo",
      "problems.6.desc":
        "Script che sovrascrivono file o eseguono codice pericoloso senza review.",
      "problems.6.fix": "Ogni modifica va approvata prima di essere applicata.",

      "problems.7.title": "Plugin AI disconnessi",
      "problems.7.desc":
        "Assistenti esterni non conoscono i tuoi layer attivi né il Processing Toolbox.", // #spellok
      "problems.7.fix": "L'assistente è nativo in QGIS e conosce il tuo progetto.",

      "solution.label": "Soluzione",
      "solution.title": "Chiedi, approva, fatto",
      "solution.subtitle":
        "Descrivi cosa ti serve. Strata propone, tu decidi — senza uscire da QGIS.",

      "solution.chat.title": "Chat accanto alla mappa",
      "solution.chat.desc":
        "Un pannello laterale dove chiedi in italiano, allegui file e vedi subito le risposte mentre lavori sulla mappa.",
      "solution.chat.f1": "Risposte in tempo reale",
      "solution.chat.f2": "Cronologia salvata per progetto",
      "solution.chat.f3": "Modifiche da approvare prima di applicarle",

      "solution.agent.title": "Tre modalità, una sola app",
      "solution.agent.desc":
        "Pianifica un intervento, eseguilo sul progetto o fai solo domande. Scegli il livello di autonomia che preferisci.",
      "solution.agent.f1": "Pianifica · Esegui · Chiedi",
      "solution.agent.f2": "Azioni GIS integrate",
      "solution.agent.f3": "Personalizza come lavora l'assistente",

      "demo.label": "Demo",
      "demo.title": "Vedilo in azione",
      "demo.subtitle":
        "Guarda come Strata risponde a richieste reali su progetti GIS, con approvazione prima di ogni modifica.",
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

      "speed.label": "Produttività GIS",
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

      "features.label": "Vantaggi",
      "features.title": "Tutto integrato, zero complessità",
      "features.subtitle":
        "L'assistente lavora dentro QGIS con i tuoi dati, senza uscire dall'app.",

      "features.1.title": "Fa il lavoro al posto tuo",
      "features.1.desc":
        "Buffer, export, analisi, download dati: chiedi in linguaggio naturale e Strata si occupa del resto.",

      "features.2.title": "Si adatta al tuo modo di lavorare",
      "features.2.desc":
        "Imposta regole e preferenze così l'assistente risponde come il tuo team si aspetta.",

      "features.3.title": "Conosce i tuoi file",
      "features.3.desc":
        "Script, progetti e documenti del workspace sono disponibili come contesto per le risposte.",

      "features.4.title": "I layer solo quando vuoi",
      "features.4.desc":
        "Condividi attributi e geometrie con l'AI solo se lo decidi tu, con pieno controllo.",

      "features.5.title": "AI già inclusa",
      "features.5.desc":
        "Con il piano Pro non serve gestire chiavi API o crediti: è tutto pronto.",

      "features.6.title": "I tuoi dati, le tue regole",
      "features.6.desc":
        "In azienda puoi tenere tutto in locale. Nel cloud decidi tu cosa condividere.",

      "start.label": "Come iniziare",
      "start.title": "Pronto in pochi minuti",
      "start.subtitle":
        "Nessuna configurazione complicata. Scarica, apri il tuo progetto QGIS e inizia subito.",
      "start.1.title": "Scarica Strata",
      "start.1.desc":
        "Installa su Mac, Windows o Linux. È QGIS con l'assistente AI già dentro.",
      "start.2.title": "Apri il tuo progetto",
      "start.2.desc":
        "Layer, mappe e file che usi ogni giorno. L'AI li vede e li capisce.",
      "start.3.title": "Chiedi cosa ti serve",
      "start.3.desc":
        "Scrivi in italiano cosa vuoi fare. Strata propone, tu approvi, lui esegue.",
      "start.cta.download": "Scarica gratis",
      "start.cta.cloud": "Prova Strata Cloud",

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

      "team.label": "Team",
      "team.title": "Chi c'è dietro Strata",
      "team.subtitle": "Costruiamo il prodotto e conosciamo il dominio GIS.",
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
        "Strata è costruito sul cuore open-source di QGIS. Questo garantisce piena compatibilità con i tuoi flussi GIS esistenti e ti protegge dal vincolo di formati proprietari. Paghi per l'infrastruttura cloud, la semplicità d'uso e la nostra esperienza nell'ottimizzazione dei modelli AI applicati alla cartografia, mantenendo la trasparenza e la solidità del software libero.",
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
        "Strata brings advanced language models into QGIS. Pro plan ready to use, Cloud Team for collaboration, Enterprise on-premise with local AI.",

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
      "hero.wordmark": "Pro · Cloud Team · Enterprise",
      "hero.subtitle":
        "Bring advanced language models into QGIS. Choose the convenience of our managed Cloud or the full security of a 100% local, on-premise infrastructure.",
      "hero.pillPlan": "Plan",
      "hero.pillAgent": "Agent",
      "hero.pillAsk": "Ask",
      "hero.pillTools": "19 integrated GIS tools",
      "hero.ctaPrimary": "Get Started",
      "hero.ctaSecondary": "Contact sales",

      "plans.label": "Plans & solutions",
      "plans.title": "Choose how to adopt Strata",
      "plans.subtitle":
        "From instant activation to enterprise governance. One AI agent, three deployment modes.",
      "plans.pro.badge": "Pro",
      "plans.pro.title": "Pro — Turnkey",
      "plans.pro.slogan": "Activate and use, zero configuration.",
      "plans.pro.desc":
        "Forget managing API keys, OpenAI credits, and billing limits. With Pro you get a native AI assistant in QGIS, ready to use with a single monthly subscription. We connect you to the fastest, most advanced models on the market.",
      "plans.team.badge": "Team",
      "plans.team.title": "Cloud Team — Collaboration",
      "plans.team.slogan": "Your team's geographic knowledge, centralized.",
      "plans.team.desc":
        "Share context. Strata Cloud securely indexes your team's project files, layers, and scripts. The AI agent learns from your organization's maps to deliver precise, consistent answers for every member, simplifying collaboration on complex GIS projects.",
      "plans.team.cta": "Log in to Strata Cloud →",
      "plans.enterprise.badge": "Ent.",
      "plans.enterprise.title": "Enterprise — Total privacy & local AI",
      "plans.enterprise.slogan": "Your geographic data never leaves your network.",
      "plans.enterprise.desc":
        "Built for organizations with strict compliance, confidentiality, and data governance requirements. Strata Enterprise runs entirely on-premise: the GIS software and AI agent, powered by optimized open-source LLMs like Llama or Mistral, run locally on company servers or dedicated workstations. No data sent to third parties, full data sovereignty and infrastructure control.",

      "privacy.label": "Security & privacy",
      "privacy.title": "Your map, your data. No compromise on security.",
      "privacy.subtitle":
        "For public agencies, utilities, and organizations handling sensitive territorial data. Strata Enterprise keeps maps, layers, and GIS documents inside your infrastructure.",
      "privacy.1.title": "100% offline AI",
      "privacy.1.desc":
        "Native support for local LLMs. Run complex territorial analysis using your company hardware, with no active internet connection required.",
      "privacy.2.title": "Data isolation",
      "privacy.2.desc":
        "Layer and GIS document indexing runs on encrypted databases within your infrastructure.",
      "privacy.3.title": "Regulatory compliance",
      "privacy.3.desc":
        "The ideal solution for public administration, utilities, military contractors, and infrastructure organizations managing sensitive data that cannot be exported abroad.",
      "privacy.closing":
        "Strata Enterprise: zero egress, full audit trail, complete control over models, policies, and infrastructure.",

      "mockup.mapLabel": "GIS map",
      "mockup.chatTitle": "AI Assistant",
      "mockup.modePlan": "Plan",
      "mockup.modeAgent": "Agent",
      "mockup.modeAsk": "Ask",
      "mockup.userMsg": "Buffer municipalities by 500m and export GeoJSON",
      "mockup.aiMsg":
        "I inspected the municipalities layer, ran the buffer, and saved output/buffer_500m.geojson. Review the proposal?",
      "mockup.input": "buffer municipalities by 500m and export GeoJSON…",

      "problems.label": "Common problems",
      "problems.title": "Sound familiar in QGIS?",
      "problems.subtitle":
        "GIS workflows often mean PyQGIS scripts, forum searches, and copy-paste from generic chatbots. Strata closes the gap between question and action in your project.",
      "problems.carousel.label": "Common problems carousel",
      "problems.prev": "Previous problem",
      "problems.next": "Next problem",

      "problems.1.title": "PyQGIS is hard",
      "problems.1.desc":
        "Copying snippets from ChatGPT without project context. Syntax errors, wrong APIs, hours lost.",
      "problems.1.fix": "Strata runs the code for you, with your approval.",

      "problems.2.title": "Layer inspection is tedious",
      "problems.2.desc":
        "Understanding fields, CRS, and extents means manual scripts or Python console round-trips.",
      "problems.2.fix": "Reads layers and fields directly from your project.",

      "problems.3.title": "Missing Python deps",
      "problems.3.desc":
        "geopandas, osmnx, pandas… often not installed in the QGIS environment.",
      "problems.3.fix": "Install missing libraries in one click, always under your control.",

      "problems.4.title": "Fetching data is slow",
      "problems.4.desc":
        "Overpass, GeoJSON, GADM — manual fetch, save, import layer.",
      "problems.4.fix": "Download and import data in a single step.",

      "problems.5.title": "Scattered context",
      "problems.5.desc":
        "Project files, scripts, layers, and attributes everywhere — generic AI can't see them.",
      "problems.5.fix": "Uses your project files and layers as context for answers.",

      "problems.6.title": "Uncontrolled edits",
      "problems.6.desc":
        "Scripts overwriting files or running risky code without review.",
      "problems.6.fix": "Every change must be approved before it is applied.",

      "problems.7.title": "Disconnected AI plugins",
      "problems.7.desc":
        "External assistants don't know your active layers or Processing Toolbox.",
      "problems.7.fix": "The assistant is built into QGIS and knows your project.",

      "solution.label": "Solution",
      "solution.title": "Ask, approve, done",
      "solution.subtitle":
        "Describe what you need. Strata proposes, you decide — without leaving QGIS.",

      "solution.chat.title": "Chat beside the map",
      "solution.chat.desc":
        "A side panel where you ask in plain language, attach files, and see answers while you work on the map.",
      "solution.chat.f1": "Real-time answers",
      "solution.chat.f2": "History saved per project",
      "solution.chat.f3": "Changes to approve before applying",

      "solution.agent.title": "Three modes, one app",
      "solution.agent.desc":
        "Plan an intervention, run it on the project, or just ask questions. Choose the level of autonomy you prefer.",
      "solution.agent.f1": "Plan · Run · Ask",
      "solution.agent.f2": "Built-in GIS actions",
      "solution.agent.f3": "Customize how the assistant works",

      "demo.label": "Demo",
      "demo.title": "See it in action",
      "demo.subtitle":
        "See how Strata handles real GIS project requests, with approval before every change.",
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

      "speed.label": "GIS productivity",
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

      "features.label": "Benefits",
      "features.title": "Everything built in, zero complexity",
      "features.subtitle":
        "The assistant works inside QGIS with your data, without leaving the app.",

      "features.1.title": "Does the work for you",
      "features.1.desc":
        "Buffer, export, analysis, data download: ask in plain language and Strata handles the rest.",

      "features.2.title": "Fits how you work",
      "features.2.desc":
        "Set rules and preferences so the assistant responds the way your team expects.",

      "features.3.title": "Knows your files",
      "features.3.desc":
        "Scripts, projects, and workspace documents are available as context for answers.",

      "features.4.title": "Layers only when you want",
      "features.4.desc":
        "Share attributes and geometries with the AI only if you decide to, with full control.",

      "features.5.title": "AI included",
      "features.5.desc":
        "With the Pro plan you don't need to manage API keys or credits: it's ready to go.",

      "features.6.title": "Your data, your rules",
      "features.6.desc":
        "On-premise for enterprise. In the cloud, you decide what to share.",

      "start.label": "Get started",
      "start.title": "Ready in minutes",
      "start.subtitle":
        "No complicated setup. Download, open your QGIS project, and start right away.",
      "start.1.title": "Download Strata",
      "start.1.desc":
        "Install on Mac, Windows, or Linux. It's QGIS with the AI assistant already inside.",
      "start.2.title": "Open your project",
      "start.2.desc":
        "Layers, maps, and files you use every day. The AI sees and understands them.",
      "start.3.title": "Ask for what you need",
      "start.3.desc":
        "Write what you want to do in plain language. Strata proposes, you approve, it runs.",
      "start.cta.download": "Download free",
      "start.cta.cloud": "Try Strata Cloud",

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

      "team.label": "Team",
      "team.title": "The people behind Strata",
      "team.subtitle": "We build the product and know the GIS domain.",
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
        "Strata is built on QGIS's open-source core. This ensures full compatibility with your existing GIS workflows and protects you from proprietary format lock-in. You pay for cloud infrastructure, ease of use, and our expertise in optimizing AI models for cartography — while keeping the transparency and reliability of free software.",
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
