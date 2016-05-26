#ifndef OSOBA_HEADER
#define OSOBA_HEADER

#include "asn.hpp"

enum interests {NARTY=1, LYZWY=2, KSIAZKI=4, PLYWANIE=8};

class Osoba: public ASN_SEQUENCE
{
    public:
        ASN_UTF8STRING imie;
        ASN_UTF8STRING nazwisko;
        ASN_INTEGER wiek;
        ASN_ENUMERATED plec;
        ASN_INTEGER zainteresowania;
        
        Osoba()
        {
            plec = {"kobieta", "mezczyzna"};
            object_ptrs = {&imie, &nazwisko, &wiek, &plec, &zainteresowania};
        }
        
        Osoba(std::string imie1, std::string nazwisko1, int wiek1, bool plec1, int zainteresowania1): imie(imie1), nazwisko(nazwisko1), wiek(wiek1), zainteresowania(zainteresowania1)
        {
            plec = {"kobieta", "mezczyzna"};
            plec = plec1;
            object_ptrs = {&imie, &nazwisko, &wiek, &plec, &zainteresowania};
        }
        
};
#endif
