#!/bin/bash
# verificer_paa_enhed.sh — enhedsløkke for funktion 23 (distributionsartefakt_rpm) i én kommando.
# Kørsel på Jolla Phone 2026:
#   verificer_paa_enhed.sh          henter seneste GitHub Release-asset (aarch64.rpm)
#   verificer_paa_enhed.sh v1.1.0   pusher først tagget (git push origin v1.1.0 udløser
#                                   release.yml: mål-byg aarch64 + GitHub Release) og venter
#                                   på det nye release, så intet manuelt trin står mellem
#                                   byg og hentning (planen: automatisering af enhedsløkke)
# Kun github.com bruges direkte: seneste tag løses via redirect fra releases/latest
# (url_effective), asset hentes via download-URL releases/download/<tag>/<fil>.
# GitHubs API-endepunkt giver 403 på enheden (målt 2026-11-25, ordre fra-13) og bruges ikke.
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

# Løser et release-tag via github.com-redirect (uden API):
#   curl -sIL -o /dev/null -w '%{url_effective}' https://github.com/<repo>/releases/latest
#   → https://github.com/<repo>/releases/tag/<tag>
tag_via_redirect() {
    curl -sIL -o /dev/null -w '%{url_effective}' "https://github.com/${REPO}/releases/$1" \
        | sed -E 's#.*/releases/tag/##'
}

# Finder aarch64.rpm-asset-navnet for et tag uden API: expanded_assets/<tag> først, fallback releases/tag/<tag>
asset_navn_for_tag() {
    local navn
    navn="$(curl -fsSL --retry 2 "https://github.com/${REPO}/releases/expanded_assets/$1" \
        | grep -oE "${APP}-[0-9][^\"']*\.aarch64\.rpm" | head -1 || true)"
    if [ -z "$navn" ]; then
        navn="$(curl -fsSL --retry 2 "https://github.com/${REPO}/releases/tag/$1" \
            | grep -oE "${APP}-[0-9][^\"']*\.aarch64\.rpm" | head -1 || true)"
    fi
    echo "$navn"
}

diagnose_asset_opslag() {
    echo "diagnose: expanded_assets http $(curl -s -o /dev/null -w '%{http_code}' "https://github.com/${REPO}/releases/expanded_assets/$1") / release-side http $(curl -s -o /dev/null -w '%{http_code}' "https://github.com/${REPO}/releases/tag/$1")"
}

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
    ASSET_NAVN=""
    while [ "$VENT" -lt 90 ]; do
        ASSET_NAVN="$(asset_navn_for_tag "$TAG")"
        if [ -n "$ASSET_NAVN" ]; then
            break
        fi
        VENT=$((VENT + 1))
        sleep 30
    done
    if [ -z "$ASSET_NAVN" ]; then
        echo "FEJL: intet ${APP}-*.aarch64.rpm-asset i release for tag $TAG efter 45 min ventetid"
        diagnose_asset_opslag "$TAG"
        exit 1
    fi
    TAG_OP="$TAG"
else
    trin "løser seneste tag via github.com-redirect (releases/latest)"
    TAG_OP="$(tag_via_redirect latest)"
    if ! [[ "$TAG_OP" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        echo "FEJL: kunne ikke løse seneste tag via redirect, fik: $TAG_OP"
        exit 1
    fi
    echo "seneste tag: $TAG_OP"
    ASSET_NAVN="$(asset_navn_for_tag "$TAG_OP")"
    if [ -z "$ASSET_NAVN" ]; then
        echo "FEJL: fandt intet ${APP}-*.aarch64.rpm-asset i release $TAG_OP"
        diagnose_asset_opslag "$TAG_OP"
        exit 1
    fi
fi

ASSET_URL="https://github.com/${REPO}/releases/download/${TAG_OP}/${ASSET_NAVN}"
trin "henter asset via direkte download-URL (${TAG_OP}/${ASSET_NAVN})"
RPM="${TMP}/${ASSET_NAVN}"
curl -fsSL --retry 2 "$ASSET_URL" -o "$RPM"
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
VERDICTSTI="$HOME/.local/share/harbour/harbour-skrynja-2/selvtest-verdict.json"
if [ -f "$VERDICTSTI" ]; then
    echo "verdict: $(cat "$VERDICTSTI")"
fi
LOGSTI="$HOME/.local/share/harbour/harbour-skrynja-2/skrynja-2.log"
if [ -f "$LOGSTI" ]; then
    echo "log (sidste 10 linjer):"
    tail -n 10 "$LOGSTI"
fi
echo "exit-kode: $KODE"
if [ "$KODE" -ne 0 ]; then
    echo "FEJL: --selvtest gav exit $KODE (forventet 0)"
    exit "$KODE"
fi
echo "ENHED OK: release installeret, rpm-indhold verificeret, --selvtest exit 0"
