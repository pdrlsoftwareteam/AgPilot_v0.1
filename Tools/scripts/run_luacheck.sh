#!/bin/bash
# Run lua check for all lua files passing AP specific config

cd "$(dirname "$0")"
cd ../..

luacheck */ --config libraries/AG_Scripting/tests/luacheck.lua
