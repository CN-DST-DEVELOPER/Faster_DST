MAKE = make

export CFLAGS LDFLAGS

libpreload:
	$(MAKE) -C patch preload
	mv patch/libpreload.so ./build/

luajit:
	$(MAKE) -C LuaJIT 
	mv LuaJIT/src/libluajit.so ./build/

lua51:
	$(MAKE) -C lua-5.1.4 linux_so
	mv lua-5.1.4/src/liblua.so ./build

final: clean lua51 luajit libpreload

install:
	cp -f ./build/* ${DST_PATH} 

single:
	$(MAKE) -C patch preload
	mv patch/main ./temp/

clean:
	cd patch && make clean
	cd LuaJIT && make clean
	cd lua-5.1.4 && make clean

.PHONY: libpreload luajit lua51 final install single clean