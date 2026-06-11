#!/usr/bin/env bash

set -Eeuo pipefail

UPSTREAM_URL="${UPSTREAM_URL:-https://github.com/qgis/QGIS.git}"
UPSTREAM_BRANCH="${UPSTREAM_BRANCH:-master}"
SYNC_BRANCH="${SYNC_BRANCH:-automation/qgis-upstream-master}"
BASE_BRANCH="${BASE_BRANCH:-master}"
CONFLICT_ISSUE_TITLE="${CONFLICT_ISSUE_TITLE:-QGIS upstream sync conflicts}"
DRY_RUN="${DRY_RUN:-false}"

REMOTE_UPSTREAM_REF="refs/remotes/qgis-upstream/${UPSTREAM_BRANCH}"
REPORT_FILE=""

log()
{
  printf '==> %s\n' "$*"
}

abort_merge_if_active()
{
  if git rev-parse -q --verify MERGE_HEAD >/dev/null 2>&1; then
    git merge --abort || true
  fi
}

cleanup()
{
  local status=$?
  if [[ ${status} -ne 0 ]]; then
    abort_merge_if_active
  fi
  if [[ -n "${REPORT_FILE}" && -f "${REPORT_FILE}" ]]; then
    rm -f "${REPORT_FILE}"
  fi
}
trap cleanup EXIT

require_clean_tree()
{
  if [[ -n "$( git status --porcelain )" ]]; then
    git status --short
    echo "Working tree must be clean before syncing upstream." >&2
    exit 1
  fi
}

require_gh()
{
  if [[ "${DRY_RUN}" == "true" ]]; then
    return 0
  fi

  if ! command -v gh >/dev/null 2>&1; then
    echo "GitHub CLI 'gh' is required for PR/issue management." >&2
    exit 1
  fi
}

github_issue_number()
{
  gh issue list \
    --state open \
    --limit 100 \
    --json number,title \
    --jq ".[] | select(.title == \"${CONFLICT_ISSUE_TITLE}\") | .number" \
    | head -n 1
}

write_blocked_report()
{
  local reason="$1"
  local details="$2"
  local base_sha="$3"
  local upstream_sha="$4"

  REPORT_FILE="$( mktemp )"
  {
    echo "# QGIS upstream sync blocked"
    echo
    echo "The nightly Strata sync could not safely merge QGIS upstream."
    echo
    echo "- Base branch: \`${BASE_BRANCH}\`"
    echo "- Sync branch: \`${SYNC_BRANCH}\`"
    echo "- Upstream: \`${UPSTREAM_URL} ${UPSTREAM_BRANCH}\`"
    echo "- Base SHA: \`${base_sha}\`"
    echo "- Upstream SHA: \`${upstream_sha}\`"
    echo "- Reason: ${reason}"
    echo
    echo "## Details"
    echo
    if [[ -n "${details}" ]]; then
      echo '```'
      echo "${details}"
      echo '```'
    else
      echo "No additional details were reported."
    fi
    echo
    echo "## Resolution policy"
    echo
    echo "- Keep Strata release triggers, artifact names, signing/upload behavior, and Strata-specific scripts."
    echo "- Preserve \`ENABLE_AI_ASSISTANT\`, all AI sources/includes/tests, and ONNX/SentencePiece wiring."
    echo "- Accept upstream QGIS changes where compatible, but do not use automatic \`ours\` or \`theirs\` conflict resolution."
    echo "- Resolve conflicts manually in a branch, run CI, merge that PR, and let this sync action run again."
  } > "${REPORT_FILE}"
}

create_or_update_blocked_issue()
{
  local reason="$1"
  local details="$2"
  local base_sha="$3"
  local upstream_sha="$4"

  write_blocked_report "${reason}" "${details}" "${base_sha}" "${upstream_sha}"

  if [[ "${DRY_RUN}" == "true" ]]; then
    log "Dry run: would create or update issue '${CONFLICT_ISSUE_TITLE}'."
    cat "${REPORT_FILE}"
    return 0
  fi

  local issue_number
  issue_number="$( github_issue_number || true )"

  if [[ -n "${issue_number}" ]]; then
    log "Updating existing conflict issue #${issue_number}."
    gh issue edit "${issue_number}" --body-file "${REPORT_FILE}"
  else
    log "Creating conflict issue."
    gh issue create --title "${CONFLICT_ISSUE_TITLE}" --body-file "${REPORT_FILE}"
  fi
}

close_blocked_issue_if_present()
{
  if [[ "${DRY_RUN}" == "true" ]]; then
    return 0
  fi

  local issue_number
  issue_number="$( github_issue_number || true )"
  if [[ -n "${issue_number}" ]]; then
    log "Closing resolved conflict issue #${issue_number}."
    gh issue close "${issue_number}" --comment "The latest upstream sync produced a clean PR from \`${SYNC_BRANCH}\`."
  fi
}

collect_conflicted_files()
{
  git diff --name-only --diff-filter=U
}

declare -a GUARD_FAILURES=()

guard_fail()
{
  GUARD_FAILURES+=( "$*" )
}

assert_path_exists()
{
  local path="$1"
  if [[ ! -e "${path}" ]]; then
    guard_fail "Required Strata path is missing: ${path}"
  fi
}

assert_glob_has_matches()
{
  local pattern="$1"
  shopt -s nullglob
  local matches=( ${pattern} )
  shopt -u nullglob

  if [[ ${#matches[@]} -eq 0 ]]; then
    guard_fail "Required Strata file pattern has no matches: ${pattern}"
  fi
}

assert_git_grep()
{
  local pattern="$1"
  shift

  if ! git grep -n -- "${pattern}" -- "$@" >/dev/null 2>&1; then
    guard_fail "Required pattern '${pattern}' was not found in: $*"
  fi
}

assert_file_contains()
{
  local file="$1"
  local pattern="$2"

  if [[ ! -f "${file}" ]]; then
    guard_fail "Required file is missing: ${file}"
    return
  fi

  if ! grep -Eq "${pattern}" "${file}"; then
    guard_fail "Required pattern '${pattern}' was not found in ${file}"
  fi
}

assert_file_lacks()
{
  local file="$1"
  local pattern="$2"

  if [[ ! -f "${file}" ]]; then
    guard_fail "Required file is missing: ${file}"
    return
  fi

  if grep -Eq "${pattern}" "${file}"; then
    guard_fail "Forbidden regression pattern '${pattern}' was found in ${file}"
  fi
}

run_safety_guards()
{
  GUARD_FAILURES=()

  local conflict_markers
  conflict_markers="$( git grep -n -E '^(<{7}|={7}|>{7})([[:space:]].*)?$' -- . 2>/dev/null || true )"
  if [[ -n "${conflict_markers}" ]]; then
    guard_fail "Conflict markers remain in tracked files:
${conflict_markers}"
  fi

  assert_path_exists "src/app/ai"
  assert_glob_has_matches "src/app/ai/*.h"
  assert_glob_has_matches "src/app/ai/*.cpp"
  assert_glob_has_matches "tests/src/app/testqgsai*"
  assert_path_exists "resources/themes/Strata Auto"
  assert_path_exists "resources/themes/Strata Auto/style.qss"
  assert_path_exists ".github/workflows/release-strata.yml"
  assert_path_exists ".github/workflows/build-macos-qt6.yml"
  assert_path_exists ".github/workflows/windows-qt6.yml"
  assert_path_exists ".github/workflows/package-macos-release.yml"
  assert_path_exists "scripts/build-strata-app.sh"
  assert_path_exists "scripts/run-strata-dev.sh"
  assert_path_exists "scripts/patch-macos-bundle.sh"

  assert_git_grep "ENABLE_AI_ASSISTANT" CMakeLists.txt src/app/CMakeLists.txt tests/src/app/CMakeLists.txt
  assert_git_grep "HAVE_AI_ASSISTANT" src/app/CMakeLists.txt src/app/qgisapp.cpp src/app/qgisapp.h
  assert_git_grep "onnxruntime" CMakeLists.txt src/app/CMakeLists.txt vcpkg/vcpkg.json
  assert_git_grep "sentencepiece" CMakeLists.txt src/app/CMakeLists.txt vcpkg/vcpkg.json

  assert_file_contains ".github/workflows/build-macos-qt6.yml" "strata-app-"
  assert_file_contains ".github/workflows/build-macos-qt6.yml" "strata-sdk-"
  assert_file_contains ".github/workflows/windows-qt6.yml" "strata-windows-qt6"
  assert_file_contains ".github/workflows/windows-qt6.yml" "strata-sdk-x64-windows"
  assert_file_contains ".github/workflows/release-strata.yml" "strata-v"

  assert_file_lacks ".github/workflows/build-macos-qt6.yml" "qgis-app-|name:[[:space:]]*qgis-sdk"
  assert_file_lacks ".github/workflows/windows-qt6.yml" "name:[[:space:]]*qgis-windows-qt6|name:[[:space:]]*qgis-sdk"
  assert_file_lacks ".github/workflows/package-macos-release.yml" "qgis-app-"

  if [[ ${#GUARD_FAILURES[@]} -ne 0 ]]; then
    printf '%s\n' "${GUARD_FAILURES[@]}"
    return 1
  fi
}

write_pr_body()
{
  local base_sha="$1"
  local upstream_sha="$2"
  local body_file="$3"

  {
    echo "Automated nightly sync from \`qgis/QGIS:${UPSTREAM_BRANCH}\` into Strata."
    echo
    echo "- Base branch: \`${BASE_BRANCH}\`"
    echo "- Base SHA: \`${base_sha}\`"
    echo "- Upstream SHA: \`${upstream_sha}\`"
    echo "- Sync branch: \`${SYNC_BRANCH}\`"
    echo
    echo "Safety guards passed before this PR was created:"
    echo
    echo "- No conflict markers in tracked files"
    echo "- Strata AI sources, tests, theme, release scripts, and release workflows are still present"
    echo "- \`ENABLE_AI_ASSISTANT\`, \`onnxruntime\`, and \`sentencepiece\` wiring still exists"
    echo "- Strata release artifact names did not regress to QGIS artifact names"
    echo
    echo "This PR still requires normal CI and human review before merge."
  } > "${body_file}"
}

create_or_update_pr()
{
  local base_sha="$1"
  local upstream_sha="$2"
  local body_file
  body_file="$( mktemp )"
  write_pr_body "${base_sha}" "${upstream_sha}" "${body_file}"

  if [[ "${DRY_RUN}" == "true" ]]; then
    log "Dry run: would create or update PR from ${SYNC_BRANCH} to ${BASE_BRANCH}."
    cat "${body_file}"
    rm -f "${body_file}"
    return 0
  fi

  local head_selector="${SYNC_BRANCH}"
  if [[ -n "${GITHUB_REPOSITORY_OWNER:-}" ]]; then
    head_selector="${GITHUB_REPOSITORY_OWNER}:${SYNC_BRANCH}"
  fi

  local pr_number
  pr_number="$(
    gh pr list \
      --state open \
      --limit 100 \
      --base "${BASE_BRANCH}" \
      --head "${head_selector}" \
      --json number \
      --jq '.[0].number' 2>/dev/null || true
  )"

  if [[ -z "${pr_number}" && "${head_selector}" != "${SYNC_BRANCH}" ]]; then
    pr_number="$(
      gh pr list \
        --state open \
        --limit 100 \
        --base "${BASE_BRANCH}" \
        --head "${SYNC_BRANCH}" \
        --json number \
        --jq '.[0].number' 2>/dev/null || true
    )"
  fi

  if [[ -n "${pr_number}" ]]; then
    log "Updating existing upstream sync PR #${pr_number}."
    gh pr edit "${pr_number}" \
      --title "chore(sync): merge qgis/QGIS ${UPSTREAM_BRANCH}" \
      --body-file "${body_file}"
  else
    log "Creating upstream sync PR."
    gh pr create \
      --base "${BASE_BRANCH}" \
      --head "${SYNC_BRANCH}" \
      --title "chore(sync): merge qgis/QGIS ${UPSTREAM_BRANCH}" \
      --body-file "${body_file}"
  fi

  rm -f "${body_file}"
}

main()
{
  require_clean_tree
  require_gh

  log "Fetching base branch ${BASE_BRANCH} from origin."
  git fetch --prune origin "+refs/heads/${BASE_BRANCH}:refs/remotes/origin/${BASE_BRANCH}"
  git fetch --prune origin "+refs/heads/${SYNC_BRANCH}:refs/remotes/origin/${SYNC_BRANCH}" >/dev/null 2>&1 || true

  log "Fetching QGIS upstream ${UPSTREAM_BRANCH}."
  git fetch --no-tags "${UPSTREAM_URL}" "+refs/heads/${UPSTREAM_BRANCH}:${REMOTE_UPSTREAM_REF}"

  local base_ref="refs/remotes/origin/${BASE_BRANCH}"
  local base_sha
  local upstream_sha
  base_sha="$( git rev-parse "${base_ref}" )"
  upstream_sha="$( git rev-parse "${REMOTE_UPSTREAM_REF}" )"

  git checkout -B "${SYNC_BRANCH}" "${base_ref}"

  log "Merging ${REMOTE_UPSTREAM_REF} into ${SYNC_BRANCH} without automatic conflict strategy."
  if ! git merge --no-ff --no-commit "${REMOTE_UPSTREAM_REF}"; then
    local conflicted_files
    conflicted_files="$( collect_conflicted_files )"
    abort_merge_if_active
    create_or_update_blocked_issue "merge conflicts" "${conflicted_files}" "${base_sha}" "${upstream_sha}"
    exit 1
  fi

  if git diff --quiet && git diff --cached --quiet; then
    log "No upstream changes to sync."
    close_blocked_issue_if_present
    return 0
  fi

  if ! run_safety_guards; then
    local guard_details
    guard_details="$( printf '%s\n' "${GUARD_FAILURES[@]}" )"
    abort_merge_if_active
    create_or_update_blocked_issue "safety guard failure" "${guard_details}" "${base_sha}" "${upstream_sha}"
    exit 1
  fi

  git add -A
  git commit \
    -m "chore(sync): merge qgis/QGIS ${UPSTREAM_BRANCH}" \
    -m "Upstream: ${UPSTREAM_URL}" \
    -m "Upstream SHA: ${upstream_sha}"

  log "Pushing sync branch ${SYNC_BRANCH}."
  if [[ "${DRY_RUN}" == "true" ]]; then
    log "Dry run: skipping push."
  else
    git push --force-with-lease origin "${SYNC_BRANCH}:${SYNC_BRANCH}"
  fi

  create_or_update_pr "${base_sha}" "${upstream_sha}"
  close_blocked_issue_if_present
}

main "$@"
