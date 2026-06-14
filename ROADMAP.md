# Roadmap Strata: diventare il “Cursor per QGIS/GIS”

> Documento operativo per trasformare Strata da fork AI-native di QGIS a workspace GIS agentico, locale, sicuro, riproducibile e vendibile.
>
> **Revisione:** giugno 2026 — allineata allo stato reale del codice in `src/app/ai/` (~68 file). Ogni fase e item di backlog riporta lo stato implementativo: `[FATTO]`, `[PARZIALE]`, `[MANCANTE]`.

---

## Indice

0. [Stato attuale](#0-stato-attuale-giugno-2026)
1. [Premessa strategica](#1-premessa-strategica)
2. [North Star del prodotto](#2-north-star-del-prodotto)
3. [Mappa Cursor → Strata](#3-mappa-cursor--strata)
4. [Roadmap per fasi](#4-roadmap-per-fasi)
5. [Fase 0 — Fondamenta prodotto e distribuzione](#fase-0--fondamenta-prodotto-e-distribuzione)
6. [Fase 1 — Assistant operativo MVP](#fase-1--assistant-operativo-mvp)
7. [Fase 2 — Strata Context Engine](#fase-2--strata-context-engine)
8. [Fase 3 — GIS Tab](#fase-3--gis-tab)
9. [Fase 4 — Agent Mode v2](#fase-4--agent-mode-v2)
10. [Fase 5 — Workflow Composer e riproducibilità](#fase-5--workflow-composer-e-riproducibilità)
11. [Fase 6 — Vertical Packs](#fase-6--vertical-packs)
12. [Fase 7 — Team ed Enterprise](#fase-7--team-ed-enterprise)
13. [Fase 8 — Strata CLI, GIS Workers e automazioni](#fase-8--strata-cli-gis-workers-e-automazioni)
14. [Fase 9 — GIS Review](#fase-9--gis-review)
15. [Fase 10 — Marketplace, SDK e community](#fase-10--marketplace-sdk-e-community)
16. [Backlog dettagliato](#5-backlog-dettagliato)
17. [Metriche](#6-metriche)
18. [Sequenza consigliata](#7-sequenza-consigliata)
19. [Priorità assolute](#8-priorità-assolute)
20. [Cosa evitare](#9-cosa-evitare)
21. [Roadmap compatta](#10-roadmap-compatta)

---

# 0. Stato attuale (giugno 2026)

## Legenda

| Marcatore | Significato |
|---|---|
| `[FATTO]` | Implementato e utilizzabile in produzione |
| `[PARZIALE]` | Base presente, ma incompleto rispetto agli acceptance criteria della fase |
| `[MANCANTE]` | Nessuna implementazione nel codice |

## Sintesi per fase

| Fase | Completamento | Già presente | Gap principali |
|---:|---:|---|---|
| 0 Fondamenta | ~30% | Release multi-piattaforma (`release-strata.yml`), macOS sign+notarize in CI, landing page (`docs/`), branding Strata | Demo project bundled, auto-update in-app, telemetria opt-in, onboarding wizard, checksum release, Windows signing |
| 1 Assistant MVP | ~75% | Dock assistant (`qgsaichatdockwidget`), 5 provider LLM (`qgsaimodelrouter`), 21 tool (`tools/`), Ask/Plan/Agent, `run_python`, safety/trust/audit, review file | Tool GIS nativi (Processing/style/layout/export), risk classification formale, execution log strutturato, modalità Expert |
| 2 Context Engine | ~60% | RAG locale SQLite (`qgsaiworkspaceindex`), layer chunking, embeddings ONNX, semantic search, privacy consents | Project graph completo, CRS/stili/layout indexing, context packs espliciti, context preview UI |
| 3 GIS Tab | 0% | — | Intera fase da costruire |
| 4 Agent v2 | ~40% | Loop multi-step (max 8 iter/turn), Plan→Agent handoff, review/diff/rollback file (`qgsaireviewpatchengine`), session history | JSON planner, step verifier, GIS diff/rollback (layer/stile/layout), agent memory |
| 5 Workflow Composer | ~5% | Hint nel system prompt per salvare script | `.strataflow`, workflow runner, provenance, report generator |
| 6–10 | 0% | Rules/skills workspace (`.strata/rules`, `.strata/skills`) | Vertical packs, enterprise, CLI, GIS Review, marketplace |

## Architettura AI esistente

```mermaid
flowchart LR
  UI[QgsAiChatDockWidget] --> SM[QgsAiAgentSessionManager]
  SM --> MR[QgsAiModelRouter]
  SM --> TR[QgsAiToolRegistry]
  SM --> WI[QgsAiWorkspaceIndex]
  TR --> Tools["21 tool GIS/file/Python"]
  TR --> RE[QgsAiReviewPatchEngine]
  MR --> P["OpenAI / OpenRouter / Codex / Claude / Plan"]
```

**Directory chiave:** `src/app/ai/` — orchestrazione, provider, tool registry, context engine RAG, review patch.

## Prossimo focus (ri-prioritizzato)

1. Tool GIS nativi (`run_processing_algorithm`, `style_layer`, `create_layout`, `export_map`, inspect strutturati).
2. GIS diff/rollback + risk classification formale.
3. Completare Fase 0 (demo project, onboarding wizard, telemetria opt-in, auto-update).
4. GIS Tab rule-based.
5. Workflow Composer `.strataflow`.

---

# 1. Premessa strategica

Strata parte da una base corretta: è un fork AI-native di QGIS con assistant integrato, pensato per chattare con le mappe, usare agenti geospaziali multi-step e interrogare il workspace direttamente dentro il desktop.

La strategia “alla Cursor”, però, non significa solo aggiungere una chat. Cursor ha costruito valore con più livelli:

- predizione della prossima azione;
- contesto completo del progetto;
- agenti capaci di modificare l’ambiente;
- review e controllo umano;
- automazioni da CLI;
- workflow riproducibili;
- privacy e governance enterprise;
- team adoption;
- marketplace/estensioni.

Per Strata, l’equivalente non è:

> “Chat dentro QGIS.”

ma:

> **Ambiente GIS agentico, locale, sicuro e riproducibile, capace di trasformare richieste tecniche in analisi, mappe, workflow e report verificabili.**

---

# 2. North Star del prodotto

## Visione

Strata deve diventare il workspace GIS dove un tecnico può passare da una richiesta in linguaggio naturale a un output GIS verificabile:

- layer;
- analisi spaziali;
- stili cartografici;
- layout;
- report;
- script PyQGIS;
- modelli Processing;
- workflow riutilizzabili.

## Promessa prodotto

> “Descrivi il risultato GIS che vuoi. Strata capisce progetto, dati, CRS, layer, campi e vincoli; propone un piano; esegue azioni controllate; mostra diff e log; produce output riproducibili.”

## Principi guida

| Principio | Implicazione pratica |
|---|---|
| Context-first | L’assistente deve capire progetto, layer, CRS, campi, stili, Processing history, layout e file sorgenti. |
| Action-first | Ogni risposta deve poter diventare azione: creare layer, eseguire Processing, scrivere PyQGIS, generare report. |
| Review-first | Nessuna modifica distruttiva senza preview, diff, undo e log. |
| Local-first | Dati sensibili GIS devono poter rimanere sul device o in infrastruttura cliente. |
| Reproducible-first | Ogni operazione deve generare script, parametri, provenance e workflow riutilizzabile. |
| Vertical-first | La crescita deve partire da workflow ad alto valore: PA, ambiente, agricoltura, catasto, urbanistica, utilities. |

---

# 3. Mappa Cursor → Strata

| Cursor | Funzione Cursor | Equivalente Strata da costruire |
|---|---|---|
| Cursor Chat | Chat con il codebase | Chat con progetto QGIS, layer, CRS, campi, stili, Processing history |
| Cursor Tab | Predizione prossima azione | **GIS Tab**: suggerimenti contestuali su layer, CRS, errori, campi, layout |
| Agent Mode | Esecuzione multi-step | **GIS Agent**: pianifica, esegue Processing/PyQGIS, verifica output |
| Composer | Cambiamenti multi-file | **Workflow Composer**: pipeline multi-layer, multi-step, riusabile |
| Codebase Indexing | Indicizzazione codice | **Project Indexing**: layer metadata, schema, preview campi, geometrie, CRS, stili |
| Rules | Convenzioni di progetto/team | **GIS Rules**: CRS standard, naming layer, scale, simboli, export, policy dati |
| Code Review / Bugbot | Review automatica PR | **GIS Review**: valida geometrie, CRS, campi, output, layout, regressioni |
| CLI | Agent da terminale | **Strata CLI**: batch GIS, report, QA, export, pipeline in CI |
| Cloud Agents | Agent in ambienti remoti | **GIS Workers**: agenti su container/QGIS headless/PostGIS |
| Enterprise | SSO, SCIM, governance | Admin console, audit log, model policy, dati on-prem, ruoli |
| Marketplace | Skills, MCP, plugin | Marketplace di GIS skills, template verticali, connettori dati |

---

# 4. Roadmap per fasi

## Vista sintetica

| Fase | Orizzonte | Obiettivo | Esito atteso | Stato |
|---:|---:|---|---|---|
| 0 | 0–4 settimane | Stabilizzare base tecnica e posizionamento | Strata installabile, misurabile, demo ripetibile | `[PARZIALE ~30%]` |
| 1 | 1–2 mesi | Chat agentica utile in QGIS | L’assistente esegue task GIS base con sicurezza | `[PARZIALE ~75%]` |
| 2 | 2–4 mesi | Context Engine | Strata capisce realmente il progetto | `[PARZIALE ~60%]` |
| 3 | 4–6 mesi | GIS Tab | Suggerimenti automatici contestuali | `[MANCANTE 0%]` |
| 4 | 6–9 mesi | Agent Mode maturo | Workflow multi-step affidabili, con review e rollback | `[PARZIALE ~40%]` |
| 5 | 9–12 mesi | Workflow Composer + reproducibility | Pipeline riusabili, script, provenance, report | `[MANCANTE ~5%]` |
| 6 | 12–15 mesi | Vertical packs | Use case vendibili per PA/agri/ambiente/catasto | `[MANCANTE 0%]` |
| 7 | 15–18 mesi | Team/Enterprise | Governance, on-prem, admin, audit, analytics | `[MANCANTE 0%]` |
| 8 | 18+ mesi | Ecosistema | Marketplace, SDK, GIS workers, cloud/self-hosted agents | `[MANCANTE 0%]` |

---

# Fase 0 — Fondamenta prodotto e distribuzione [PARZIALE ~30%]

## Obiettivo

Rendere Strata provabile senza attrito, stabile abbastanza per early adopter e misurabile.

**Già fatto:** pipeline release su tag `strata-v*` (`.github/workflows/release-strata.yml`), build macOS/Linux/Windows, firma+notarizzazione macOS in CI (`build-macos-qt6.yml`), landing page (`docs/`), README con setup AI, check versione Strata (`qgsversioninfo.cpp`).

**Da completare:** demo project bundled, auto-update in-app, telemetria opt-in, onboarding wizard AI, checksum pubblici, Windows code signing, crash reporting Strata-branded.

Questa fase è fondamentale perché un prodotto “alla Cursor” deve essere facile da provare. Se l’utente deve superare warning di sicurezza, installare dipendenze manualmente o leggere troppa documentazione, la crescita individual-led diventa difficile.

## Priorità

| Priorità | Feature | Descrizione | Stato | Implementazione |
|---|---|---|---|---|
| P0 | Installer firmati | Firma macOS, Windows code signing, notarizzazione Apple, checksum pubblici | `[PARZIALE]` | macOS CI sign+notarize; Win/Linux non firmati; checksum assenti |
| P0 | Auto-update | Aggiornamento in-app con canale stable/beta/nightly | `[PARZIALE]` | Check versione + banner welcome (`qgswelcomescreen.cpp`); no install in-app |
| P0 | Crash reporting opt-in | Log anonimi, errori PyQGIS, errori agent, OS, versione QGIS base | `[PARZIALE]` | Crash handler QGIS upstream (`src/crashhandler/`); no opt-in upload, no campi agent |
| P0 | Demo project incluso | Progetto QGIS campione con layer, errori CRS, layout, dati tabellari | `[MANCANTE]` | Solo progetti test in `tests_ai/Dati/` (non shipped) |
| P0 | Telemetria locale/opt-in | Eventi minimi: feature usate, task completati, failure rate | `[MANCANTE]` | — |
| P1 | First-run onboarding | Config provider AI, privacy mode, scelta modello, progetto demo | `[PARZIALE]` | Banner one-shot (`maybeShowWelcomeBanner`); settings dialog provider |
| P1 | Documentation minima | “5 task che Strata risolve in 5 minuti” | `[PARZIALE]` | README + landing; manca guida operativa |
| P1 | Benchmark baseline | Misurare tempo utente vs tempo Strata su workflow ripetibili | `[MANCANTE]` | Solo claim illustrativo in landing |

## Deliverable

- `Strata.dmg` firmato e notarizzato.
- `StrataSetup.exe` firmato.
- `Strata.AppImage` con checksum.
- Progetto demo: `municipality_boundary.gpkg`, `parcels.gpkg`, `land_use.gpkg`, `roads.gpkg`.
- Pagina “Try Strata in 10 minutes”.
- Video demo da 60–90 secondi.

## Acceptance criteria

- Installazione completata da nuovo utente in meno di 5 minuti.
- Primo prompt eseguibile senza leggere documentazione.
- Crash rate inferiore al 2% sulle sessioni demo.
- Almeno 5 workflow demo riproducibili.

## Non-goal

Non costruire ancora enterprise, marketplace o cloud. Prima serve una demo ripetibile e stabile.

---

# Fase 1 — Assistant operativo MVP [PARZIALE ~75%]

## Obiettivo

Portare l’assistente da “chat che risponde” a **assistant che fa cose concrete dentro QGIS**.

**Già fatto:** dock assistant production-grade (`qgsaichatdockwidget`), router 5 provider (`qgsaimodelrouter`), 21 tool registrati (`qgsaitoolregistry`), modalità Ask/Plan/Agent (`qgsaiagentsessionmanager`), `run_python` con approval (`qgsairunpythontool`), workspace trust, audit log, review/diff/rollback file, chat history SQLite, rules/skills workspace.

**Da completare:** tool GIS nativi (Processing, styling, layout, export), risk classification formale (low/medium/high/critical), execution log strutturato JSON, modalità Expert, output strutturato standardizzato.

## Feature P0

### 1. Tool registry GIS `[FATTO]` — `tools/qgsaitoolregistry.*`

Creare un registro esplicito di tool chiamabili dall’agente.

**Implementato (21 tool):** `read_file`, `search_files`, `list_files`, `list_project_layers`, `get_active_canvas_extent`, `capture_map_canvas`, `read_message_log`, `add_layer_from_file`, `describe_layer`, `propose_edit/create/delete/multi_edit`, `run_python`, `install_python_package`, `download_file`, `index_status/search_workspace/reindex_*`, `echo`.

Esempio target (roadmap originale):

```text
inspect_project()
inspect_layer(layer_id)
inspect_fields(layer_id)
inspect_crs()
run_processing_algorithm(algorithm_id, params)
run_pyqgis(code, safety_level)
create_memory_note(scope, content)
create_layer_from_expression(...)
style_layer(...)
create_layout(...)
export_map(...)
```

Ogni tool deve avere metadati espliciti:

```yaml
name: run_processing_algorithm
category: geoprocessing
risk_level: medium
requires_confirmation: true
modifies_project: true
modifies_filesystem: true
supports_dry_run: true
returns:
  - output_layers
  - logs
  - warnings
  - execution_time
```

**Tool roadmap ancora `[MANCANTE]`:**

| Tool target | Stato | Nota |
|---|---|---|
| `inspect_project()` | `[MANCANTE]` | Parziale via snapshot layer nel system prompt |
| `inspect_layer()` | `[PARZIALE]` | Esiste `describe_layer` |
| `inspect_fields()` | `[PARZIALE]` | Incluso in `describe_layer` |
| `inspect_crs()` | `[MANCANTE]` | — |
| `run_processing_algorithm()` | `[MANCANTE]` | Oggi via `run_python` |
| `run_pyqgis()` | `[FATTO]` | Implementato come `run_python` |
| `create_memory_note()` | `[MANCANTE]` | — |
| `style_layer()` | `[MANCANTE]` | Oggi via `run_python` |
| `create_layout()` | `[MANCANTE]` | Oggi via `run_python` |
| `export_map()` | `[MANCANTE]` | Oggi via `run_python` |

### 2. Modalità Ask / Plan / Agent `[FATTO]` — `qgsaiagentsessionmanager.*`

| Modalità | Cosa può fare | Conferma richiesta | Stato |
|---|---|---|---|
| Ask | Risponde, spiega, suggerisce | No | `[FATTO]` agent `reviewer`, 10 tool read-only |
| Plan | Propone piano strutturato | No modifica | `[FATTO]` agent `planner`, blocchi `<proposed_plan>`, Accept/Reject |
| Agent | Esegue tool e modifica progetto | Sì, per azioni rischiose | `[FATTO]` agent `editor`, tutti i tool |
| Expert | Permette PyQGIS avanzato | Sempre conferma | `[MANCANTE]` | Non implementata come modalità separata |

### 3. Safety layer per PyQGIS `[PARZIALE]` — `qgsaiworkspacetrust.*`, `qgsaipythonapprovaldialog.*`, `qgsaiauditlog.*`

Classificare automaticamente il codice prima dell’esecuzione.

| Rischio | Esempi | Policy | Stato |
|---|---|---|---|
| Low | Leggere layer, campi, CRS | Può eseguire | `[PARZIALE]` read-only tools senza approval |
| Medium | Creare layer temporanei, simbologia | Conferma semplice | `[PARZIALE]` approval dialog, non policy automatica |
| High | Scrivere file, modificare layer persistenti | Preview + conferma | `[PARZIALE]` `propose_*` + review patch engine |
| Critical | Delete, overwrite, network, shell, pip install | Conferma forte o blocco | `[PARZIALE]` `requiresApproval()` + euristiche advisory (`detectRiskMarkers`) |

### 4. Output strutturato dell’agente `[PARZIALE]` — `qgsaichatdockwidget.*`

Ogni risposta operativa deve avere formato standard:

```markdown
## Obiettivo interpretato
...

## Piano
1. ...
2. ...

## Azioni proposte
- Tool: ...
- Input: ...
- Output previsto: ...

## Rischi
- ...

## Conferma richiesta
[Accetta] [Modifica piano] [Annulla]
```

**Implementato:** Plan mode con Accept/Reject, domande strutturate (`qgis_ai_questions`), review proposals per patch file. **Manca:** template output standardizzato per ogni risposta operativa.

### 5. Execution log `[PARZIALE]` — `qgsaiauditlog.*`, `qgsaimessagelogbuffer.*`

Ogni run deve produrre un log leggibile:

```json
{
  "run_id": "strata_run_2026_06_13_001",
  "prompt": "...",
  "project_path": "...",
  "tools_called": [],
  "layers_created": [],
  "layers_modified": [],
  "files_written": [],
  "processing_algorithms": [],
  "pyqgis_scripts": [],
  "warnings": [],
  "duration_ms": 0
}
```

**Implementato:** audit log tool rischiosi (`qgsaiauditlog`), message log buffer (`read_message_log`), token/cost tracking per sessione (`QgsAiUsage`). **Manca:** log JSON strutturato per run con `layers_created/modified`, `processing_algorithms`, `pyqgis_scripts`.

## Workflow da supportare entro fine fase

| Workflow | Prompt esempio | Output | Stato | Via attuale |
|---|---|---|---|---|
| CRS check | “Controlla se i layer hanno CRS coerente” | Report CRS + fix proposto | `[PARZIALE]` | `describe_layer` + `run_python` |
| Buffer | “Crea buffer di 100m dalle strade e intersecalo con le particelle” | Layer buffer + intersection | `[PARZIALE]` | `run_python` (Processing via PyQGIS) |
| Dissolve | “Unisci poligoni per comune” | Layer dissolto | `[PARZIALE]` | `run_python` |
| Styling | “Colora uso suolo per categoria” | Simbologia categorizzata | `[PARZIALE]` | `run_python` |
| Layout | “Crea una mappa PDF con scala, legenda e nord” | Layout + PDF | `[PARZIALE]` | `run_python` |
| QA geometrie | “Trova geometrie invalide e correggile” | Report + layer corretto | `[PARZIALE]` | `run_python` |
| Export | “Esporta questi layer in GeoPackage” | File `.gpkg` | `[PARZIALE]` | `run_python` |

## Acceptance criteria

- 80% dei workflow demo completati senza intervento manuale.
- 100% delle azioni distruttive richiedono conferma.
- Ogni esecuzione genera log.
- Ogni output layer ha nome, CRS, sorgente e parametri tracciati.
- L’utente può annullare o rivedere un piano prima dell’esecuzione.

---

# Fase 2 — Strata Context Engine [PARZIALE ~60%]

## Obiettivo

Costruire il vero vantaggio competitivo: **comprensione del workspace GIS**.

**Già fatto:** RAG locale SQLite (`qgsaiworkspaceindex`), layer chunking (`qgsailayerchunker`), embeddings ONNX E5-small (`qgsaiembeddingprovider`), semantic search (`search_workspace`), reindex automatico (`qgsaiindexingscheduler`, `qgsailayerindexcoordinator`), privacy consents (layer indexing, vision), snapshot primi 10 layer nel system prompt.

**Da completare:** project graph strutturato, CRS/stili/layout/processing history indexing, context packs espliciti, context preview UI, layer cards YAML formali.

Cursor è forte perché conosce il codebase. Strata deve conoscere il progetto QGIS.

## Componenti

### 1. Project graph

Rappresentare il progetto come grafo.

```text
Project
├── Layers
│   ├── VectorLayer
│   │   ├── CRS
│   │   ├── Geometry type
│   │   ├── Fields
│   │   ├── Feature count
│   │   ├── Extent
│   │   ├── Data source
│   │   └── Style
│   ├── RasterLayer
│   │   ├── CRS
│   │   ├── Bands
│   │   ├── Resolution
│   │   ├── NoData
│   │   └── Extent
│   └── PointCloudLayer
├── Layouts
├── Processing history
├── Relations
├── Joins
├── Expressions
├── Variables
└── Project rules
```

### 2. Layer summaries

Per ogni layer generare una scheda sintetica:

```yaml
layer_id: parcels_001
name: Catasto particelle
type: vector
geometry: polygon
crs: EPSG:32632
feature_count: 18423
fields:
  - name: comune
    type: string
    null_ratio: 0.01
    unique_count: 12
  - name: area_ha
    type: double
    min: 0.02
    max: 82.4
extent:
  xmin: ...
  ymin: ...
quality:
  invalid_geometries: 17
  duplicated_ids: 3
suggested_actions:
  - fix_geometries
  - create_spatial_index
  - classify_by_area
```

### 3. Semantic index

Indicizzare:

- nomi layer;
- nomi campi;
- alias campi;
- valori campione;
- metadata;
- stili;
- history Processing;
- layout;
- script PyQGIS generati;
- note utente;
- regole `.stratarules`.

### 4. Privacy-aware indexing

Policy proposta:

| Tipo dato | Default | Invio cloud |
|---|---|---|
| Nome layer | Consentito | Sì, se opt-in |
| Nomi campi | Consentito | Sì, se opt-in |
| Valori campione | Bloccato | Solo opt-in esplicito |
| Geometrie | Bloccato | Mai di default |
| Coordinate | Bloccato | Solo local model/on-prem |
| Raster pixel | Bloccato | Solo local/on-prem |
| Data source path | Mascherato | Hash/path relativo |
| Metadata sensibili | Bloccato | Redaction |

### 5. Context budget manager

L’agente non deve inviare tutto al modello. Deve selezionare contesto.

```text
Prompt utente
→ intent detection
→ layer relevance scoring
→ field relevance scoring
→ risk classification
→ context pack
→ tool plan
```

### 6. Context packs

Esempi:

```yaml
context_pack: crs_debug
includes:
  - project_crs
  - all_layer_crs
  - layer_extents
  - transformation_warnings
excludes:
  - feature_values
  - geometries
```

```yaml
context_pack: styling
includes:
  - selected_layer_fields
  - unique_values_sample
  - current_style
  - geometry_type
```

## Feature P0

| Feature | Descrizione | Stato | Implementazione |
|---|---|---|---|
| Project index locale | SQLite/duckdb locale con metadata progetto | `[FATTO]` | `qgsaiworkspaceindex` (SQLite, cosine similarity) |
| Layer cards | Schede layer leggibili dall’agente | `[PARZIALE]` | `describe_layer` + layer chunks; manca formato YAML scheda |
| Context selector | Sceglie quali info passare al modello | `[PARZIALE]` | RAG top-K=8 in system prompt; manca UI selector |
| Sensitive data guard | Blocca coordinate/geometrie/valori senza consenso | `[FATTO]` | Consents layer indexing + vision; `wrapUntrusted` |
| Refresh automatico | Aggiorna index quando layer/progetto cambia | `[FATTO]` | `qgsaiindexingscheduler`, `qgsailayerindexcoordinator` |
| Search workspace | “Trova il layer con le particelle catastali” | `[FATTO]` | `search_workspace`, `index_status` tools |

## Acceptance criteria

- L’assistente risponde correttamente a domande su layer, CRS, campi e layout.
- Nessuna geometria viene inviata al modello senza opt-in.
- Il context pack usato è visibile all’utente.
- L’indice si aggiorna dopo aggiunta/rimozione/modifica layer.
- Latenza massima per inspect progetto piccolo: meno di 1 secondo.
- Latenza massima per progetto medio: meno di 5 secondi.

---

# Fase 3 — GIS Tab [MANCANTE 0%]

## Obiettivo

Creare l’equivalente GIS della predizione di prossima azione di Cursor.

**Già fatto:** nessuna implementazione nel codice.

**Da completare:** intera fase — suggestion engine, trigger contestuali, UI inline chip + side panel, ranking, shortcut accetta/ignora.

Per Strata, il GIS Tab deve suggerire azioni mentre l’utente lavora.

## Concetto

> **GIS Tab osserva il contesto corrente e propone la prossima azione GIS probabile.**

Non deve essere invasivo. Deve apparire come suggerimento leggero, accettabile con shortcut.

## Trigger iniziali

| Trigger | Suggerimento |
|---|---|
| Layer con CRS diverso dal progetto | “Riproietta in CRS progetto” |
| Layer senza spatial index | “Crea indice spaziale” |
| Geometrie invalide | “Esegui fix geometries” |
| Campo categorico selezionato | “Crea simbologia categorizzata” |
| Campo numerico selezionato | “Crea mappa graduata” |
| Layer poligonale selezionato | “Calcola area/perimetro” |
| Due layer compatibili selezionati | “Esegui intersection/clip/join by location” |
| Layout aperto | “Aggiungi scala, legenda, nord, titolo” |
| Errore Processing | “Spiega e proponi fix” |
| Layer grande | “Crea spatial index / semplifica / filtra” |
| Export ripetuto | “Salva come modello riutilizzabile” |
| Sequenza buffer → clip → dissolve | “Converti in workflow” |

## UX proposta

### Inline action chip

```text
⚡ Suggerimento Strata
Questo layer ha CRS EPSG:4326, il progetto è EPSG:32632.
[Tab] Riproietta layer temporaneo  [Alt+Tab] Vedi piano  [Esc] Ignora
```

### Side panel

```markdown
## Suggerimenti contestuali

1. Riproietta `roads_wgs84` in EPSG:32632
   Motivo: CRS diverso dal progetto.
   Rischio: basso.
   Output: nuovo layer temporaneo.

2. Crea spatial index per `parcels`
   Motivo: 184k feature, operazioni spaziali lente.
   Rischio: basso.

3. Correggi 17 geometrie invalide
   Motivo: possibile errore in intersection.
   Rischio: medio.
```

## Ranking dei suggerimenti

Score proposto:

```text
score =
  user_context_relevance * 0.30
+ historical_acceptance * 0.20
+ current_error_severity * 0.20
+ workflow_probability * 0.20
+ low_risk_bonus * 0.10
```

## Feature P0

| Feature | Descrizione | Stato |
|---|---|---|
| Suggestion engine rule-based | Prima versione senza ML complesso | `[MANCANTE]` |
| Shortcut accetta/ignora | Tab accetta, Esc ignora | `[MANCANTE]` |
| Explain why | Ogni suggerimento spiega perché appare | `[MANCANTE]` |
| Safe suggestions only | Prima solo azioni non distruttive | `[MANCANTE]` |
| Suggestion memory | Non riproporre suggerimenti ignorati troppe volte | `[MANCANTE]` |

## Feature P1

| Feature | Descrizione | Stato |
|---|---|---|
| Personalized suggestions | Impara dai workflow accettati | `[MANCANTE]` |
| Multi-step suggestions | Propone sequenze, non solo azioni singole | `[MANCANTE]` |
| Layout suggestions | Migliora cartografia e export | `[MANCANTE]` |
| Processing error fix | Suggerisce fix quando un algoritmo fallisce | `[MANCANTE]` |
| Project hygiene suggestions | Naming, CRS, cartelle output, campi mancanti | `[MANCANTE]` |

## Acceptance criteria

- Almeno 10 trigger GIS Tab funzionanti.
- Tasso accettazione suggerimenti > 20% sui workflow demo.
- Nessun suggerimento distruttivo in automatico.
- L’utente può disattivare GIS Tab per progetto o globalmente.
- Latenza suggerimento inferiore a 500 ms per trigger semplici.

---

# Fase 4 — Agent Mode v2 [PARZIALE ~40%]

## Obiettivo

Rendere l’agente abbastanza affidabile da svolgere workflow GIS multi-step reali.

**Già fatto:** loop multi-step (max 8 iterazioni/turn), Plan→Agent handoff, review/diff/rollback su file workspace (`qgsaireviewpatchengine`), human approval per tool rischiosi, session history SQLite, provider fallback, streaming+cancel, token/cost tracking.

**Da completare:** JSON planner validabile, step verifier, GIS diff/rollback (layer/stile/layout), agent memory, riesecuzione workflow salvati.

## Architettura agentica

```text
User prompt
→ Intent classifier
→ Context Engine
→ Planner
→ Risk assessor
→ Tool executor
→ Verifier
→ Diff generator
→ User review
→ Commit / rollback
→ Provenance log
```

## Planner

Il planner deve produrre un piano in JSON validabile.

```json
{
  "goal": "Create a buffer around roads and intersect with parcels",
  "assumptions": [
    "roads layer contains line geometries",
    "parcels layer contains polygon geometries"
  ],
  "steps": [
    {
      "id": "step_1",
      "tool": "inspect_layer",
      "params": { "layer": "roads" },
      "risk": "low"
    },
    {
      "id": "step_2",
      "tool": "run_processing_algorithm",
      "algorithm": "native:buffer",
      "params": {
        "distance": 100,
        "segments": 8
      },
      "risk": "medium"
    }
  ],
  "expected_outputs": [
    "roads_buffer_100m",
    "parcels_within_road_buffer"
  ],
  "requires_user_approval": true
}
```

## Verifier

Dopo ogni step, l’agente deve verificare:

| Verifica | Esempio |
|---|---|
| Output esiste | Layer creato correttamente |
| CRS coerente | Output CRS = CRS progetto |
| Geometrie valide | Nessun errore o errori noti |
| Feature count plausibile | Non zero, non esploso in modo anomalo |
| Campi attesi | Campi originali preservati |
| Extent plausibile | Output dentro area attesa |
| Performance | Operazione non eccessivamente lenta |
| Warnings | Processing warnings raccolti |

## Diff GIS

Serve un equivalente del diff codice.

### Diff layer

```markdown
## Diff layer: `parcels`

### Prima
- Feature: 18.423
- CRS: EPSG:4326
- Geometrie invalide: 17
- Campi: 12

### Dopo
- Feature: 18.423
- CRS: EPSG:32632
- Geometrie invalide: 0
- Campi: 12

### Modifiche
- Riproiezione in EPSG:32632
- Fix geometrie
- Creato spatial index
```

### Diff stile

```markdown
## Diff stile: `land_use`

- Renderer: single symbol → categorized
- Campo categoria: `land_use_type`
- Classi create: 8
- Opacità: 100% → 70%
- Label: disattivate → attivate su `name`
```

### Diff layout

```markdown
## Diff layout: `Map A4`

- Aggiunta legenda
- Aggiunta barra di scala
- Aggiunto nord
- Titolo impostato
- Export PDF generato
```

## Rollback

Ogni run deve avere rollback.

| Tipo modifica | Rollback |
|---|---|
| Layer temporaneo creato | Rimuovi layer |
| Layer persistente scritto | Conserva backup o scrivi nuovo file |
| Stile cambiato | Ripristina QML precedente |
| Layout modificato | Snapshot layout XML |
| Campo aggiunto | Rimuovi campo se sicuro |
| Feature edit | Undo command QGIS o backup layer |
| File overwrite | Vietato senza backup |

## Feature P0

| Feature | Descrizione | Stato | Implementazione |
|---|---|---|---|
| Planner JSON | Piano strutturato validabile | `[PARZIALE]` | Plan mode con `<proposed_plan>` testo; manca JSON schema |
| Step-by-step execution | Esecuzione controllata per step | `[PARZIALE]` | Loop tool max 8 iter/turn; non step isolati con pause |
| Tool risk levels | Low/medium/high/critical | `[PARZIALE]` | `requiresApproval()` + euristiche; manca policy automatica |
| Verifier per step | Controlli dopo ogni azione | `[MANCANTE]` | — |
| GIS diff v1 | Diff layer/stile/layout | `[MANCANTE]` | Solo diff file testo (`qgsaireviewpatchengine`) |
| Rollback v1 | Undo delle modifiche create da Strata | `[PARZIALE]` | `undoLastApply` per file; niente rollback GIS |
| Human approval | Conferma per azioni medium/high | `[FATTO]` | Approval dialog PyQGIS/pip/download; review patch |
| Agent run history | Storico run consultabile | `[PARZIALE]` | Chat history SQLite; manca storico run strutturato |

## Acceptance criteria

- L’agente completa almeno 20 workflow multi-step demo.
- Ogni workflow ha piano, log, diff e output.
- Ogni modifica a file/layer persistente è reversibile o protetta da backup.
- Fallimento di uno step interrompe il piano e propone fix.
- L’utente può rieseguire un workflow precedente.

---

# Fase 5 — Workflow Composer e riproducibilità [MANCANTE ~5%]

## Obiettivo

Trasformare le azioni dell’agente in asset riusabili.

**Già fatto:** hint nel system prompt per salvare script Processing in cartella profilo via `propose_create_file`; rules/skills workspace (`.strata/rules`, `.strata/skills`).

**Da completare:** formato `.strataflow`, workflow runner, parameter editor, export PyQGIS/Processing model, provenance metadata, report generator.

Se Strata si limita a “fare cose”, resta un assistant. Se trasforma ogni azione in workflow, diventa ambiente produttivo.

## Concetto

> Ogni sessione Agent può diventare un workflow riproducibile, modificabile e condivisibile.

## Oggetti principali

### `.strataflow`

Formato dichiarativo per workflow GIS.

```yaml
name: parcels_near_roads_analysis
version: 1
description: "Find parcels within 100m of main roads"
inputs:
  - id: roads
    type: vector
    geometry: line
    required_fields: ["road_type"]
  - id: parcels
    type: vector
    geometry: polygon
parameters:
  buffer_distance:
    type: number
    default: 100
    unit: meters
steps:
  - id: fix_roads
    algorithm: native:fixgeometries
    input: roads
  - id: buffer_roads
    algorithm: native:buffer
    input: fix_roads
    params:
      DISTANCE: "{{buffer_distance}}"
  - id: intersect_parcels
    algorithm: native:intersection
    inputs:
      INPUT: parcels
      OVERLAY: buffer_roads
outputs:
  - id: parcels_near_roads
    type: vector
    format: gpkg
report:
  include_map: true
  include_summary_table: true
```

### Workflow UI

| Area UI | Funzione |
|---|---|
| Left panel | Lista step |
| Center canvas | Grafo workflow |
| Right panel | Parametri step |
| Bottom panel | Log/output/errori |
| Top actions | Run, dry-run, export, schedule |

### Conversione automatica

Strata deve poter dire:

> “Hai eseguito buffer → intersection → styling → export. Vuoi salvarlo come workflow riutilizzabile?”

## Provenance

Ogni output deve sapere da dove viene.

```json
{
  "output": "parcels_near_roads.gpkg",
  "created_by": "Strata",
  "run_id": "run_001",
  "input_layers": [
    {
      "name": "roads",
      "checksum": "sha256:..."
    },
    {
      "name": "parcels",
      "checksum": "sha256:..."
    }
  ],
  "algorithms": [
    "native:buffer",
    "native:intersection"
  ],
  "parameters": {
    "buffer_distance": 100
  },
  "created_at": "2026-06-13T10:32:00Z"
}
```

## Report generation

Feature chiave per monetizzazione verticale.

Output:

- PDF tecnico;
- DOCX/ODT;
- HTML report;
- Markdown report;
- GeoPackage + metadata;
- QGIS layout PDF;
- CSV riepilogativo.

Template report:

```text
1. Obiettivo analisi
2. Dati utilizzati
3. Metodo
4. Parametri
5. Risultati
6. Mappe
7. Tabelle
8. Avvertenze
9. Log riproducibilità
```

## Feature P0

| Feature | Descrizione | Stato |
|---|---|---|
| Save run as workflow | Convertire sessione agente in `.strataflow` | `[MANCANTE]` |
| Workflow runner | Rieseguire workflow su nuovi dati | `[MANCANTE]` |
| Parameter editor | Modificare distanza buffer, campi, layer input | `[MANCANTE]` |
| Export PyQGIS | Generare script equivalente | `[MANCANTE]` |
| Export Processing model | Se possibile, generare modello QGIS Processing | `[MANCANTE]` |
| Report generator v1 | Report tecnico con mappa + tabella + log | `[MANCANTE]` |
| Provenance metadata | Metadati su ogni output | `[MANCANTE]` |

## Acceptance criteria

- 10 workflow demo salvabili e rieseguibili.
- Workflow eseguibile su progetto diverso se schema compatibile.
- Export PyQGIS funzionante.
- Report PDF generato automaticamente.
- Ogni output include provenance.

---

# Fase 6 — Vertical Packs

## Obiettivo

Passare da prodotto generico a prodotto vendibile.

Il mercato non compra “AI per QGIS” in astratto. Compra workflow che riducono tempo, errori e costi.

## Vertical pack structure

```text
vertical-pack/
├── pack.yaml
├── rules.stratarules
├── workflows/
│   ├── qa_catasto.strataflow
│   ├── buffer_vincoli.strataflow
│   └── report_particelle.strataflow
├── report_templates/
├── styles/
├── sample_projects/
├── prompts/
└── docs/
```

## Pack 1 — PA locale / urbanistica

### Workflow

| Workflow | Descrizione |
|---|---|
| Controllo CRS progetto | Verifica CRS tra PRG, particelle, vincoli |
| Buffer vincoli | Buffer su fiumi, strade, aree protette |
| Overlay particelle-vincoli | Identifica particelle impattate |
| Report vincoli particella | PDF per particella/comune |
| Layout delibera | Mappa pronta per documento amministrativo |
| QA geometrie comunali | Geometrie invalide, buchi, sovrapposizioni |
| Export open data | GeoPackage/GeoJSON con metadata |

### Regole

```yaml
default_crs: EPSG:32632
layer_naming:
  pattern: "{theme}_{municipality}_{date}"
required_layout_elements:
  - title
  - scale_bar
  - north_arrow
  - legend
  - data_sources
```

## Pack 2 — Agricoltura/agri-tech

### Workflow

| Workflow | Descrizione |
|---|---|
| Import particelle aziendali | Normalizza shapefile/GeoPackage |
| Calcolo superfici | Ettari per coltura/parcella |
| Buffer corsi d’acqua | Zone rispetto/limitazioni |
| Overlay suolo/pendenza | Analisi vocazionalità |
| Report aziendale | Mappe e tabelle per azienda |
| QA fascicolo | Campi mancanti, geometrie invalide |
| Export per consulente | Pacchetto dati + PDF |

### Prompt killer

```text
Carica le particelle aziendali, calcola gli ettari per coltura,
e genera un report PDF con mappa, tabella superfici e warning
sulle geometrie invalide.
```

## Pack 3 — Ambiente/forestazione

### Workflow

| Workflow | Descrizione |
|---|---|
| Analisi copertura suolo | Classificazione e aggregazione aree |
| Buffer aree protette | Interferenze con vincoli ambientali |
| Report intervento forestale | Mappa, superfici, particelle, vincoli |
| Change detection semplice | Confronto layer temporali |
| QA raster/vector | Extent, risoluzione, CRS, NoData |
| Export relazione tecnica | PDF/Markdown/DOCX |

## Pack 4 — Utilities/infrastrutture

### Workflow

| Workflow | Descrizione |
|---|---|
| Buffer reti | Distanze da condotte/cavi |
| Intersezione proprietà | Particelle interessate da infrastruttura |
| Mappa cantiere | Layout tecnico |
| QA asset | Duplicati, geometrie spezzate, attributi mancanti |
| Report interferenze | Tabelle + mappa per tratto |
| Export CAD/GIS | GeoPackage/DXF dove compatibile |

## Feature P0

| Feature | Descrizione |
|---|---|
| Pack installer | Installare pack verticali |
| Pack rules | Regole per dominio |
| Pack workflows | Workflow già pronti |
| Pack sample data | Progetti demo per vendere/provare |
| Pack report templates | Report specifici |
| Pack prompts | Prompt guidati |
| Pack validation | Verifica requisiti input |

## Acceptance criteria

- Almeno 2 vertical pack completi.
- Ogni pack ha 5+ workflow funzionanti.
- Ogni pack ha progetto demo e report di esempio.
- Un utente nuovo può completare un workflow verticale in meno di 15 minuti.
- Almeno 5 utenti target testano ogni pack.

---

# Fase 7 — Team ed Enterprise

## Obiettivo

Rendere Strata vendibile a organizzazioni: enti pubblici, società GIS, utilities, consulenze ambientali, agritech.

## Feature P0

### 1. Workspace team

```text
Organization
├── Workspaces
│   ├── Projects
│   ├── Rules
│   ├── Workflows
│   ├── Packs
│   └── Audit logs
├── Users
├── Roles
└── Policies
```

### 2. Ruoli

| Ruolo | Permessi |
|---|---|
| Viewer | Usa Ask, legge report |
| Analyst | Esegue workflow approvati |
| GIS Specialist | Crea workflow, modifica rules |
| Admin | Gestisce utenti, modelli, policy |
| Security Officer | Audit, log, data policy |
| Developer | Crea tool/plugin/pack |

### 3. Model policy

```yaml
models:
  allowed:
    - openai:gpt-5.5
    - anthropic:claude
    - local:llama
  blocked:
    - unknown-provider
data_policy:
  send_layer_names: true
  send_field_names: true
  send_sample_values: false
  send_geometries: false
  send_rasters: false
approval:
  require_for_file_write: true
  require_for_persistent_layer_edit: true
```

### 4. Audit log

Registrare:

- prompt;
- modello usato;
- context pack;
- tool chiamati;
- layer modificati;
- file scritti;
- workflow eseguiti;
- approvazioni;
- errori;
- rollback;
- export.

Formato:

```json
{
  "org_id": "municipality_x",
  "user_id": "analyst_1",
  "project_id": "urban_plan_2026",
  "run_id": "run_001",
  "model": "local:llama",
  "data_sent_external": false,
  "tools": ["inspect_project", "native:buffer"],
  "files_written": ["outputs/buffer_roads.gpkg"],
  "approved_by": "analyst_1",
  "timestamp": "2026-06-13T12:00:00Z"
}
```

### 5. On-prem/local deployment

Modalità:

| Modalità | Descrizione |
|---|---|
| Local only | Modello locale, nessun server |
| BYOK | Utente porta API key |
| Team cloud | Backend Strata gestisce account/policy |
| Private cloud | Deploy su cloud cliente |
| On-prem | Deploy completo su infrastruttura cliente |
| Air-gapped | Nessuna rete esterna, modelli locali |

### 6. Admin console

Funzioni:

- utenti;
- ruoli;
- modelli consentiti;
- provider consentiti;
- budget token;
- policy dati;
- allowed tools;
- blocked tools;
- audit export;
- workflow approvati;
- pack installati;
- usage analytics.

## Feature P1

| Feature | Descrizione |
|---|---|
| SSO/SAML | Login enterprise |
| SCIM | Provisioning utenti |
| License server | Gestione seat |
| Cost controls | Budget per utente/team |
| Private model gateway | Routing verso modelli privati |
| Data residency | Regione/ambiente dati |
| Compliance export | Report uso AI per audit |

## Acceptance criteria

- Admin può bloccare invio geometrie a modelli cloud.
- Admin può permettere solo modelli locali.
- Ogni azione agentica è auditabile.
- Un workflow può essere approvato e bloccato in versione.
- Deploy on-prem documentato.
- Almeno 1 pilota enterprise/ente completato.

---

# Fase 8 — Strata CLI, GIS Workers e automazioni

## Obiettivo

Portare Strata oltre il desktop, con CLI e agenti eseguibili in batch.

## Strata CLI

Esempi:

```bash
strata inspect project.qgz
strata ask project.qgz "quali layer hanno CRS diverso?"
strata run workflow.strataflow --project project.qgz --out outputs/
strata report project.qgz --template agri_report --out report.pdf
strata qa project.qgz --rules rules.stratarules
strata agent "crea buffer di 100m dalle strade" --project project.qgz --dry-run
```

## GIS Workers

Worker isolati per eseguire QGIS headless, Processing, PyQGIS, GDAL, PostGIS.

```text
Strata Desktop
→ Strata Orchestrator
→ GIS Worker
   ├── QGIS headless
   ├── GDAL/OGR
   ├── Python/PyQGIS
   ├── PostGIS connection
   ├── Model gateway
   └── Artifact store
```

## Use case

| Use case | Descrizione |
|---|---|
| QA notturno | Verifica dataset ogni notte |
| Report mensile | Genera mappe/report automatici |
| Batch su comuni | Esegue workflow su 100 comuni |
| Validazione upload | Controlla dati caricati da consulenti |
| CI geodata | Testa layer e schema prima del merge |
| Agent remoto | Esegue task lunghi senza bloccare desktop |

## Feature P0

| Feature | Descrizione |
|---|---|
| `strata inspect` | Ispezione progetto/layer |
| `strata run` | Esecuzione `.strataflow` |
| `strata qa` | Validazione rules |
| `strata report` | Generazione report |
| Docker image | Worker QGIS headless |
| Artifact output | Salvataggio log, layer, PDF |
| Dry-run | Preview senza scrittura |

## Feature P1

| Feature | Descrizione |
|---|---|
| Worker pool | Più worker paralleli |
| Kubernetes deployment | Deploy enterprise |
| Scheduled runs | Esecuzioni pianificate |
| Web dashboard | Monitor run |
| Remote desktop preview | Vedere output worker |
| Human approval queue | Approvazioni asincrone |
| API REST | Integrazione con sistemi esterni |

## Acceptance criteria

- Workflow eseguibile sia da desktop sia da CLI.
- Worker produce stessi output del desktop.
- Ogni run genera artifact bundle.
- Batch su 50 progetti demo completato senza intervento.
- Dry-run non scrive file.

---

# Fase 9 — GIS Review

## Obiettivo

Creare un revisore automatico di progetti GIS.

Strata Review deve controllare progetto, layer, workflow e output prima della consegna.

## Categorie review

| Categoria | Check |
|---|---|
| CRS | CRS incoerente, trasformazioni sospette |
| Geometrie | Invalidità, self-intersection, buchi, duplicati |
| Attributi | Campi mancanti, null, valori fuori dominio |
| Topologia | Overlap, gap, dangling lines |
| Stili | Simbologia mancante, legenda errata |
| Layout | Scala, nord, legenda, font, margini, fonti |
| Output | File mancanti, formati errati, path rotti |
| Performance | Layer enormi senza index |
| Privacy | Dati sensibili in export |
| Reproducibility | Mancanza log/provenance |

## Output review

```markdown
# Strata GIS Review

## Risultato
Status: Warning

## Problemi critici
1. Layer `parcels` contiene 17 geometrie invalide.
2. Layer `roads` è in EPSG:4326 mentre progetto è EPSG:32632.

## Problemi medi
1. `land_use` non ha legenda nel layout finale.
2. Export PDF non include data source.

## Fix proposti
- Esegui `native:fixgeometries` su `parcels`.
- Riproietta `roads` in EPSG:32632.
- Aggiorna layout con legenda e fonti dati.

## Azioni
[Applica fix sicuri] [Crea piano] [Ignora]
```

## Feature P0

| Feature | Descrizione |
|---|---|
| Review progetto | Analizza progetto QGIS |
| Review layer | Analizza layer singolo |
| Review workflow | Analizza `.strataflow` |
| Review output | Analizza report/PDF/layer generati |
| Fix suggestions | Suggerisce correzioni |
| Rules-based review | Usa `.stratarules` |
| Report review | Esporta review in PDF/Markdown |

## Acceptance criteria

- 30 check GIS implementati.
- Falsi positivi gestibili con ignore/commenti.
- Review eseguibile da desktop e CLI.
- Fix sicuri applicabili con un click.
- Report review allegabile a consegna tecnica.

---

# Fase 10 — Marketplace, SDK e community

## Obiettivo

Rendere Strata estendibile da terzi.

Strata dovrebbe costruire un marketplace verticale: GIS skills, workflow, report template, rules, data connectors e tool.

## Oggetti marketplace

| Oggetto | Esempio |
|---|---|
| Skill | “Come fare overlay catastale” |
| Workflow | “Buffer vincoli ambientali” |
| Report template | “Relazione tecnica PA” |
| Style pack | “Standard cartografia comunale” |
| Rules pack | “Regole QA GeoPackage” |
| Data connector | “OpenStreetMap / Copernicus / Catasto locale” |
| Tool plugin | “Geocoder”, “DEM analysis”, “PostGIS QA” |
| Agent persona | “Urban planner assistant”, “Agronomist assistant” |

## SDK

### Tool SDK

```python
from strata.sdk import tool, ToolContext

@tool(
    name="calculate_parcel_summary",
    risk="low",
    modifies_project=False
)
def calculate_parcel_summary(ctx: ToolContext, parcel_layer: str):
    layer = ctx.get_layer(parcel_layer)
    return {
        "feature_count": layer.featureCount(),
        "area_sum": ...
    }
```

### Skill format

```yaml
name: agronomy_parcel_report
description: "Generate agronomic parcel report"
version: 1
domain: agriculture
requires:
  - polygon_layer
  - crop_field
  - area_field
instructions:
  - Check CRS.
  - Validate geometries.
  - Calculate hectares by crop.
  - Generate map and summary table.
```

### Rules format

```yaml
name: municipality_cartography_rules
version: 1
rules:
  crs:
    required: EPSG:32632
  layout:
    require_north_arrow: true
    require_scale_bar: true
    require_data_sources: true
  export:
    allowed_formats: [pdf, gpkg]
```

## Governance marketplace

| Area | Regola |
|---|---|
| Security | Tool con risk level e permissions |
| Review | Approvazione manuale per plugin verificati |
| Signing | Package firmati |
| Versioning | Semver obbligatorio |
| Compatibility | Versione Strata/QGIS supportata |
| License | Licenza dichiarata |
| Data policy | Dichiarazione dati trattati |
| Test project | Ogni workflow deve avere test/demo |

## Acceptance criteria

- SDK documentato.
- 10 plugin/skills interni pubblicati.
- Package signing funzionante.
- Installazione one-click.
- Rating/feedback minimo.
- Compatibilità versionata.

---

# 5. Backlog dettagliato

## A. Context Engine

| ID | Feature | Priorità | Fase | Stato |
|---|---|---:|---:|---|
| CTX-001 | Project graph locale | P0 | 2 | `[PARZIALE]` snapshot layer + RAG index |
| CTX-002 | Layer metadata extractor | P0 | 2 | `[FATTO]` `describe_layer`, `qgsailayerchunker` |
| CTX-003 | CRS analyzer | P0 | 2 | `[PARZIALE]` via `describe_layer`; manca tool dedicato |
| CTX-004 | Field profiler | P0 | 2 | `[PARZIALE]` sample features in `describe_layer` |
| CTX-005 | Geometry quality summary | P0 | 2 | `[MANCANTE]` |
| CTX-006 | Raster metadata summary | P1 | 2 | `[PARZIALE]` chunking raster in `qgsailayerchunker` |
| CTX-007 | Point cloud metadata summary | P2 | 3 | `[MANCANTE]` |
| CTX-008 | Style summarizer | P1 | 2 | `[MANCANTE]` |
| CTX-009 | Layout summarizer | P1 | 2 | `[MANCANTE]` |
| CTX-010 | Processing history parser | P1 | 2 | `[MANCANTE]` |
| CTX-011 | Semantic search workspace | P0 | 2 | `[FATTO]` `search_workspace` |
| CTX-012 | Privacy-aware context packs | P0 | 2 | `[PARZIALE]` consents + `wrapUntrusted`; manca pack espliciti |
| CTX-013 | Context preview UI | P0 | 2 | `[MANCANTE]` |
| CTX-014 | Context refresh watcher | P1 | 2 | `[FATTO]` `qgsaiindexingscheduler`, `qgsailayerindexcoordinator` |
| CTX-015 | Sensitive data redaction | P0 | 2 | `[FATTO]` consents + redaction in audit log |

## B. Agent

| ID | Feature | Priorità | Fase | Stato |
|---|---|---:|---:|---|
| AGT-001 | Tool registry | P0 | 1 | `[FATTO]` `qgsaitoolregistry` (21 tool) |
| AGT-002 | Tool permission model | P0 | 1 | `[FATTO]` `allowedToolsForActiveAgent`, `requiresApproval()` |
| AGT-003 | Ask/Plan/Agent modes | P0 | 1 | `[FATTO]` 3 modalità + 3 agent interni |
| AGT-004 | JSON planner | P0 | 4 | `[PARZIALE]` Plan mode testo; manca JSON schema |
| AGT-005 | Step executor | P0 | 4 | `[PARZIALE]` loop tool; manca esecuzione step isolata |
| AGT-006 | Step verifier | P0 | 4 | `[MANCANTE]` |
| AGT-007 | Risk classifier | P0 | 1 | `[PARZIALE]` euristiche advisory; manca policy formale |
| AGT-008 | PyQGIS sandbox checks | P0 | 1 | `[PARZIALE]` approval dialog + workspace trust |
| AGT-009 | Execution history | P0 | 1 | `[PARZIALE]` audit log + chat history; manca log JSON run |
| AGT-010 | Agent memory per project | P1 | 4 | `[MANCANTE]` |
| AGT-011 | Multi-agent roles | P2 | 8 | `[MANCANTE]` |
| AGT-012 | Background agent | P1 | 8 | `[MANCANTE]` |
| AGT-013 | Human approval queue | P1 | 8 | `[MANCANTE]` |

## C. GIS Tab

| ID | Feature | Priorità | Fase | Stato |
|---|---|---:|---:|---|
| TAB-001 | Rule-based suggestion engine | P0 | 3 | `[MANCANTE]` |
| TAB-002 | CRS mismatch suggestion | P0 | 3 | `[MANCANTE]` |
| TAB-003 | Spatial index suggestion | P0 | 3 | `[MANCANTE]` |
| TAB-004 | Fix geometries suggestion | P0 | 3 | `[MANCANTE]` |
| TAB-005 | Styling suggestion | P0 | 3 | `[MANCANTE]` |
| TAB-006 | Layout suggestion | P1 | 3 | `[MANCANTE]` |
| TAB-007 | Processing error fix | P1 | 3 | `[MANCANTE]` |
| TAB-008 | Workflow detection | P1 | 5 | `[MANCANTE]` |
| TAB-009 | Personalized ranking | P2 | 5 | `[MANCANTE]` |
| TAB-010 | Suggestion analytics | P1 | 3 | `[MANCANTE]` |

## D. Review e diff

| ID | Feature | Priorità | Fase | Stato |
|---|---|---:|---:|---|
| REV-001 | Layer diff | P0 | 4 | `[MANCANTE]` |
| REV-002 | Style diff | P0 | 4 | `[MANCANTE]` |
| REV-003 | Layout diff | P1 | 4 | `[MANCANTE]` |
| REV-004 | File diff/provenance | P0 | 5 | `[PARZIALE]` diff file via `qgsaireviewpatchengine` |
| REV-005 | GIS review checks | P0 | 9 | `[MANCANTE]` |
| REV-006 | Rules-based review | P0 | 9 | `[MANCANTE]` |
| REV-007 | Fix suggestions | P1 | 9 | `[MANCANTE]` |
| REV-008 | Review report export | P1 | 9 | `[MANCANTE]` |
| REV-009 | Ignore false positive | P1 | 9 | `[MANCANTE]` |

## E. Workflow Composer

| ID | Feature | Priorità | Fase | Stato |
|---|---|---:|---:|---|
| WFL-001 | `.strataflow` schema | P0 | 5 | `[MANCANTE]` |
| WFL-002 | Save run as workflow | P0 | 5 | `[MANCANTE]` |
| WFL-003 | Workflow runner | P0 | 5 | `[MANCANTE]` |
| WFL-004 | Parameter editor | P0 | 5 | `[MANCANTE]` |
| WFL-005 | Workflow graph UI | P1 | 5 | `[MANCANTE]` |
| WFL-006 | Export PyQGIS | P0 | 5 | `[MANCANTE]` |
| WFL-007 | Export Processing model | P1 | 5 | `[MANCANTE]` |
| WFL-008 | Workflow templates | P1 | 6 | `[MANCANTE]` |
| WFL-009 | Workflow versioning | P1 | 7 | `[MANCANTE]` |
| WFL-010 | Workflow approval | P1 | 7 | `[MANCANTE]` |

## F. Enterprise

| ID | Feature | Priorità | Fase | Stato |
|---|---|---:|---:|---|
| ENT-001 | Workspace/org model | P0 | 7 | `[MANCANTE]` |
| ENT-002 | Roles and permissions | P0 | 7 | `[MANCANTE]` |
| ENT-003 | Model allowlist/blocklist | P0 | 7 | `[MANCANTE]` |
| ENT-004 | Data policy controls | P0 | 7 | `[PARZIALE]` consents locali AI; manca admin policy |
| ENT-005 | Audit log | P0 | 7 | `[PARZIALE]` `qgsaiauditlog` locale; manca audit enterprise |
| ENT-006 | Usage analytics | P1 | 7 | `[MANCANTE]` |
| ENT-007 | SSO/SAML | P1 | 7 | `[MANCANTE]` |
| ENT-008 | SCIM | P2 | 7 | `[MANCANTE]` |
| ENT-009 | On-prem deployment | P0 | 7 | `[MANCANTE]` |
| ENT-010 | Air-gapped mode | P2 | 8 | `[MANCANTE]` |

---

# 6. Metriche

## Product metrics

| Metrica | Target iniziale |
|---|---:|
| Activation rate | > 40% degli installati completano primo task |
| Time to first value | < 10 minuti |
| Agent task success rate | > 70% MVP, > 85% fase 4 |
| GIS Tab acceptance rate | > 20% fase 3, > 35% fase 5 |
| Workflow rerun success | > 80% |
| Report generation success | > 85% |
| Crash-free sessions | > 98% |
| Weekly active users / installs | > 25% |
| Prompt-to-output median time | < 5 minuti per workflow demo |

## Business metrics

| Metrica | Target |
|---|---:|
| Utenti beta attivi | 100–300 |
| Team pilota | 5–10 |
| Conversione beta → paid | 5–10% |
| Prezzo individuale test | 15–30 €/mese |
| Prezzo team test | 300–1.000 €/mese |
| Enterprise pilot | 3–10k €/anno iniziale |
| Vertical pack paid adoption | > 20% utenti paganti |

## Quality metrics

| Metrica | Target |
|---|---:|
| Azioni distruttive senza conferma | 0 |
| Invio geometrie cloud senza opt-in | 0 |
| Workflow con provenance completa | > 95% |
| Rollback riusciti | > 90% |
| False positive GIS Review | < 30% iniziale, < 15% maturo |

---

# 7. Sequenza consigliata

> **Nota:** la sequenza originale assumeva partenza da zero. A giugno 2026 Fase 1 (~75%) e Fase 2 (~60%) hanno basi solide. Gli sprint sotto partono dai **gap reali** e saltano ciò che è già `[FATTO]`.

## Già completato (non ripetere)

- Tool registry (21 tool), Ask/Plan/Agent modes, `run_python` + approval
- Workspace trust, audit log, review/diff/rollback file
- RAG locale SQLite, semantic search, layer indexing, privacy consents
- Release pipeline multi-piattaforma, macOS sign+notarize in CI, landing page

## Primo trimestre (Q3 2026)

```text
Completare Fase 1 gap + Fase 4 inizio + Fase 0 gap critici
```

Obiettivo: tool GIS nativi, diff/rollback GIS, demo ripetibile.

### Sprint 1 — Tool GIS nativi

- `run_processing_algorithm` tool con schema parametri.
- `inspect_project`, `inspect_crs` tool strutturati.
- `style_layer`, `create_layout`, `export_map` tool.
- Risk classification formale (low/medium/high/critical) con policy automatica.

### Sprint 2 — GIS diff e execution log

- GIS diff v1 (layer metadata: feature count, CRS, campi).
- Execution log JSON strutturato per run.
- Step verifier base (output esiste, CRS coerente, feature count plausibile).
- JSON planner (schema validabile oltre a `<proposed_plan>` testo).

### Sprint 3 — Fase 0 gap critici

- Demo project bundled (`municipality_boundary.gpkg`, `parcels.gpkg`, `land_use.gpkg`, `roads.gpkg` + `.qgz`).
- First-run onboarding wizard AI (provider → privacy → modello → demo).
- Checksum SHA256 nelle release GitHub.
- Guida “5 task in 5 minuti”.

### Sprint 4 — Context Engine completamento

- Project graph strutturato (CRS, stili, layout, processing history).
- Context packs espliciti (`crs_debug`, `styling`, ecc.).
- Context preview UI.
- Layer cards formato YAML.

## Secondo trimestre (Q4 2026)

```text
Fase 3 GIS Tab + Fase 4 maturo + inizio Fase 5
```

Obiettivo: Strata sembra “Cursor-like” con suggerimenti contestuali e workflow salvabili.

### Sprint 5 — GIS Tab v1

- Suggestion engine rule-based.
- 10 trigger: CRS mismatch, spatial index, fix geometries, styling, layout.
- UI inline chip + shortcut Tab/Esc.

### Sprint 6 — Agent v2 maturo

- GIS diff stile e layout.
- Rollback GIS v1 (layer temporanei, stile QML, layout snapshot).
- Agent run history consultabile.
- 20 workflow multi-step demo affidabili.

### Sprint 7 — Workflow Composer inizio

- Schema `.strataflow` v1.
- Save run as workflow.
- Workflow runner base.

### Sprint 8 — Fase 0 completamento + beta

- Auto-update in-app (almeno download guidato + canale stable).
- Telemetria opt-in (feature usate, task success/failure).
- Crash reporting Strata-branded con opt-in upload.
- Benchmark baseline su 5 workflow demo.
- Beta utenti tecnici (100–300).

## Terzo trimestre (Q1 2027)

```text
Fase 5 completa + inizio Fase 6
```

Obiettivo: workflow riproducibili e output vendibili.

### Sprint 9

- Parameter editor workflow.
- Export PyQGIS da workflow.
- Provenance metadata su ogni output.

### Sprint 10

- Report generator v1 (PDF tecnico con mappa + tabella + log).
- Export Processing model.
- Workflow Composer UI v1.

### Sprint 11

- Vertical pack PA/urbanistica (5+ workflow).
- Template report PA.
- Rules pack PA.

### Sprint 12

- Vertical pack agricoltura.
- Dry-run workflow.
- Artifact bundle per run.

## Quarto trimestre (Q2 2027)

```text
Fase 6 + inizio Fase 7
```

Obiettivo: monetizzazione verticale e primo pilota enterprise.

### Sprint 13–14

- Pack ambiente + utilities.
- Pack validation e sample data.

### Sprint 15–16

- Admin policy dati, model allowlist.
- Audit log enterprise.
- On-prem beta, team workspace.
- Primo pilota enterprise/ente.

---

# 8. Priorità assolute

Se si dovesse ridurre tutto a 10 feature, ecco lo stato e il focus:

| # | Feature | Stato | Focus |
|---:|---|---|---|
| 1 | **Tool registry GIS sicuro** | `[FATTO]` | Estendere con tool GIS nativi (Processing, style, layout, export) |
| 2 | **Context Engine locale** | `[PARZIALE ~60%]` | Completare project graph, context packs, preview UI |
| 3 | **Layer cards leggibili dall’agente** | `[PARZIALE]` | Formalizzare formato YAML; oggi via `describe_layer` |
| 4 | **Plan mode con JSON validabile** | `[PARZIALE]` | Plan mode testo funziona; manca JSON schema |
| 5 | **Agent execution step-by-step** | `[PARZIALE]` | Loop tool ok; manca verifier e step isolati |
| 6 | **GIS diff e rollback** | `[MANCANTE]` | Solo diff file; priorità alta |
| 7 | **GIS Tab rule-based** | `[MANCANTE]` | Priorità alta dopo tool GIS nativi |
| 8 | **Save run as workflow** | `[MANCANTE]` | Dipende da `.strataflow` schema |
| 9 | **Report tecnico automatico** | `[MANCANTE]` | — |
| 10 | **Vertical pack PA/agri** | `[MANCANTE]` | Dopo workflow composer |

**Prossime 3 priorità concrete (ordine di esecuzione):**

1. Tool GIS nativi + risk classification formale (chiude gap Fase 1).
2. GIS diff/rollback + execution log strutturato (chiude gap Fase 4).
3. Demo project + onboarding wizard (chiude gap Fase 0 critici).

Queste 10 feature creano il loop di valore:

```text
contesto → piano → azione → verifica → diff → output → workflow → report
```

Questo è il loop che Strata deve possedere.

---

# 9. Cosa evitare

## Evitare 1: chat generica

Una chat che risponde a domande su QGIS è facilmente replicabile.

Meglio:

```text
Prompt → piano → azione GIS → verifica → output
```

## Evitare 2: fork troppo pesante prima della validazione

Il fork dà controllo, ma aumenta il costo di manutenzione. Conviene tenere separati:

```text
strata-core-agent
strata-context-engine
strata-qgis-ui
strata-cli
strata-workers
```

Così il valore resta riusabile anche se domani serve un plugin, un server o un web GIS.

## Evitare 3: agenti troppo autonomi

Nel GIS professionale, un agente che modifica dati senza review è pericoloso. Serve un modello tipo:

```text
autonomia bassa per modifiche persistenti
autonomia media per layer temporanei
autonomia alta per analisi read-only
```

## Evitare 4: partire dal mercato generico

Meglio validare su 1–2 verticali:

```text
PA locale
agricoltura
ambiente
urbanistica
utilities
```

---

# 10. Roadmap compatta

```markdown
## Phase 0 — Product foundation [PARZIALE ~30%]
- [PARZIALE] Signed installers (macOS CI only)
- [PARZIALE] Auto-update (check versione, no install in-app)
- [MANCANTE] Demo project
- [PARZIALE] First-run onboarding (banner only)
- [PARZIALE] Crash/log telemetry opt-in (crash handler upstream)

## Phase 1 — Operational assistant MVP [PARZIALE ~75%]
- [FATTO] Ask/Plan/Agent modes
- [FATTO] Tool registry (21 tool)
- [PARZIALE] PyQGIS safety layer (approval, manca policy formale)
- [MANCANTE] Processing tools nativi
- [PARZIALE] Execution logs (audit log, manca JSON run)
- [PARZIALE] 10 base workflows (via run_python, non tool nativi)

## Phase 2 — Context Engine [PARZIALE ~60%]
- [PARZIALE] Project graph (snapshot layer, manca grafo completo)
- [PARZIALE] Layer cards (describe_layer, manca YAML)
- [PARZIALE] CRS/field/geometry summaries
- [FATTO] Semantic workspace search
- [PARZIALE] Privacy-aware context packs
- [FATTO] Sensitive data guard

## Phase 3 — GIS Tab [MANCANTE 0%]
- [MANCANTE] Contextual next-action suggestions
- [MANCANTE] CRS mismatch fixes
- [MANCANTE] Spatial index suggestions
- [MANCANTE] Geometry QA suggestions
- [MANCANTE] Styling/layout suggestions
- [MANCANTE] Suggestion ranking and memory

## Phase 4 — Mature Agent Mode [PARZIALE ~40%]
- [PARZIALE] JSON planner (testo only)
- [PARZIALE] Step executor (loop tool)
- [MANCANTE] Step verifier
- [MANCANTE] GIS diff
- [PARZIALE] Rollback (file only)
- [PARZIALE] Agent run history (chat history)
- [PARZIALE] Multi-step workflow reliability

## Phase 5 — Workflow Composer [MANCANTE ~5%]
- [MANCANTE] Save run as `.strataflow`
- [MANCANTE] Workflow runner
- [MANCANTE] Parameter editor
- [MANCANTE] Export PyQGIS
- [MANCANTE] Export QGIS Processing model
- [MANCANTE] Provenance
- [MANCANTE] Report generator

## Phase 6 — Vertical packs [MANCANTE 0%]
- [MANCANTE] PA/urbanistica pack
- [MANCANTE] Agricoltura pack
- [MANCANTE] Ambiente/forestazione pack
- [MANCANTE] Utilities pack
- [MANCANTE] Rules, templates, workflows, reports

## Phase 7 — Team/Enterprise [MANCANTE 0%]
- [MANCANTE] Workspaces
- [MANCANTE] Roles
- [MANCANTE] Model policies
- [PARZIALE] Data policies (consents locali)
- [PARZIALE] Audit logs (locale minimo)
- [MANCANTE] SSO/SAML
- [MANCANTE] On-prem/private cloud
- [MANCANTE] Usage analytics

## Phase 8 — CLI and GIS workers [MANCANTE 0%]
- [MANCANTE] Strata CLI
- [MANCANTE] QGIS headless workers
- [MANCANTE] Batch workflow execution
- [MANCANTE] Scheduled QA/reporting
- [MANCANTE] Docker/Kubernetes deployment
- [MANCANTE] Artifact bundles

## Phase 9 — GIS Review [MANCANTE 0%]
- [MANCANTE] Project review
- [MANCANTE] Layer review
- [MANCANTE] Workflow review
- [MANCANTE] Output review
- [MANCANTE] Rules-based QA
- [MANCANTE] Fix suggestions
- [MANCANTE] Review reports

## Phase 10 — Marketplace and SDK [MANCANTE 0%]
- [MANCANTE] Tool SDK
- [PARZIALE] Skills (rules/skills workspace)
- [MANCANTE] Rules packs
- [MANCANTE] Workflow packs
- [MANCANTE] Report templates
- [MANCANTE] Data connectors
- [MANCANTE] Verified marketplace
```

---

# 11. Posizionamento finale desiderato

## Prima versione vendibile

> **Strata è un assistant AI per QGIS che capisce il progetto, genera workflow Processing/PyQGIS, produce mappe e report, e mantiene tutto tracciabile.**

## Versione matura

> **Strata è il Cursor dei GIS: un ambiente agentico per analisi geospaziale, cartografia, QA dati e report tecnici, con deployment locale, team governance e workflow riproducibili.**

## Claim pratico

> “Da richiesta tecnica a mappa, analisi e report verificabile in pochi minuti.”

---

# 12. Note operative finali

**Stato a giugno 2026:** Strata ha superato l’MVP chat. Il cuore AI (`src/app/ai/`, ~68 file) è production-grade: dock assistant, 5 provider, 21 tool, RAG locale, review file. Il prossimo salto di valore non è “più chat”, ma **azioni GIS native verificabili** (tool Processing, diff/rollback GIS, workflow riproducibili).

La priorità vera è:

```text
tool GIS nativi + diff/rollback GIS
→ completare Fase 0 (demo, onboarding)
→ GIS Tab
→ workflow riproducibili (.strataflow)
→ verticali
→ enterprise
```

Se Strata salta direttamente a “agent autonomo” senza diff, rollback e provenance, rischia di sembrare potente ma non affidabile.  
Se invece costruisce il loop:

```text
contesto → piano → azione → verifica → diff → output → workflow → report
```

può diventare un prodotto realmente differenziato rispetto a una semplice chat dentro QGIS.

