#!/usr/bin/env bash
set -euo pipefail
# Device tests require explicit user authorization; do not invoke --device
# automatically from agents or generic test/build runners.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OS_ROOT="$(cd "${SCRIPT_DIR}/../../tab5-os" && pwd)"
RUN_DEVICE=0

for arg in "$@"; do
    case "${arg}" in
        --device) RUN_DEVICE=1 ;;
        *) echo "opcao desconhecida: ${arg}" >&2; exit 2 ;;
    esac
done

cd "${OS_ROOT}"
python3 -m pytest -q \
    "${SCRIPT_DIR}/test_terminal_micro_shell_contract.py" \
    "${SCRIPT_DIR}/test_terminal_no_quickbar_and_enter_submit_contract.py" \
    "${SCRIPT_DIR}/test_terminal_manifest_package_contract.py"

if [ "${RUN_DEVICE}" -eq 1 ]; then
    python3 "${SCRIPT_DIR}/device/test_terminal_device_help_enter.py"
fi
