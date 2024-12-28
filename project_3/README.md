# Bodovaný domácí úkol č. 3

## Zadání úlohy

Cílem domácího úkolu je implementace algoritmu PageRank (PR) s paralelním načítáním dat a paralelním výpočtem skóre
PageRank (data-driven přístup nebo topology-driven přístup).

## Popis problému

PageRank je algoritmus, který každému vrcholu v orientovaném grafu přiřadí skóre, které popisuje jeho "
důležitost" v ráci struktury grafu. Byl vyvinut pro organizaci výsledků vyhledávání na Webu (Google), ale lze jej použít
pro ohodnocení vrcholů v každém orientovaném grafu.

Page Rank lze definovat rekurzivně: pro každý vrchol z množiny všech vrcholů $u \in V$ platí, že jeho PageRank je
dán je dán vztahem:

$$ PR(u) = \frac{1-d}{|V|} + d \sum_{v \in N - (u)}{\frac{PR(v)}{|N + (v)|}},$$

kde je ***d*** "teleportační" konstanta (nastavme na 0.85), $N^-(u)$ je množina všech vrcholů, ze kterých vede
hrana do ***u*** a $N^+(v)$ je množina všech vrcholů, *do kterých vede hrana z* ***v***.

Jinými slovy, PR vrcholu ***u*** odpovídá součtu příspěvků PR ze všech vrcholů grafu, ze kterých vede hrana do ***u***,
příčemž každý "donor" rozděluje své PR rovnoměrně mezi všechny vrcholy, ke kterým z něj vede hrana.

Takto definovaný výpočet PR je rekurzivní. Pro první iteraci inicializujeme PR všech vrcholů na stejnou hodnotu,
odpovídající $\frac{1}{|V|}$. Poté opakujeme výpočet PR pro všechny vrcholy v grafu dokud se jeho hodnota
nestabilizuje (tj. změna PR pro jednotlivé vrcholy není menší než předem daný threshold).

Prakticky lze provést výpočet PR mnoha způsoby. Ukázali jsme si data-driven přístup (v každé iteraci přepočítáme PR pro
všechny vrcholy v grafu) a topology-driven přístup (v každé iteraci spočítáme PR jen pro ty vrcholy, u kterých došlo ke
změně PR u některého z donorů, $v \in N^-(u))$. Je totiž zřejmé, že pokud se nezměnil PR žádného donora, nebude
se měnit ani PR ***u***. Paralelizace obou přístupů je vcelku triviální.

Klíčovým aspektem výpočtu PR je vhodná reprezentace grafu a znalost $N^-(u))$ a $|N^+(u)|$ pro každý
vrchol. Graf můžeme snadno reprezentovat pomocí matice sousednosti. Tedy matice, která bude mít stejný
opočet řádků a sloupců (roven ***|V|***) a na pozici ***ij*** hodnotu 1 pokud vede z ***i*** do ***j*** hrana a 0 pokud
ne. Je jasné, že matice sousednosti bude pro velké grafy s desítkami či stovkami tisíc vrcholů řídká, je
jí proto třeba reprezentovat jako přídkou matici. Reprezentace klasickou hustou maticí by i pro relativně malé grafy
měla velkou paměťovou náročnost (bitmapa 100000x100000 zabírá cca 1.2 GB). Je tedy lepší matici sousednosti
reprezentovat jako řídkou binární matici, tj. pro každý vrchol si uložit seznam vrcholů, ke kterým z něj vede hrana.

Na základě toho můžeme zformulovat následující zadání třetího úkolu.

## Zadání

Implementujte paralelní výpočet Page Rank pro
graf [Berkely-Stanford web graph](https://snap.stanford.edu/data/web-BerkStan.txt.gz), který

1. pomocí více vláken korektně načte do paměti matici sousednosti
2. využije více vláken také pro výpočet hodnoty PR pro všechny vrcholy

## Doporučení

Při řešení vycházejte z toho, co jsme si ukázali na cvičeních na příkladu PageRanku. Při načítání dat je třeba dát si
pozor na rozdělení práce mezi vlákna (každé bude načítat jen část souboru) a současný přstup k datové
struktuře repzentující řídkou matici.

## Max. bodové hodnocení

33

## Reference

1. Page, L., Brin, S., Motwani, R. & Winograd, T. (1998). The PageRank Citation Ranking: Bringing Order to the Web ().
   Stanford Digital Library Technologies Project
2. https://medium.com/analytics-vidhya/parallel-pagerank-an-overview-of-algorithms-and-their-performance-ce3b2a2bfdb6






