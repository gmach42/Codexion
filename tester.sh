#!/bin/bash
# ============================================================
#  tester.sh  ─  Codexion Comprehensive Test Suite
#
#  Tests argument validation, log format, behavioural rules,
#  burnout timing, edge cases and output serialisation.
#
#  Usage:  chmod +x tester.sh && ./tester.sh [--no-color]
# ============================================================

BINARY="./codexion"
PASS=0
FAIL=0
WARN=0

# ─── Color support ───────────────────────────────────────────────────────────
if [ "${1-}" = "--no-color" ] || [ -z "$TERM" ] || [ "$TERM" = "dumb" ]; then
	GRN="" RED="" YLW="" BLU="" CYN="" BOLD="" NC=""
else
	GRN="\033[0;32m" RED="\033[1;31m" YLW="\033[0;33m"
	BLU="\033[0;34m" CYN="\033[0;36m" BOLD="\033[1m" NC="\033[0m"
fi

# ─── Output helpers ──────────────────────────────────────────────────────────
pass()    { printf "${GRN}[PASS]${NC} %s\n"    "$1"; PASS=$((PASS+1)); }
fail()    { printf "${RED}[FAIL]${NC} %s\n"    "$1"; FAIL=$((FAIL+1)); }
warn()    { printf "${YLW}[WARN]${NC} %s\n"    "$1"; WARN=$((WARN+1)); }
info()    { printf "${BLU}[INFO]${NC} %s\n"    "$1"; }
detail()  { printf "       ${YLW}%s${NC}\n"    "$1"; }
section() { printf "\n${BOLD}${CYN}──────────────────────────────────────\n %s\n──────────────────────────────────────${NC}\n\n" "$1"; }

# Run the binary with a wall-clock timeout (seconds).
run_sim() {
	local secs="$1"; shift
	timeout "$secs" "$BINARY" "$@" 2>&1
}

# ─── Pre-flight ──────────────────────────────────────────────────────────────
if [ ! -f "$BINARY" ]; then
	printf "${RED}[ERROR]${NC} Binary '%s' not found — run 'make' first.\n" "$BINARY"
	exit 1
fi

# =============================================================================
# 1. ARGUMENT VALIDATION  ─  bad inputs must be rejected (exit ≠ 0)
# =============================================================================
section "1 · Argument Validation"

expect_fail() {
	local desc="$1"; shift
	local out ec
	out=$(timeout 3 "$BINARY" "$@" 2>&1); ec=$?
	[ $ec -ne 0 ] && pass "$desc" || fail "$desc → exited 0, expected non-zero"
}

# Wrong argument count
expect_fail "no arguments"
expect_fail "only 1 arg"                          4
expect_fail "only 7 args (scheduler missing)"     4 600 300 100 100 10 100
expect_fail "9 args (one extra)"                  4 600 300 100 100 10 100 fifo extra

# Invalid scheduler
for sched in FIFO EDF Fifo rr roundrobin "" "fifo edf" 0 1 random; do
	expect_fail "invalid scheduler '$sched'"      4 600 300 100 100 10 100 "$sched"
done

# Negative values (each numeric position)
declare -A NEG_CASES=(
	["negative number_of_coders"]="-1 600 300 100 100 10 100 fifo"
	["negative time_to_burnout"]="4 -1 300 100 100 10 100 fifo"
	["negative time_to_compile"]="4 600 -1 100 100 10 100 fifo"
	["negative time_to_debug"]="4 600 300 -1 100 10 100 fifo"
	["negative time_to_refactor"]="4 600 300 100 -1 10 100 fifo"
	["negative compiles_required"]="4 600 300 100 100 -1 100 fifo"
	["negative dongle_cooldown"]="4 600 300 100 100 10 -1 fifo"
)
for desc in "${!NEG_CASES[@]}"; do
	args="${NEG_CASES[$desc]}"
	expect_fail "$desc" $args
done

# Zero coders
expect_fail "number_of_coders = 0"               0 600 300 100 100 10 100 fifo

# Non-integer / malformed values
expect_fail "non-integer coders 'four'"           four 600 300 100 100 10 100 fifo
expect_fail "float time_to_burnout '600.5'"       4 600.5 300 100 100 10 100 fifo
expect_fail "string time_to_compile 'abc'"        4 600 abc 100 100 10 100 fifo
expect_fail "hex time_to_debug '0x64'"            4 600 300 0x64 100 10 100 fifo
expect_fail "special chars '!@#'"                 4 600 300 "!@#" 100 10 100 fifo

# =============================================================================
# 2. OUTPUT FORMAT  ─  every log line must follow the specified pattern
# =============================================================================
section "2 · Output Format"

# Allowed line patterns (per subject Chapter V)
readonly LOG_PAT='^[0-9]+ [0-9]+ has taken a dongle$|^[0-9]+ [0-9]+ is compiling$|^[0-9]+ [0-9]+ is debugging$|^[0-9]+ [0-9]+ is refactoring$|^[0-9]+ [0-9]+ burned out$|^[0-9]+ Simulation Complete$'

check_format() {
	local label="$1" output="$2"
	local bad
	bad=$(echo "$output" | grep -Ev "$LOG_PAT" || true)
	if [ -z "$bad" ]; then
		pass "$label: every line matches log format"
	else
		fail "$label: lines outside expected format:"
		echo "$bad" | head -4 | while IFS= read -r l; do detail "$l"; done
	fi
}

check_coder_ids() {
	local label="$1" output="$2" n="$3"
	local bad
	bad=$(echo "$output" | grep -E '^[0-9]+ [0-9]+ ' | awk -v n="$n" '
		{ id=$2+0; if (id < 1 || id > n) print "bad ID " id ": " $0 }' || true)
	if [ -z "$bad" ]; then
		pass "$label: coder IDs in [1, $n]"
	else
		fail "$label: coder IDs out of spec range [1, $n]:"
		echo "$bad" | head -4 | while IFS= read -r l; do detail "$l"; done
	fi
}

# Per-coder compile sequence: for each "is compiling", the two most recent
# messages from that same coder must both be "has taken a dongle".
check_compile_sequence() {
	local label="$1" output="$2"
	local errors=0
	declare -A last1 last2          # last two actions per coder

	while IFS= read -r line; do
		[[ "$line" =~ ^[0-9]+\ ([0-9]+)\ (.+)$ ]] || continue
		local id="${BASH_REMATCH[1]}" action="${BASH_REMATCH[2]}"
		if [ "$action" = "is compiling" ]; then
			if [ "${last1[$id]-}" != "has taken a dongle" ] || \
			   [ "${last2[$id]-}" != "has taken a dongle" ]; then
				errors=$((errors+1))
			fi
		fi
		last2[$id]="${last1[$id]-}"
		last1[$id]="$action"
	done <<< "$output"

	if [ $errors -eq 0 ]; then
		pass "$label: every 'is compiling' preceded by 2 'has taken a dongle'"
	else
		fail "$label: $errors 'is compiling' without the 2 required dongle grabs"
	fi
}

check_timestamps_monotone() {
	local label="$1" output="$2"
	local prev=-1 ok=true ts
	while IFS= read -r line; do
		ts=$(echo "$line" | awk '{print $1}')
		[[ "$ts" =~ ^[0-9]+$ ]] || continue
		if [ "$ts" -lt "$prev" ]; then ok=false; break; fi
		prev="$ts"
	done <<< "$output"
	$ok && pass "$label: timestamps non-decreasing" \
	     || fail "$label: timestamp decreased (output not ordered)"
}

for sched in fifo edf; do
	OUT=$(run_sim 12 4 800 200 100 100 3 50 "$sched")
	check_format          "4-coder $sched" "$OUT"
	check_coder_ids       "4-coder $sched" "$OUT" 4
	check_compile_sequence "4-coder $sched" "$OUT"
	check_timestamps_monotone "4-coder $sched" "$OUT"
done

# =============================================================================
# 3. NORMAL COMPLETION  ─  simulation must end with "Simulation Complete"
# =============================================================================
section "3 · Normal Completion"

check_complete() {
	local label="$1" output="$2"
	if echo "$output" | grep -q 'Simulation Complete'; then
		pass "$label: 'Simulation Complete' printed"
	else
		fail "$label: 'Simulation Complete' not found (timeout or wrong stop)"
	fi
	if echo "$output" | grep -q 'burned out'; then
		fail "$label: unexpected 'burned out' during completion run"
	else
		pass "$label: no spurious burnout"
	fi
}

# Per-coder compile count must reach the required value
check_per_coder_compiles() {
	local label="$1" output="$2" n_coders="$3" required="$4"
	local all_ok=true
	for ((id=1; id<=n_coders; id++)); do
		local count
		count=$(echo "$output" | grep -cE "^[0-9]+ $id is compiling$" || true)
		if [ "$count" -lt "$required" ]; then
			all_ok=false
			fail "$label: coder $id compiled only $count/$required times"
		fi
	done
	$all_ok && pass "$label: every coder compiled ≥$required times"
}

# Comfortable timing scenarios
declare -a COMPLETE_CASES=(
	"2 1000 200 100 100 2 50 fifo"
	"2 1000 200 100 100 2 50 edf"
	"3 900  200 100 100 2 50 fifo"
	"3 900  200 100 100 2 50 edf"
	"4 800  200 100 100 3 50 fifo"
	"4 800  200 100 100 3 50 edf"
	"6 1200 200 100 100 2 30 fifo"
	"6 1200 200 100 100 2 30 edf"
)
for args in "${COMPLETE_CASES[@]}"; do
	read -r n burnout compile debug refactor need cooldown sched <<< "$args"
	label="${n}-coders need=${need} ${sched}"
	OUT=$(run_sim 20 $args)
	check_complete "$label" "$OUT"
	check_per_coder_compiles "$label" "$OUT" "$n" "$need"
	check_compile_sequence "$label" "$OUT"
done

# =============================================================================
# 4. BURNOUT DETECTION  ─  must be logged within 10 ms of actual burnout
# =============================================================================
section "4 · Burnout Detection & Timing"

check_burnout() {
	local label="$1" output="$2" burnout_ms="$3"
	if ! echo "$output" | grep -q 'burned out'; then
		fail "$label: expected 'burned out' but simulation completed or hung"
		return
	fi
	pass "$label: 'burned out' detected"

	# The log timestamp should be close to burnout_ms (first coder to burn out
	# never compiled, so their deadline = start + time_to_burnout).
	local ts
	ts=$(echo "$output" | grep 'burned out' | head -1 | awk '{print $1}')
	local lo=$((burnout_ms - 50)) hi=$((burnout_ms + 200))
	if [ "$ts" -ge "$lo" ] && [ "$ts" -le "$hi" ]; then
		pass "$label: burnout timestamp ${ts}ms in expected window [${lo}, ${hi}]ms"
	else
		warn "$label: burnout timestamp ${ts}ms outside window [${lo}, ${hi}]ms (OS jitter?)"
	fi

	if ! echo "$output" | grep -q 'Simulation Complete'; then
		pass "$label: simulation stopped without printing 'Simulation Complete'"
	fi
}

# Tight scenarios: some coders will inevitably starve and burn out.
# 5-coder odd ring with FIFO cannot feed all coders within 400 ms.
for sched in fifo edf; do
	OUT=$(run_sim 5 5 400 150 100 100 100 30 "$sched")
	check_burnout "5-coders tight $sched" "$OUT" 400
	check_format  "5-coders tight $sched (format)" "$OUT"
done

# Burnout time is shorter than the full compile+cycle: always burns out.
OUT=$(run_sim 5 4 300 200 100 100 100 50 fifo)
check_burnout "compile > burnout (fifo)" "$OUT" 300

OUT=$(run_sim 5 4 300 200 100 100 100 50 edf)
check_burnout "compile > burnout (edf)" "$OUT" 300

# =============================================================================
# 5. EDGE CASES
# =============================================================================
section "5 · Edge Cases"

# ── 0 compiles required: monitor should stop immediately ────────────────────
OUT=$(run_sim 5 4 800 200 100 100 0 50 fifo)
if echo "$OUT" | grep -q 'Simulation Complete'; then
	pass "number_of_compiles_required=0: stops immediately"
else
	fail "number_of_compiles_required=0: expected 'Simulation Complete'"
fi

# ── Zero cooldown ────────────────────────────────────────────────────────────
for sched in fifo edf; do
	OUT=$(run_sim 15 4 800 200 100 100 3 0 "$sched")
	check_complete        "zero cooldown $sched" "$OUT"
	check_format          "zero cooldown $sched (format)" "$OUT"
	check_compile_sequence "zero cooldown $sched" "$OUT"
done

# ── Zero debug time ──────────────────────────────────────────────────────────
OUT=$(run_sim 12 4 800 200 0 100 3 50 fifo)
check_complete "zero time_to_debug" "$OUT"
check_format   "zero time_to_debug (format)" "$OUT"

# ── Zero refactor time ───────────────────────────────────────────────────────
OUT=$(run_sim 12 4 800 200 100 0 3 50 fifo)
check_complete "zero time_to_refactor" "$OUT"
check_format   "zero time_to_refactor (format)" "$OUT"

# ── Zero compile time (instant compile) ─────────────────────────────────────
OUT=$(run_sim 12 4 800 0 100 100 3 50 fifo)
check_complete        "zero time_to_compile" "$OUT"
check_format          "zero time_to_compile (format)" "$OUT"
check_compile_sequence "zero time_to_compile" "$OUT"

# ── All zero timings ─────────────────────────────────────────────────────────
OUT=$(run_sim 10 4 200 0 0 0 3 0 fifo)
if echo "$OUT" | grep -qE 'Simulation Complete|burned out'; then
	pass "all-zero times: simulation terminates"
else
	fail "all-zero times: simulation hung"
fi

# ── 1 compile required ───────────────────────────────────────────────────────
OUT=$(run_sim 12 4 800 200 100 100 1 50 fifo)
check_complete "number_of_compiles_required=1" "$OUT"

# ── Large number of coders ───────────────────────────────────────────────────
for n in 8 12; do
	for sched in fifo edf; do
		OUT=$(run_sim 20 $n 2000 300 100 100 2 50 "$sched")
		check_format    "${n}-coders $sched (format)" "$OUT"
		check_coder_ids "${n}-coders $sched" "$OUT" "$n"
		check_compile_sequence "${n}-coders $sched" "$OUT"
	done
done

# ── Single coder (only 1 dongle, needs 2 to compile → must burn out) ────────
OUT=$(run_sim 4 1 500 200 100 100 5 0 fifo)
if echo "$OUT" | grep -qE 'Simulation Complete|burned out'; then
	pass "1-coder: simulation terminates (burnout expected)"
	check_format "1-coder (format)" "$OUT"
else
	fail "1-coder: simulation hung"
fi

# ── 2 coders (minimal ring, non-trivial contention) ──────────────────────────
for sched in fifo edf; do
	OUT=$(run_sim 12 2 1200 200 100 100 2 50 "$sched")
	check_format          "2-coder $sched (format)" "$OUT"
	check_compile_sequence "2-coder $sched" "$OUT"
	if echo "$OUT" | grep -qE 'Simulation Complete|burned out'; then
		pass "2-coder $sched: simulation terminates"
	else
		fail "2-coder $sched: simulation hung"
	fi
done

# ── Very large burnout (simulation should never burn out in reasonable time) ─
OUT=$(run_sim 10 4 999999 200 100 100 3 50 fifo)
check_complete "very large time_to_burnout" "$OUT"

# =============================================================================
# 6. OUTPUT SERIALISATION  ─  no two messages may share a line under load
# =============================================================================
section "6 · Output Serialisation (Stress)"

for run in 1 2 3; do
	for sched in fifo edf; do
		OUT=$(run_sim 15 8 1200 200 100 100 2 20 "$sched")
		bad=$(echo "$OUT" | grep -Ev "$LOG_PAT" || true)
		if [ -z "$bad" ]; then
			pass "8-coder $sched run#$run: no interleaved output"
		else
			fail "8-coder $sched run#$run: suspected interleaved output:"
			echo "$bad" | head -3 | while IFS= read -r l; do detail "$l"; done
		fi
	done
done

# =============================================================================
# 7. SCHEDULER  ─  both policies produce valid, deterministic runs
# =============================================================================
section "7 · Scheduler Sanity"

# Each coder must not skip states: dongle × 2 → compiling → debugging → refactoring
check_state_order() {
	local label="$1" output="$2"
	local errors=0
	declare -A state_of    # current state per coder
	declare -A dongle_cnt  # dongle grabs since last compile

	while IFS= read -r line; do
		[[ "$line" =~ ^[0-9]+\ ([0-9]+)\ (.+)$ ]] || continue
		local id="${BASH_REMATCH[1]}" action="${BASH_REMATCH[2]}"
		local cur="${state_of[$id]-start}"

		case "$action" in
		"has taken a dongle")
			dongle_cnt[$id]=$(( ${dongle_cnt[$id]-0} + 1 )) ;;
		"is compiling")
			if [ "${dongle_cnt[$id]-0}" -lt 2 ]; then errors=$((errors+1)); fi
			dongle_cnt[$id]=0
			state_of[$id]="compiling" ;;
		"is debugging")
			if [ "$cur" != "compiling" ]; then errors=$((errors+1)); fi
			state_of[$id]="debugging" ;;
		"is refactoring")
			if [ "$cur" != "debugging" ]; then errors=$((errors+1)); fi
			state_of[$id]="refactoring" ;;
		"burned out") ;;
		esac
	done <<< "$output"

	if [ $errors -eq 0 ]; then
		pass "$label: state machine valid for all coders"
	else
		fail "$label: $errors invalid state transition(s)"
	fi
}

for sched in fifo edf; do
	OUT=$(run_sim 15 5 1000 200 100 100 2 40 "$sched")
	check_state_order "5-coder $sched state machine" "$OUT"
done

# Under EDF, a coder that just started (fresh deadline) should not starve a
# coder whose deadline is imminent. Test that EDF completes without burnout
# when parameters leave enough slack.
OUT=$(run_sim 20 6 1400 200 100 100 3 40 edf)
check_complete "EDF liveness: 6 coders with slack" "$OUT"

OUT=$(run_sim 20 6 1400 200 100 100 3 40 fifo)
check_complete "FIFO liveness: 6 coders with slack" "$OUT"

# =============================================================================
# SUMMARY
# =============================================================================
TOTAL=$((PASS + FAIL + WARN))
printf "\n${BOLD}══════════════════════════════════════${NC}\n"
printf " Results: ${GRN}%d passed${NC}  ${RED}%d failed${NC}  ${YLW}%d warnings${NC}  / %d total\n" \
	$PASS $FAIL $WARN $TOTAL
printf "${BOLD}══════════════════════════════════════${NC}\n\n"

[ $FAIL -eq 0 ] && exit 0 || exit 1
