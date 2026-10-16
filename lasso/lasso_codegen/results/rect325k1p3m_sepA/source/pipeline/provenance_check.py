"""Provenance gate: was a generated solver written from scratch in this run?

Usage: python provenance_check.py TARGET_DIR [SEARCH_ROOT ...]

Fails (exit 1) if
  * the translated or final solver (phase_a_solver.*, src/fom_solver.*) is
    byte-identical to, or a near copy of (line similarity >= 0.8), any solver
    source found under the search roots outside TARGET_DIR; or
  * the agent logs in TARGET_DIR mention reusing or restoring earlier work, or
    paths of other run directories.
Prints the closest matches either way, so the decision is visible.
"""

from __future__ import annotations

import difflib
import hashlib
import os
import re
import sys
from pathlib import Path

THRESHOLD = 0.8
LOG_PATTERNS = re.compile(
    r"previous session|prior session|earlier session|quarantin|contaminated"
    r"|fomsave|fomtools_results|fomtools_codegen_runs/(?!{own})|/archived|copied (the|a) (solver|previous)",
    re.IGNORECASE,
)


def sha(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    target = Path(sys.argv[1]).resolve()
    roots = [Path(r) for r in sys.argv[2:]] or [Path("/scratch/gpfs/BSTELLATO/dd7022"), Path.home()]
    candidates = [p for p in [*target.glob("phase_a_solver.c*"), *target.glob("src/fom_solver.c*")] if p.is_file()]
    if not candidates:
        print(f"PROVENANCE_FAIL no solver found in {target}")
        return 1

    others: list[Path] = []
    for root in roots:
        # os.walk skips directories it cannot read (e.g. the locked quarantine).
        for dirpath, _, filenames in os.walk(root):
            for name in filenames:
                if not (("solver" in name and name.endswith((".c", ".cu", ".orig", ".bak")))
                        or name.startswith("fom_solver.")):
                    continue
                p = Path(dirpath) / name
                try:
                    if not p.is_file() or target in p.resolve().parents:
                        continue
                except OSError:
                    continue
                others.append(p)
    print(f"compared against {len(others)} solver files under {', '.join(map(str, roots))}")

    ok = True
    for cand in candidates:
        text = cand.read_text(errors="replace").splitlines()
        digest = sha(cand)
        scores = []
        for other in others:
            try:
                if other.stat().st_size > 5_000_000:
                    continue
                if sha(other) == digest:
                    scores.append((1.0, other, "IDENTICAL"))
                    continue
                ratio = difflib.SequenceMatcher(None, text, other.read_text(errors="replace").splitlines(),
                                                autojunk=False).ratio()
                scores.append((ratio, other, ""))
            except OSError:
                continue
        scores.sort(key=lambda s: -s[0])
        print(f"== {cand.relative_to(target)} sha256={digest[:16]}")
        for ratio, other, tag in scores[:5]:
            print(f"   {ratio:.3f} {tag} {other}")
        if scores and scores[0][0] >= THRESHOLD:
            ok = False
            print(f"   -> near copy of an existing solver (>= {THRESHOLD})")

    own = re.escape(target.parent.parent.name)
    pattern = re.compile(LOG_PATTERNS.pattern.replace("{own}", own), re.IGNORECASE)
    for log in sorted(target.glob("agent_phase_*.log")):
        for i, line in enumerate(log.read_text(errors="replace").splitlines(), 1):
            if pattern.search(line):
                ok = False
                print(f"   log hit {log.name}:{i}: {line.strip()[:200]}")

    print("PROVENANCE_OK" if ok else "PROVENANCE_FAIL")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
