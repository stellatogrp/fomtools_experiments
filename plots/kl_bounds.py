"""Sample-based KL quantile bounds next to the MIP bounds of the paper's runs.

    uv run python plots/kl_bounds.py                     # every experiment in make_plots.RUNS
    uv run python plots/kl_bounds.py --experiment lasso

For each variant this draws two FRESH, independent sample sets, a pilot
(``--pilot-seed``) and a test set (``--seed``), both with seeds that differ from
the run's simulator and start-sample seeds, so neither is tied to the MIP bounds.
It runs the simulator on both up to the experiment's K cut-off and applies the
sample convergence bound (Langford and Caruana, 2001; Sambharya and Stellato,
Thm. 3) on the test set:

    r <= kl^{-1}( r_hat | log(2 / delta') / N ),   r = P(residual_K > eps).

The tolerances at each K come from the pilot alone: ``--n-tol`` geometric points
from ``--grid-lo`` times the pilot quantile to ``--grid-hi`` times the pilot max.
The MIP bound is not used, so the KL quantile is a stand-alone comparison and
can land above the MIP bound. With ``--union K`` (default) delta' = delta /
(n_tol * K_cut), so every curve holds for all K at once with probability at
least 1 - delta; with ``--union tol`` it holds per K.

Outputs (nothing under results/ is touched):

- ``plots/kl_<experiment>.csv``: one row per K, per variant: the MIP bound, the
  test sample max, the pilot and test empirical quantiles, the certified
  ``--quantile`` quantile, and the fraction of test samples above the MIP bound;
- ``plots/kl_samples/<experiment>_<variant>.npz``: the pilot and test samples and residuals.
"""

from __future__ import annotations

import argparse
import csv
import subprocess
import sys
from pathlib import Path

import numpy as np
from omegaconf import OmegaConf
from scipy.optimize import brentq

REPO = Path(__file__).resolve().parents[1]
OUT = Path(__file__).resolve().parent
sys.path.insert(0, str(REPO))
sys.path.insert(0, str(OUT))

from make_plots import RUNS, read  # noqa: E402


def kl(q: float, p: float) -> float:
    """Bernoulli KL divergence kl(q || p)."""
    p = min(max(p, 1e-300), 1 - 1e-16)
    out = 0.0
    if q > 0:
        out += q * np.log(q / p)
    if q < 1:
        out += (1 - q) * np.log((1 - q) / (1 - p))
    return out


def kl_inv(q: float, c: float) -> float:
    """sup { p in [q, 1] : kl(q || p) <= c }."""
    if q >= 1 or kl(q, 1 - 1e-16) <= c:
        return 1.0
    return brentq(lambda p: kl(q, p) - c, q, 1 - 1e-16, xtol=1e-14)


def simulate(spec, samples: np.ndarray, K: int) -> np.ndarray:
    """Residuals ``(len(samples), K)``, the same way ``fomexp.kloop.run_simulator`` runs them."""
    from fomtools.verifier import Verifier

    vp = Verifier(method="simulator", obbt_solver=None)
    _, step = spec.setup(vp, "simulator", samples, K + 1)
    for k in range(1, K + 1):
        step(k)
    return np.asarray(vp.solve(), dtype=float)[:, :K]


def run_experiment(name: str, args) -> None:
    from fomexp.run import load_adapter

    run_rel, K_cut, variants = RUNS[name]
    run = REPO / run_rel
    adapter = None
    tables: dict[str, dict[str, np.ndarray]] = {}
    for variant in variants:
        cfg = OmegaConf.load(run / variant / ".hydra" / "config.yaml")
        used = {int(cfg.simulator.seed), int(cfg.schedule.start_samples_seed)}
        for seed in (args.seed, args.pilot_seed):
            if seed in used:
                raise SystemExit(f"seed {seed} was used by {run_rel}/{variant} {sorted(used)}")
        cfg.simulator.k_max = K_cut
        if adapter is None:
            adapter = load_adapter(cfg.experiment.dir)

        def fresh(seed: int, n: int):
            cfg.simulator.seed = seed
            cfg.simulator.n_samples = n
            spec = adapter.build(cfg, variant)
            R = simulate(spec, spec.sim_samples, K_cut)
            if spec.reference is not None:
                diff = np.abs(spec.reference(spec.sim_samples, K_cut)[:, :K_cut] - R).max()
                print(f"[{name}/{variant}] seed {seed}: simulator vs NumPy max abs diff {diff:.1e}",
                      flush=True)
            return spec.sim_samples, R

        pilot_samples, P = fresh(args.pilot_seed, args.pilot_samples)
        samples, R = fresh(args.seed, args.n_samples)
        N = R.shape[0]
        (OUT / "kl_samples").mkdir(exist_ok=True)
        np.savez_compressed(
            OUT / "kl_samples" / f"{name}_{variant}.npz",
            samples=samples, residuals=R, seed=args.seed,
            pilot_samples=pilot_samples, pilot_residuals=P, pilot_seed=args.pilot_seed,
        )

        bounds = read(run / variant / "bounds.csv")
        mip = np.array([float(bounds[K]["bound"]) if K in bounds else np.nan for K in range(1, K_cut + 1)])

        # Per-K tolerances from the pilot only (independent of the test samples and of
        # the MIP bounds).
        q = args.quantile
        grids = [
            np.geomspace(max(args.grid_lo * np.quantile(P[:, i], q), 1e-300),
                         args.grid_hi * P[:, i].max(), args.n_tol)
            for i in range(K_cut)
        ]
        per_K = args.n_tol
        n_union = per_K * (K_cut if args.union == "K" else 1)
        c = np.log(2 * n_union / args.delta) / N
        c_fixed = np.log(2 * per_K / args.delta) / N  # per-K, for comparison
        floor = kl_inv(0.0, c)

        cols = {k: np.full(K_cut, np.nan) for k in
                ("bound", "sample_max", "pilot_q", "emp_q", "kl_q", "kl_q_perK",
                 "emp_risk_at_bound")}
        missing = []
        for i in range(K_cut):
            r, grid = R[:, i], grids[i]
            emp = [(r > e).mean() for e in grid]
            ok = grid[np.array([kl_inv(p, c) for p in emp]) <= 1 - q]
            ok_perK = grid[np.array([kl_inv(p, c_fixed) for p in emp]) <= 1 - q]
            if not ok.size:
                missing.append(i + 1)
            cols["bound"][i] = mip[i]
            cols["sample_max"][i] = r.max()
            cols["pilot_q"][i] = np.quantile(P[:, i], q)
            cols["emp_q"][i] = np.quantile(r, q)
            cols["kl_q"][i] = ok.min() if ok.size else np.nan
            cols["kl_q_perK"][i] = ok_perK.min() if ok_perK.size else np.nan
            if np.isfinite(mip[i]):
                cols["emp_risk_at_bound"][i] = (r > mip[i]).mean()
        tables[variant] = cols
        above = int(np.sum(cols["kl_q"] > mip))
        print(
            f"[{name}/{variant}] N={N} seed={args.seed} pilot N={P.shape[0]} seed={args.pilot_seed} "
            f"tolerances/K={per_K} union={n_union} c={c:.4f} risk floor={floor:.4f} "
            f"KL q above MIP at {above}/{K_cut} K",
            flush=True,
        )
        if missing:
            print(f"[{name}/{variant}] WARNING: no certified tolerance at K={missing}; "
                  "raise --grid-hi", flush=True)

    tag = f"q{round(100 * q)}"
    path = OUT / f"kl_{name}.csv"
    with open(path, "w", newline="") as f:
        fields = ["K"] + [f"{v}_{k.replace('_q', '_' + tag)}" for v in tables for k in tables[v]]
        writer = csv.writer(f)
        writer.writerow(fields)
        for i in range(K_cut):
            writer.writerow([i + 1] + [f"{tables[v][k][i]:.6e}" for v in tables for k in tables[v]])
    print(f"wrote {path.relative_to(REPO)}", flush=True)

    show = sorted({1, 5, 10, K_cut // 2, K_cut} & set(range(1, K_cut + 1)))
    print(f"\n{name}: {tag} = {q:.0%} quantile, delta = {args.delta}, union over {args.union}")
    print(f"{'variant':>18} {'K':>4} {'emp q':>10} {'KL q':>10} {'KL q/K':>10} "
          f"{'smp max':>10} {'MIP':>10} {'P^(>MIP)':>9}")
    for v, t in tables.items():
        for K in show:
            i = K - 1
            print(f"{v:>18} {K:>4} {t['emp_q'][i]:10.3e} {t['kl_q'][i]:10.3e} {t['kl_q_perK'][i]:10.3e} "
                  f"{t['sample_max'][i]:10.3e} {t['bound'][i]:10.3e} "
                  f"{t['emp_risk_at_bound'][i]:9.4f}")
    print(flush=True)


def write_tables(tag: str) -> None:
    """``plots/kl_tables.md``: every K of every ``kl_<experiment>.csv``, one table per variant."""
    lines = [
        f"# KL {tag} bounds vs MIP bounds, every K",
        "",
        "Columns: empirical quantile, KL-certified quantile, max over the test samples, "
        "MIP bound, and MIP / KL ratio (below 1: the KL bound overestimates, marked *).",
        "",
    ]
    for name, (_, K_cut, variants) in RUNS.items():
        path = OUT / f"kl_{name}.csv"
        if not path.exists():
            continue
        rows = list(csv.DictReader(open(path)))
        for v in variants:
            lines += [f"## {name} / {v}", "",
                      f"| K | emp {tag} | KL {tag} | sample max | MIP | MIP / KL |",
                      "|---:|---:|---:|---:|---:|---:|"]
            for r in rows[:K_cut]:
                g = lambda k: float(r[f"{v}_{k}"])  # noqa: E731
                kl_q, mip = g(f"kl_{tag}"), g("bound")
                lines.append(
                    f"| {r['K']} | {g(f'emp_{tag}'):.3e} | {kl_q:.3e} | {g('sample_max'):.3e} | "
                    f"{mip:.3e} | {mip / kl_q:.2f}{'*' if kl_q > mip else ''} |"
                )
            lines.append("")
    (OUT / "kl_tables.md").write_text("\n".join(lines))
    print(f"wrote {(OUT / 'kl_tables.md').relative_to(REPO)}", flush=True)


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    p.add_argument("--experiment", choices=list(RUNS), default=None)
    p.add_argument("--n-samples", type=int, default=1000)
    p.add_argument("--seed", type=int, default=1, help="fresh; must differ from the runs' seeds")
    p.add_argument("--delta", type=float, default=0.05)
    p.add_argument("--quantile", type=float, default=0.95)
    p.add_argument("--pilot-samples", type=int, default=1000)
    p.add_argument("--pilot-seed", type=int, default=2,
                   help="independent pilot that places each K's tolerance grid")
    p.add_argument("--n-tol", type=int, default=100, help="tolerances per K")
    p.add_argument("--grid-lo", type=float, default=0.9,
                   help="each K's grid starts at this times the pilot quantile")
    p.add_argument("--grid-hi", type=float, default=2.0,
                   help="each K's grid ends at this times the pilot max")
    p.add_argument("--union", choices=["K", "tol"], default="K")
    p.add_argument("--tables", action="store_true",
                   help="only write plots/kl_tables.md from the existing kl_<experiment>.csv")
    args = p.parse_args()
    if args.tables:
        write_tables(f"q{round(100 * args.quantile)}")
        return
    if args.experiment is not None:
        run_experiment(args.experiment, args)
        return
    # One process per experiment: their sibling modules share names (problem.py).
    rest = [a for a in sys.argv[1:]]
    for name in RUNS:
        done = subprocess.run([sys.executable, __file__, "--experiment", name, *rest], cwd=REPO)
        if done.returncode:
            raise SystemExit(f"{name} failed")


if __name__ == "__main__":
    main()
