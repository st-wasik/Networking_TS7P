# Networking_TS7P




## 1.Opis

Aplikacja klient-serwer do testowania autorskiego protokołu binarnego TS7P.<br/>
Protokół warstwy transportowej: UDP
. <br/>
Opis stanów protokołu znajduje się w pliku TS7P_opis.pdf. <br/>
Połączenie uzyskuje się uruchamiając dwie aplikacje dla klientów oraz jedną dla serwera. <br/>
Przed uruchomieniem należy skonfugurować projekt (kompilator, linker) do pracy z użytymi bibliotekami. <br/>
Przebieg transmisji zaobserwować można za pomocą programu WireShark.

## 2.Program
Wykorzystane biblioteki: <br/>
* BOOST 1.65.1 https://www.boost.org
* SFML 2.4.2 https://www.sfml-dev.org

### Poznan 2017