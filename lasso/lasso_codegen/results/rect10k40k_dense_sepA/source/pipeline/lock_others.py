"""Hide every other solver from a running generation (chmod 000), and restore it.

Usage: python lock_others.py lock NEW_RUN_DIR MANIFEST
       python lock_others.py unlock MANIFEST

lock: every solver source, optimization report and agent log of other runs (scratch
and $HOME), the other agents' transcripts under ~/.claude/projects and the files
this user owns in /tmp are set to mode 000; MANIFEST records each path and its mode.
FOMTools itself, the pipeline, NEW_RUN_DIR and unrelated projects are left alone.
unlock: restores every mode recorded in MANIFEST (idempotent).
"""

from __future__ import annotations

import os
import re
import stat
import sys

HOME = os.path.expanduser("~")
SCRATCH = "/scratch/gpfs/BSTELLATO/dd7022"
KEEP = [HOME + "/fomtools", HOME + "/fomtools_rect10k40k_k32", HOME + "/fomtools_full_pipeline",
        HOME + "/.local", HOME + "/.cache", HOME + "/.conda", HOME + "/.config",
        HOME + "/codex_mip_balas_gpu.ZiFAli", HOME + "/pdhcg_src", SCRATCH + "/gcs_balas_campaign",
        SCRATCH + "/quarantine_2026-10-16"]


def wanted(dirpath: str, name: str) -> bool:
    if ("solver" in name and name.endswith((".c", ".cu", ".orig", ".bak"))) or name.startswith("fom_solver."):
        return True
    if name.startswith("OPTIMIZATIONS") and name.endswith(".md"):
        return True
    if re.match(r"(agent_phase_|fomtools_phase_).*\.log$", name):
        return True
    return "/.claude/projects/" in dirpath + "/" and "fomtools-codegen-runs" in dirpath


def lock(new: str, manifest: str) -> int:
    new = os.path.realpath(new)
    new_project = re.sub(r"[^A-Za-z0-9-]", "-", new)
    keep = KEEP + [new]
    paths = []
    for root in (SCRATCH, HOME):
        for dirpath, dirs, files in os.walk(root):
            if any(dirpath == k or dirpath.startswith(k + "/") for k in keep) or new_project in dirpath:
                dirs[:] = []
                continue
            paths += [os.path.join(dirpath, f) for f in files if wanted(dirpath, f)]
    for dirpath, dirs, files in os.walk("/tmp"):
        dirs[:] = [d for d in dirs if not d.startswith("claude-")]
        for f in files:
            p = os.path.join(dirpath, f)
            try:
                st = os.lstat(p)
            except OSError:
                continue
            if st.st_uid == os.getuid() and stat.S_ISREG(st.st_mode) and not f.startswith("fom_deploy"):
                paths.append(p)
    locked = 0
    with open(manifest, "a") as out:
        for p in paths:
            try:
                st = os.lstat(p)
                if not stat.S_ISREG(st.st_mode) or st.st_uid != os.getuid():
                    continue
                mode = stat.S_IMODE(st.st_mode)
                if mode == 0:
                    continue
                out.write(f"{mode:o}\t{p}\n")
                out.flush()
                os.chmod(p, 0)
                locked += 1
            except OSError:
                continue
    print(f"LOCKED {locked} files (manifest {manifest})")
    return 0


def unlock(manifest: str) -> int:
    restored = failed = 0
    for line in open(manifest):
        mode, path = line.rstrip("\n").split("\t", 1)
        try:
            os.chmod(path, int(mode, 8))
            restored += 1
        except OSError:
            failed += 1
    print(f"UNLOCKED {restored} files, {failed} missing")
    return 0


if __name__ == "__main__":
    if sys.argv[1] == "lock":
        raise SystemExit(lock(sys.argv[2], sys.argv[3]))
    raise SystemExit(unlock(sys.argv[2]))
