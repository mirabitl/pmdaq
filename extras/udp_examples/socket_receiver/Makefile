#This is for pfring_rcv.c
#
# PF_RING
#
PFRINGDIR  ?= /usr/lib
LIBPFRING  ?= ${PFRINGDIR}/libpfring.a

#
# PF_RING aware libpcap
#
O_FLAG     = -g -DHAVE_PF_RING
EXTRA_LIBS =
PCAPDIR    ?= /usr/local/lib
LIBPCAP    ?= ${PCAPDIR}/libpcap.a  ${EXTRA_LIBS}

#
# Search directories
#
INCLUDE    ?= -I${PFRINGDIR} -I${PCAPDIR} -Ithird-party -I/opt/pmdaq/include -I/usr/include/jsoncpp

CC         = ${CROSS_COMPILE}g++
CFLAGS     =  ${O_FLAG} -Wall ${INCLUDE} -g

#
# User and System libraries
#
LDFLAGS       = -L/usr/local/pmdaq/lib -lpmdaq ${LIBPFRING} ${LIBPCAP} -lpthread  -lrt -ldl -lssl -lcrypto -llog4cxx -lcpprest -ljsoncpp -lboost_system -lcrypto -lssl -lz 

PFPROGS   = pfring_rcv
TARGETS   = ${PFPROGS}

all: ${TARGETS}

$(PFPROGS): $(PFPROGS).o ${LIBPFRING}
	${CC} ${CFLAGS} $(PFPROGS).o ${LDFLAGS} -o $@

clean:
	@rm -f ${TARGETS} *.o *~
