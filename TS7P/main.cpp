// TS7 zad 9
#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <time.h>
#include <bitset>
#include <algorithm>
#include <locale.h>

#include <boost/dynamic_bitset.hpp>

using HOST = std::pair<sf::IpAddress, unsigned short>;

const unsigned int rozmiarBufora = 4096;

//przechowuje komunikaty przesyłane przez klientów i serwer
class flags
{
public:
	void reset() { GetID = 0; INV = 0; FIN = 0; ACK = 0; CNF = 0; DSC = 0; }
	flags() { GetID = 0; INV = 0; FIN = 0; ACK = 0; CNF = 0; DSC = 0; }
	bool
		GetID : 1,
		INV : 1,
		FIN : 1,
		: 1,
		ACK : 1,
		CNF : 1,
		DSC : 1,
		: 1;
	// do wypisywania zawartości na ekran
	std::string toString()
	{
		std::string result = "[Op:";
		if (GetID) result += " GetID";
		if (INV) result += " INV";
		if (FIN) result += " FIN";
		result += "]"; result += char(178); result += "[Ans:";
		if (ACK) result += " ACK";
		if (CNF) result += " CNF";
		if (DSC) result += " DSC";
		return result += "]";
	}
};

// odwraca kolejność bitów w bitsecie
template <std::size_t s>
void bitReverse(std::bitset<s>& b)
{
	auto temp = b;
	for (int i = 0; i < s; i++)
	{
		b[i] = temp[s - 1 - i];
	}
}

// do przechowywania pakietów protokołu
class TS7P
{
public:
	TS7P() :identyfikator(0), komunikaty(), dlugoscDanych(0), dane(nullptr) {}
	~TS7P() { if (dane) delete[] dane; }

	flags komunikaty;
	sf::Uint32 dlugoscDanych;
	char* dane;
	sf::Uint8 identyfikator;

	void clear()
	{
		identyfikator = 0;
		komunikaty.reset();
		dlugoscDanych = 0;

		if (dane) delete[] dane;
		dane = nullptr;
	}

	// wypełnienie pakietu danymi z odebranej tablicy znaków
	void fromCharArray(const char* array, std::size_t size)
	{
		// Instnienie tej funkcji wynika z początkowych założeń projektu, które później uległy zmianie. 
		// Funkcja wywoływana była już w programie, więc nie została usunięta, tylko zmienione zostało jej ciało.
		from_bitSet(char_to_bitSet(array, size));
	}

	// zamiana pakietu na tablicę znaków przeznaczoną do wysłania
	char* toCharArray()
	{
		// Istnienie tej funkcji wynika z początkowych założeń projektu, które później uległy zmianie. 
		// Funkcja wywoływana była już w programie, więc nie została usunięta, tylko zmienione zostało jej ciało.
		char* result = bitSet_to_char(this->to_bitSet());
		return result;
	}

	unsigned int getSize()
	{
		return sizeof(identyfikator) + sizeof(komunikaty) + sizeof(dlugoscDanych) + dlugoscDanych;
	}

	// do wypisywania na ekran
	std::string toString()
	{
		std::ostringstream result;
		result << "[ID: " << unsigned int(identyfikator) << "]" << char(178) << komunikaty.toString() << char(178) << "[L: " << dlugoscDanych << "] ";
		for (int i = 0; i < dlugoscDanych; i++)
		{
			result << dane[i];
		}
		return result.str();
	}

	// do pobrania tekstu, który pakiet przechowuje
	std::string getText()
	{
		std::string result;
		for (int i = 0; i < dlugoscDanych; i++)
		{
			result += dane[i];
		}
		return result;
	}

private:
	// do ustawienia wysyłanych bitów w odpowiedniej kolejności
	boost::dynamic_bitset<> to_bitSet()
	{
		boost::dynamic_bitset<> result;
		result.push_back(0);
		result.push_back(0);

		std::bitset<8> temp2(identyfikator);
		for (int i = 0; i < 8; i++)
		{
			result.push_back(temp2[i]);
		}

		for (int i = dlugoscDanych - 1; i >= 0; i--)
		{
			std::bitset<8> znak(dane[i]);
			for (int i = 0; i < 8; i++)
			{
				result.push_back(znak[i]);
			}
		}

		std::bitset<32> temp(dlugoscDanych);
		for (int i = 0; i < 32; i++)
		{
			result.push_back(temp[i]);
		}
		result.push_back(komunikaty.DSC);
		result.push_back(komunikaty.CNF);
		result.push_back(komunikaty.ACK);
		result.push_back(komunikaty.FIN);
		result.push_back(komunikaty.INV);
		result.push_back(komunikaty.GetID);

		return result;
	}

	// do wypełnienia pakietu odebranymi danymi
	void from_bitSet(boost::dynamic_bitset<>& bitset)
	{
		this->komunikaty.GetID = bitset[bitset.size() - 1]; bitset.pop_back();
		this->komunikaty.INV = bitset[bitset.size() - 1];  bitset.pop_back();
		this->komunikaty.FIN = bitset[bitset.size() - 1];  bitset.pop_back();

		this->komunikaty.ACK = bitset[bitset.size() - 1];  bitset.pop_back();
		this->komunikaty.CNF = bitset[bitset.size() - 1];  bitset.pop_back();
		this->komunikaty.DSC = bitset[bitset.size() - 1];  bitset.pop_back();

		std::bitset<32> temp;
		for (int i = 31; i >= 0; i--)
		{
			temp[i] = bitset[bitset.size() - 1]; bitset.pop_back();
		}
		this->dlugoscDanych = temp.to_ulong();

		if (dane) delete[] dane;
		dane = new char[dlugoscDanych];

		for (int i = 0; i < dlugoscDanych; i++)
		{
			std::bitset<8> znak;
			for (int i = 7; i >= 0; i--)
			{
				znak[i] = bitset[bitset.size() - 1];  bitset.pop_back();
			}
			dane[i] = znak.to_ulong();
		}

		std::bitset<8> temp2;
		for (int i = 7; i >= 0; i--)
		{
			temp2[i] = bitset[bitset.size() - 1];  bitset.pop_back();
		}
		identyfikator = temp2.to_ulong();

		bitset.pop_back();
		bitset.pop_back();
	}

	// zmiana tablicy char na bitset
	static boost::dynamic_bitset<> char_to_bitSet(const char * array, std::size_t size)
	{
		boost::dynamic_bitset<> result;
		for (int i = size - 1; i >= 0; i--)
		{
			for (int j = 0; j < 8; j++)
			{
				result.push_back((array[i] >> j) & 1);
			}
		}
		return result;
	}
	// zmiana bitsetu na tablicę char, która zostanie przesłana
	static char* bitSet_to_char(boost::dynamic_bitset<> b)
	{
		char* result = new char[b.size() / 8];

		for (int i = 0; i < b.size() / 8; i++)
		{
			std::bitset<8> znak;

			for (int j = 0; j < 8; j++)
			{
				znak[j] = b[(b.size() - j - 1 - i * 8)];
			}
			bitReverse(znak);
			result[i] = znak.to_ulong();
		}
		return result;
	}
};

// służy do wpisywania tekstu do pakietu
TS7P& operator<<(TS7P& k, const std::string& message)
{
	k.dlugoscDanych = message.size();
	if (k.dlugoscDanych > rozmiarBufora - (sizeof(k.identyfikator) + sizeof(k.komunikaty) + sizeof(k.dlugoscDanych)))
	{
		k.dlugoscDanych = rozmiarBufora - (sizeof(k.identyfikator) + sizeof(k.komunikaty) + sizeof(k.dlugoscDanych));
	}
	if (k.dane) delete[] k.dane;

	k.dane = new char[k.dlugoscDanych];

	for (int i = 0; i < k.dlugoscDanych; i++)
	{
		k.dane[i] = message[i];
	}
	return k;
}

// program klienta
class client final
{
public:
	client() : portSerwera(27000), portHosta(0), identyfikatorSesji(0), finish(0) {}

	sf::IpAddress adresSerwera;
	unsigned short portSerwera;
	unsigned short portHosta;
	sf::UdpSocket socket;
	sf::Uint8 identyfikatorSesji;
	bool finish;

	void run()
	{
		// bindowanie socketu do portu 0 (automatyczne przydzielenie portu przez OS)
		if (socket.bind(portHosta) != sf::Socket::Done)
		{
			std::cout << "Blad bindowania. Exit..." << std::endl;
			std::cin.ignore(2);
			exit(-1);
		}
		else
		{
			socket.setBlocking(false); // wyłączenie blokowania programu przez funkcje odebierające
			std::string temp;
			std::cout << " Server IP: "; std::cin >> temp;
			adresSerwera = sf::IpAddress(temp.c_str());
			system("cls");
			std::cout << "TS7P UDP CLIENT " << sf::IpAddress::getLocalAddress().toString() << ":" << socket.getLocalPort() << std::endl;
			std::cout << "ADRES serwera: " << adresSerwera.toString() << ":" << portSerwera << std::endl;
		}

		// wysłanie żądania identyfikatora sesji
		std::cout << "Uzyskiwanie identyfikatora sesji..." << std::endl;
		TS7P t;
		t.komunikaty.GetID = 1;
		sendTS7P(t);

		//ODBIERANIE DANYCH
		while (!finish)
		{
			TS7P aktualny_pakiet;
			sf::Socket::Status status;

			for (int i = 0; i < 10; i++)
			{
				sf::sleep(sf::milliseconds(100)); // próba odebebrania pakietu jest powtarzana 10 razy każdorazowo po odczekaniu 100ms
				aktualny_pakiet.clear();
				status = receiveTS7P(aktualny_pakiet);

				if (status == sf::Socket::Done)
				{
					// obsługa odebranego pakietu
					if (aktualny_pakiet.komunikaty.ACK)
					{
						identyfikatorSesji = aktualny_pakiet.identyfikator;
					}
					if (aktualny_pakiet.komunikaty.CNF && aktualny_pakiet.komunikaty.DSC)
					{
						std::cout << "***Nie mozna nawiazac komunikacji z drugim klientem!" << std::endl;
					}
					else if (aktualny_pakiet.komunikaty.CNF)
					{
						std::cout << "***Drugi klient przyjal twoje zaproszenie!" << std::endl;
					}
					else if (aktualny_pakiet.komunikaty.DSC)
					{
						std::cout << "***Drugi klient odrzucil twoje zaproszenie!" << std::endl;
					}
					else if (aktualny_pakiet.komunikaty.INV)
					{
						int w;
						std::cout << "***Drugi klient zaprasza do polaczenia.   Tak-> 1,   Nie-> 0" << std::endl;
						std::cin >> w;
						TS7P odp;
						if (w)
						{
							odp.komunikaty.CNF = 1;
							sendTS7P(odp);
						}
						else
						{
							odp.komunikaty.DSC = 1;
							sendTS7P(odp);
						}
					}

					if (aktualny_pakiet.dlugoscDanych > 0)
					{
						std::cout << "***WIADOMOSC od drugiego klienta: " << aktualny_pakiet.getText() << std::endl;
					}
					// koniec obsługi odebranego pakietu
				}
				else
				{
					if (status != 1)
						std::cout << "Receive status: " << status << std::endl;
				}
			}

			// interfejs użytkownika
			{
				unsigned int wybor = 0;
				std::cout << " 1- Wyslij zaproszenie\n 2- Wyslij wiadomosc\n 3- Wyswitl identyfikator\n 4- Odswiez\n 5- Zakoncz\n 6- SendACK" << std::endl;
				//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cin >> wybor;
				if (std::cin.fail()) { std::cin.clear(); wybor = 99; std::cin.ignore(); }

				switch (wybor)
				{
				case 1: {
					TS7P kom;
					kom = TS7P();
					kom.komunikaty.INV = 1;
					sendTS7P(kom);
					break;
				}
				case 2: {
					TS7P kom;
					std::string wiadomosc;
					char * temp = nullptr;
					std::cout << "Wpisz wiadomosc: ";
					//std::cin.sync();
					std::cin.ignore();
					std::getline(std::cin, wiadomosc);
					//std::cin >> wiadomosc;
					if (std::cin.fail()) { std::cin.clear(); }
					kom = TS7P();
					kom << wiadomosc;
					sendTS7P(kom);
					break;
				}
				case 3: {
					std::cout << "Identyfikator sesji: " << (unsigned int)identyfikatorSesji << std::endl;
					break;
				}
				case 4: {
					std::cout << std::flush;
					break;
				}
				case 5: {
					TS7P kom;
					kom.komunikaty.FIN = 1;
					sendTS7P(kom);
					finish = 1;
					break;
				}
				case 6: {
					TS7P kom;
					kom.komunikaty.ACK = 1;
					sendTS7P(kom);
					break;
				}
				default: {
					std::cout << "Zly wybor!" << std::endl;
					break;
				}
				}
			}
			//koniec interfejsu użytkownika

		}
		// zakończenie działania programu, odebranie ostatniego pakietu z ACK
		TS7P a;
		socket.setBlocking(true);
		receiveTS7P(a);

		sf::sleep(sf::seconds(5));
	}

	// służy do wysyłania pakietów
	void sendTS7P(TS7P t)
	{
		t.identyfikator = identyfikatorSesji;
		// przetworzenie pakietu t na tablicę char
		char* dane = t.toCharArray();

		// wysłanie do serwera pakietu przetworzonego na tablicę char. 
		auto status = socket.send(dane, t.getSize(), adresSerwera, portSerwera);
		if (status != sf::Socket::Done)
		{
			std::cout << "SEND ERROR: " << status << std::endl;
		}
		else
		{
			std::cout << "SEND <- " << adresSerwera.toString() << ":" << portSerwera << " <- " << t.toString() << std::endl;
		}
		delete[] dane;
	}

	// służy do odbierania pakietów
	sf::Socket::Status receiveTS7P(TS7P& t)
	{
		char* bufor = new char[rozmiarBufora];
		size_t odebrano; // liczba odebranych bajtów

		sf::IpAddress nadawca; // adres IP nadawcy
		unsigned short portNadawcy; // port nadawcy
		sf::Socket::Status status;

		// odebranie pakietu
		status = socket.receive(bufor, rozmiarBufora, odebrano, nadawca, portNadawcy);

		if (status != sf::Socket::Done)
		{
			if (status != sf::Socket::NotReady)
				std::cout << "REICIVE error code: " << status << std::endl;
		}
		else
		{
			// wypełnienie pakietu t odebranymi danymi
			t.fromCharArray(bufor, odebrano);
			std::cout << "REC >>> " << nadawca.toString() << ":" << portNadawcy << " -> " << t.toString() << std::endl;

			// jeżeli odebrany pakiet nie jest potwierdzeniem (ACK) to odsyłane do serwera jest potwierdzenie odebrania danych
			if (t.komunikaty.ACK == 0)
			{
				TS7P ack; ack.komunikaty.ACK = 1;
				sendTS7P(ack);
			}
		}
		delete[] bufor;
		return status;
	}
};

// program serwera
class server final
{
public:
	//std::pair<ID, <IP, PORT>> - dane od połączonych klientach
	std::pair<sf::Uint8, std::pair<sf::IpAddress, unsigned short>> host1, host2;
	const unsigned int portSerwera;
	sf::UdpSocket socket;
	bool komunikacja_klientow; // oznacza, że klienci są połączeni

	server() : portSerwera(27000), komunikacja_klientow(0)
	{
		srand(time(NULL));
		// bindowanie socketu do portu 27000
		if (socket.bind(portSerwera) != sf::Socket::Done)
		{
			std::cout << "Blad bindowania portu. Exit..." << std::endl;
			std::cin.ignore(2);
			exit(-1);
		}
		system("cls");
		std::cout << "TS7P UDP SERVER " << sf::IpAddress::getLocalAddress().toString() << ":" << socket.getLocalPort() << std::endl;
	}

	// służy do wysyłania pakietów do określonego klienta
	void sendTS7P(TS7P& t, std::pair<sf::IpAddress, unsigned short> h)
	{
		char* wyslij = t.toCharArray();
		// wysłanie danych
		if (socket.send(wyslij, t.getSize(), h.first, h.second))
		{
			std::cout << "Blad wysylania danych." << std::endl;
		}
		else
		{
			std::cout << "SEND <- " << h.first.toString() << ":" << h.second << " <- " << t.toString() << std::endl;
		}
		delete[] wyslij;
	}

	// służy do wysyłania pakietu z potwierdzeniem odbioru do określonego klienta
	void sendACK(std::pair<sf::IpAddress, unsigned short> h, sf::Uint8 session)
	{
		char* dane;
		TS7P pakiet;
		pakiet.komunikaty.ACK = 1;
		pakiet.identyfikator = session;

		dane = pakiet.toCharArray();
		if (socket.send(dane, pakiet.getSize(), h.first, h.second) != sf::Socket::Done)
		{
			std::cout << "Blad wysylania ACK." << std::endl;
		}
		else
		{
			std::cout << "ACKback " << h.first.toString() << ":" << h.second << " <- " << pakiet.toString() << std::endl;
		}
		delete[] dane;
	}

	// służy do odbierania pakietu od klienta
	HOST receiveTS7P(TS7P& t)
	{
		char* odbierz = new char[rozmiarBufora];
		size_t odebrano;

		sf::IpAddress nadawca;
		unsigned short portNadawcy;
		// odebranie danych
		if (socket.receive(odbierz, rozmiarBufora, odebrano, nadawca, portNadawcy) != sf::Socket::Done)
		{
			std::cout << "Blad odbierania danych." << std::endl;
		}
		else
		{
			// wypełnienie pakietu t odebranymi danymi
			t.fromCharArray(odbierz, odebrano);
			std::cout << "REC>>>> " << nadawca.toString() << ":" << portNadawcy << " -> " << t.toString() << std::endl;

			// Jeżeli odebrany pakiet nie był potwierdzeniem, to odsyłane jest potwierdzenie odbioru. 
			// Potwierdzenie nie jest wysyłane klientom z nieustawionym ID (0)
			if (t.identyfikator != 0 && t.komunikaty.ACK == 0)
			{
				sendACK({ nadawca,portNadawcy }, t.identyfikator);
			}
		}
		delete[] odbierz;

		// zwrócenie informacji o nadawcy
		HOST host{ nadawca, portNadawcy };
		return{ host };
	}

	void run()
	{
		while (true)
		{
			std::cout << "\nOczekiwanie na nastepny pakiet..." << std::endl;
			TS7P aktualny_pakiet;
			// odebranie i obsługa pakietu
			auto aktualny_klient = receiveTS7P(aktualny_pakiet);

			if (aktualny_pakiet.komunikaty.ACK)
			{
				std::cout << "        " << aktualny_klient.first.toString() << ":" << aktualny_klient.second << " Got 'ACK' from this client." << std::endl;
			}
			else if (aktualny_pakiet.komunikaty.GetID)
			{
				TS7P odpowiedz;
				do {
					// wylosowanie unikalnego ID dla klienta, od którego odebrane zostały dane
					sf::Uint8 random;
					random = rand() % 254 + 1;
					if (random != host1.first && random != host2.first)
					{
						// wybranie wolnego wpisu na serwerze do zapamiętania danych klienta (std::pair<..> host1, host2)
						if (host1.first == 0)
						{
							// <ID, <IP, PORT>>
							host1.first = random;
							host1.second.first = aktualny_klient.first;
							host1.second.second = aktualny_klient.second;

							odpowiedz.identyfikator = random;
							odpowiedz.komunikaty.ACK = 1;
							std::cout << "***KLIENT 1 polaczony, sesja:" << (unsigned int)host1.first << " " << host1.second.first << ":" << host1.second.second << std::endl;
						}
						else if (host2.first == 0)
						{
							// <ID, <IP, PORT>>
							host2.first = random;
							host2.second.first = aktualny_klient.first;
							host2.second.second = aktualny_klient.second;

							odpowiedz.identyfikator = random;
							odpowiedz.komunikaty.ACK = 1;
							std::cout << "***KLIENT 2 polaczony, sesja:" << (unsigned int)host2.first << ", " << host1.second.first << ":" << host1.second.second << std::endl;
						}
						else
						{
							std::cout << "***ZBYT DUZA LICZBA KLIENTOW. EXIT..." << std::endl;
							std::cin.ignore(2);
							exit(-1);
						}
					}

				} while (odpowiedz.identyfikator == 0);

				sendTS7P(odpowiedz, aktualny_klient);
			}
			else if (aktualny_pakiet.komunikaty.FIN)
			{
				// rozłączanie klientów
				if (host1.first == aktualny_pakiet.identyfikator)
				{
					std::cout << "***KLIENT 1 rozloczony" << std::endl;
					host1.first = 0;
					komunikacja_klientow = 0;
				}
				else if (host2.first == aktualny_pakiet.identyfikator)
				{
					std::cout << "***KLIENT 2 rozloczony" << std::endl;
					host2.first = 0;
					komunikacja_klientow = 0;
				}
				else
				{
					std::cout << "***PROBA ROZLACZENIA NIEPODLACZONEGO KLIENTA" << std::endl;
				}
				// gdy nie ma już żadnych klientów - wyłączenie serwera
				if (host1.first == 0 && host2.first == 0) { sf::sleep(sf::seconds(3)); return; }
			}
			else if (aktualny_pakiet.komunikaty.INV)
			{
				// jeśli klienci nie są połączeni
				if (!komunikacja_klientow)
				{
					if (host1.first && host2.first)
					{
						if (aktualny_pakiet.identyfikator == host1.first)
						{
							aktualny_pakiet.identyfikator = host2.first; // zmiana ID sesji na ID drugiego klienta
							sendTS7P(aktualny_pakiet, host2.second); // przesyłanie do drugiego klienta
						}
						else if (aktualny_pakiet.identyfikator == host2.first)
						{
							aktualny_pakiet.identyfikator = host1.first; // zmiana ID sesji na ID drugiego klienta
							sendTS7P(aktualny_pakiet, host1.second); // przesyłanie do drugiego klienta
						}
						else
						{
							std::cout << "NIEROZPOZNANY KLIENT (INV)" << std::endl;
						}
					}
					else
					{
						TS7P kom;
						kom.identyfikator = aktualny_pakiet.identyfikator;
						kom.komunikaty.CNF = 1;
						kom.komunikaty.DSC = 1;
						sendTS7P(kom, aktualny_klient);
					}
				}
			}
			else if (aktualny_pakiet.komunikaty.CNF)
			{
				// przekazanie odebranego pakietu do drugiego klienta - analogicznie jak dla INV
				komunikacja_klientow = 1;
				if (aktualny_klient == host1.second)
				{
					aktualny_pakiet.identyfikator = host2.first;
					sendTS7P(aktualny_pakiet, host2.second);
				}
				else if (aktualny_klient == host2.second)
				{
					aktualny_pakiet.identyfikator = host1.first;
					sendTS7P(aktualny_pakiet, host1.second);
				}
				else
				{
					std::cout << "NIEROZPOZNANY KLIENT (CNF)" << std::endl;
				}
			}
			else if (aktualny_pakiet.komunikaty.DSC)
			{
				// przekazanie odebranego pakietu do drugiego klienta - analogicznie jak dla INV
				if (aktualny_klient == host1.second)
				{
					aktualny_pakiet.identyfikator = host2.first;
					sendTS7P(aktualny_pakiet, host2.second);
				}
				else if (aktualny_klient == host2.second)
				{
					aktualny_pakiet.identyfikator = host1.first;
					sendTS7P(aktualny_pakiet, host1.second);
				}
				else
				{
					std::cout << "NIEROZPOZNANY KLIENT (DSC)" << std::endl;
				}
			}
			else
			{
				// odebranie pakietu, w którym nie jest ustawiona żadna flaga (INV, CNF itd.), oznacza, że przesyłana jest wiadomość
				if (komunikacja_klientow)
				{
					// jeżeli klienci są połączeni
					// przekazanie odebranego pakietu do drugiego klienta - analogicznie jak dla INV
					if (aktualny_klient == host1.second)
					{
						aktualny_pakiet.identyfikator = host2.first;
						sendTS7P(aktualny_pakiet, host2.second);
					}
					else if (aktualny_klient == host2.second)
					{
						aktualny_pakiet.identyfikator = host1.first;
						sendTS7P(aktualny_pakiet, host1.second);
					}
					else
					{
						std::cout << "NIEROZPOZNANY KLIENT (MESSAGE)" << std::endl;
					}
				}
				else
				{
					// jeżeli klienci sie są połączeni, to odsyłana jest informacja od tym do klienta nadającego wiadomość
					TS7P kom;
					if (aktualny_klient.first == host1.second.first)
						kom.identyfikator = host1.first;
					else
						kom.identyfikator = host2.first;

					kom.komunikaty.CNF = 1;
					kom.komunikaty.DSC = 1;
					sendTS7P(kom, aktualny_klient);
				}
			}
		}
		sf::sleep(sf::seconds(10));
	}
};

int main()
{
	setlocale(LC_ALL, "pl_PL");
	unsigned int wybor;
	do
	{
		std::cout << "0-Exit\n1-Server\n2-Client\n>" << std::flush;
		std::cin >> wybor;
	} while (wybor > 2);

	if (wybor == 0) return 0;
	if (wybor == 1)
	{
		server s;
		s.run();
	}
	else if (wybor == 2)
	{
		client c;
		c.run();
	}

	return 0;
}
