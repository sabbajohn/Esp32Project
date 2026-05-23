#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/components/brl/drivers/subghz"
DST="$ROOT/core/subghz_legacy"

mkdir -p "$DST"

rm -rf "$DST/protocols" "$DST/blocks"
cp -R "$SRC/protocols" "$DST/"
cp -R "$SRC/blocks" "$DST/"

cp "$SRC/types.h" "$DST/"
cp "$SRC/receiver.c" "$DST/"
cp "$SRC/receiver.h" "$DST/"
cp "$SRC/registry.c" "$DST/"
cp "$SRC/registry.h" "$DST/"
cp "$SRC/environment.c" "$DST/"
cp "$SRC/environment.h" "$DST/"
cp "$SRC/subghz_keystore.c" "$DST/"
cp "$SRC/subghz_keystore.h" "$DST/"
cp "$SRC/subghz_keystore_i.h" "$DST/"

echo "Synced subghz legacy source to: $DST"
