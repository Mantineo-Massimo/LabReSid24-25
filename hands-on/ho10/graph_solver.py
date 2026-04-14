import threading
import time

def compute(op1, op2, operation, result_list, index):
    """Funzione per le operazioni elementari"""
    if operation == '*':
        res = op1 * op2
    elif operation == '+':
        res = op1 + op2
    elif operation == '-':
        res = op1 - op2
    else:
        res = 0
    
    print(f"[Thread {chr(65+index)}] Eseguo: {op1} {operation} {op2} = {res}")
    result_list[index] = res

def main():
    print("Risoluzione equazione: y = (2 * 6) + (1 + 4) * (5 - 2)")
    print("Avvio dei thread per le operazioni indipendenti...\n")

    # Lista per i risultati (per catturare i valori di ritorno)
    results = [None] * 3
    
    # Lancio i primi 3 thread in parallelo
    tA = threading.Thread(target=compute, args=(2, 6, '*', results, 0))
    tB = threading.Thread(target=compute, args=(1, 4, '+', results, 1))
    tC = threading.Thread(target=compute, args=(5, 2, '-', results, 2))

    tA.start()
    tB.start()
    tC.start()

    # Sincronizzazione: attendo i risultati (Join)
    tA.join()
    tB.join()
    tC.join()

    print("\nRisultati parziali ottenuti. Calcolo i nodi dipendenti...")

    # Nodo D = B * C
    resD = results[1] * results[2]
    print(f"[Main] Nodo D (B * C): {results[1]} * {results[2]} = {resD}")

    # Nodo E = A + D (Risultato finale)
    final_y = results[0] + resD
    print(f"[Main] Risultato Finale y (A + D): {results[0]} + {resD} = {final_y}")

if __name__ == "__main__":
    main()
