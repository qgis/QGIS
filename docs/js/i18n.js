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

  const translations = {
    it: {
      "meta.title": "Strata — GIS con AI nativa",
      "meta.description":
        "Strata: il GIS con AI nativa. Scarica gratis per macOS, Windows e Linux.",

      "nav.download": "Download",
      "nav.login": "ACCEDI",

      "hero.title": "L'agente AI nativo per il GIS.",
      "hero.subtitle":
        "Scarica, apri il progetto, chiedi cosa ti serve. Cloud gestito o on-premise.",
      "hero.ctaPrimary.macos": "Scarica per macOS",
      "hero.ctaPrimary.windows": "Scarica per Windows",
      "hero.ctaPrimary.linux": "Scarica per Linux",
      "hero.ctaPrimary.fallback": "Scarica Strata",
      "hero.linkEnterprise": "Enterprise",
      "hero.linkDemo": "Vedi demo",
      "hero.linkCloud": "Strata Cloud",
      "hero.demo.alt":
        "Demo Strata: mappa di priorità soccorsi dopo il terremoto in Venezuela",

      "download.title": "Scarica Strata",
      "download.subtitle": "macOS, Windows e Linux. Binari da GitHub Releases.",
      "download.action": "Scarica",
      "download.allReleases": "Tutte le release su GitHub →",
      "download.status.fallback": "Releases GitHub",
      "download.status.complete": "Ultima release: {version}",
      "download.macos.title": "macOS",
      "download.macos.file": "File .dmg",
      "download.windows.title": "Windows",
      "download.windows.file": "File -win64.exe",
      "download.linux.title": "Linux",
      "download.linux.file": "File .AppImage",

      "demo.label": "Demo",
      "demo.title": "Vedilo in azione",
      "demo.desc":
        "Chiedi in linguaggio naturale, ispeziona layer, esegue e mostra il risultato accanto alla mappa.",
      "demo.plan.alt": "Strata in Plan mode con piano approvabile prima delle modifiche",

      "why.label": "Perché Strata",
      "why.title": "Chiedi, approva, fatto",
      "why.1.title": "Fa il lavoro al posto tuo",
      "why.1.desc":
        "Buffer, export, analisi: chiedi in naturale, Strata esegue con la tua approvazione.",
      "why.2.title": "Conosce il tuo progetto",
      "why.2.desc":
        "Layer, script e documenti del workspace come contesto — non snippet generici.",
      "why.3.title": "Tre modalità, una sola app",
      "why.3.desc":
        "Pianifica, esegui o chiedi. Scegli il livello di autonomia senza uscire da Strata.",
      "why.4.title": "Da ore a minuti",
      "why.4.desc":
        "Ispeziona layer, buffer, export GeoJSON: una richiesta al posto di script manuali.",

      "privacy.label": "Sicurezza e privacy",
      "privacy.title": "I tuoi dati, le tue regole",
      "privacy.1.title": "AI 100% offline",
      "privacy.1.desc": "LLM locali per analisi sul territorio, senza internet.",
      "privacy.2.title": "Isolamento dei dati",
      "privacy.2.desc": "Layer e documenti indicizzati su database cifrati interni.",
      "privacy.3.title": "Enterprise on-premise",
      "privacy.3.desc": "Zero egress, audit completo, pieno controllo per dati sensibili.",
      "privacy.enterprise": "Contatta le vendite Enterprise →",

      "plans.label": "Piani",
      "plans.title": "Scegli come adottare Strata",
      "plans.pro.badge": "Pro",
      "plans.pro.title": "Pro — Chiavi in mano",
      "plans.pro.desc": "Abbonamento mensile, AI inclusa, zero API key da gestire.",
      "plans.team.badge": "Team",
      "plans.team.title": "Cloud Team",
      "plans.team.desc": "Contesto condiviso del team, indicizzato in Strata Cloud.",
      "plans.team.cta": "Accedi alla Strata Cloud →",
      "plans.enterprise.badge": "Ent.",
      "plans.enterprise.title": "Enterprise",
      "plans.enterprise.desc": "On-premise con LLM locali. Zero egress, pieno controllo.",

      "team.label": "Team",
      "team.title": "Chi c'è dietro Strata",
      "team.francesco.role": "Co-founder, informatico",
      "team.valerio.role": "Co-founder, esperto GeoAI",
      "team.massimo.role": "Co-founder, esperto GIS",
      "team.demo": "Prenota una demo",

      "closing.title": "Prova Strata oggi.",
      "closing.demo": "Prenota una demo",

      "footer.disclaimer":
        "Strata is an independent commercial service based on QGIS. It is not endorsed by or affiliated with QGIS.ORG or the QGIS project.",
      "footer.github": "GitHub",
      "footer.contact": "Contatto",
    },
    en: {
      "meta.title": "Strata — The AI-native GIS",
      "meta.description":
        "Strata: the AI-native GIS. Download free for macOS, Windows, and Linux.",

      "nav.download": "Download",
      "nav.login": "SIGN IN",

      "hero.title": "The native AI agent for GIS.",
      "hero.subtitle":
        "Download, open your project, ask for what you need. Managed cloud or on-premise.",
      "hero.ctaPrimary.macos": "Download for macOS",
      "hero.ctaPrimary.windows": "Download for Windows",
      "hero.ctaPrimary.linux": "Download for Linux",
      "hero.ctaPrimary.fallback": "Download Strata",
      "hero.linkEnterprise": "Enterprise",
      "hero.linkDemo": "See demo",
      "hero.linkCloud": "Strata Cloud",
      "hero.demo.alt":
        "Strata demo: rescue priority map after the Venezuela earthquake",

      "download.title": "Download Strata",
      "download.subtitle": "macOS, Windows, and Linux. Binaries from GitHub Releases.",
      "download.action": "Download",
      "download.allReleases": "All releases on GitHub →",
      "download.status.fallback": "GitHub Releases",
      "download.status.complete": "Latest release: {version}",
      "download.macos.title": "macOS",
      "download.macos.file": ".dmg file",
      "download.windows.title": "Windows",
      "download.windows.file": "-win64.exe file",
      "download.linux.title": "Linux",
      "download.linux.file": ".AppImage file",

      "demo.label": "Demo",
      "demo.title": "See it in action",
      "demo.desc":
        "Ask in plain language, inspect layers, run, and show results beside the map.",
      "demo.plan.alt": "Strata in Plan mode with an approvable plan before changes",

      "why.label": "Why Strata",
      "why.title": "Ask, approve, done",
      "why.1.title": "Does the work for you",
      "why.1.desc":
        "Buffer, export, analysis: ask in plain language, Strata runs it with your approval.",
      "why.2.title": "Knows your project",
      "why.2.desc":
        "Layers, scripts, and workspace documents as context — not generic snippets.",
      "why.3.title": "Three modes, one app",
      "why.3.desc":
        "Plan, run, or ask. Choose your level of autonomy without leaving Strata.",
      "why.4.title": "From hours to minutes",
      "why.4.desc":
        "Inspect layers, buffer, export GeoJSON: one request instead of manual scripts.",

      "privacy.label": "Security & privacy",
      "privacy.title": "Your data, your rules",
      "privacy.1.title": "100% offline AI",
      "privacy.1.desc": "Local LLMs for territorial analysis, no internet required.",
      "privacy.2.title": "Data isolation",
      "privacy.2.desc": "Layers and documents indexed on encrypted internal databases.",
      "privacy.3.title": "Enterprise on-premise",
      "privacy.3.desc": "Zero egress, full audit, total control for sensitive data.",
      "privacy.enterprise": "Contact Enterprise sales →",

      "plans.label": "Plans",
      "plans.title": "Choose how to adopt Strata",
      "plans.pro.badge": "Pro",
      "plans.pro.title": "Pro — Turnkey",
      "plans.pro.desc": "Monthly subscription, AI included, no API keys to manage.",
      "plans.team.badge": "Team",
      "plans.team.title": "Cloud Team",
      "plans.team.desc": "Shared team context, indexed in Strata Cloud.",
      "plans.team.cta": "Log in to Strata Cloud →",
      "plans.enterprise.badge": "Ent.",
      "plans.enterprise.title": "Enterprise",
      "plans.enterprise.desc": "On-premise with local LLMs. Zero egress, full control.",

      "team.label": "Team",
      "team.title": "The people behind Strata",
      "team.francesco.role": "Co-founder, software engineer",
      "team.valerio.role": "Co-founder, GeoAI specialist",
      "team.massimo.role": "Co-founder, GIS expert",
      "team.demo": "Book a demo",

      "closing.title": "Try Strata today.",
      "closing.demo": "Book a demo",

      "footer.disclaimer":
        "Strata is an independent commercial service based on QGIS. It is not endorsed by or affiliated with QGIS.ORG or the QGIS project.",
      "footer.github": "GitHub",
      "footer.contact": "Contact",
    },
  };

  function detectLanguage() {
    const stored = localStorage.getItem(STORAGE_KEY);
    if (stored && translations[stored]) return stored;
    const browser = (navigator.language || "it").slice(0, 2).toLowerCase();
    return translations[browser] ? browser : "it";
  }

  function detectPlatform() {
    const ua = navigator.userAgent.toLowerCase();
    const platform = (navigator.platform || "").toLowerCase();

    if (/mac|darwin|iphone|ipad/.test(`${platform} ${ua}`)) return "macos";
    if (/win/.test(`${platform} ${ua}`)) return "windows";
    if (/linux/.test(`${platform} ${ua}`) && !/android/.test(ua)) return "linux";
    return null;
  }

  let currentLang = detectLanguage();
  let currentPlatform = detectPlatform();
  let latestCompleteRelease = null;

  function translate(key, replacements = {}) {
    let text = translations[currentLang]?.[key] || translations.it[key] || "";
    Object.entries(replacements).forEach(([name, value]) => {
      text = text.replaceAll(`{${name}}`, value);
    });
    return text;
  }

  function heroDownloadLabel(platform) {
    const key = platform ? `hero.ctaPrimary.${platform}` : "hero.ctaPrimary.fallback";
    return translate(key);
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

  function updateHeroDownloadButtons() {
    const platform = currentPlatform;
    const label = heroDownloadLabel(platform);
    const releaseUrl = latestCompleteRelease?.html_url || RELEASES_URL;
    const asset = platform ? latestCompleteRelease?.requiredAssets?.[platform] : null;

    document.querySelectorAll("[data-hero-download]").forEach((link) => {
      link.textContent = label;
      link.setAttribute("href", asset?.browser_download_url || releaseUrl);
      if (platform) {
        link.setAttribute("data-download-asset", platform);
      }
    });
  }

  function updateDownloadReleaseUi() {
    const releaseUrl = latestCompleteRelease?.html_url || RELEASES_URL;

    document.querySelectorAll("[data-release-link]").forEach((link) => {
      link.setAttribute("href", releaseUrl);
    });

    document.querySelectorAll("[data-download-asset]").forEach((link) => {
      const platform = link.getAttribute("data-download-asset");
      const asset = latestCompleteRelease?.requiredAssets?.[platform];
      if (!link.hasAttribute("data-hero-download") || platform === currentPlatform) {
        link.setAttribute("href", asset?.browser_download_url || releaseUrl);
      }
    });

    document.querySelectorAll("[data-download-file]").forEach((el) => {
      const platform = el.getAttribute("data-download-file");
      const asset = latestCompleteRelease?.requiredAssets?.[platform];
      if (asset?.name) {
        el.textContent = asset.name;
      }
    });

    document.querySelectorAll("[data-release-status]").forEach((status) => {
      status.textContent = latestCompleteRelease
        ? translate("download.status.complete", { version: latestCompleteRelease.version })
        : translate("download.status.fallback");
    });

    updateHeroDownloadButtons();
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
      if (key === "hero.ctaPrimary") return;
      if (dict[key] !== undefined) {
        el.textContent = dict[key];
      }
    });

    document.querySelectorAll("[data-i18n-alt]").forEach((el) => {
      const key = el.getAttribute("data-i18n-alt");
      if (dict[key] !== undefined) {
        el.setAttribute("alt", dict[key]);
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
  }

  function setLanguage(lang) {
    if (!translations[lang]) return;
    currentLang = lang;
    localStorage.setItem(STORAGE_KEY, lang);
    applyTranslations(lang);
  }

  document.addEventListener("DOMContentLoaded", () => {
    applyTranslations(currentLang);
    initDownloadReleaseLinks();

    document.querySelectorAll("[data-lang]").forEach((btn) => {
      btn.addEventListener("click", () => setLanguage(btn.getAttribute("data-lang")));
    });

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

  window.STRATA_I18N = { setLanguage, RELEASES_URL, RELEASES_API_URL, findLatestCompleteRelease, detectPlatform };
})();
