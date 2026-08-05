"""PlatformIO native env: expose clang++ under the name g++.

Windows dev machines often ship LLVM/clang but not MinGW/g++, and PlatformIO's
native platform hardcodes `g++` regardless of $CXX. We prepend two directories
to PATH so g++ resolves to clang without changing PlatformIO internals:

1. scripts/shim/ — batch files g++.bat / gcc.bat / ar.bat that forward to
   clang++ / clang / llvm-ar respectively.
2. LLVM's bin — so the shims can find `clang++` and friends via PATH.

Override the LLVM install location with the LLVM_BIN env var if it's not at
the default `C:\\Program Files\\LLVM\\bin`.
"""

import os
from pathlib import Path

Import("env")  # type: ignore  # provided by SCons

_DEFAULT_LLVM_BIN = r"C:\Program Files\LLVM\bin"
_llvm_bin = os.environ.get("LLVM_BIN", _DEFAULT_LLVM_BIN)

_SHIM_DIR = str((Path(env["PROJECT_DIR"]) / "scripts" / "shim").resolve())


def _prepend(path_entry: str) -> None:
    current = os.environ.get("PATH", "")
    if path_entry and path_entry not in current:
        os.environ["PATH"] = path_entry + os.pathsep + current


_prepend(_SHIM_DIR)
if os.path.isdir(_llvm_bin):
    _prepend(_llvm_bin)
