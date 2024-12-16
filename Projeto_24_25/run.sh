#!/bin/bash

# IP e porta do servidor
SERVER="tejo.tecnico.ulisboa.pt"
PORT="59000"
LOCAL_IP="193.136.128.108"
LOCAL_PORT="58000"

# Lista de números a testar
TEST_NUMBERS=("01" "02" "03" "04" "05" "06" "07" "08" "09" "10" "11"ec)

# Diretório de saída para os relatórios
OUTPUT_DIR="reports"

# Cria o diretório de saída, se não existir
mkdir -p "$OUTPUT_DIR"

# Loop pelos números de teste
for NUMBER in "${TEST_NUMBERS[@]}"; do
    OUTPUT_FILE="${OUTPUT_DIR}/report${NUMBER}.html"
    echo "Running test for number: $NUMBER"
    
    # Envia o comando e salva o resultado
    echo "$LOCAL_IP $LOCAL_PORT $NUMBER" | tr -d '\r' | nc "$SERVER" "$PORT" > "$OUTPUT_FILE"
    
    # Verifica se o arquivo foi gerado corretamente
    if [ -s "$OUTPUT_FILE" ]; then
        echo "Report saved: $OUTPUT_FILE"
    else
        echo "Error: No data received for number $NUMBER"
    fi
done

echo "All tests completed. Check the '$OUTPUT_DIR' directory for results."
