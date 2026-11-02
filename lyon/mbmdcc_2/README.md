# lyon_mbmdcc_2

`lyon_mbmdcc_2` keeps the MBMDCC register protocol and manager commands. Its
register handler uses the shared `code_libhv::SocketHandler` for TCP framing,
transactions, and reply synchronization, without linking `lyon/mpi2`.
Register requests wait up to 400 ms for an acknowledgement.

Build this module with `-DPMDAQ_BUILD_LIBHV_DRIVERS=ON` after installing
`libhv`. This option is separate from `PMDAQ_BUILD_MPI2`, and the module does
not link `lyon_mpi2`. The original `lyon_mbmdcc` target remains available and
unchanged.
