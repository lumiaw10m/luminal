#!/bin/sh
set -e

VERSION=2.1.112

echo "Installing bash, Node.js and ripgrep..."
apk add -q --no-progress bash nodejs npm ripgrep

mkdir -p /var/lib/claude
chmod 700 /var/lib/claude

cat > /etc/profile.d/claude.sh <<'EOF'
[ -x /bin/bash ] && export SHELL=/bin/bash
export DISABLE_AUTOUPDATER=1 USE_BUILTIN_RIPGREP=0 CLAUDE_CONFIG_DIR=/var/lib/claude
EOF

echo "Installing Claude Code $VERSION..."
DISABLE_AUTOUPDATER=1 npm install -g --no-audit --no-fund --loglevel=error "@anthropic-ai/claude-code@$VERSION"

. /etc/profile.d/claude.sh
if ! claude --version >/dev/null 2>&1; then
    echo "Claude Code was installed, but it didn't start. Try running: claude --version" >&2
    exit 1
fi

echo
echo "Claude Code $VERSION is ready. Open a new tab and run: claude"
echo "Newer versions don't run on 32-bit ARM, so automatic updates are turned off."
