#!/bin/bash
# check_clean_workdir: before a translation agent starts, the target directory
# (the current directory) must hold only FOMTools' scaffold and this pipeline's
# build files, so the agent cannot find a solver from any earlier attempt (on
# 2026-10-15 an agent restored one from a leftover folder). Prints the offending
# entries and returns 1 if anything else is present.
check_clean_workdir() {
  local entry bad=()
  while IFS= read -r entry; do
    case "$entry" in
      include|python|src|test) ;;
      Makefile|Makefile.generated|spec.json|SPEC.md|MATRIX_STATS.md|README.md|data.npz) ;;
      immutable.sha256|immutable_check.log|CUDA_FIXED_OK|build_fixed.log|stub_link.log) ;;
      fom_solver.o|prof_solver.o) bad+=("$entry") ;;
      *.o|verify_bin|bench_bin|profile_bin) ;;
      *) bad+=("$entry") ;;
    esac
  done < <(ls -A)
  while IFS= read -r entry; do
    case "$entry" in
      fom_ops.c|fom_ops.cu|fom_data.c|fom_linsolve.c|fom_linsolve.cu|fom_profile.c) ;;
      *) bad+=("src/$entry") ;;
    esac
  done < <(ls -A src)
  # include/ and test/ may hold only the checksummed scaffold files.
  while IFS= read -r entry; do
    [[ -n "$entry" ]] && bad+=("$entry")
  done < <(LC_ALL=C comm -23 <(find include test -mindepth 1 | LC_ALL=C sort) \
                    <({ awk '{print $2}' immutable.sha256; echo include; echo test; } | LC_ALL=C sort))
  if (( ${#bad[@]} )); then
    echo "not a clean scaffold: ${bad[*]}"
    return 1
  fi
  echo "clean scaffold"
}
