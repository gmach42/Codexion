#!/bin/bash
# tester2.sh — Codexion simple test suite (~20 tests)
# Usage: chmod +x tester2.sh && ./tester2.sh

BIN="./codexion"
PASS=0; FAIL=0

[ ! -f "$BIN" ] && echo "Binary not found — run 'make' first." && exit 1

ok()   { echo "[PASS] $1"; PASS=$((PASS+1)); }
ko()   { echo "[FAIL] $1"; FAIL=$((FAIL+1)); }
run()  { local t=$1; shift; timeout "$t" "$BIN" "$@" 2>&1; }
check() {
    local label="$1" out="$2" want="$3" no="$4"
    [ -n "$want" ] && ! echo "$out" | grep -q "$want" && ko "$label (missing: $want)" && return
    [ -n "$no"   ] &&   echo "$out" | grep -q "$no"   && ko "$label (unexpected: $no)" && return
    ok "$label"
}

echo "=== Regular Tests ==="

# 1. 2 coders, fifo — must complete
OUT=$(run 10 2 1000 200 100 100 2 50 fifo)
check "2 coders fifo completes"        "$OUT" "Simulation Complete" "burned out"

# 2. 2 coders, edf — must complete
OUT=$(run 10 2 1000 200 100 100 2 50 edf)
check "2 coders edf completes"         "$OUT" "Simulation Complete" "burned out"

# 3. 4 coders, fifo — must complete
OUT=$(run 12 4 800 200 100 100 3 50 fifo)
check "4 coders fifo completes"        "$OUT" "Simulation Complete" "burned out"

# 4. 4 coders, edf — must complete
OUT=$(run 12 4 800 200 100 100 3 50 edf)
check "4 coders edf completes"         "$OUT" "Simulation Complete" "burned out"

# 5. Output contains all expected states
OUT=$(run 12 3 800 200 100 100 2 50 fifo)
for state in "has taken a dongle" "is compiling" "is debugging" "is refactoring"; do
    check "state '$state' appears" "$OUT" "$state" ""
done

# 6. Timestamps are printed (first field is a number)
OUT=$(run 10 3 800 200 100 100 2 50 fifo)
if echo "$OUT" | grep -qE '^[0-9]+ '; then
    ok "timestamps printed as numbers"
else
    ko "timestamps printed as numbers"
fi

# 7. Two dongle grabs before every compile
OUT=$(run 10 4 800 200 100 100 2 50 fifo)
ERRORS=0
declare -A D
while IFS= read -r line; do
    [[ "$line" =~ ^[0-9]+\ ([0-9]+)\ (.+)$ ]] || continue
    id="${BASH_REMATCH[1]}"; act="${BASH_REMATCH[2]}"
    [ "$act" = "has taken a dongle" ] && D[$id]=$((${D[$id]-0}+1)) && continue
    if [ "$act" = "is compiling" ] && [ "${D[$id]-0}" -lt 2 ]; then ERRORS=$((ERRORS+1)); fi
    [ "$act" = "is compiling" ] && D[$id]=0
done <<< "$OUT"
[ $ERRORS -eq 0 ] && ok "each compile preceded by 2 dongle grabs" \
                  || ko "each compile preceded by 2 dongle grabs ($ERRORS violations)"
unset D

# 8. No interleaved/garbled output lines under load
OUT=$(run 12 6 1200 200 100 100 2 30 fifo)
BAD=$(echo "$OUT" | grep -Ev '^[0-9]+ [0-9]+ .+$|^[0-9]+ Simulation Complete$' || true)
[ -z "$BAD" ] && ok "no garbled output lines (fifo)" || ko "no garbled output lines (fifo)"

OUT=$(run 12 6 1200 200 100 100 2 30 edf)
BAD=$(echo "$OUT" | grep -Ev '^[0-9]+ [0-9]+ .+$|^[0-9]+ Simulation Complete$' || true)
[ -z "$BAD" ] && ok "no garbled output lines (edf)" || ko "no garbled output lines (edf)"

# 9. edf also completes without burnout
OUT=$(run 15 5 1000 200 100 100 2 40 edf)
check "5 coders edf liveness"          "$OUT" "Simulation Complete" "burned out"

# 10. Burnout is logged when it should happen
OUT=$(run 5 4 300 200 100 100 99 0 fifo)
check "burnout logged when compile > burnout" "$OUT" "burned out" "Simulation Complete"

echo ""
echo "=== Edge Cases ==="

# 11. Wrong argument count → non-zero exit
timeout 3 "$BIN" 4 600 300 100 100 10 100 2>/dev/null; EC=$?
[ $EC -ne 0 ] && ok "wrong arg count exits non-zero" || ko "wrong arg count exits non-zero"

# 12. Invalid scheduler → non-zero exit
timeout 3 "$BIN" 4 600 300 100 100 10 100 rr 2>/dev/null; EC=$?
[ $EC -ne 0 ] && ok "invalid scheduler exits non-zero" || ko "invalid scheduler exits non-zero"

# 13. Negative coders → non-zero exit
timeout 3 "$BIN" -1 600 300 100 100 10 100 fifo 2>/dev/null; EC=$?
[ $EC -ne 0 ] && ok "negative coders exits non-zero" || ko "negative coders exits non-zero"

# 14. Zero compiles required → stops immediately
OUT=$(run 5 4 800 200 100 100 0 50 fifo)
check "0 compiles_required stops immediately" "$OUT" "Simulation Complete" "burned out"

# 15. Zero cooldown — should still complete
OUT=$(run 12 4 800 200 100 100 3 0 fifo)
check "zero cooldown completes"        "$OUT" "Simulation Complete" "burned out"

# 16. Zero debug time — should still complete
OUT=$(run 10 4 800 200 0 100 3 50 fifo)
check "zero time_to_debug completes"   "$OUT" "Simulation Complete" "burned out"

# 17. Zero compile time — instant compiles
OUT=$(run 10 4 800 0 100 100 3 50 fifo)
check "zero time_to_compile completes" "$OUT" "Simulation Complete" "burned out"

# 18. 1 coder (only 1 dongle, needs 2) — must burn out
OUT=$(run 4 1 400 200 100 100 5 0 fifo)
check "1 coder burns out (cannot compile)" "$OUT" "burned out" "Simulation Complete"

# 19. Very large burnout window — should always complete
OUT=$(run 10 4 999999 200 100 100 3 50 fifo)
check "very large burnout window"      "$OUT" "Simulation Complete" "burned out"

# 20. 8 coders — larger ring still completes
OUT=$(run 20 8 2000 300 100 100 2 50 edf)
check "8 coders edf completes"         "$OUT" "Simulation Complete" "burned out"

echo ""
echo "Results: $PASS passed, $FAIL failed / $((PASS+FAIL)) total"
[ $FAIL -eq 0 ] && exit 0 || exit 1
