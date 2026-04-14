#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>    // Definizioni per il protocollo Internet (es. htons, inet_addr)
#include <netinet/ip.h>    // Struttura dell'intestazione IP (struct iphdr)
#include <netinet/ip_icmp.h> // Struttura dell'intestazione ICMP (struct icmphdr)
#include <arpa/inet.h>     // Funzioni per la manipolazione di indirizzi IP
#include <sys/time.h>      // Definizioni per la gestione dei tempi (struct timeval)

#define PACKET_SIZE 64     // Dimensione totale del pacchetto ICMP (header + payload)
#define TIMEOUT_SEC 2      // Timeout per l'attesa della risposta in secondi

/**
 * Calcola il checksum ICMP (RFC 1071 / RFC 792)
 * Il checksum è il complemento a uno della somma del complemento a uno di tutte le parole a 16 bit nel messaggio.
 * @param addr Puntatore all'inizio dei dati da calcolare
 * @param len Lunghezza dei dati in byte
 * @return Valore del checksum a 16 bit
 */
unsigned short compute_checksum(void *addr, int len) {
    int sum = 0;
    unsigned short *ptr = addr;
    
    // Somma i valori a 16 bit (2 byte alla volta)
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    
    // Se la lunghezza è dispari, aggiunge l'ultimo byte residuo
    if (len == 1)
        sum += *(unsigned char *)ptr;
    
    // Gestione del riporto (carry): aggiunge i bit eccedenti i 16 bit inferiori
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);               // Aggiunge l'eventuale ulteriore riporto
    
    // Restituisce il complemento a uno del risultato finale
    return (unsigned short)(~sum);
}

/**
 * Costruisce un pacchetto ICMP di tipo ECHO REQUEST (Tipo 8)
 * @param icmp Puntatore alla struttura icmphdr da inizializzare
 * @param sequence Numero di sequenza per identificare la richiesta
 */
void build_icmp_packet(struct icmphdr *icmp, int sequence) {
    // Definizione dei campi obbligatori secondo RFC 792
    icmp->type = ICMP_ECHO;          // Tipo 8: Echo Request
    icmp->code = 0;                  // Codice 0 per Echo Request
    icmp->un.echo.id = htons(getpid());   // ID univoco (usiamo il PID del processo)
    icmp->un.echo.sequence = htons(sequence); // Numero progressivo della richiesta
    icmp->checksum = 0;              // Deve essere 0 prima del calcolo
    
    // Aggiunta di un payload (dati opzionali) per riempire il pacchetto fino a PACKET_SIZE
    char *data = (char *)(icmp + 1); // Sposta il puntatore subito dopo l'header ICMP
    memset(data, 'A', PACKET_SIZE - sizeof(*icmp));
    
    // Calcolo finale del checksum sull'intero pacchetto ICMP
    icmp->checksum = compute_checksum(icmp, PACKET_SIZE);
}

int main(int argc, char *argv[]) {
    // Verifica degli argomenti da riga di comando
    if (argc != 2) {
        printf("Utilizzo: %s <indirizzo IP>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in dest;
    char packet[PACKET_SIZE];
    struct icmphdr *icmp = (struct icmphdr *)packet; // Mapping della struttura sull'array dei dati

    /**
     * Creazione di un SOCKET RAW (AF_INET, SOCK_RAW)
     * IPPROTO_ICMP indica che il socket gestira direttamente i pacchetti ICMP.
     * Richiede privilegi di amministratore (root).
     */
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket");
        printf("Errore: Assicurarsi di eseguire come root (sudo).\n");
        exit(1);
    }

    /**
     * Configurazione del Timeout sulla ricezione (SO_RCVTIMEO)
     * Impedisce che recvfrom() rimanga bloccata indefinitamente in caso di host irraggiungibile.
     */
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Inizializzazione della struttura di destinazione (Socket Address)
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(argv[1]); // Conversione stringa IP (es. "8.8.8.8") in formato binario

    // Costruzione del pacchetto ICMP ECHO REQUEST
    build_icmp_packet(icmp, 1);

    printf("Invio ICMP Echo Request a %s...\n", argv[1]);
    
    /**
     * Invio del pacchetto tramite sendto()
     * Nota: Il kernel incapsula automaticamente il pacchetto ICMP dentro un pacchetto IP.
     */
    if (sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&dest, sizeof(dest)) <= 0) {
        perror("sendto");
        close(sock);
        exit(1);
    }

    // Buffer per ricevere la risposta (deve contenere sia header IP che header ICMP)
    char recv_buffer[1024];
    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    
    /**
     * Attesa della risposta ICMP ECHO REPLY
     * La funzione recvfrom() restituisce il pacchetto IP completo (Header IP + Header ICMP + Payload).
     */
    int bytes = recvfrom(sock, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&src, &src_len);
    if (bytes < 0) {
        perror("Nessuna risposta (timeout)");
        close(sock);
        exit(1);
    }

    /**
     * Analisi del pacchetto ricevuto
     * L'header IP si trova all'inizio del buffer.
     */
    struct iphdr *ip_hdr = (struct iphdr *)recv_buffer;
    
    /**
     * Aritmetica dei puntatori per trovare l'header ICMP
     * L'offset dell'ICMP è dato dalla lunghezza dell'header IP.
     * 'ip_hdr->ihl' esprime la lunghezza in parole da 32 bit (4 byte). 
     * Quindi: recv_buffer + (ihl * 4)
     */
    struct icmphdr *icmp_reply = (struct icmphdr *)(recv_buffer + (ip_hdr->ihl * 4));
    
    /**
     * Verifica del tipo di pacchetto ICMP (Tipo 0 = Echo Reply) e dell'ID univoco
     * L'ID deve corrispondere a quello inviato (il PID del nostro processo).
     */
    if (icmp_reply->type == ICMP_ECHOREPLY && 
        ntohs(icmp_reply->un.echo.id) == getpid()) {
        printf("Ricevuta risposta da %s: seq=%d, TTL=%d, bytes=%d\n",
               inet_ntoa(src.sin_addr),
               ntohs(icmp_reply->un.echo.sequence),
               ip_hdr->ttl,
               bytes);
    } else {
        printf("Risposta ricevuta dal kernel ma non e' un Echo Reply valido per noi (Tipo: %d)\n", icmp_reply->type);
    }

    close(sock); // Chiusura socket
    return 0;
}
