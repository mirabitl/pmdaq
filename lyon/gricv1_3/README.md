# gricv1_3

Variante de `gricv1_2` qui garde les formats de trames et le traitement métier,
mais partage un même décodeur TCP entre les handlers register, SLC et data.
Le décodeur accepte les trames fragmentées ou regroupées dans une lecture,
rejette les longueurs et terminateurs invalides et se resynchronise sur le
prochain début de trame.

Les demandes register installent une attente avant l'envoi, puis le callback
réseau complète la réponse par identifiant de transaction et réveille le
demandeur. Les réponses sont bornées à 400 ms et les requêtes vers un même
handler register sont sérialisées.

Pour compiler ce module, installer `libhv` puis configurer PMDAQ avec
`-DPMDAQ_BUILD_MPI2=ON`. Cette option active `lyon_mpi2` et `lyon_gricv1_3` ;
elle est désactivée par défaut pour conserver les builds existants.
