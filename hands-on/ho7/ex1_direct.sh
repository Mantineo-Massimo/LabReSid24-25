#!/bin/bash

# Esercizio 1: Indirizzamento Diretto
# Questo script mostra i comandi da eseguire nel terminale per l'Esercizio 1.
# Non e' necessario eseguirlo direttamente come script, basta seguire i passaggi
# o copiarli nel terminale passo passo.

echo ">>> Creazione Topologia in Mininet (2 host, 1 interruttore/switch)"
echo "Esegui il seguente comando per avviare mininet:"
echo "sudo mn --topo single,2"
echo ""

echo ">>> Apertura terminali xterm"
echo "Una volta dentro la CLI di mininet (mininet>), digita:"
echo "xterm h1 h2"
echo ""

echo ">>> Su Xterm di h1 (Ascolto)"
echo "Apri Wireshark sull'interfaccia 'any' o 's1-eth1' per catturare il traffico."
echo "Quindi, sul terminale di h1 avvia netcat in ascolto sulla porta 5432:"
echo "nc -l -p 5432"
echo ""

echo ">>> Su Xterm di h2 (Connessione)"
echo "Collegati all'IP di h1 (che di default in Mininet e' 10.0.0.1) sulla porta 5432:"
echo "nc 10.0.0.1 5432"
echo ""

echo ">>> Invia un messaggio da h2 a h1, osservandolo su Wireshark!"
