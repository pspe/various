#!/bin/bash


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./:lib/

rm *.o lib/*.so* prog




#gcc -Wall -fvisibility=hidden -fvisibility-inlines-hidden -fPIC -c ObjA.cpp singleton.cpp
#gcc -Wall -fvisibility-ms-compat -fvisibility-inlines-hidden -fPIC -c ObjA.cpp singleton.cpp
gcc -ggdb -Wall -fPIC -c ObjA.cpp singleton.cpp
gcc -ggdb -lstdc++ -shared -Wl,-soname,libobja.so.1 -Wl,--version-script=a.vscript -o libobja.so.1.0  ObjA.o singleton.o


#gcc -lstdc++ -shared -Wl,-soname,libobja.so.1 -o libobja.so.1.0  ObjA.o singleton.o

gcc -ggdb -Wall -fvisibility=hidden -fvisibility-inlines-hidden -fPIC -c ObjB.cpp singleton.cpp
#gcc -ggdb -lstdc++ -shared -Wl,-soname,libobjb.so.1 -o libobjb.so.1.0  ObjB.o singleton.o
gcc -ggdb -lstdc++ -shared -Wl,-soname,libobjb.so.1 -Wl,--version-script=a.vscript -o libobjb.so.1.0  ObjB.o singleton.o



mv libob*.so.1.0 lib/
cd lib
ln -sf libobja.so.1.0 libobja.so
ln -sf libobjb.so.1.0 libobjb.so
ln -sf libobja.so.1.0 libobja.so.1
ln -sf libobjb.so.1.0 libobjb.so.1
cd ..


gcc -Wall -Llib -L/opt/lib -L/lib -lobja -lobjb prog.cpp -o prog

chmod a+x prog
