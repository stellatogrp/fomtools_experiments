"""Put the shared ``common`` package on the path.

Run as a module from ``control/rapdhg`` (``python -m verify.verify``), which puts
that directory on ``sys.path``, so the sibling ``codegen`` package imports
directly; ``common`` lives one level further up. See ``codegen/__init__.py``.
"""

from __future__ import annotations

import sys
from pathlib import Path

_shared = str(Path(__file__).resolve().parents[2])
if _shared not in sys.path:
    sys.path.append(_shared)
