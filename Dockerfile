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

# Copy from https://github.com/CN-DST-DEVELOPER/DediServer_Docker/blob/main/Dockerfile

FROM ghcr.io/cn-dst-developer/dediserver_docker:latest
LABEL org.opencontainers.image.source https://github.com/cn-dst-developer/faster_dst

ADD ./build/* /var/faster_dst/
ADD ./start_luajit.sh /usr/bin/start_dst_dedicated_server_luajit.sh

ENTRYPOINT [ "bash", "/usr/bin/start_dst_dedicated_server_luajit.sh" ] 