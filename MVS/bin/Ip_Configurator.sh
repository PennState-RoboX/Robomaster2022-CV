#!/bin/bash

ROOT_PATH=$(cd "$(dirname "$0")";pwd)

export LD_LIBRARY_PATH=${ROOT_PATH}:/opt/MVS/lib/64

exec ${ROOT_PATH}/Ip_Configurator -platform xcb
