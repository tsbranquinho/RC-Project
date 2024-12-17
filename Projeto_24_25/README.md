-> Para correres faz make nesta pasta
-> Depois faz make run-server ou make run-client
-> Também podes ir a cada página individual e fazer make e make run, por enquanto não faças a partir do build
-> Isto pode ser mudado, mas por enquanto também está temporário porque quero silenciar todos os makes

//duvidas

fazer dicas (tipo dizer o sitio da peça msm) acrescentar um menu de HELP no player 
documento stor
Se cliente receber mensagens mal formatadas: temina interação com o servidor e informa o utilizador imprimindo uma mensagem de erro no écran. ...

CORRIGIR DINAMICIDADE DAS PASTAS OU ALTERAR SÓ O SÍTIO DA BUILD
PROBLEMAS COM EXIT NOK
meter na make para receber argumentos ...
ver show_trials e FIN

podemos testar com o servidor daqui
# Projeto_RC

## Client Commands to implement

To test the commands, run `./player -n 193.136.138.142 -p 58011`

- start ✅
- try ✅
- quit ✅
- exit ✅
- debug ✅
- trials ✅ / Missing FIN ainda mandamos a chave quando estamos a meio
- scoreboard ✅
//Extras
- set ✅
- hint

## Server Commands to implement
- start ✅
- try ✅
- quit ✅
- debug ✅
- trials ✅ / Missing FIN and formatting
- scoreboard ✅
- hint

--set PLID done, try dynamic formatting done, debug done

ssh sigma.ist.utl.pt -l ist1106635
scp -r  Projeto_24_25/ ist1106635@sigma.tecnico.ulisboa.pt:~/Branquinho2
echo "193.136.128.108 58067 10" | tr -d '\r' | nc tejo.tecnico.ulisboa.pt 59000 > reports/report10.html