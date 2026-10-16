"""Transcript gate: did the agent touch anything outside its own run?

Usage: python transcript_check.py TARGET_DIR

Reads every Claude Code session recorded for TARGET_DIR
(~/.claude/projects/<encoded TARGET_DIR>/*.jsonl) and fails (exit 1) if any
tool call mentions
  * another run under fomtools_codegen_runs/, or another target of the own run
    (the own target directory, the own run root and its holder logs are
    allowed), or the older studies under ~/fomtools_codegen_runs;
  * another agent's transcripts under ~/.claude/projects (the own project
    directory, which holds the agent's memory, is allowed);
  * the quarantine, archives, /tmp/fomsave or the published experiment repos.
Also fails if no session is found, so a missing transcript never passes.
Prints every offending call.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

FORBIDDEN = re.compile(
    r"fomtools_codegen_runs|\.claude/projects|quarantin|contaminated|fomsave|fomtools_experiments"
    r"|/archived|benchmark_7solvers|benchmark_prev",
    re.IGNORECASE,
)


def encode(path: Path) -> str:
    return re.sub(r"[^A-Za-z0-9-]", "-", str(path))


def main() -> int:
    target = Path(sys.argv[1]).resolve()
    root = target.parent.parent
    project = Path.home() / ".claude" / "projects" / encode(target)
    sessions = sorted(project.glob("*.jsonl"))
    if not sessions:
        print(f"TRANSCRIPT_FAIL no sessions found in {project}")
        return 1

    calls = 0
    hits = []
    for session in sessions:
        for line in session.read_text(errors="replace").splitlines():
            try:
                entry = json.loads(line)
            except json.JSONDecodeError:
                continue
            content = (entry.get("message") or {}).get("content")
            if not isinstance(content, list):
                continue
            for block in content:
                if not isinstance(block, dict) or block.get("type") != "tool_use":
                    continue
                calls += 1
                args = dict(block.get("input", {}))
                # A note the agent writes into its own memory directory is its own text.
                if str(args.get("file_path", "")).startswith(str(project)):
                    args.pop("content", None)
                    args.pop("new_string", None)
                    args.pop("old_string", None)
                text = json.dumps(args)
                text = text.replace(str(target), "<OWN>")
                # The run root itself and its holder logs are allowed; sibling targets are not.
                text = re.sub(re.escape(str(root)) + r"(?=/holders|/?[\"'\s;)]|/?$)", "<ROOT>", text)
                text = text.replace(".claude/projects/" + encode(target), "<OWN_PROJECT>")
                if FORBIDDEN.search(text):
                    hits.append(f"{session.name[:8]} {entry.get('timestamp', '')}: {text[:300]}")

    print(f"scanned {calls} tool calls in {len(sessions)} sessions ({project})")
    for hit in hits:
        print(f"   hit {hit}")
    print("TRANSCRIPT_OK" if not hits else "TRANSCRIPT_FAIL")
    return 0 if not hits else 1


if __name__ == "__main__":
    raise SystemExit(main())
