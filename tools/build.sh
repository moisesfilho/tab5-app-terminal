#!/usr/bin/env bash
# build.sh - Compila e empacota o app Terminal para Tab5 OS
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
DIST_DIR="${APP_DIR}/dist"
SDK_DIR="${TAB5_SDK_PATH:-${APP_DIR}/../tab5-os/sdk/tab5-app-sdk}"
PACK_TOOL="${SDK_DIR}/tools/pack.py"
WASI_CLANG="${WASI_SDK_PATH:-/home/moises/.wasi-sdk}/bin/clang"

mkdir -p "${DIST_DIR}"

if [ -x "${WASI_CLANG}" ] && [ -f "${APP_DIR}/src/main.c" ]; then
    echo "[INFO] Compilando WebAssembly com wasi-sdk clang..."
    WRAPPER="$(mktemp "${TMPDIR:-/tmp}/tab5-terminal-entrypoint.XXXXXX.c")"
    cleanup_wrapper() {
        rm -f "${WRAPPER}"
    }
    trap cleanup_wrapper EXIT
    cat >"${WRAPPER}" <<'EOF'
#include "tab5_sdk.h"

extern int main(int, char **);

TAB5_APP_ENTRYPOINT_EXPORT int tab5_wasm_app_main(void)
{
    return main(0, NULL);
}
EOF
    "${WASI_CLANG}" -O2 -I"${SDK_DIR}/include" \
        -Wl,--export=main -Wl,--export=tab5_app_on_ui_event -Wl,--export=on_ui_event \
        -Wl,--export=tab5_app_on_theme_changed -Wl,--export=on_theme_changed \
        -Wl,--export=tab5_app_on_open_file -Wl,--export=on_open_file -Wl,--allow-undefined \
        -o "${APP_DIR}/app.wasm" "${APP_DIR}/src/main.c" "${WRAPPER}"
else
    echo "[WARN] wasi-sdk nao encontrado; mantendo app.wasm existente"
fi

echo "[INFO] Empacotando com Tab5 Pack Tool..."
python3 "${PACK_TOOL}" "${APP_DIR}" -o "${DIST_DIR}"

echo "[OK] Build e empacotamento concluidos com sucesso em ${DIST_DIR}/com.tab5.terminal.tab5pkg"
