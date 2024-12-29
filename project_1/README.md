# Bodovaný úkol číslo 1

## Zadání

Cílem domácího úkolu je implementace paralelního řešení problému rozložení zařízení (Single-Row Facility Layout Problem,
SRFLP).

## Popis problému

SRFLP spočívá v hledání lineárního uspořádání sady zařízení, které bude minimalizovat (vážený)
součet vzdáleností mezi každými dvěma zařízeními. Jako ilustraci si můžeme představit robotické rameno, které odebírá
výrobky ze čtyř míst (např. pásů), jak je ukázáno na obr. 1. Každý pás ale dodává výrobky s různou intenzitou. Pokud by
byly pásy na kterých se výrobky objevují často umístěny daleko od sebe (na obrázku např. m1, m4), bude využití robota
neefektivní (bude se často přesunovat po dlouhé dráze mezi m1 a m4). Navíc šířka pásů může být různá.

![Obrázek 1](https://homel.vsb.cz/~kro080/PAI-2024/U1/srflp-2.png)

Matematicky lze základní verzi SRFLP formulovat následujícím způsobem: mějme množinu zařízení,
$$F = {1, 2, ..., n}$$ se známou šířkou, $$L = {l_1, l_2, ..., l_n}.$$ Dále mějme
matici $$C = (c_{ij}) \in R^{n \times n}$$ s váhami přechodů mezi zařízením  ***i*** a  ***j***. Tu si můžeme
představit jako odhad pravděpodobnosti, že rameno bude z pozice ***i*** budepředcházet pozici ***j***.

Cílem řešení SRFLP je najít lineární uspořádání (=permutaci) zařízení $$\pi = (\pi_1, \pi_2, ..., \pi_n),$$ která
bude minimalizovat následující cenovou funkci:

$$ \min_{\underset{\pi \in S_n}{}} f_{SRFLP}(\pi),$$

$$ f_{SRFLP} (\pi) = \sum_{1 \leq i < j \leq n} [c_{\pi_{i}\pi_i \cdot d(\pi_{i}, \pi_{j})}], $$

$$ d(\pi_{i}, \pi_{j}) = \frac{l_{\pi_{i}} + l_{\pi_{j}}}{2} + \sum_{i \leq k \leq\ j} l_{\pi_k}. $$

Konkrétní problém, který chceme řešit (instanci SRFLP) lze popsat např. takto:

1. 10
2. 1 1 1 1 1 1 1 1 1 1
3. 0 30 17 11 24 25 24 17 16 22
4. 0 0 21 23 26 24 27 19 11 32
5. 0 0 0 24 18 23 31 36 28 19
6. 0 0 0 0 19 18 33 25 20 28
7. 0 0 0 0 0 15 37 27 17 16
8. 0 0 0 0 0 0 27 23 29 24
9. 0 0 0 0 0 0 0 27 31 24
10. 0 0 0 0 0 0 0 0 14 18
11. 0 0 0 0 0 0 0 0 0 24
12. 0 0 0 0 0 0 0 0 0 0

Význam řádků na obr. 2 je následující: řádek (1) obsahuje dimenzi problému, t.j. počet zařízení (***n***). Řádek (2)
obsahuje šířky zařízení, $$l_1, l_2, ..., l_n.$$

V případě této instance mají všechna zařízení shodnou šířku 1. řádky (3) - (12) pak obsahují matici  ***C***, tj. váhy
přechodů mezi zařízeními, $$c_{ij}.$$ Dolní část matice je naplněna nulami, vzhledem k tomu, že je problém
symetrický, $$c_{ij} = c_{ji},$$ představme si tam stejné hodnoty jako nad hlavní diagonálou. Na základě toho
všecho tedy můžeme
zformulovat následující zadání prvního úkolu.

Pozn. Řešení (jedno z možných) je [0 4 1 9 6 3 7 2 5 8].

## Zadání

Vypracujte paralelní řešení SRFLP (instance [Y-10_t.txt](https://homel.vsb.cz/~kro080/PAI-2024/U1/Y-10_t.txt)) pomocí
metody Branch and Bound se sdílením nejlepšího dosud nalezeného řešení.

## Doporučení

Při řešení vycházejte z toho, co jsme si ukázali na cvičeních na příkladu TSP. Problém je analogický (ale ne totožný).

## Maximální bodové hodnocení

33

## Reference

1. Kothari, R., Ghosh, D. The single row facility layout problem: state of the art. OPSEARCH 49, 442–462 (
   2012). https://doi.org/10.1007/s12597-012-0091-4
