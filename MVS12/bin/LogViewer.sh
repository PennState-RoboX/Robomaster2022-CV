#!/bin/bash

ROOT_PATH=$(cd "$(dirname "$0")";pwd)

export LD_LIBRARY_PATH=${ROOT_PATH}

exec ${ROOT_PATH}/LogViewer -platform xcb

