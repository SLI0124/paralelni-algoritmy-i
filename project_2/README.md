# Bodovaný domácí úkol č. 2

## Zadání úlohy

Cílem domácího úkolu je implementace paralelního řešení shlukovacího algoritmu Affinity Propagation.

## Popis problému

Affinity Propagation (AP) je algoritmus pro shlukování, který patří do kategorie shlukovacích metod
založených na reprezentantech. Na rozdíl od některých shlukovacích algoritmů, které vyžadují předem stanovený počet
shluků, AP identifikuje středy shluků, nazývané reprezentanti, a přiřazuje data do takto definovaných shluků iterativně.

Obrázek 1: příklad AP

![Obrázek 1](https://homel.vsb.cz/~kro080/PAI-2024/U2/AP.png)

## Princip AP lze popsat v několika krocích:

1. AP vychází z matice podobnosti, *S*, která reprezentuje podobnost mezi dvojicemi bodů $x_i, x_j$. Podobnost lze
   vypočítat jako zápornou hodnoty čtverce vzdálenosti,$S(i,j) = -||x_i - x_j||^2$. Jinými slovy, pro
   d-dimenzionální body je výpočet: $S(i,j) = - \sum_{k=1}^{d}{(x_i[k] - x_j[k])}^2$. Zvláštní roli mají hodnoty
   na diagonále *S*. Ty nastavíme na medián hodnot podobností (zdůvodnění najdete
   např. [zde](https://www.geeksforgeeks.org/affinity-propagation-in-ml-to-find-the-number-of-clusters/)).

2. Vlastní AP využívá dvě matice: matici odpovědnosti ***R*** a matici dostupnosti ***A***.
    - Odpovědnost (***R***): $R(i,k)$ reprezentuje "odpovědnost" (vhodnost) bodu ***i*** být reprezentantem pro
      bod ***k***, vzhledem k ostatním potenciálním reprezentantům ***k***.
    - Dostupnost (***A***): $A(i,k)$ reprezentuje "dostupnost" bodu ***k*** vybrat bod ***i*** jako reprezentanta.
      Ukazuje, jak moc bod ***k*** upřednostňuje bod ***i*** jako reprezentanta.

   Obě matice jsou na začátku inicializovány samými nulami.

3. Algoritmus iterativně aktualizuje matice odpovědnosti a dostupnosti na základě následujících pravidel:
    - $R(i,k) = S(i,k) - \max_{k' \neq k} \{ A(i,k') + S(i,k') \}$,
    - $A(i,k) = \min \{ 0, R(k,k) + \sum_{i' \neq i,k} \max \{ 0, R(i',k) \} \}$,
    - $A(k,k) = \sum_{i' \neq k} \max \{ 0, R(i',k) \}$.

4. Reprezentanti a přiřazení do shluků je určeno na základě hodnot v maticích odpovědnosti a dostupnosti. Body s
   vysokými hodnotami v obou maticích mají pravděpodobnost stát se reprezentanty. Uvažujme kriteriální
   matici, $C = R + A$. V takové matici je reprezentantem každého řádku zvolen ten bod, v jehož sloupci je největší
   hodnota.

   Například v této matici ***C***:

   ![matice](https://homel.vsb.cz/~sli0124/pa1/matrix.png)

   je bod $x_1$ (první řádek) reprezentován sám sebou ($x_1$), protože nejvyšší hodnota na prvním řádku je v
   prvním sloupci. Bod $x_2$ je také reprezentován $x_1$ (nejvyšší hodnota na druhém řádku je opět v prvním
   sloupci), stejně jako $x_3$. Podle stejné logiky je reprezentantem $x_4$ $x_4$ a
   reprezentantem $x_5$ zase $x_4$. V datech jsou tedy
   dva shluky, $\{x_1, x_2, x_3\}$ a $\{x_4, x_5\}$.

5. Body 3 - 4 opakujeme dokud se shluky nestabilizují nebo po předem určený počet iterací.

Na základě popisu Affinity Propagation můžeme zformulovat následující zadání druhého úkolu.

## Zadání

Implementujte paralelní řešení Affinity Propagation (na
datasetu [MNIST](https://homel.vsb.cz/~kro080/PAI-2024/U2/archive.zip)). Popis datasetu najdete
např. [zde](https://paperswithcode.com/dataset/mnist) (
original) nebo [zde](https://www.kaggle.com/datasets/oddrationale/mnist-in-csv) (jednodušší formát). Cílem úkolu není
naučit se shlukovat MNIST podle skutečných tříd, ale testovat
paralelní implementaci. Label třídy tedy pro potřeby shlukování můžeme ignorovat. Lze na něm ověřit, kolik algoritmus
najde shluků a jak správně objekty přiřadí.

## Doporučení

Při řešení vycházejte z toho, co jsme si ukázali na cvičeních na příkladu K-Means. O problému se dá uvažovat jako o
shlukování se a) automatickým určením počtu shluků a b) bez nutnosti opakovaně počítat vzdálenosti mezi body a
centroidy. Jako v případě předchozí úlohy je problém analogický, ale ne totožný.

## Max. bodové hodnocení

33

## Reference

1. Brendan J. Frey; Delbert Dueck (2007). "Clustering by passing messages between data points". Science. 315 (5814):
   972–976. Bibcode:2007Sci...315..972F. CiteSeerX 10.1.1.121.3145. doi:10.1126/science.1136800. PMID 17218491. S2CID
   6502291
2. Thavikulwat, Precha. “Affinity Propagation: A Clustering Algorithm for Computer-Assisted Business Simulations and
   Experiential Exercises.” Developments in Business Simulation and Experiential Learning 35 (2014): n. pag.
3. https://www.geeksforgeeks.org/affinity-propagation-in-ml-to-find-the-number-of-clusters/
