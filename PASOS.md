# Instalar bitpit

```bash
wget https://github.com/optimad/bitpit/archive/bitpit-1.9.0.zip && unzip bitpit-1.9.0.zip -d ./lib && rm bitpit-1.9.0.zip

mv lib/bitpit-bitpit-1.9.0 lib/bitpit && cd lib/bitpit && mkdir build && cd build && ccmake ..
```

De dependencias, eu tiven que instalar as que indica en INSTALL.md e, a maiores, PETSc. Hai algunhas que son só para módulos específicos que en principio non imos usar, pero ó intentar instalar só o módulo PABLO (máis aqueles dos que depende) o ccmake mostraba outros erros diferentes.

Tras executar o `make` os headers non están onde INSTALL.md di que deberían estar, e con `make install` igual, tampouco (a menos que estea buscando eu mal) están onde din.

# Makefile

No Makefile están comentadas diferentes formas que intentei eu de engadir os ficheiros para a compilación. Tal vez aí haia tamén algún erro, non traballei moito con Makefiles antes.

# Código

As últimas liñas de `src/main.cpp` son as que teño para probar a libraría.

Ó intentar compilar o proxecto sae a seguinte mensaxe de erro. Non dá erro a liña do include e o LSP si que me ofrece para autocompletar as funcións que aparecen no manual (https://optimad.github.io/bitpit/documentation/manual/1.9.0/classbitpit_1_1ParaTree.html):

```
/usr/bin/ld: obj/main.o: warning: relocation against `_ZN6bitpit8ParaTree16DEFAULT_LOG_FILEB5cxx11E' in read-only section `.text.startup'
/usr/bin/ld: obj/main.o: in function `main':
main.cpp:(.text.startup+0x41a): undefined reference to `bitpit::ParaTree::DEFAULT_LOG_FILE[abi:cxx11]'
/usr/bin/ld: main.cpp:(.text.startup+0x422): undefined reference to `bitpit::ParaTree::ParaTree(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)'
/usr/bin/ld: main.cpp:(.text.startup+0x444): undefined reference to `bitpit::ParaTree::getL() const'
/usr/bin/ld: main.cpp:(.text.startup+0x463): undefined reference to `bitpit::ParaTree::~ParaTree()'
/usr/bin/ld: obj/main.o: in function `main.cold':
main.cpp:(.text.unlikely+0x6b): undefined reference to `bitpit::ParaTree::~ParaTree()'
/usr/bin/ld: warning: creating DT_TEXTREL in a PIE
collect2: error: ld returned 1 exit status
make: *** [Makefile:83: main] Error 1
```
