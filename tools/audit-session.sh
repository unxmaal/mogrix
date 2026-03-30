#!/bin/bash
# tools/audit-session.sh
# Run after each Claude Code session before merging
# Usage: tools/audit-session.sh [base_ref]
#   base_ref defaults to HEAD~1, pass a branch point for multi-commit sessions

BASE="${1:-HEAD~1}"

echo "=== Session Audit (vs $BASE) ==="
echo ""

# Shared infrastructure changes — these need human review
INFRA_PATHS="cross/bin/ cross/include/ cross/crt/ cross/lib32/ rules/generic.yaml rules/platform.yaml mogrix/rebuild.py mogrix/emitter/spec.py mogrix/rules/engine.py mcm-engine.yaml rpmmacros.irix compat/include/"

echo "── Shared infrastructure changes ──"
INFRA_CHANGES=0
for pattern in $INFRA_PATHS; do
  git diff "$BASE" --name-only | grep "^$pattern" | while read f; do
    echo "  ⚠️  $f"
    INFRA_CHANGES=1
  done
done
if [ "$INFRA_CHANGES" = "0" ]; then
  echo "  (none — clean session)"
fi

echo ""
echo "── Package-level changes (expected) ──"
echo "  Rules modified: $(git diff "$BASE" --name-only | grep '^rules/packages/' | wc -l)"
echo "  Patches added:  $(git diff "$BASE" --name-only | grep '^patches/packages/' | wc -l)"
echo "  Packages skipped: $(git diff "$BASE" -- rules/packages/ | grep '+.*skip:.*true' | wc -l)"

echo ""
echo "── New compat functions (should be rare) ──"
git diff "$BASE" --name-only | grep "^compat/" | while read f; do
  echo "  $f"
done

echo ""
echo "── MCP enforcement config changes ──"
if git diff "$BASE" --name-only | grep -q "mcm-engine.yaml"; then
  git diff "$BASE" -- mcm-engine.yaml | grep "^[+-]" | grep -v "^[+-][+-][+-]"
else
  echo "  (unchanged)"
fi

echo ""
echo "── CLAUDE.md changes ──"
if git diff "$BASE" --name-only | grep -q "CLAUDE.md"; then
  echo "  ⚠️  CLAUDE.md was modified — review changes"
else
  echo "  (unchanged)"
fi
