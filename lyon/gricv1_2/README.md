# gricv1_2

Variante de `gricv1` conservant les formats de trames et la logique de
configuration, avec la couche TCP de `lyon/mpi2` (`libhv`) à la place de
NetLink. Le gestionnaire initialise les trois clients GRICV1 (register, SLC et
data) par carte et attend leur connexion avant de poursuivre.

Pour compiler ce module, installer `libhv` puis configurer PMDAQ avec
`-DPMDAQ_BUILD_MPI2=ON`. Cette option active `lyon_mpi2` et `lyon_gricv1_2` ;
elle est désactivée par défaut pour conserver les builds existants.
