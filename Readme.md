# Faster_DST

## Description

This is a third-party patch that replaces the lua engine of the DST linux server with luajit to improve performance. The idea of adoption is similar to paintdream's [DontStarveLuaJIT](https://github.com/paintdream/DontStarveLuaJIT)

## Install

## Use our pre-build docker image

Here is a simple docker-compose.

```yaml
version: "3"
services:
  DST_DediServer_LUAJIT:
    container_name: dst_server_luajit
    # the image used here is not the same with https://github.com/CN-DST-DEVELOPER/DediServer_Docker/pkgs/container/dediserver_docker
    image: ghcr.io/cn-dst-developer/faster_dst:latest
    ports:
      # [the port you allowed in for firewall]:[server_port from server.ini in your game save]
      - 10998:10998/udp
      - 10999:10999/udp
    volumes:
      # The path left should be modified to your game save directory, which is the parent directory of Cluster_1 normally
      - ~/dst/saves:/usr/steamapps/dst/saves
      # The path left should be modified to where you are going to save your mods, so you don't need redownload every mods while rebuild your game container. this line NOT REQUIRED
      - ~/dst/mods:/usr/steamapps/dst/dedicated_server/mods
      # The path left should be modified to where you are going to save your V2 mods, so you don't need redownload every mods while rebuild your game container. this line NOT REQUIRED
      - ~/dst/ugc_mods:/usr/steamapps/workshop/content/322330
    environment:
      # modify this to your game save name, it will be MyDediServer by default
      - CLUSTER_NAME=MyDediServer
    logging:
      options:
        max-size: 10m
        max-file: "3"
```

For further information please refer to the Readme at [DediServer_Docker](https://github.com/CN-DST-DEVELOPER/DediServer_Docker)

Likely `ghcr.io/cn-dst-developer/dediserver_docker` has a DockerHub mirror at `suv001/cn-dst-developer.dediserver_docker`, there is also a mirror for this at `suv001/cn-dst-developer.faster_dst`

## Install manually

**NOTE: The common dependence on different Linux versions are usually different, the pre-build release may not work on your computer. So we strongly recommend you using the [docker image](#Use-our-pre-build-docker-image) which is pre-installed dontstarve_dedicated_server and this patch.**

Download latest [Release](https://github.com/CN-DST-DEVELOPER/Faster_DST/releases). Upload to your server and unpack. Go to the extracted folder and execute

```shell
sh ./setup.sh [dst_install_path] install
```

Replace `[dst_install_path]` to your install path of dst, the default path download using Steam is `"/root/.steam/steam/SteamApps/common/Don't Starve Together Dedicated Server"`.

Use `dontstarve_dedicated_server_nullrenderer_x64_luajit` instead `dontstarve_dedicated_server_nullrenderer_x64` in your start game script.

## Uninstall

The install with docker can simplely be uninstalled by stop and remove the container "dst_server_luajit".

Otherwise, if this is installed manually, you can uninstall with this command in your extracted folder.

```
sh ./setup.sh [dst_install_path] uninstall
```

## For Developers

### Compilation Yourself

Attention:

- This project needs to be compiled with gcc-4.8.4, which you is the default compiler of Ubuntu 14.04 LTS. So you can import the apt source of Ubuntu 14.04 LTS and run `apt install gcc-4.8.4`.
- Due to the use of git submodule to import some of dependencies, you need run `git submodule update --init --recursive` to pull all submodule after clone this repository.
- Compile and debug require the environment variable `export DST_PATH=[dst_install_path]/bin64`
- This was developed with VSCode. If you are using another dev-environment, please refer to the configuration file at `./.vscode/launch.json`

After clone the whole repository and all of the submodules, you can start complie by executing this command in project root folder:

```shell
make libpreload CFLAGS="-fPIC -shared -O2"
make luajit CFLAGS=-O2
make lua51
```

And then copy `./build/*` to your [dst_install_path]，run these command

```shell
export LD_PRELOAD=./libpreload.so
./dontstarve_dedicated_server_nullrenderer_x64
```

to start game with this patch.

### Note

Feature about function entry search, function signature generation, signature matching are provide by [FunctionLocater](https://github.com/zxcvbnm3057/FunctionLocater).

## LICENSE

```
   Copyright 2023 Fengying <zxcvbnm3057@outlook.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```
