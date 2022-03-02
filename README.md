#### Buzera Tiberiu 333CA

<h2 align = "center">ALGORITMI PARALELI SI DISTRIBUITI</h2>
<h2 align = "center">Tema #1 Paralelizarea unui algoritm genetic</h2>


# Cerinta
Pornind de la implementarea secventială, am scris un program paralel care rulează
un algoritm genetic pentru a rezolva problema rucsacului. Programul a fost scris
ı̂n C/C++ si a fost paralelizat utilizând PThreads. 

Implementarea temei trebuie să conducă la aceeasi solutie ca implementarea
secventială si să scaleze cu numărul de fire de executie.

# Folosire

- pentru a rula un test anume
```
./tema1_par /inputs/inNrTest nrGeneratii nrThreaduri
```
- pentru a rula scriptul care compara versiunea secventiala cu cea paralelizata 
- utilizatorul trebuie sa se afle in acelasi folder in care se afla si skel si sol
```
./test.sh
```

# Functia de pornire a threadurilor
Am pornit de la scheletul de cod pentru inplementarea secventiala, la care am
adaugat numarul de coruri la rularea programului.
Am adaugat in fisierul genetic_algorithm.c o functie numita paralel_solved in 
care creez vectorii de generatii, initializez bariera si pornesc si opresc 
threadurile si eliberez memoria folosita la genratii. 
De asemenea, in functia aceasta se creaza si o structura de tipul
thread_parameters in care se vor afla informatii pentru fiecare thread, cum 
ar fi un pointer la vectorii de generatii si doua variabile start si end, pentru
a stii fiecare thread cate elemente sa prelucreze.

# Paralelizari
Fiecare thread care incepe apeleaza functia *run_genetic_algorithm care primeste
ca argument o strucuta de tipul thread_parameters. In aceasta functie primul
lucru paralelizat este initializarea generatiilor care in loc sa porneasca de la
0 la numarul de obiecte, porneste de la start la end pe fiecare thread. Aceasta
modificare a fost facuta si pentru functia compute_fitness_function si pentru 
forul care updateaza indesii generatiilor. Dupa fiecare din acestea am pus bariere
pentru a se astepta toate threadurile, intru cat este nevoie de vectorii de
generatii updatati.

O a doua paralelizare pe care am facut-o a fost la cele 4 foruri care calculeaza
primii cei mai buni 30%, mutatiile la urmatoarele 40% si crossover pentru 
ultimii 30%, pentru acestea am caluclat din nou un start si un end folosind
formulele din laborator dar in loc sa folosesc numarul de obiecte am folsit
30% respectiv 20 din acestea si am parcurs forurile de la noul start la noul
end. Pentru cel de-al 4 lea for am adaugat un if in care verifica daca ne aflam
la ultimul thread la ultimul element din vector si continuam fara sa mai facem 
crossover. Am adaugat o bariera doar la final deoarece variabilele necesare
pentru fiecare for nu se intersecteaza.

Ultima paralelizare a fost pentru quicksort in care am facut quicksort pentru
fiecare thread, astfel avand numarul de threaduri de portiuni ordonate
descrescator. Dupa aceasta am pus o bariera pentru a se astepta threadurile
pentru a intra in mergesort. Pentru merge sort am partcurs un for de nr de coruri
-1 pasi in care calculam 2 variabile start si end la fiecare pas care desemneaza
de unde incep si de termina vectorii la care trebuie dat merge.
Se da merge in metoda traditionala cu 3 while-uri si dupa copiez toate elementele
ordonate inapoi in vectorul de generatii.
La final mai adaug o bariera pentru a nu trece celelalte threaduri mai departe
in timp ce primul inca face mergesort.

# Dificultati
Dificultatile cele mai mari ale temei au fost gasirea tuturor portiunilor de cod
care trebuiau paralelizate si a celor ce trebuiau sa ramana doar pentru un singur
core si paralelizarea quicksortului si folosirea mergesorului impreuna cu acesta.