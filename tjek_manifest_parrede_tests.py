#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Tjekker at manifestets funktioner og parrede tests hænger sammen 1:1.

- strategisk_rækkefølge skal svare til funktionernes id i samme rækkefølge,
  og stop_og_afvis må ikke være sprunget over (skal ligge før diff_og_uddata).
- hver funktion med passerer: true skal have sin parrede testfil registreret
  i tests/tests.pro, og testklassen + alle testslots skal findes i testfilen.
"""
import json
import os
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
MANIFEST = os.path.join(ROOT, "realiseringsmanifest_for_funktioner_og_parrede_tests.json")
TESTS_PRO = os.path.join(ROOT, "tests", "tests.pro")


def main():
    with open(MANIFEST, encoding="utf-8") as f:
        manifest = json.load(f)
    funktioner = manifest["funktioner"]
    rækkefølge = manifest["strategisk_rækkefølge"]
    ids = [fn["id"] for fn in funktioner]

    rækkefølge_ok = ids == list(rækkefølge)
    stop_ok = False
    if "stop_og_afvis" in rækkefølge and "diff_og_uddata" in rækkefølge:
        stop_ok = rækkefølge.index("stop_og_afvis") < rækkefølge.index("diff_og_uddata")
    if rækkefølge_ok and stop_ok:
        print("rækkefølge ok; stop_og_afvis ikke sprunget over")
    else:
        print("RÆKKEFØLGE FEJL: rækkefølge brudt eller stop_og_afvis sprunget over")
        return 1

    with open(TESTS_PRO, encoding="utf-8") as f:
        tests_pro = f.read()

    ok = 0
    mangler = []
    for fn in funktioner:
        pt = fn["parret_test"]
        testfil = os.path.join(ROOT, pt["testfil"])
        if not os.path.exists(testfil):
            if fn.get("passerer"):
                mangler.append(fn["id"] + " (testfil mangler)")
            continue
        if os.path.basename(pt["testfil"]) not in tests_pro:
            if fn.get("passerer"):
                mangler.append(fn["id"] + " (ikke i tests.pro)")
            continue
        with open(testfil, encoding="utf-8") as f:
            indhold = f.read()
        if pt["testklasse"] not in indhold:
            if fn.get("passerer"):
                mangler.append(fn["id"] + " (testklasse mangler)")
            continue
        slot_mangler = None
        for slot in pt.get("testslots", []):
            if slot not in indhold:
                slot_mangler = slot
                break
        if slot_mangler is not None:
            if fn.get("passerer"):
                mangler.append(fn["id"] + " (testslot mangler: " + slot_mangler + ")")
            continue
        ok += 1

    total = len(funktioner)
    print("%d/%d funktioner har parret testfil registreret i tests/tests.pro" % (ok, total))
    if mangler:
        print("mangler parring for: " + ", ".join(mangler))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
