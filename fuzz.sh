#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<EOF
Usage: $0 [N]

Generates random inputs, compresses with gzip, and compares decompression
of the gzip via system zcat vs local zcatc (run under emu2).

Arguments:
  N                Number of iterations (default: 10)

Environment variables:
  BYTES            Size of random input in bytes (default: 1024)
  EMU2             Path to emu2 executable (default: emu2)

Examples:
  $0               # Run 10 iterations with 1 KiB inputs
  $0 100           # Run 100 iterations
  BYTES=4096 $0 50 # Run 50 iterations with 4 KiB inputs
EOF
  exit "${1:-0}"
}

# Parse arguments
if [[ $# -gt 1 ]]; then
  echo "ERROR: Too many arguments" >&2
  usage 1
fi

if [[ $# -eq 1 ]]; then
  case "$1" in
    -h|--help)
      usage 0
      ;;
    *[!0-9]*)
      echo "ERROR: Argument must be a positive integer" >&2
      usage 1
      ;;
  esac
fi

N="${1:-10}"
BYTES="${BYTES:-1024}"
EMU2="${EMU2:-emu2}"

if ! command -v "${EMU2}" >/dev/null 2>&1; then
  echo "ERROR: '${EMU2}' not found in PATH or not executable" >&2
  exit 2
fi
if [[ ! -f zcatc ]]; then
  echo "ERROR: zcatc binary not found. Build it with: nasm zcatc.nasm" >&2
  exit 2
fi
if ! command -v zcat >/dev/null 2>&1; then
  echo "ERROR: system 'zcat' not found in PATH" >&2
  exit 2
fi
if ! command -v gzip >/dev/null 2>&1; then
  echo "ERROR: 'gzip' not found in PATH" >&2
  exit 2
fi

workdir="$(mktemp -d)"
trap 'rm -rf "${workdir}"' EXIT

for i in $(seq 1 "${N}"); do
  # Generate random input of BYTES bytes
  dd if=/dev/urandom of="${workdir}/in.bin" bs="${BYTES}" count=1 status=none
  # gzip with -n to avoid filename/mtime in header (not required, but cleaner)
  gzip -c -n "${workdir}/in.bin" > "${workdir}/in.gz"

  # Decompress with system zcat and our zcatc
  if ! zcat "${workdir}/in.gz" > "${workdir}/out.zcat"; then
    echo "ERROR: zcat failed on iteration ${i}" >&2
    exit 1
  fi
  if ! "${EMU2}" zcatc < "${workdir}/in.gz" > "${workdir}/out.zcatc"; then
    echo "ERROR: zcatc failed on iteration ${i}" >&2
    exit 1
  fi

  if ! cmp -s "${workdir}/out.zcat" "${workdir}/out.zcatc"; then
    # Copy test file to current directory for debugging
    testfile="fuzz-fail-$(date +%Y%m%d-%H%M%S).gz"
    cp "${workdir}/in.gz" "${testfile}"
    echo "\nMismatch on iteration ${i}" >&2
    echo "Test file saved to: ${testfile}" >&2
    echo "sha256 sums:" >&2
    command -v sha256sum >/dev/null 2>&1 && sha256sum "${workdir}/out.zcat" "${workdir}/out.zcatc" >&2 || true
    exit 1
  fi
  printf "."
done

echo
echo "All ${N} iterations passed."
