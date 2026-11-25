#!/bin/bash
# verificer_paa_enhed.sh — enhedsløkke for funktion 23 (distributionsartefakt_rpm) i én kommando.
# Kørsel på Jolla Phone 2026:
#   verificer_paa_enhed.sh          henter seneste GitHub Release-asset (aarch64.rpm)
#   verificer_paa_enhed.sh v1.1.0   pusher først tagget (git push origin v1.1.0 udløser
#                                   release.yml: mål-byg aarch64 + GitHub Release) og venter
#                                   på det nye release, så intet manuelt trin står mellem
#                                   byg og hentning (planen: automatisering af enhedsløkke)
# Derefter i samme kørsel: hent asset → rpm -qpl-tjek (12 forventede stier) →
# pkcon install-local → appens --selvtest (verdict-JSON + exit-kode).
# Udgangskode 0 = succeskriterium målt på enhed.
set -euo pipefail

REPO="storefyr/Skrynja_2"
APP="harbour-skrynja-2"
TAG="${1:-}"
if [ -n "$TAG" ] && ! [[ "$TAG" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "FEJL: tag skal have formatet vX.Y.Z (f.eks. v1.1.0), fik: $TAG"
    exit 1
fi
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

TRIN=0
trin() { TRIN=$((TRIN + 1)); echo; echo "== trin $TRIN: $1 =="; }

ASSET_URL=""
if [ -n "$TAG" ]; then
    trin "udløser mål-byg og udgivelse ved tag-push ($TAG → release.yml)"
    if command -v gh >/dev/null 2>&1; then
        gh auth setup-git
    fi
    git clone --depth 1 "https://github.com/${REPO}.git" "$TMP/klon"
    git -C "$TMP/klon" tag "$TAG"
    git -C "$TMP/klon" push origin "$TAG"
    echo "tag $TAG push'et; release.yml bygger aarch64-RPM og opretter GitHub Release"

    trin "venter på release for tag $TAG (intet manuelt trin mellem byg og hentning)"
    VENT=0
    while [ "$VENT" -lt 90 ]; do
        ASSET_URL="$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/tags/${TAG}" 2>/dev/null \
            | grep -oE "\"browser_download_url\": *\"[^\"]*${APP}-[0-9][^\"]*\.aarch64\.rpm\"" \
            | head -1 | sed -E 's/.*"([^"]+)"$/\1/')" || ASSET_URL=""
        if [ -n "$ASSET_URL" ]; then
            break
        fi
        VENT=$((VENT + 1))
        sleep 30
    done
    if [ -z "$ASSET_URL" ]; then
        echo "FEJL: intet ${APP}-*.aarch64.rpm-asset i release for tag $TAG efter 45 min ventetid"
        exit 1
    fi
else
    trin "henter seneste GitHub Release-asset (aarch64.rpm)"
    ASSET_URL="$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/latest" \
        | grep -oE "\"browser_download_url\": *\"[^\"]*${APP}-[0-9][^\"]*\.aarch64\.rpm\"" \
        | head -1 | sed -E 's/.*"([^"]+)"$/\1/')"
fi
if [ -z "$ASSET_URL" ]; then
    echo "FEJL: fandt intet ${APP}-*.aarch64.rpm-asset i seneste release"
    exit 1
fi

trin "henter asset ($(basename "$ASSET_URL"))"
RPM="${TMP}/$(basename "$ASSET_URL")"
curl -fsSL "$ASSET_URL" -o "$RPM"
echo "hentet: $RPM ($(stat -c%s "$RPM") bytes)"

trin "rpm -qpl: tjekker forventede sti-linjer"
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

trin "pkcon install-local"
devel-su pkcon install-local -y "$RPM"

trin "kører ${APP} --selvtest"
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
