#!/bin/bash
# Keep one 1 h A100 holder (holder.sbatch; the gputest partition starts it at once,
# unlike longer jobs) running for the agents' builds. The next one is submitted only
# when none is pending and the current one is within HOLDER_MIN_LEFT + 10 minutes of
# its end. The id of a running holder with at least HOLDER_MIN_LEFT minutes left goes
# to $HOLDER_FILE (written atomically), which della/make reads on every call. Run it
# in its own tmux window on the login node while the agents work; stop it with Ctrl-C
# (the holders then end on their own within the hour). Polls squeue once a minute.
set -uo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/env.sh"
cd "$PROJECT_DIR"
min_left="${HOLDER_MIN_LEFT:-20}"
mkdir -p "$(dirname "$HOLDER_FILE")" slurm/logs

minutes_left() {  # squeue %L: [days-]hours:minutes:seconds or minutes:seconds
  local left="$1" d=0 h=0 m=0
  [[ $left == *-* ]] && { d="${left%%-*}"; left="${left#*-}"; }
  IFS=: read -r -a parts <<< "$left"
  case ${#parts[@]} in
    3) h=$((10#${parts[0]})); m=$((10#${parts[1]})) ;;
    2) m=$((10#${parts[0]})) ;;
  esac
  echo $((d * 1440 + h * 60 + m))
}

while true; do
  best="" pending=0 most=0
  while read -r id state left; do
    if [[ $state == RUNNING ]]; then
      mins=$(minutes_left "$left")
      (( mins > most )) && most=$mins
      [[ -z $best && $mins -ge $min_left ]] && best="$id"
    elif [[ $state == PENDING ]]; then
      pending=$((pending + 1))
    fi
  done < <(squeue -h -u "$USER" -n fom-holder -o "%i %T %L" --sort=-L)
  if [[ -n $best ]]; then
    [[ "$(cat "$HOLDER_FILE" 2> /dev/null)" != "$best" ]] && echo "$(date -Is) holder $best"
    echo "$best" > "$HOLDER_FILE.tmp" && mv "$HOLDER_FILE.tmp" "$HOLDER_FILE"
  else
    : > "$HOLDER_FILE.tmp" && mv "$HOLDER_FILE.tmp" "$HOLDER_FILE"  # make waits, never builds here
  fi
  # The next holder: only when none is pending and every running one is close to
  # its end (a gputest job starts at once, so this keeps one running, two briefly).
  if (( pending == 0 && most < min_left + 10 )); then
    sbatch --parsable lasso/lasso_codegen/della/holder.sbatch > /dev/null \
      && echo "$(date -Is) queued a holder"
  fi
  sleep 60
done
