(function () {
  const root = document.getElementById("hero-demo-carousel");
  if (!root || typeof DEMO_CHAPTERS === "undefined") return;

  const prefersReducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  let activeIndex = 0;
  const loadedChapters = new Set();

  function t(key, fallback) {
    const lang = document.documentElement.lang || "it";
    const dict = window.STRATA_TRANSLATIONS && window.STRATA_TRANSLATIONS[lang];
    return (dict && dict[key]) || fallback || key;
  }

  function chapterTitle(chapter) {
    const lang = document.documentElement.lang || "it";
    const dict = window.STRATA_TRANSLATIONS;
    return dict?.[lang]?.[chapter.titleKey] || dict?.it?.[chapter.titleKey] || chapter.slug;
  }

  function buildMarkup() {
    root.innerHTML = `
      <div class="demo-carousel">
        <div class="demo-carousel-tabs" role="tablist" aria-label="Demo chapters"></div>
        <p class="demo-carousel-title" id="demo-carousel-title"></p>
        <div class="demo-carousel-viewer-wrap">
          <button type="button" class="demo-carousel-nav demo-carousel-nav-prev" aria-label="${t("demo.carousel.prev", "Previous")}">‹</button>
          <div class="demo-carousel-viewer" id="demo-carousel-viewer"></div>
          <button type="button" class="demo-carousel-nav demo-carousel-nav-next" aria-label="${t("demo.carousel.next", "Next")}">›</button>
        </div>
      </div>
    `;

    const tablist = root.querySelector(".demo-carousel-tabs");
    DEMO_CHAPTERS.forEach((chapter, index) => {
      const tab = document.createElement("button");
      tab.type = "button";
      tab.className = "demo-carousel-tab";
      tab.role = "tab";
      tab.id = `demo-tab-${chapter.slug}`;
      tab.setAttribute("aria-controls", "demo-carousel-viewer");
      tab.setAttribute("aria-selected", index === 0 ? "true" : "false");
      tab.dataset.index = String(index);
      tab.dataset.i18n = chapter.titleKey;
      tab.textContent = chapterTitle(chapter);
      tablist.appendChild(tab);
    });

    root.querySelector(".demo-carousel-nav-prev").addEventListener("click", () => goTo(activeIndex - 1));
    root.querySelector(".demo-carousel-nav-next").addEventListener("click", () => goTo(activeIndex + 1));
    tablist.addEventListener("click", (event) => {
      const tab = event.target.closest(".demo-carousel-tab");
      if (!tab) return;
      goTo(Number(tab.dataset.index));
    });
  }

  function renderViewer(index) {
    const chapter = DEMO_CHAPTERS[index];
    const viewer = root.querySelector("#demo-carousel-viewer");
    const title = root.querySelector("#demo-carousel-title");
    if (!viewer || !chapter) return;

    title.textContent = chapterTitle(chapter);
    title.id = "demo-carousel-title";

    viewer.innerHTML = "";

    if (prefersReducedMotion || !loadedChapters.has(chapter.slug)) {
      const img = document.createElement("img");
      img.src = chapter.poster;
      img.alt = chapterTitle(chapter);
      img.width = 1280;
      img.height = 720;
      img.decoding = "async";
      img.className = "demo-carousel-media";
      viewer.appendChild(img);
    }

    if (!prefersReducedMotion && loadedChapters.has(chapter.slug)) {
      mountAnimatedMedia(chapter, viewer);
    } else if (!prefersReducedMotion) {
      scheduleChapterLoad(chapter, viewer);
    }
  }

  function mountAnimatedMedia(chapter, viewer) {
    viewer.innerHTML = "";

    const video = document.createElement("video");
    video.className = "demo-carousel-media";
    video.muted = true;
    video.loop = true;
    video.playsInline = true;
    video.autoplay = true;
    video.poster = chapter.poster;
    video.setAttribute("aria-label", chapterTitle(chapter));

    const source = document.createElement("source");
    source.src = chapter.webm;
    source.type = "video/webm";
    video.appendChild(source);

    video.addEventListener("error", () => {
      const img = document.createElement("img");
      img.src = chapter.gif;
      img.alt = chapterTitle(chapter);
      img.width = 1024;
      img.height = 576;
      img.decoding = "async";
      img.className = "demo-carousel-media";
      viewer.innerHTML = "";
      viewer.appendChild(img);
    });

    viewer.appendChild(video);
    video.play().catch(() => {});
  }

  function loadChapter(chapter, viewer) {
    if (loadedChapters.has(chapter.slug) || prefersReducedMotion) return;
    loadedChapters.add(chapter.slug);

    if (DEMO_CHAPTERS[activeIndex].slug === chapter.slug) {
      mountAnimatedMedia(chapter, viewer);
    }

    const preloadLink = document.createElement("link");
    preloadLink.rel = "preload";
    preloadLink.as = "video";
    preloadLink.href = chapter.webm;
    document.head.appendChild(preloadLink);
  }

  function scheduleChapterLoad(chapter, viewer) {
    const run = () => loadChapter(chapter, viewer);
    if ("requestIdleCallback" in window) {
      requestIdleCallback(run, { timeout: 1200 });
    } else {
      setTimeout(run, 1200);
    }
  }

  function preloadAdjacent(index) {
    const neighbors = [index - 1, index + 1].filter((i) => i >= 0 && i < DEMO_CHAPTERS.length);
    neighbors.forEach((i) => {
      const chapter = DEMO_CHAPTERS[i];
      if (loadedChapters.has(chapter.slug)) return;
      const link = document.createElement("link");
      link.rel = "prefetch";
      link.href = chapter.webm;
      link.as = "video";
      document.head.appendChild(link);
    });
  }

  function updateTabs(index) {
    root.querySelectorAll(".demo-carousel-tab").forEach((tab, i) => {
      tab.setAttribute("aria-selected", i === index ? "true" : "false");
    });
  }

  function goTo(index) {
    const total = DEMO_CHAPTERS.length;
    activeIndex = ((index % total) + total) % total;
    updateTabs(activeIndex);
    renderViewer(activeIndex);
    preloadAdjacent(activeIndex);
  }

  buildMarkup();
  goTo(0);

  document.addEventListener("strata:langchange", () => {
    root.querySelectorAll(".demo-carousel-tab").forEach((tab) => {
      const chapter = DEMO_CHAPTERS[Number(tab.dataset.index)];
      tab.textContent = chapterTitle(chapter);
    });
    root.querySelector(".demo-carousel-nav-prev").setAttribute(
      "aria-label",
      t("demo.carousel.prev", "Previous")
    );
    root.querySelector(".demo-carousel-nav-next").setAttribute(
      "aria-label",
      t("demo.carousel.next", "Next")
    );
    renderViewer(activeIndex);
  });

  document.addEventListener("DOMContentLoaded", () => {
    root.querySelectorAll(".demo-carousel-tab").forEach((tab) => {
      const chapter = DEMO_CHAPTERS[Number(tab.dataset.index)];
      tab.textContent = chapterTitle(chapter);
    });
    renderViewer(activeIndex);
  });
})();
