# code_libhv

`lyon_code_libhv` centralizes the `libhv` TCP client lifecycle and common
socket-handler behavior used by the GRICV1 and MBMDCC drivers. `SocketHandler`
owns connection management, transaction IDs, framed-message reassembly,
register-reply synchronization, and sending; drivers retain their
hardware-specific `processPacket` implementations.

`gricv1_2` keeps its specialized data-stream corruption recovery by overriding
`processBuffer`. Build this component through
`-DPMDAQ_BUILD_LIBHV_DRIVERS=ON`; it does not depend on `lyon_mpi2`.
