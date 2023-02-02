if [ $# -eq 2 ];then
    if [ -d $1 ] && [ -x $1/bin64/dontstarve_dedicated_server_nullrenderer_x64 ];then
        if [ $2 = "install" ];then
            cp -f ./liblua.so $1/bin64
            cp -f ./libluajit.so $1/bin64
            cp -f ./libpreload.so $1/bin64

            cat > $1/bin64/dontstarve_dedicated_server_nullrenderer_x64_luajit <<-"EOF"
            export LD_PRELOAD=./libpreload.so
            ./dontstarve_dedicated_server_nullrenderer_x64 "$@"
            unset LD_PRELOAD
EOF
            chmod --reference=$1/bin64/dontstarve_dedicated_server_nullrenderer_x64 $1/bin64/dontstarve_dedicated_server_nullrenderer_x64_luajit
            echo "setup: DST_LUAJIT install successfully!"

        elif [ $2 = "uninstall" ];then
            rm -f $1/bin64/liblua.so
            rm -f $1/bin64/libluajit.so
            rm -f $1/bin64/libpreload.so
            rm -f $1/bin64/fast_dst.cache
            rm -f $1/bin64/dontstarve_dedicated_server_nullrenderer_x64_luajit
            echo "setup: DST_LUAJIT uninstall successfully!"
        else
            echo "setup: command line parameter error, please chech your input."
            echo "       optional parameters: [install] [uninstall]"
        fi
    else
        echo "setup: DST installation path input error!!" 
        echo "       default install path as sample: \"/root/.steam/steam/SteamApps/common/Don't Starve Together Dedicated Server\""
    fi
else
    echo "setup: parameter error!" 
    echo "       usage: sh ./setup.sh [your_dst_install_path] [install/uninstall]" 
fi

