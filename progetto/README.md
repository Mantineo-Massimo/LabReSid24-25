# Server HTTP Concorrente per Prenotazione Aule Studio

Progetto per il corso di **Laboratorio di Reti e Sistemi Distribuiti** (Anno Accademico 2024/2025) - Università degli Studi di Messina.

**Autori**: 
- Massimo Mantineo (Matricola: 541924)
- Pierluca Tino Castorina (Matricola: 545101)

## Descrizione
Il progetto consiste nella realizzazione di un server HTTP concorrente scritto in C, progettato per gestire un sistema di prenotazione di aule studio. L'architettura è basata sul pattern **Thread Pool**, che garantisce elevate prestazioni limitando il sovraccarico causato dai frequenti cambi di contesto. La sincronizzazione delle risorse condivise è gestita in modo robusto tramite *Mutex* e *Semafori*.

## Struttura del Repository
- `server_http/`: Codice sorgente in C dell'applicativo (gestione socket, parsing HTTP, API REST e Thread Pool).
- `report/`: Relazione tecnica dettagliata scritta in LaTeX.
- `presentation/`: Presentazione in LaTeX (Beamer) utilizzata per esporre il progetto.
- `assets/`: Immagini e diagrammi architettonici.

## Come compilare ed eseguire il server
1. Entrare nella cartella del server:
   ```bash
   cd server_http
   ```
2. Compilare il progetto tramite il `Makefile` fornito:
   ```bash
   make
   ```
3. Avviare l'eseguibile:
   ```bash
   ./bin/server
   ```
   Il server si porrà in ascolto all'indirizzo `http://localhost:8080`.

## Documentazione
Tutta la documentazione tecnica relativa alle scelte architetturali, allo stress test (tramite Apache Bench) e alle API è disponibile nei file `Report.pdf` e `Presentation.pdf` situati nelle rispettive cartelle.
