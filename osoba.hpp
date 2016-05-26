/** \file
 * Name: osoba.hpp
 * Purpose: klasa Osoby
 * @author: Agata Kłoss
*/ 

#ifndef OSOBA_HEADER
#define OSOBA_HEADER

#include <iostream>
#include "asn.hpp"

/// flagi zainteresowań
enum interests {NARTY=1, LYZWY=2, KSIAZKI=4, PLYWANIE=8};

/// klasa Osoba jako przykład użycia ASN_SEQUENCE
class Osoba: public ASN_SEQUENCE, public IDisplayable
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
        
        void display()
        {
            std::cout << "Imię: " << (std::string)imie << std::endl;
            std::cout << "Nazwisko: " << (std::string)nazwisko << std::endl;
            std::cout << "Wiek: "  << wiek << std::endl;
            std::cout << "Płeć: ";
            if (plec == 0)
            {
                std::cout << "kobieta";
            }
            else
            {
                std::cout << "mężczyzna";
            }
            std::cout << std::endl;
            
            std::cout << "Zainteresowania: ";
            if (zainteresowania & NARTY)
            {
                std::cout << "narty, ";
            }
            if (zainteresowania & LYZWY)
            {
                std::cout << "łyżwy, ";
            }
            if (zainteresowania & KSIAZKI)
            {
                std::cout << "książki, ";
            }
            if (zainteresowania & PLYWANIE)
            {
                std::cout << "pływanie, ";
            }
            std::cout << std::endl;
            
        }
};
#endif
