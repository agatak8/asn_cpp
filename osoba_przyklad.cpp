/** \file
 * Name: osoba_przyklad.cpp
 * Purpose: wyświetlenie 3 przykładowych osób
 * @author: Agata Kłoss
*/ 

#include "osoba.hpp"

int main()
{
    Osoba a("Hubert", "Pólkowski", 71, 1, KSIAZKI | PLYWANIE), 
    b("Emre", "Shively-Ertas", 19, 1, NARTY | LYZWY | KSIAZKI), 
    c("Elizabeth", "Reedy", 18, 0, NARTY | PLYWANIE);
    
    a.display();
    std::cout << std::endl;
    b.display();
    std::cout << std::endl;
    c.display();
    
}
