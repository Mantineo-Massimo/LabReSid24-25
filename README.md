# Laboratorio di Reti e Sistemi Distribuiti (LabReSid24-25)

![Università degli Studi di Messina](https://img.shields.io/badge/Università-Messina-blue)
![Codice](https://img.shields.io/badge/Linguaggio-C%20%2F%20Python-green)
![Stato](https://img.shields.io/badge/Stato-Completato-brightgreen)

Benvenuto nel repository ufficiale del corso di **Laboratorio di Reti e Sistemi Distribuiti**, anno accademico 2024/2025. Questo repository raccoglie 19 esercitazioni (Hands-On) che coprono l'intero spettro della programmazione di rete, dai socket di basso livello fino ai sistemi distribuiti e al machine learning federato.

## 👨‍💻 Autore
*   **Nome:** Massimo Mantineo
*   **Matricola:** 541924
*   **Corso:** Reti di Calcolatori e Sistemi Distribuiti Mod. B

---

## 📂 Struttura del Repository
Il materiale è organizzato cronologicamente nella cartella `hands-on/`. Ogni cartella contiene:
*   Il codice sorgente (`.c`, `.py`, `.sh`).
*   Un `Makefile` per la compilazione automatica.
*   Una relazione tecnica dettagliata in formato **PDF**.

### Indice degli Hands-On

| ID | Titolo / Argomento | Tecnologie Principali |
|:---|:---|:---|
| **HO1** | Fondamenti di Socket (TCP/UDP) | C, Socket API, Wireshark |
| **HO2** | Socket Blocking vs Non-Blocking | C, `fcntl` |
| **HO3** | Multiplexing dell'I/O: `select()` | C, I/O Multiplexing |
| **HO4** | Alte Prestazioni: `epoll` | C, Linux Epoll |
| **HO5** | Socket Raw e ICMP (Ping) | C, Raw Sockets |
| **HO6** | Comunicazione Multicast | C, Multicast Groups |
| **HO7** | Emulazione di Rete e QoS | Mininet, Python, TC |
| **HO8** | Affinità di Processore e Multicore | C, `sched_setaffinity` |
| **HO9** | Programmazione Modulare e Makefile | C, Modular Design |
| **HO10** | Algoritmi su Grafi (Pathfinding) | C, Python |
| **HO11** | Sincronizzazione: Mutex e Semafori | Pthreads, Semaphores |
| **HO12** | Consumer-Producer e Barriere | Pthreads, Barriers |
| **HO13** | Design Pattern Multi-Thread | Pthreads |
| **HO14** | **MidTerm:** Bank TCP Server | Multithreaded TCP Server |
| **HO15** | Algoritmi Distribuiti: Flooding | IPC, FIFO (Named Pipes) |
| **HO16** | Leader Election (LCR & FloodMax) | Distributed Algorithms |
| **HO17** | Bellman-Ford (Distance-Vector) | Routing Distribuito |
| **HO18** | MapReduce con Ray.io | Python, Distributed Compute |
| **HO19** | Federated Learning con PyTorch | FL, Ray.io, Deep Learning |

---

## 🛠️ Tecnologie Utilizzate
*   **Linguaggi:** ANSI C (POSIX standard), Python 3.12.
*   **Framework Distribuiti:** Ray.io (Actor Model, MapReduce).
*   **Deep Learning:** PyTorch & Torchvision (Federated Learning).
*   **Strumenti di Rete:** Mininet, Wireshark, `iproute2`.
*   **Documentazione:** LaTeX (Beamer per presentazioni).

## 🚀 Come Eseguire
Ogni Hands-On può essere compilato singolarmente. Ad esempio, per l'HO14:
```bash
cd hands-on/ho14
make
./server
./client
```
Per gli esercizi basati su Ray (`ho18`, `ho19`), è consigliato l'uso del virtual environment incluso nella cartella:
```bash
cd hands-on/ho19
./venv/bin/python federated_learning.py
```

---

## 📄 Licenza
Questo progetto è realizzato a scopi didattici per il corso accademico. Tutti i diritti riservati.
