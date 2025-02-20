#!/bin/sh
set -e

rm -rf -- build/**

mkdir -p build/linux
mkdir -p build/windows

scons platform=linux target=template_release
$GODOTPATH --headless project/project.godot --export-release Linux ../build/linux/nmh_viewer.x86_64

scons platform=windows target=template_release
$GODOTPATH --headless project/project.godot --export-release "Windows Desktop" ../build/windows/nmh_viewer.exe

cd build
zip nmh-godot.zip linux/** windows/**