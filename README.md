# Paralelní algoritmy I

Cílem předmětu je získat přehled v oblasti návrhu, realizace a hodnocení paralelních algoritmů. Praktické osvojení
paralelních programovacích technik pro vybrané paralelní architektury. Pracovní znalosti v oblasti paralelních systémů a
jejich programování, zejména: samostatný návrh paralelních algoritmů, resp. paralelizace sekvenčních algoritmů.
Praktická realizace paralelního algoritmu na bázi modelu předávání zpráv. Analýza algoritmu a vyhodnocení implementace.
Optimalizace a zvyšování efektivity.

## Projekty

Zadání projektů je k dispozici v jednotlivých složkách projektů a jejich popis je vždy uveden v souboru `README.md`.

1. [Single-Row Facility Layout Problem](project_1/README.md)
2. [Affinity Propagation Clustering](project_2/README.md)
3. [Parallel Page Rank](project_3/README.md)

## Spuštění

Predispozice pro sestavení a spuštění projektů jsou následující:

- C++ kompilátor (např. GCC, Clang, MSVC) verze podporující standard C++17 a vyšší
- CMake verze 3.27 a vyšší
- OpenMP verzi 3.0. a vyšší (měl by být součástí většiny linuxových distribucí, Microsoft Visual Studio 2022 podporuje
  pouze verzi 2.0)

Každý projekt je samostatným CMake projektem, který lze sestavit a spustit následujícím způsobem:

### Sestavení

  ```shell
  mkdir build && cd build
  cmake ..
  cmake --build .
  ```

### Spuštění

#### Single-Row Facility Layout Problem

  ```shell
  ./srflp
  ```

#### Affinity Propagation Clustering

  ```shell
  ./affinity_propagation
  ```

#### Page Rank

  ```shell
  ./page_rank
  ```
