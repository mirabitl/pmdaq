# lyon_mpi2

Transport MPI sortant basé sur `libhv` (`hv::TcpClient`). Les données reçues
sont remises au callback de l'endpoint sous forme de segments, comme avec
l'ancienne couche MPI ; le callback reste responsable du réassemblage des
messages applicatifs.

`endpointId(ipv4, port)` conserve le format historique :
`(inet_addr(ipv4) << 32) | port`. Les adresses utilisées ici doivent être des
adresses IPv4 numériques.

```cpp
mpi2::MpiMessageHandler transport;
const auto id = mpi2::MpiMessageHandler::endpointId("192.0.2.10", 10002);
transport.addHandler(id, [](uint64_t, uint16_t length, char* data) {
  // Traiter les octets reçus (valides pendant la durée du callback).
});
const int socket = transport.connect("192.0.2.10", 10002);
// connect() starts asynchronously; send() returns -1 until the connection is up.
transport.send(id, request, requestLength);
```

`connect()` retourne le descripteur créé (valeur négative en cas d'erreur), pas
la confirmation de la connexion TCP. `disconnect(id)` arrête un client et
`close()` arrête tous les clients.

Compiler cette bibliothèque en activant `PMDAQ_BUILD_MPI2` dans CMake. Après
avoir cloné `https://github.com/ithewei/libhv.git`, compiler et installer
`libhv` selon ses instructions afin que `find_package(libhv CONFIG REQUIRED)`
puisse le localiser.
