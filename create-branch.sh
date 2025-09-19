#!/bin/bash

PACKAGE="kf6-kio"

# Parse options
while getopts "f:h" opt; do
  case $opt in
    f) FEDORA_VERSION="$OPTARG" ;;
    h)
      cat <<EOF
Usage: $0 [-f fedora_version]
  -f  Override the Fedora release version (default: the Fedora version "$0" is run at)
  -h  Show this help message

Example:
  $0
  $0 -f 43
EOF
      exit 0
      ;;
    *)
      echo "Usage: $0 [-f fedora_version]" >&2
      exit 1
      ;;
  esac
done

FEDORA_VERSION="${FEDORA_VERSION:-$(rpm -E %fedora)}"

# Fetch the latest version for the package
TMP_OUTPUT=$(mktemp)
printf "\r\e[K🔍 Querying the latest version for package %s" "$PACKAGE"
dnf --releasever="$FEDORA_VERSION" repoquery --queryformat="%{VERSION}" --latest-limit=1 "$PACKAGE" 2>&1 | tee "$TMP_OUTPUT" | while read -r line; do
  printf "\r\e[K🔍 %s" "${line:0:(($COLUMNS - 3))}"
done

if [ "${PIPESTATUS[0]}" -ne 0 ]; then
  printf "\r\e[K❌ Failed to query the latest version for package %s\n" "$PACKAGE"
  rm -f "$TMP_OUTPUT"
  exit 1
fi

VERSION=$(tail -n 1 "$TMP_OUTPUT")
rm -f "$TMP_OUTPUT"

if [ -z "$VERSION" ]; then
  printf "\r\e[K❌ Failed to query the latest version for package %s\n" "$PACKAGE"
  exit 1
fi

printf "\r\e[K📦 Package version: %s\n" "${VERSION:0:(($COLUMNS - 3))}"

# Fetch the corresponding tag from upstream
TAG="v$VERSION"
printf "\r\e[K📥 Fetching tag %s from upstream" "${TAG:0:(($COLUMNS - 3))}"
git fetch --no-tags upstream "refs/tags/$TAG:refs/upstream/$TAG" 2>&1 | while read -r line; do
  printf "\r\e[K📥 %s" "${line:0:(($COLUMNS - 3))}"
done

if [ "${PIPESTATUS[0]}" -ne 0 ]; then
  printf "\r\e[K❌ Failed to fetch tag %s\n" "$TAG"
  exit 1
fi

printf "\r\e[K🏷 Upstream tag: %s\n" "${TAG:0:(($COLUMNS - 3))}"

# Clear git note on the root commit
ROOT_COMMIT=$(git rev-list --max-parents=0 HEAD)

if git notes show "$ROOT_COMMIT" &>/dev/null; then
  git notes remove "$ROOT_COMMIT" &>/dev/null
fi

# Mark branch as the build source in the git note
BRANCH="customize/$TAG"
git notes add -m "$BRANCH" "$ROOT_COMMIT"
printf "\r\e[K📤 Marking %s as the build branch" "$BRANCH"
git push origin refs/notes/commits --force 2>&1 | while read -r line; do
  printf "\r\e[K📤 %s" "${line:0:(($COLUMNS - 3))}"
done

if [ "${PIPESTATUS[0]}" -ne 0 ]; then
  printf "\r\e[K❌ Failed to mark %s as the build branch\n" "$BRANCH"
  exit 1
fi

printf "\r\e[K📝 Root commit note: %s\n" "${BRANCH:0:(($COLUMNS - 3))}"

# Branch off tag
if git show-ref --verify --quiet "refs/heads/$BRANCH"; then
  printf "\r\e[K🟢 Branch %s already exists\n" "$BRANCH"
  git checkout "$BRANCH" &>/dev/null
  exit 0
fi

git checkout "refs/upstream/$TAG" -b "$BRANCH" &>/dev/null
printf "\r\e[K✨ New branch %s created\n" "$BRANCH"
