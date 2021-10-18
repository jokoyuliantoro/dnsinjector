This is a collection of DNS injector that I use in several DNS load testing projects.

dns-loadgen.c is based on libnet to inject the UDP DNS packets directly to a network interface. It reads domain.txt file in the same directory as the input to be converted to DNS queries. Once the query packet is crafted (DNS -> UDP -> IP -> MAC), it injects the packet to the interface. It ignores DNS response by the help of iptables. The source IP is crafted as well thus the load can simulate huge requests from various IPv4 addresses. 
One single dns-loadgen using single core can generate around 53K RPS on top of Ubuntu VM with usleep set at 100. It can be tuned to generate more RPS but I choose to launch another dns-loadgen to increase the RPS.
