# gricv1_3

Variante de `gricv1_2` qui garde les formats de trames et le traitement métier,
avec un client TCP géré par le composant partagé `lyon/code_libhv` pour chaque
handler register, SLC et data, sans dépendance à `lyon_mpi2`.
Le décodeur accepte les trames fragmentées ou regroupées dans une lecture,
rejette les longueurs et terminateurs invalides et se resynchronise sur le
prochain début de trame.

Les demandes register installent une attente avant l'envoi, puis le callback
réseau complète la réponse par identifiant de transaction et réveille le
demandeur. Les réponses sont bornées à 400 ms et les requêtes vers un même
handler register sont sérialisées.

Pour compiler ce module, installer `libhv` puis configurer PMDAQ avec
`-DPMDAQ_BUILD_LIBHV_DRIVERS=ON`. Cette option active `lyon_gricv1_2`,
`lyon_gricv1_3` et `lyon_mbmdcc_2`. Elle est indépendante de
`PMDAQ_BUILD_MPI2` ; ces modules réutilisent `lyon/code_libhv` pour le
transport TCP.
