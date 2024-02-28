# Instalar bitpit

```bash
wget https://github.com/optimad/bitpit/archive/bitpit-1.9.0.zip && unzip bitpit-1.9.0.zip -d ./lib && rm bitpit-1.9.0.zip

mv lib/bitpit-bitpit-1.9.0 lib/bitpit && cd lib/bitpit && mkdir build && cd build && ccmake ..
```

De dependencias, eu tiven que instalar as que indica en INSTALL.md e, a maiores, PETSc. Hai algunhas que son só para módulos específicos que en principio non imos usar, pero ó intentar instalar só o módulo PABLO (máis aqueles dos que depende) o ccmake mostraba outros erros diferentes (pode solventarse instalando `voloctree` xunto con PABLO).

Tamén hai que configurar o CMAKE_INSTALL_PREFIX para que apunte ó directorio `build` que acabamos de crear, así tras facer `make install` si que aparecen os directorios `include/` e `lib/`.

# CMake

Engadido `cmake/modules/FindBITPIT.cmake`. Debería funcionar, pero ó compilar con `make` dáme o erro que aparece máis abaixo.

En `cmake/CMakeLibraries.cmake` hai comentada outra forma que intentei (por iso está nos módulos de CMake LibFindMacros), pero aí `BITPIT_USE_FILE` é unha variable vacía, e non sei por que.

```
/usr/bin/ld: /tmp/ccbxl11C.ltrans0.ltrans.o: in function `main':
<artificial>:(.text+0x65dc4): undefined reference to `bitpit::ParaTree::ParaTree(unsigned char, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)'
collect2: error: ld returned 1 exit status
make[2]: *** [CMakeFiles/rule-based-classifier-cpp.dir/build.make:251: rule-based-classifier-cpp] Error 1
make[2]: Leaving directory '/home/frd/Documents/4o/TFG/octree-mpi'
make[1]: *** [CMakeFiles/Makefile2:83: CMakeFiles/rule-based-classifier-cpp.dir/all] Error 2
make[1]: Leaving directory '/home/frd/Documents/4o/TFG/octree-mpi'
make: *** [Makefile:91: all] Error 2
```
