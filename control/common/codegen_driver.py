"""Emit two C solvers for one recorded iteration: as translated, and optimized.

:func:`fomtools.codegen.generate.generate_code` runs both agent phases in one
directory, and the code optimization phase overwrites the translation it started from.
That is right for shipping one solver and wrong here, where the two are the
experiment: the emitted solver and the optimized solver are two implementations
of the same algorithm and both have to fly.

So the translation phase runs once, its verified directory is copied, and the
code optimization phase runs on the copy.  Both solvers then descend from the same
translation, which is what makes the comparison between them a statement about
the code optimization phase rather than about two independent agent runs.

The code optimization phase runs on the copy through
:func:`fomtools.codegen.generate.run_code_optimization`, the function
``generate_code`` itself calls, so the gating is fomtools' own: in rounds, the
editable sources (solver, linear solves, matrix-vector products) are re-verified
under the kernel rules and benchmarked head to head, and the translated sources
come back if no round is kept.

Agent runs are expensive and nondeterministic, so both phases are skipped when
the directory already holds a solver that verifies.  Pass ``force=True`` to
regenerate.
"""

from __future__ import annotations

import shutil
from dataclasses import dataclass, field
from pathlib import Path

from fomtools.codegen import CodegenConfig, TerminationSpec
from fomtools.codegen.emit import scaffold
from fomtools.codegen.extract_scheduled import extract_scheduled_ir
from fomtools.codegen.generate import extract_fom_ir, generate_code, run_code_optimization
from fomtools.codegen.harness import build_and_verify
from fomtools.codegen.ir import FomIR
from fomtools.codegen.targets import get_target

from common.recording import ScheduledRecording

#: Iteration count the solvers are generated at.  ``settings.max_iter`` is read
#: at run time by the emitted loop, so this only sets the default and the size of
#: the verify and benchmark runs; the flights choose their own.  Kept small
#: because the emitted benchmark runs five thousand solves and the optimization
#: phase runs it repeatedly: twenty iterations is long enough that the loop, not
#: the per-solve reset, is what the agent sees in its profile.
DEFAULT_MAX_ITERS = 20


def config_for(
    max_iters: int = DEFAULT_MAX_ITERS, tol_verify: float | None = None
) -> CodegenConfig:
    """The code-generation configuration both phases run under.

    ``tol_verify`` is the *absolute* agreement the generated solver must reach
    against the reference oracle, over samples drawn uniformly from the
    parameter boxes.  Leave it alone unless the iterates of the method are large
    at those samples: the tolerance does not scale with them, so a solver that
    is exact to a few parts in 10^11 can still miss an absolute bound if its
    state has grown by four orders of magnitude.  Raising it is only honest
    alongside a check at realistic parameters, which
    :func:`common.study.check_equivalence` performs.
    """
    return CodegenConfig(
        termination=TerminationSpec(max_iters=max_iters),
        agent_optimize_max_turns=120,
        **({} if tol_verify is None else {"tol_verify": tol_verify}),
    )


def default_driver(config: CodegenConfig):
    """The coding agent both phases use."""
    from fomtools.codegen.agent.claude_sdk import ClaudeAgentDriver

    return ClaudeAgentDriver(config=config)


def extract(recording, boxes, config: CodegenConfig | None = None, name: str = "fom") -> FomIR:
    """The IR alone, without running an agent.

    Used by anything that needs the emitted dimensions or the reference oracle
    without paying for a generation run.

    A stationary :class:`~common.recording.Recording` is extracted with its
    recorded iterates, which turns on the stationarity check -- the only thing
    that would catch a method whose coefficients vary with the iteration index.
    A :class:`~common.recording.ScheduledRecording` is a method that varies on
    purpose, and goes down the other path: extraction probes its build function
    and validates the fit against a held-out probe, so it has its own check and
    needs no iterates.
    """
    config = config or config_for()
    if isinstance(recording, ScheduledRecording):
        _check_tables_cover(recording, config)
        return extract_scheduled_ir(recording.build, recording.schedules, config=config, name=name)
    return extract_fom_ir(
        recording.verifier,
        recording.next_state,
        config=config,
        state=recording.state,
        params=recording.params,
        name=name,
        iterates=recording.iterates,
        param_values=recording.param_values(boxes),
    )


def _check_tables_cover(recording, config: CodegenConfig) -> None:
    """Refuse to generate a loop longer than the schedule tables describe.

    Past the end of a table the emitted accessor clamps to its last entry.  That
    is memory safe and algorithmically wrong: the coefficients would freeze, and
    for a restarted method they would freeze mid-period.  Better to fail here
    than to ship a solver that quietly runs a different method at large
    iteration counts.
    """
    wanted = config.termination.max_iters
    if recording.max_iters < wanted:
        raise ValueError(
            f"the schedule tables cover {recording.max_iters} iterations but the solver is "
            f"being generated for {wanted}. Past the end of a table the emitted accessor "
            f"clamps to its last value, which freezes the coefficients instead of "
            f"continuing the schedule. Build the tables for at least {wanted} iterations."
        )


def translate(
    recording,
    boxes,
    out_dir: Path,
    name: str,
    config: CodegenConfig | None = None,
    driver=None,
    force: bool = False,
) -> tuple[FomIR, Path]:
    """Run the translation phase into ``out_dir/emitted``. Returns the IR and path.

    ``recording`` is either a :class:`common.recording.Recording`, for a method
    whose coefficients are fixed, or a
    :class:`common.recording.ScheduledRecording`, for one whose coefficients vary
    with the iteration index.  ``boxes`` is the matching
    :class:`common.recording.ParameterBoxes`; the scheduled path carries its
    boxes inside the build function and ignores it here.
    """
    config = config or config_for()
    emitted = Path(out_dir) / "emitted"

    if not force:
        ir = extract(recording, boxes, config=config, name=name)
        if _verifies(emitted, config, ir):
            print(f"reusing the verified solver already in {emitted}")
            return ir, emitted

    if isinstance(recording, ScheduledRecording):
        _check_tables_cover(recording, config)
        result = generate_code(
            recording.build,
            out_dir=emitted,
            config=config,
            driver=driver or default_driver(config),
            name=name,
            code_optimization=False,
            schedules=recording.schedules,
        )
    else:
        result = generate_code(
            recording.verifier,
            recording.next_state,
            emitted,
            config=config,
            driver=driver or default_driver(config),
            state=recording.state,
            params=recording.params,
            name=name,
            code_optimization=False,
            iterates=recording.iterates,
            param_values=recording.param_values(boxes),
        )
    if not result.translate.success:
        raise RuntimeError(
            f"the translation phase did not produce a verified solver in {emitted}.\n"
            f"{result.translate.log[-4000:]}"
        )
    return result.ir, emitted


@dataclass
class OptimizeResult:
    """Outcome of running the code optimization phase on a copy of the emitted solver."""

    path: Path
    kept: bool
    notes: str = ""
    emitted_us: float | None = None  # per solve, at the generated iteration count
    optimized_us: float | None = None
    rounds: list[dict] = field(default_factory=list)  # one per agent round
    rule_friction: list[dict] = field(default_factory=list)  # what the rules blocked


def optimize(
    ir: FomIR,
    emitted: Path,
    out_dir: Path,
    config: CodegenConfig | None = None,
    driver=None,
    force: bool = False,
) -> OptimizeResult:
    """Copy the emitted solver and run the code optimization phase on the copy."""
    config = config or config_for()
    optimized = Path(out_dir) / "optimized"

    if not force and _verifies(optimized, config, ir):
        print(f"reusing the optimized solver already in {optimized}")
        return OptimizeResult(path=optimized, kept=True, notes="reused from a previous run")

    if optimized.exists():
        shutil.rmtree(optimized)
    shutil.copytree(emitted, optimized)
    target = scaffold(ir, optimized, config)

    result = run_code_optimization(
        ir, target, optimized, driver or default_driver(config), config, profile=True
    )
    (optimized / "agent_code_optimization.log").write_text(
        result.optimize.agent_log or result.optimize.log
    )
    if result.optimizations_md:
        print(f"wrote {result.optimizations_md}")
    stop = result.rounds[-1].stop_reason if result.rounds else None
    return OptimizeResult(
        path=optimized,
        kept=result.kept,
        notes="" if result.kept else (stop or "no round was kept"),
        emitted_us=result.translated_us,
        optimized_us=result.final_us,
        rounds=[r.as_dict() for r in result.rounds],
        rule_friction=result.rule_friction,
    )


def _verifies(directory: Path, config: CodegenConfig, ir: FomIR | None = None) -> bool:
    """Whether ``directory`` already holds a solver that passes ``make verify``.

    With ``ir``, the files ``scaffold`` generates (matrix data and the verify
    harness, which are not committed) are first rewritten from it, keeping the
    sources the agent may edit, so a committed solver is reused, not regenerated.
    Reuse needs the fomtools commit the solver was generated with: a later one may
    declare kernels the committed sources do not define.
    """
    if not (directory / "src" / "fom_solver.c").exists():
        return False
    target = get_target(config)
    if ir is not None:
        editable = [directory / rel for rel in target.optimization_editable_paths()]
        kept = {path: path.read_text() for path in editable if path.exists()}
        scaffold(ir, directory, config)
        for path, text in kept.items():
            path.write_text(text)
    return build_and_verify(directory, target).success
