import sys,os,commands
import  os
import re
import sys


#print "----------------------------------------------"
Decider('MD5-timestamp')

fres=os.popen('uname -r')
kl=fres.readline().split(".")

platform="UBUNTU"
if (kl[len(kl)-1][0:3] == 'el5'):
    platform="SLC5"

if (kl[len(kl)-2][0:3] == 'el6'):
    platform="SLC6"
print kl[len(kl)-2][0:3]
fres=os.popen('uname -p')
kp=fres.readline()
osv=kp[0:len(kp)-1]

print platform,osv

Bit64=False
Bit64=os.uname()[4]=='x86_64'

kl=os.uname()[2].split(".")
platform="UBUNTU"
if (kl[len(kl)-1][0:3] == 'el5'):
    platform="SLC5"

if (kl[len(kl)-2][0:3] == 'el6'):
    platform="SLC6"

Arm=os.uname()[4]=='armv7l'

if Arm or platform=="UBUNTU":
  boostsystem='boost_system'
  boostthread='boost_thread'
else:
  boostsystem='boost_system-mt'
  boostthread='boost_thread-mt'


package=os.path.basename(os.getcwd())
  

Use_Mongoose=os.path.exists("/usr/local/include/mongoose")

# includes
INCLUDES=['./include',"/usr/include/boost141/","/usr/include/cpprest","/usr/local/include","/usr/include/log4cxx"]
  
CPPFLAGS=["-pthread","-O2","-std=c++11"]
LIBRARIES=['cpprest','pthread', 'ssl','crypto',boostsystem,"log4cxx"]


#Library path XDAQ,DHCAL and ROOT + Python
if (Bit64):
  LIBRARY_PATHS=["/usr/lib64","/usr/local/lib"]
else:
  LIBRARY_PATHS=["/usr/lib","/usr/local/lib"]

if (Use_Mongoose):
  LIBRARY_PATHS.append('/usr/local/lib')
  LIBRARIES.append('mongoose')
  INCLUDES.append('/usr/local/include/')
  INCLUDES.append('/usr/local/include/mongoose')

#link flags
LDFLAGS=["-fPIC","-dynamiclib"]


# Create the Environment
env = Environment(CPPPATH=INCLUDES,CPPFLAGS=CPPFLAGS,LINKFLAGS=LDFLAGS, LIBS=LIBRARIES,LIBPATH=LIBRARY_PATHS)

#print LIBRARY_SOURCES
LIBRARY_SOURCES=['src/baseServer.cxx','src/fsmw.cxx','src/monitor.cxx']


#Shared library
lyname="./lib/pmdaq"
print "building ",lyname
lypack=env.SharedLibrary(lyname,LIBRARY_SOURCES)

EXE_LIBPATH=LIBRARY_PATHS
EXE_LIBPATH.append("#lib")
EXE_LIBS=LIBRARIES
EXE_LIBS.append("pmdaq")


print LIBRARY_SOURCES
builderd=env.Program("./bin/pmd",source=["src/baseServer.cc"],LIBPATH=EXE_LIBPATH,LIBS=EXE_LIBS)


#myinc=[]
#for x in Glob("include/*.hh"):
#  myinc.append("include/"+x.name)
#print plist
#env.Install("lib",lypack)
#env.Install("bin",builderd)
#env.Install("../include",myinc)
###env.Install("/opt/dhcal/lib",levbdimdaq)


env.Alias('install', ["lib","bin"])



