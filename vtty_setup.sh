echo "Killing ALL socat instances"
sudo pkill socat

sudo ls > /dev/null
echo Setting Up [Master/Slave] Ports
sudo socat -d -d -d -d -ls pty,link=/dev/master,raw,echo=0,user=david.valdespino,group=staff pty,link=/dev/master_side,raw,echo=0,user=david.valdespino,group=staff &> /dev/null &
echo Setting Up [Master/Slave] Ports for DEBUG
sudo socat -d -d -d -d -ls pty,link=/dev/slave,raw,echo=0,user=david.valdespino,group=staff pty,link=/dev/slave_side,raw,echo=0,user=david.valdespino,group=staff &> /dev/null &
