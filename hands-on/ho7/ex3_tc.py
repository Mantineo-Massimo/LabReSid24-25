#!/usr/bin/python3
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.link import TCLink

# Classe da passare come parametro al costruttore del Router
class GenericRouter(Node):
    def config(self, **params):
        super(GenericRouter, self).config(**params)
        # Enable forwarding on the router
        self.cmd('sysctl net.ipv4.ip_forward=1')

    def terminate(self):
        self.cmd('sysctl net.ipv4.ip_forward=0')
        super(GenericRouter, self).terminate()

class RouterTopo(Topo):
    def build(self, **_opts):
        # Aggiungiamo il router e i due host
        router = self.addNode('r0', cls=GenericRouter)
        h1 = self.addHost('h1', ip=None)
        h2 = self.addHost('h2', ip=None)

        # Link tra h1 e r0, h2 e r0 (Con vincoli fisici usando TCLink)
        self.addLink(h1, router, intfName1='h1-eth0', intfName2='r0-eth1', bw=100, delay='75ms')
        self.addLink(h2, router, intfName1='h2-eth0', intfName2='r0-eth2', bw=100, delay='75ms')

def run():
    topo = RouterTopo()
    # Inizializziamo senza controller e con TCLink
    net = Mininet(topo=topo, link=TCLink, controller=None)
    net.start()
    
    info('*** Configurazione parametri di rete tramite comandi Unix...\n')
    
    # SETUP ROUTER r0 IPs
    r0 = net.get('r0')
    r0.setIP('10.0.1.254', prefixLen=24, intf='r0-eth1')
    r0.setIP('10.0.2.254', prefixLen=24, intf='r0-eth2')
    
    # SETUP HOST h1
    h1 = net.get('h1')
    h1.setIP('10.0.1.1', prefixLen=24, intf='h1-eth0')
    h1.setDefaultRoute('via 10.0.1.254')
    
    # SETUP HOST h2
    h2 = net.get('h2')
    h2.setIP('10.0.2.1', prefixLen=24, intf='h2-eth0')
    h2.setDefaultRoute('via 10.0.2.254')
    
    info('*** Test constraints with ping...\n')
    info('*** (Delay 75ms per link = 75h1->r0 + 75r0->h2 = 150ms and back = 300ms RTT)\n')
    
    info('*** Avvio della CLI per validare infrastruttura (es. iperf, ping)\n')
    info('*** Esempio CLI: iperf h1 h2\n')
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run()
