"""Run one FOMTools agent phase on an existing scaffold, exactly as generate_code does.

Usage: python fomtools_agent.py TARGET_DIR {openmp,cuda} {A,B} TOL_VERIFY

Builds the task the way ``fomtools.codegen.generate.generate_code`` builds it:
FOMTools' own ``phase_a_instructions`` / ``phase_b_instructions`` text, verbatim;
the target's verify command; ``make bench`` and 60 turns for the optimization
phase; and FOMTools' default agent driver (``make_agent_driver()``) with its
verify-and-reprompt loop. The scaffold is loaded with ``fomtools.codegen.spec``.

The only difference: ``generate_code`` restores the deterministic files by
re-scaffolding before each verify. Re-emitting multi-GB sources is impractical
here, so the restore hook instead checks them against ``immutable.sha256`` and
stops the phase if any changed.
"""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

from fomtools.codegen.agent import make_agent_driver
from fomtools.codegen.agent.driver import AgentTask
from fomtools.codegen.agent.prompts import phase_a_instructions, phase_b_instructions
from fomtools.codegen.config import Backend, CodegenConfig, TerminationSpec, Version
from fomtools.codegen.spec import load
from fomtools.codegen.targets import get_target


def main() -> int:
    workdir = Path(sys.argv[1]).resolve()
    backend = Backend.OPENMP if sys.argv[2] == "openmp" else Backend.CUDA
    phase = sys.argv[3]
    tol = float(sys.argv[4])

    ir = load(workdir)
    config = CodegenConfig(
        version=Version.LARGE_SCALE,
        backend=backend,
        termination=TerminationSpec(max_iters=ir.termination.max_iters),
        tol_verify=tol,
    )
    target = get_target(config)

    def check_immutable() -> None:
        subprocess.run(["sha256sum", "--quiet", "-c", "immutable.sha256"], cwd=workdir, check=True)

    if phase == "A":
        task = AgentTask(
            workdir=workdir,
            solver_rel_path=target.solver_source_path(),
            verify_cmd=target.verify_command(),
            instructions=phase_a_instructions(ir, target),
            phase="A_translate",
            restore_hook=check_immutable,
        )
    else:
        task = AgentTask(
            workdir=workdir,
            solver_rel_path=target.solver_source_path(),
            verify_cmd=target.verify_command(),
            instructions=phase_b_instructions(ir, target),
            phase="B_optimize",
            max_turns=60,
            bench_cmd=["make", "bench"],
            restore_hook=check_immutable,
        )
    (workdir / f"prompt_phase_{phase}.txt").write_text(task.instructions)

    result = make_agent_driver().run(task)
    (workdir / f"agent_phase_{phase}_fomtools.log").write_text(result.log)
    print(f"FOMTOOLS_AGENT phase={phase} success={result.success} turns={result.turns_used}")
    return 0 if result.success else 1


if __name__ == "__main__":
    sys.exit(main())
