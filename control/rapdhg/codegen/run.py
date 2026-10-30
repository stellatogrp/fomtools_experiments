"""Reproduce the paper's closed-loop flight study, every repetition, from scratch.

From ``control/rapdhg/``:

    FOMTOOLS_CODEGEN_AGENT=1 uv run python -m codegen.run --force    # reps 1 2 3
    uv run python -m codegen.run --reps 2                            # reuse rep 2's solvers

Each repetition is an independent pair of agent runs, and then everything after
them, without an agent:

1. ``codegen.generate --max-iters 512``: translation, then code optimization on a
   copy (schedule tables of 512, which keeps the agent's benchmark short);
2. ``codegen.extend_tables``: the tables re-scaffolded to ``study.MAX_ITERS``, keeping
   every source the agent wrote, then verified again;
3. ``codegen.run_implementations``: each implementation timed and flown over the
   budget grid;
4. ``codegen.run_figures --reference figure8``: the paper figure at a 40 us budget,
   with the iterations that budget buys each implementation.

Repetition 1 works in ``out/``, the others in ``out/reps/rep<k>/`` (untracked). The
numbers the paper reads go to ``results/`` (``results/reps/rep<k>/``), tracked:
``implementations.csv``, the optimized solver's ``OPTIMIZATIONS.md``, the figure
PDFs, and ``manifest.json`` (fomtools commit, agent times and rounds, cost per
iteration of each implementation, the iterations behind the figure).
"""

from __future__ import annotations

import argparse
import json
import shutil
import sys
from datetime import datetime, timezone
from pathlib import Path

from codegen import extend_tables, generate, run_figures, run_implementations
from codegen.study import rep_out

REPO = Path(__file__).resolve().parents[3]
RESULTS = Path(__file__).parent / "results"

#: Table length the solvers are generated at, before ``extend_tables``.
GENERATION_ITERS = 512


def results_dir(rep: int) -> Path:
    return RESULTS if rep == 1 else RESULTS / "reps" / f"rep{rep}"


def record(rep: int, files: list[Path], **entries) -> Path:
    """Copy ``files`` into the repetition's results folder and write its manifest."""
    if str(REPO) not in sys.path:
        sys.path.append(str(REPO))
    from fomexp.records import code_state, environment

    out = results_dir(rep)
    out.mkdir(parents=True, exist_ok=True)
    for path in files:
        if path.exists():
            shutil.copy2(path, out / path.name)
        else:
            print(f"not recorded: {path} does not exist")
    manifest = dict(
        rep=rep,
        **entries,
        code=code_state(),
        environment=environment(),
        recorded_utc=datetime.now(timezone.utc).isoformat(timespec="seconds"),
    )
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2, default=str) + "\n")
    print(f"recorded {out}")
    return out


def run_rep(rep: int, force: bool) -> None:
    flag = ["--force"] if force else []
    gen = generate.main(["--max-iters", str(GENERATION_ITERS), "--rep", str(rep), *flag])
    extend_tables.main(["--rep", str(rep)])
    csv_path = run_implementations.main(["--no-gif", "--rep", str(rep)])
    fig = run_figures.main(["--reference", "figure8", "--no-gif", "--rep", str(rep)])
    optimizations = rep_out(rep) / "codegen" / "optimized" / "OPTIMIZATIONS.md"
    record(
        rep,
        [csv_path, optimizations, *fig["files"]],
        generation=gen,
        figure=dict(reference=fig["reference"], budget_us=fig["budget_us"], arms=fig["arms"]),
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--reps", type=int, nargs="+", default=[1, 2, 3])
    parser.add_argument("--force", action="store_true", help="regenerate rather than reuse")
    args = parser.parse_args()
    for rep in args.reps:
        print(f"\n=== repetition {rep} ===", flush=True)
        run_rep(rep, args.force)


if __name__ == "__main__":
    main()
