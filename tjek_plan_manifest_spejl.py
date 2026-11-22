#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Spejlkontrol (plan ↔ manifest) i værtsløkken — funktion 26.

Tjekker at projektbeskrivelse.adoc og realiseringsmanifestet aldrig diverger:

- planens funktionsrækkefølge (afsnittet "Strategisk rækkefølge") skal svare til
  manifestets strategisk_rækkefølge i samme rækkefølge,
- nøglereglerne (ortografi-maksimum, leveringskæde via GitHub Release,
  værtsløkke/enhedsløkke, spejlingsregel) skal findes i begge artefakter.

En PR, der kun rører det ene artefakt, fejler denne kontrol (exit != 0).

Flag: --plan <sti> og --manifest <sti> (default: artefakterne i depot-roden);
--manifest bruges af den negative kontrol med en temp-kopi af manifestet.
"""
import argparse
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
PLAN = os.path.join(ROOT, "projektbeskrivelse.adoc")
MANIFEST = os.path.join(ROOT, "realiseringsmanifest_for_funktioner_og_parrede_tests.json")

NOEGLEREGLER = [
    "Ortografi",
    "GitHub Release",
    "Værtsløkke",
    "Enhedsløkke",
    "Spejlingsregel",
]

LISTE_RE = re.compile(r"^\.\s+`([^`]+)`\s*$")


def plan_raekkefoelge(plan_sti):
    with open(plan_sti, encoding="utf-8") as f:
        linjer = f.read().split("\n")
    ids = []
    i = 0
    while i < len(linjer) and linjer[i].strip() != "== Strategisk rækkefølge":
        i += 1
    if i == len(linjer):
        return None
    i += 1
    while i < len(linjer):
        m = LISTE_RE.match(linjer[i])
        if m:
            ids.append(m.group(1))
        elif ids and linjer[i].strip() == "":
            break
        elif ids:
            break
        i += 1
    return ids


def main():
    ap = argparse.ArgumentParser(description="Spejlkontrol: planens rækkefølge == manifestets strategisk_rækkefølge; nøgleregler i begge artefakter")
    ap.add_argument("--plan", default=PLAN)
    ap.add_argument("--manifest", default=MANIFEST)
    args = ap.parse_args()

    with open(args.manifest, encoding="utf-8") as f:
        manifest = json.load(f)
    with open(args.plan, encoding="utf-8") as f:
        plan_tekst = f.read()
    with open(args.manifest, encoding="utf-8") as f:
        manifest_tekst = f.read()

    fejl = []

    plan_ids = plan_raekkefoelge(args.plan)
    manifest_ids = list(manifest["strategisk_rækkefølge"])
    if plan_ids is None:
        fejl.append("planen har ikke afsnittet 'Strategisk rækkefølge' med en funktionsliste")
    elif plan_ids != manifest_ids:
        fejl.append("rækkefølge afviger: plan=%d funktioner, manifest=%d funktioner" % (len(plan_ids), len(manifest_ids)))
        for idx, (p, m) in enumerate(zip(plan_ids, manifest_ids)):
            if p != m:
                fejl.append("  første afvigelse på plads %d: plan=%s manifest=%s" % (idx + 1, p, m))
                break

    for regel in NOEGLEREGLER:
        i_plan = regel in plan_tekst
        i_manifest = regel in manifest_tekst
        if not (i_plan and i_manifest):
            fejl.append("nøgleregel mangler: %s (plan=%s, manifest=%s)" % (regel, i_plan, i_manifest))

    if fejl:
        print("FEJL: plan og manifest spejler ikke hinanden:")
        for f in fejl:
            print("  " + f)
        return 1

    print("spejl ok: rækkefølge == strategisk_rækkefølge (%d); nøgleregler i begge artefakter" % len(manifest_ids))
    return 0


if __name__ == "__main__":
    sys.exit(main())
