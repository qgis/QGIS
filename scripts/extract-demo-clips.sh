#!/usr/bin/env bash
# Extract demo chapter clips from the Strata YouTube demo video.
# Usage: ./extract-demo-clips.sh [path-to-strata-video.mp4]

set -euo pipefail

VIDEO="${1:-$HOME/Downloads/strata-gif/strata-video.mp4}"
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DOCS="$REPO_ROOT/docs/assets/demo-chapters"
OUT_FE="$(cd "$REPO_ROOT/../strata-fe" && pwd)/public/demo-chapters"
DURATION=7
OFFSET=4
FPS=12
GIF_WIDTH=1024
WEBM_WIDTH=1280

if [[ ! -f "$VIDEO" ]]; then
  echo "Error: video not found at $VIDEO" >&2
  exit 1
fi

mkdir -p "$OUT_DOCS" "$OUT_FE"
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

# slug:start_time
CHAPTERS=(
  "intro:00:00"
  "case:00:56"
  "fetch:01:31"
  "load:03:09"
  "analysis:04:50"
  "priority:06:36"
  "result:08:35"
)

add_seconds() {
  local start="$1"
  local add="$2"
  python3 - <<PY
from datetime import datetime, timedelta
base = datetime.strptime("$start", "%M:%S")
result = base + timedelta(seconds=$add)
print(f"{result.hour:02d}:{result.minute:02d}:{result.second:02d}")
PY
}

for entry in "${CHAPTERS[@]}"; do
  slug="${entry%%:*}"
  start="${entry#*:}"
  clip_start="$(add_seconds "$start" "$OFFSET")"
  base_name="demo-chapter-${slug}"
  webm_file="$WORK_DIR/${base_name}.webm"
  gif_file="$WORK_DIR/${base_name}.gif"
  poster_png="$WORK_DIR/${base_name}-poster.png"
  poster_webp="$WORK_DIR/${base_name}-poster.webp"
  palette_file="$WORK_DIR/palette-${slug}.png"

  echo "==> ${slug} (start ${clip_start}, ${DURATION}s)"

  ffmpeg -y -ss "$clip_start" -t "$DURATION" -i "$VIDEO" \
    -vf "scale=${WEBM_WIDTH}:-2" -c:v libvpx-vp9 -crf 32 -b:v 0 -an \
    "$webm_file" 2>/dev/null

  ffmpeg -y -ss "$clip_start" -t "$DURATION" -i "$VIDEO" \
    -vf "fps=${FPS},scale=${GIF_WIDTH}:-2:flags=lanczos,palettegen" \
    -update 1 "$palette_file" 2>/dev/null

  ffmpeg -y -ss "$clip_start" -t "$DURATION" -i "$VIDEO" -i "$palette_file" \
    -lavfi "fps=${FPS},scale=${GIF_WIDTH}:-2:flags=lanczos[x];[x][1:v]paletteuse" \
    "$gif_file" 2>/dev/null

  ffmpeg -y -ss "$clip_start" -t 1 -i "$VIDEO" \
    -frames:v 1 -vf "scale=${WEBM_WIDTH}:-2" \
    -update 1 "$poster_png" 2>/dev/null

  cwebp -q 80 "$poster_png" -o "$poster_webp" 2>/dev/null

  cp "$webm_file" "$OUT_DOCS/${base_name}.webm"
  cp "$gif_file" "$OUT_DOCS/${base_name}.gif"
  cp "$poster_webp" "$OUT_DOCS/${base_name}-poster.webp"
  cp "$webm_file" "$OUT_FE/${base_name}.webm"
  cp "$gif_file" "$OUT_FE/${base_name}.gif"
  cp "$poster_webp" "$OUT_FE/${base_name}-poster.webp"

  ls -lh "$OUT_DOCS/${base_name}".*
done

echo "Done. Assets written to:"
echo "  $OUT_DOCS"
echo "  $OUT_FE"
