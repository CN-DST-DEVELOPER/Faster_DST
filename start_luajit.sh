#!/bin/bash

# Copyright 2023 Fengying <zxcvbnm3057@outlook.com>
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#Modified from https://accounts.klei.com/assets/gamesetup/linux/run_dedicated_servers.sh

install_dir="/usr/steamapps/dst/dedicated_server"
ugc_dir="/usr"
storage_root_dir="/usr/steamapps/dst"
conf_dir="saves"

if [ ! -n "$CLUSTER_NAME" ]; then
	cluster_name="MyDediServer"
else
	cluster_name=$CLUSTER_NAME
fi

function fail() {
	echo Error: "$@" >&2
	exit 1
}

function check_for_file() {
	if [ ! -e "$1" ]; then
		fail "Missing file: $1"
	fi
}

check_for_file "$storage_root_dir/$conf_dir/$cluster_name/cluster.ini"
check_for_file "$storage_root_dir/$conf_dir/$cluster_name/cluster_token.txt"
check_for_file "$storage_root_dir/$conf_dir/$cluster_name/Master/server.ini"
check_for_file "$storage_root_dir/$conf_dir/$cluster_name/Caves/server.ini"

echo "Start to update Dedicated Server"

steamcmd +force_install_dir $install_dir +login anonymous +app_update 343050 validate +quit | grep -q "Success! App '343050' fully installed."
if [ $? ]; then
	check_for_file "$install_dir/bin64"
	echo "Download Dedicated Server Successfully"
else
	fail "Dedicated Server Install Failed"
fi

echo "Start to download required mods"

mod_download=""
modlist=()

if [ -e "$storage_root_dir/$conf_dir/$cluster_name/Master/modoverrides.lua" ]; then
	modlist+=($(cat "$storage_root_dir/$conf_dir/$cluster_name/Master/modoverrides.lua" | grep -o "\[\"workshop-[0-9]*\"\]="))
fi
if [ -e "$storage_root_dir/$conf_dir/$cluster_name/Caves/modoverrides.lua" ]; then
	modlist+=($(cat "$storage_root_dir/$conf_dir/$cluster_name/Caves/modoverrides.lua" | grep -o "\[\"workshop-[0-9]*\"\]="))
fi

modlist=$(awk -v RS=' ' '!a[$1]++' <<<${modlist[@]})
for mod in ${modlist[*]}; do mod_download="$mod_download +workshop_download_item 322330 ${mod:11:0-3} validate"; done
modlist_v1=$(steamcmd +force_install_dir $ugc_dir +login anonymous $mod_download +quit | grep -o 'ERROR!\ Download\ item\ [0-9]*\ failed' | grep -o '[0-9]*')

rm "$install_dir/mods/dedicated_server_mods_setup.lua"
for mod in ${modlist_v1[*]}; do echo "ServerModSetup("${mod}")" >>"$install_dir/mods/dedicated_server_mods_setup.lua"; done

echo "Download Required Mods Finished"

cd "$install_dir/bin64" || fail

run_shared=(./dontstarve_dedicated_server_nullrenderer_x64)
run_shared+=(-persistent_storage_root "$storage_root_dir")
run_shared+=(-conf_dir "$conf_dir")
run_shared+=(-ugc_directory "$ugc_dir/steamapps/workshop")
run_shared+=(-console)
run_shared+=(-cluster "$cluster_name")
run_shared+=(-monitor_parent_process $$)

echo "Dedicated Server Started. For more detial about game server running, please check server_log.txt in shard dictionary."

cp -f /var/faster_dst/* $install_dir/bin64/
export LD_PRELOAD=./libpreload.so

nohup "${run_shared[@]}" -shard Master >/dev/null 2>&1 &
nohup "${run_shared[@]}" -shard Caves >/dev/null 2>&1