#!/usr/bin/python3
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.log import setLogLevel, info
from mininet.cli import CLI

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
        # Aggiungiamo il router e i due host senza definire gli IP qui (li setteremo con comandi unix)
        router = self.addNode('r0', cls=GenericRouter)
        h1 = self.addHost('h1', ip=None)
        h2 = self.addHost('h2', ip=None)

        # Link tra h1 e r0, h2 e r0
        self.addLink(h1, router, intfName1='h1-eth0', intfName2='r0-eth1')
        self.addLink(h2, router, intfName1='h2-eth0', intfName2='r0-eth2')

def run():
    topo = RouterTopo()
    # Inizializziamo senza controller perche' usiamo un router L3
    net = Mininet(topo=topo, controller=None)
    net.start()
    
    info('*** Routing Table on Router (Prima):\n')
    info(net['r0'].cmd('route'))
    
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
    
    info('*** Routing Table on Router (Dopo Configurazione):\n')
    info(r0.cmd('route'))
    
    info('*** Avvio della CLI per validare infrastruttura (es. con comando ping, ip route, ecc...)\n')
    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run()
