#!/usr/bin/env bash
set -e
[ -n "$PYENV_DEBUG" ] && set -x

program="${0##*/}"

export PYENV_ROOT="home/krishnan/.pyenv"
exec "/home/krishnan/.pyenv/libexec/pyenv" exec "$program" "$@"
