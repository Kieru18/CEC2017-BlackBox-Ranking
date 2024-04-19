Sposób aktywacji.

1. W main.cc (w funkcji get_database_result) wpisać poprawne dane użytkownika sql oraz nazwę database'a.

2. W main.cc (w funkcji send_api_key) wpisać poprawne dane skrzynki mailowej administratora (int.pl jest zalecane, ponieważ SMTP działa tam bez zarzutu)

3. Należy zainstalować bibliotekę POCO - do obsługi poczty, linijką: $ sudo apt-get install libpoco-dev

4. Należy wpisać kolejno linijki:

   $ mkdir build
   $ cd build
   $ cmake ..
   $ make
   i aktywować polecenie plikiem
   $ ./main
W ramach zabawy można włączyć nowy terminal, wpisać

curl -X POST -H "Content-Type: application/json" -d '{"mail": "twoj.adres@mail.com"}' http://localhost:8080/registrate
