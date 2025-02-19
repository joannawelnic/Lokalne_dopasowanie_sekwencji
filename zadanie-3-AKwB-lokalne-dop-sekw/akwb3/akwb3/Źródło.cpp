#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <set>

using namespace std;
int l = 4;   // dlugosc podciagu
int d = 0;    // prog wiarygodnosci

struct Sekwencja {
    int idSek;
    string sekwencja;
    vector<int> idNukleotydu; // przechowuje ID nukleotydu (odwolywanie po nim) i jego WIARYGODNOSC
    vector<int> wiarygodnosc;
};

vector<Sekwencja> sekwencje;  // wektor typu struktura Sekwencja

void Uzytkownik() {
    while (true) {
        cout << "Wpisz dlugosc podciagu 4-9: ";
        int lt;   // tymczasowe l
        cin >> lt;
        if (lt >= 4 && lt <= 9) {   // jesli miesci sie w przedziale
            l = lt;
            break;
        }
        else {
            cout << "Zla dlugosc podciagu." << "\n";
        }
    }
    while (true) {
        cout << "Wpisz prog wiarygodnosci 0-40: ";
        int dt;   // tymczasowe d
        cin >> dt;
        if (dt >= 0 && dt <= 40) {   // jesli miesci sie w przedziale
            d = dt;
            break;
        }
        else {
            cout << "Zly prog wiarygodnosci." << "\n";
        }
    }
}

void OdczytZPlikuFasta(const string& nazwaPliku, vector<Sekwencja>& sekwencje) {   // odczytuje i tworzy obiekty Sekwencja
    ifstream plik(nazwaPliku);
    if (!plik.is_open()) {
        cout << "Nie mozna otworzyc pliku!" << endl;
        return;
    }

    string linia;
    string obecnaSekwencja;
    int idSek = 1;

    while (getline(plik, linia)) {
        if (linia.empty()) continue;
        // w pierwszej iteracji pierwszy if sie wykona, ale if w ifie sie wykona wiec pomija elsa potem
        if (linia[0] == '>') {   // jesli linia nie zaczyna sie od '>' zostaje wczytywana obecna sekwencja (else). 2 i 3 iteracja - nie zaczyna sie od '<', dodaje w elsie linie
            if (!obecnaSekwencja.empty()) {   // w pierwszej iteracji obecnaSekwencja bedzie empty, wiec sie nie wykona. 4 iteracja - obecnaSekwencja nie jest empty
                Sekwencja nowaSekwencja;
                nowaSekwencja.idSek = idSek;
                nowaSekwencja.sekwencja = obecnaSekwencja;
                sekwencje.push_back(nowaSekwencja);   // dodanie do wektora sekwencje nowej struktury sekwencji 

                obecnaSekwencja.clear();
                idSek++;
            }
        }
        else {
            obecnaSekwencja += linia;
        }
    }

    if (!obecnaSekwencja.empty()) {
        Sekwencja nowaSekwencja;
        nowaSekwencja.idSek = idSek;
        nowaSekwencja.sekwencja = obecnaSekwencja;
        sekwencje.push_back(nowaSekwencja);
    }

    plik.close();
}

void OdczytZPlikuQual(const string& nazwaPliku, vector<Sekwencja>& sekwencje) {   // odczytuje i tworzy obiekty Sekwencja
    ifstream plik(nazwaPliku);
    if (!plik.is_open()) {
        cout << "Nie mozna otworzyc pliku!" << endl;
        return;
    }

    string linia;
    int idSek = 1;
    int aktualnaSekwencja = -1;
    int idNuk = 0;

    while (getline(plik, linia)) {
        if (linia.empty()) {
            continue;
        }

        if (linia[0] == '>') {   // jesli linia nie zaczyna sie od '>' zostaje wczytywana obecna sekwencja (else). 2 i 3 iteracja - nie zaczyna sie od '<'
            aktualnaSekwencja += 1;
            idNuk = 0;  // zerowanie indeksu nukleotydu gdy rozpoczyna dodawanie dla nowej sekwencji
        }
        else {
            istringstream stream(linia);

            int wiarygodnoscOdczyt;
            while (stream >> wiarygodnoscOdczyt) {   // dodaje do mapy dopoki nie wykona sie if wyzej '>' wtedy odwoluje sie do nastepnej sekwencji
                sekwencje[aktualnaSekwencja].idNukleotydu.push_back(++idNuk);
                sekwencje[aktualnaSekwencja].wiarygodnosc.push_back(wiarygodnoscOdczyt);
            }
        }
    }

    plik.close();
}

void UsunPonizejWiarygodnosci() {
    for (auto& sekwencjaTemp : sekwencje) {
        vector<int> nowaWiarygodnosc;
        vector<int> noweIdNukleotydu;
        string nowaSekwencja;

        for (size_t i = 0; i < sekwencjaTemp.sekwencja.size(); ++i) {
            if (sekwencjaTemp.wiarygodnosc[i] >= d) {
                nowaWiarygodnosc.push_back(sekwencjaTemp.wiarygodnosc[i]);
                noweIdNukleotydu.push_back(sekwencjaTemp.idNukleotydu[i]);
                nowaSekwencja += sekwencjaTemp.sekwencja[i];
            }
        }

        sekwencjaTemp.wiarygodnosc = nowaWiarygodnosc;
        sekwencjaTemp.idNukleotydu = noweIdNukleotydu;
        sekwencjaTemp.sekwencja = nowaSekwencja;
    }
}


// Struktura reprezentuj¹ca wierzcho³ek w grafie
struct Wierzcholek {
    int idSek;   // 1-5 ktora sekwencja
    int pozycja;   // ID nukleotydu - pierwszego w podciagu
    string podciag;  // o dlugosci l - ustawionej przez uzytkownika
    int indeks;
};

// zainicjowanie grafu
vector<vector<Wierzcholek>> graf;

// Funkcja tworz¹ca graf
void UtworzGraf() {

     //Iteracja po sekwencjach
    int indeksGrafu = 0;
    int indeksWierzcholka = 0;
    for (auto& sekwencjaTemp : sekwencje) {

        // Iteracja po mo¿liwych podci¹gach
        for (int j = 0; j < sekwencjaTemp.sekwencja.length() - l + 1; ++j) {  // dla danej sekwencji w sekwencjach iterujemy po nukleotydach w stringu
            string podciag = sekwencjaTemp.sekwencja.substr(j, l);   // ustawiamy podciag

            Wierzcholek wierzcholekTemp = { sekwencjaTemp.idSek, sekwencjaTemp.idNukleotydu[j], podciag, indeksWierzcholka++};
            //graf[j].resize(3);
            graf.push_back({ wierzcholekTemp });   // kazdy wierzcholek dodaje do grafu 
            indeksGrafu++;
        }
    }

    // dodanie nastepnikow jesli zgadzaja sie warunki
    // dodaje tylko z roznych sekwencji i pozycja podciagow musi sie zgadzac

    int temp = 0;
    for (const auto& ZbiorWierzcholkow : graf) {    // przechodze po kazdym 
        for (const auto& ZbiorWierzcholkowTemp : graf) {    // przechodze po kazdym 
            if ( ZbiorWierzcholkow[0].podciag == ZbiorWierzcholkowTemp[0].podciag 
                && ZbiorWierzcholkow[0].idSek != ZbiorWierzcholkowTemp[0].idSek
                && abs(ZbiorWierzcholkow[0].pozycja - ZbiorWierzcholkowTemp[0].pozycja) <= 10 * l ) {
                Wierzcholek sparowanyWierzcholek = ZbiorWierzcholkowTemp[0];
                graf[temp].push_back(sparowanyWierzcholek);
            }
        }
        temp++;
    }

}


int Stopien(int u) {
    return graf[u].size();
}

set<int> Sasiedzi(int u, int max_size) {
    set<int> neighbors;
    for (auto& v : graf[u]) {
        if (Stopien(v.indeks) >= max_size) {
            neighbors.insert(v.indeks);
        }
    }
    return neighbors;
}

set<int> CzescWspolna(const set<int>& set1, const set<int>& set2) {
    set<int> result;
    set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(result, result.begin()));
    return result;
}

void Printclique(const set<int>& max_clique) {
    for (auto& x : max_clique) {
        std::cout << "idSek: " << graf[x][0].idSek << ", pozycja: " << graf[x][0].pozycja << ", podciag: " << graf[x][0].podciag << '\n';
    }
}


void CliqueHeuristic(set<int>& U, set<int>& currentClique, int size, int& max_size, set<int>& max_clique) {
    if (U.empty()) {
        if (size > max_size) {
            max_size = size;
            max_clique = currentClique;
        }
        return;
    }
    
    int maxv = 0;
    int maxd = 0;
    for (auto& x : U)  // szuka w zbiorze U wierzcholek o max stopniu
    {
        if (Stopien(x) > maxd) {
            maxv = x;
            maxd = Stopien(x);
        }
    }
    int u = maxv;
    U.erase(u);  // bierze nastepnych sasiadow, bez bierzacego wierzcholka
    set<int> Nprim = Sasiedzi(u, max_size);
    U = CzescWspolna(U, Nprim);  // wierzcholki u obu, sprawdza czesc wspolna sasiadow u obu 
    currentClique.insert(u);
    CliqueHeuristic(U, currentClique, size + 1, max_size, max_clique);
}


void MaximalCliqueHeuristic() {
    int max_size = 0;
    set<int> max_clique;

    for (const auto& ZbiorWierzcholkow : graf) {    // przechodze po kazdym 
        for (const auto& wierzcholek : ZbiorWierzcholkow) {

            if (Stopien(wierzcholek.indeks) >= max_size) {
                set<int> U;
                for (const auto& v : graf[wierzcholek.indeks]) {  // przechodzi po nastepnikach
                    if (wierzcholek.indeks == v.indeks) {  // w moim wektorze nie ma tylko nastepnikow, jest tez 1 wierzcholek musi go zignorowac zeby przejsc po samych nastepnikach
                        continue;
                    }
                    if (Stopien(v.indeks) >= max_size) {
                        U.insert(v.indeks);
                    }
                }
                set<int> currentClique;
                currentClique.insert(wierzcholek.indeks);
                CliqueHeuristic(U, currentClique, 1, max_size, max_clique);
            }
        }
    }

    // Wyœwietlanie znalezionej maksymalnej kliki
    Printclique(max_clique);
}



int main() {
    OdczytZPlikuFasta("instancja1.fasta", sekwencje);
    OdczytZPlikuQual("instancja1.qual", sekwencje);

    Uzytkownik();
    UsunPonizejWiarygodnosci();
    // Wyœwietlenie wczytanych sekwencji
    for (const auto& sekwencja : sekwencje) {
        cout << "idSek: " << sekwencja.idSek << endl;
        cout << "Sekwencja: " << sekwencja.sekwencja << endl;

         //Wyœwietlenie zawartoœci mapy ID dla ka¿dej sekwencji
        cout << "Zawartosc mapy ID:" << endl;
        for (int i = 0; i < sekwencja.idNukleotydu.size(); ++i) {
            cout << "ID nukleotydu: " << sekwencja.idNukleotydu[i] << ", Wiarygodnosc: " << sekwencja.wiarygodnosc[i] << endl;
        }
    }

    int dlugoscPodciagu = l;

    // Wywo³anie funkcji tworz¹cej graf
    UtworzGraf();

    // Wypisanie grafu
    cout << "Lista nastepnikow: \n";
    for (const auto& ZbiorWierzcholkow : graf) {    // przechodze po kazdym 
        for (const auto& wierzcholek : ZbiorWierzcholkow) {
            cout << wierzcholek.indeks << " (" << wierzcholek.idSek << ", " << wierzcholek.pozycja << ", " << wierzcholek.podciag << ") ";
        }
        cout << endl;
    }

    MaximalCliqueHeuristic();
    return 0;
}