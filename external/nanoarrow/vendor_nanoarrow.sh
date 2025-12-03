#!/bin/bash

# Download and extract the latest Nanoarrow.

main() {
    local -r repo_url="https://github.com/apache/arrow-nanoarrow"
    # Check releases page: https://github.com/apache/arrow-nanoarrow/releases/
    local -r commit_sha=2cfba631b40886f1418a463f3b7c4552c8ae0dc7

    echo "Fetching $commit_sha from $repo_url"
    SCRATCH=$(mktemp -d)
    trap 'rm -rf "$SCRATCH"' EXIT
    local -r tarball="$SCRATCH/nanoarrow.tar.gz"
    wget -O "$tarball" "$repo_url/archive/$commit_sha.tar.gz"

    tar --strip-components 1 -C "$SCRATCH" -xf "$tarball"

    # Build the bundle
    python "$SCRATCH/ci/scripts/bundle.py" \
        --symbol-namespace=QgisPrivate \
        --output-dir="$(pwd)"
}

main "$@"
