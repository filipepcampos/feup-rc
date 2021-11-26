sudo iptables -A OUTPUT -p tcp --sport 4321 -j DROP
sudo iptables -A INPUT -p tcp --dport 4321 -j DROP
