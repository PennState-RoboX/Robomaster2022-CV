#!/bin/bash

ROOT_PATH=$(cd "$(dirname "$0")";pwd)
export LD_LIBRARY_PATH=/opt/MVS/bin:/opt/MVS/lib/aarch64

exec ${ROOT_PATH}/Import_Export_Features
