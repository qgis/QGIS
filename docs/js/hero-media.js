(function () {
  const img = document.getElementById("hero-demo-media");
  if (!img) return;

  const animatedSrc = img.dataset.animatedSrc;
  if (!animatedSrc) return;

  const prefersReducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
  if (prefersReducedMotion) return;

  function loadAnimatedMedia() {
    const preload = new Image();
    preload.onload = function () {
      img.src = animatedSrc;
    };
    preload.src = animatedSrc;
  }

  if ("requestIdleCallback" in window) {
    requestIdleCallback(loadAnimatedMedia, { timeout: 1500 });
  } else {
    window.setTimeout(loadAnimatedMedia, 1500);
  }
})();
