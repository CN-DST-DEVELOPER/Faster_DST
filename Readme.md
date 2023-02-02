# Faster_DST

## Description 

This is a third-party patch that replaces the lua engine of the DST linux server with luajit to improve performance. The idea of adoption is similar to paintdream's [DontStarveLuaJIT](https://github.com/paintdream/DontStarveLuaJIT)

## Install/Update

Download latest [Release](https://github.com/CN-DST-DEVELOPER/Faster_DST/releases). Upload to your server and unpack. Go to the extracted folder and execute

```shell
sh ./setup.sh [dst_install_path] install
```

Replace `[dst_install_path]` to your install path of dst, the default path download using Steam is `"/root/.steam/steam/SteamApps/common/Don't Starve Together Dedicated Server"`.

Use `dontstarve_dedicated_server_nullrenderer_x64_luajit` instead `dontstarve_dedicated_server_nullrenderer_x64` in your start game script.

## Uninstall

Execute this command in your extracted folder.

```
sh ./setup.sh [dst_install_path] uninstall
```

## For Developers

### Compilation Yourself

Attention:

- This project needs to be compiled with gcc-4.8.4, which you is the default compiler of Ubuntu 14.04 LTS. So you can import the source of Ubuntu 14.04 LTS and run `apt install gcc-4.8.4`.
- Due to the use of git submodule to import some of dependencies, you need run `git submodule update --init --recursive` to pull all submodule after clone this repository.
- Compile and debug require the environment variable `export DST_PATH=[dst_install_path]/bin64`
- This was developed with VSCode. If you are using another dev-environment, please refer to the configuration file at `./.vscode/launch.json`

After clone the whole repository and all of the submodules, you can start complie by executing this command in project root folder:

```shell
make final
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