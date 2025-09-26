
```bash
sudo apt-get install software-properties-common wget
sudo add-apt-repository universe
wget https://packages.ntop.org/apt/24.04/all/apt-ntop.deb
sudo apt install ./apt-ntop.deb
```

Once the ntop repository has been added, you can run the following commands (as root) to install ntop packages:

```bash
sudo apt-get clean all
sudo apt-get update
sudo apt-get install pfring-dkms
#nprobe ntopng n2disk cento ntap
```


You can (optionally – on supported platforms) install the ZC drivers as follows:

```bash
sudo apt-get install pfring-drivers-zc-dkms
```
