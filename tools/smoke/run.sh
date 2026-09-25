#!/usr/bin/env bash
# Smoke test: builds with ASan/UBSan, plays a scripted game from a fresh save directory,
# and fails on any sanitizer report or non-zero exit. Screenshots land in $OUT (PNG).
#
#   tools/smoke/run.sh [output dir]      (default: build-asan/smoke)
#
# The game runs in a visible window at double speed (SMOKE_SPEED, default 2; game logic is
# frame-based, so speed doesn't change the outcome). Battles are reproducible via a fixed
# POKEMON_SEED. A run that exceeds SMOKE_TIMEOUT seconds (default 900) is killed and fails;
# the replay harness also force-quits the game shortly after the script ends.
#
# Regression check: SMOKE_GOLDEN=<dir> compares every screenshot byte-for-byte with the BMPs
# in <dir> (e.g. the $OUT/bmp folder of a known-good run). Behaviour-preserving refactors must
# reproduce them exactly.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="$ROOT/build-asan"
OUT="${1:-$BUILD/smoke}"

mkdir -p "$BUILD"
if ! { cmake -S "$ROOT" -B "$BUILD" -DPOKEMON_SANITIZE=ON && cmake --build "$BUILD" --parallel; } > "$BUILD/build.log" 2>&1; then
    grep -E 'error' "$BUILD/build.log" || tail -20 "$BUILD/build.log"
    echo "SMOKE TEST FAILED: build error (full log: $BUILD/build.log)"
    exit 1
fi

RUNTIME="$(mktemp -d)"
trap 'rm -rf "$RUNTIME"' EXIT
ln -s "$ROOT/assets" "$RUNTIME/res"
mkdir -p "$RUNTIME/data" "$OUT/bmp"
rm -f "$OUT"/bmp/*.bmp "$OUT"/*.png "$OUT"/*.txt "$OUT"/*.log

# Two launches: a new game played through, then a restart that continues from its save.
run_game() { # <scenario>
    local script="$OUT/$1.txt" status=0
    python3 "$ROOT/tools/smoke/make_playthrough.py" "$ROOT/assets" "$OUT/bmp" "$1" > "$script"
    (cd "$RUNTIME" && POKEMON_REPLAY="$script" POKEMON_SEED="${POKEMON_SEED:-1}" POKEMON_SAVE_DIR="$RUNTIME/data" \
        POKEMON_REPLAY_SPEED="${SMOKE_SPEED:-2}" \
        ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
        perl -e 'alarm shift; exec @ARGV or die' "${SMOKE_TIMEOUT:-900}" "$BUILD/pokemon_uet") > "$OUT/$1.log" 2>&1 || status=$?

    if [ "$status" -eq 142 ]; then
        echo "SMOKE TEST FAILED ($1): timed out after ${SMOKE_TIMEOUT:-900}s (full log: $OUT/$1.log)"
        exit 1
    fi
    if grep -qE 'ERROR: AddressSanitizer|runtime error:' "$OUT/$1.log"; then
        grep -E -A12 'ERROR: AddressSanitizer|runtime error:' "$OUT/$1.log" | head -40
        echo "SMOKE TEST FAILED ($1): sanitizer report (full log: $OUT/$1.log)"
        exit 1
    fi
    if [ "$status" -ne 0 ]; then
        tail -20 "$OUT/$1.log"
        echo "SMOKE TEST FAILED ($1): exit code $status (full log: $OUT/$1.log)"
        exit 1
    fi
}

run_game new-game
if [ -f "$RUNTIME/data/player.sav" ]; then cp "$RUNTIME/data/player.sav" "$OUT/player.sav"; fi
run_game continue

if command -v sips >/dev/null; then
    for f in "$OUT"/bmp/*.bmp; do [ -e "$f" ] && sips -s format png "$f" --out "$OUT/$(basename "${f%.bmp}").png" >/dev/null; done
fi

if [ -n "${SMOKE_GOLDEN:-}" ]; then
    mismatches=0
    for golden in "$SMOKE_GOLDEN"/*.bmp; do
        name="$(basename "$golden")"
        if ! cmp -s "$golden" "$OUT/bmp/$name"; then
            echo "screenshot differs from golden: $name"
            mismatches=$((mismatches + 1))
        fi
    done
    if [ "$mismatches" -ne 0 ]; then
        echo "SMOKE TEST FAILED: $mismatches screenshot(s) differ from $SMOKE_GOLDEN"
        exit 1
    fi
    echo "all screenshots match $SMOKE_GOLDEN"
fi
echo "SMOKE TEST PASSED ($(ls "$OUT" | grep -c png) screenshots in $OUT)"
