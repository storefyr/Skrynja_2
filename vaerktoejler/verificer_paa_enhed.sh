#!/bin/bash
# verificer_paa_enhed.sh — enhedsløkke for funktion 23 (distributionsartefakt_rpm).
# Køres på Jolla Phone 2026 i én kommando:
#   hent seneste GitHub Release-asset (aarch64.rpm) → rpm -qpl-tjek →
#   pkcon install-local → appens --selvtest (verdict-JSON + exit-kode).
# Udgangskode 0 = succeskriterium målt på enhed.
set -euo pipefail

REPO="storefyr/Skrynja_2"
APP="harbour-skrynja-2"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "== 1/4 henter seneste GitHub Release-asset (aarch64.rpm) =="
ASSET_URL="$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/latest" \
    | grep -oE "\"browser_download_url\": *\"[^\"]*${APP}-[0-9][^\"]*\.aarch64\.rpm\"" \
    | head -1 | sed -E 's/.*"([^"]+)"$/\1/')"
if [ -z "$ASSET_URL" ]; then
    echo "FEJL: fandt intet ${APP}-*.aarch64.rpm-asset i seneste release"
    exit 1
fi
RPM="${TMP}/$(basename "$ASSET_URL")"
curl -fsSL "$ASSET_URL" -o "$RPM"
echo "hentet: $RPM ($(stat -c%s "$RPM") bytes)"

echo "== 2/4 rpm -qpl: tjek forventede sti-linjer =="
LISTING="$(rpm -qpl "$RPM")"
FORVENTEDE="/usr/bin/harbour-skrynja-2
/usr/share/applications/harbour-skrynja-2.desktop
/usr/share/harbour-skrynja-2/qml
/usr/share/harbour-skrynja-2/qml/harbour-skrynja-2.qml
/usr/share/harbour-skrynja-2/qml/pages
/usr/share/harbour-skrynja-2/qml/pages/DiffVisning.qml
/usr/share/harbour-skrynja-2/qml/pages/KonfigurationsSide.qml
/usr/share/harbour-skrynja-2/qml/pages/StartSide.qml
/usr/share/harbour-skrynja-2/qml/pages/TilladelsesDialog.qml
/usr/share/icons/hicolor/86x86/apps/harbour-skrynja-2.png
/usr/share/icons/hicolor/108x108/apps/harbour-skrynja-2.png
/usr/share/icons/hicolor/128x128/apps/harbour-skrynja-2.png
/usr/share/icons/hicolor/172x172/apps/harbour-skrynja-2.png"
ANTAL=0
while IFS= read -r linje; do
    [ -z "$linje" ] && continue
    echo "$LISTING" | grep -qxF "$linje" || { echo "FEJL: mangler i rpm: $linje"; exit 1; }
    ANTAL=$((ANTAL + 1))
done <<EOF
$FORVENTEDE
EOF
if [ "$ANTAL" -lt 12 ]; then
    echo "FEJL: forventede mindst 12 sti-linjer, tjekkede $ANTAL"
    exit 1
fi
echo "rpm -qpl ok: $ANTAL sti-linjer verificeret (binær, desktop, qml, 4 ikoner)"

echo "== 3/4 pkcon install-local =="
devel-su pkcon install-local -y "$RPM"

echo "== 4/4 kører ${APP} --selvtest =="
set +e
"$APP" --selvtest
KODE=$?
set -e
echo "exit-kode: $KODE"
if [ "$KODE" -ne 0 ]; then
    echo "FEJL: --selvtest gav exit $KODE (forventet 0)"
    exit "$KODE"
fi
echo "ENHED OK: release installeret, rpm-indhold verificeret, --selvtest exit 0"
