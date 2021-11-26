sudo iptables -D OUTPUT -p tcp --sport 4321 -j DROP
sudo iptables -D INPUT -p tcp --dport 4321 -j DROP
