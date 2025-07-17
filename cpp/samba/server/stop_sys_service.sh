#!/bin/bash

set -e

version="4.15.13"

if launchctl list | grep -q "$com.apple.smbd"; then
    echo "stop sys smb"
    sudo launchctl bootout system /System/Library/LaunchDaemons/com.apple.smbd.plist
else
    echo "sys smb not start, ignore"
fi