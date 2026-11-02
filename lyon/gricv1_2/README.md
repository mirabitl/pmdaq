# gricv1_2

Variante de `gricv1` conservant les formats de trames et la logique de
configuration, avec des clients TCP `libhv` gérés par le composant partagé
`lyon/code_libhv`, à la place de NetLink et de `lyon/mpi2`. Le gestionnaire initialise
les trois clients GRICV1 (register, SLC et data) par carte et attend leur
connexion avant de poursuivre.

Pour compiler ce module, installer `libhv` puis activer
`-DPMDAQ_BUILD_LIBHV_DRIVERS=ON`. L'option `PMDAQ_BUILD_MPI2` conserve son
rôle indépendant pour `lyon_mpi2` ; les pilotes de cette variante ne
nécessitent pas cette bibliothèque. Le handler partagé centralise le framing
TCP et la gestion des transactions ; le flux data conserve sa récupération
spécifique après corruption.
