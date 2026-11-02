# gricv0_3

`gricv0_3` is a GRICV0 driver variant using the shared `code_libhv` TCP socket
handler instead of the legacy NetLink transport. It preserves the GRICV0
commands, frame layout, and data decoding while reusing libhv connection,
frame-reassembly, transaction, and reply-wait handling.

Build it together with the other libhv drivers using
`-DPMDAQ_BUILD_LIBHV_DRIVERS=ON`. The existing `gricv0` target remains
available unchanged.
