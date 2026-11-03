# code_libhv

`lyon_code_libhv` centralizes the `libhv` TCP client lifecycle and common
socket-handler behavior used by the GRICV1 and MBMDCC drivers. `SocketHandler`
owns connection management, transaction IDs, framed-message reassembly,
register-reply synchronization, and sending; drivers retain their
hardware-specific `processPacket` implementations.

`gricv1_2` keeps its specialized data-stream corruption recovery by overriding
`processBuffer`. Build this component through
`-DPMDAQ_BUILD_LIBHV_DRIVERS=ON`; it does not depend on `lyon_mpi2`.

## Installation de libhv

Sur Debian ou Ubuntu, installez la dépendance depuis ce répertoire :

```bash
./install_libhv.sh
```

Le script installe les outils nécessaires (`build-essential`, CMake et Git),
télécharge la version `v1.3.4` de [libhv](https://github.com/ithewei/libhv),
la compile en bibliothèque partagée et l'installe dans `/usr/local`. Il utilise
`sudo` pour installer les paquets et la bibliothèque (ou doit être lancé en
root). Les sources et fichiers de construction temporaires sont supprimés à la
fin.

Depuis la racine du dépôt pmdaq, configurez ensuite le projet en gardant les
pilotes libhv activés, puis compilez-le :

```bash
cmake -S . -B build -DPMDAQ_BUILD_LIBHV_DRIVERS=ON
cmake --build build --parallel
```

Pour compiler pmdaq sans les pilotes libhv, configurez avec
`-DPMDAQ_BUILD_LIBHV_DRIVERS=OFF`.
