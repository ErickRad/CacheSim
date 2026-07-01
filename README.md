# CacheSim

## Visão Geral

Simulador parametrizável de hierarquia de cache, implementado em **C++17**.  
Suporta caches diretamente mapeadas, set-associativas e totalmente associativas,  
com políticas de substituição **Random**, **FIFO** e **LRU**.

---

## Estrutura do Projeto

```
CacheSim/
└── cache.h              # Estruturas, enums e declaração da classe
└── cache.cpp            # Implementação do simulador
└── README.md            # Esta documentação
└── Makefile             # Compilação
```

---

## Compilação

### Pré-requisitos
- **g++** com suporte a C++17 (versão ≥ 7)
- **make**

### Como compilar

```bash
make
```

O executável `simulate` será gerado na raiz do projeto.

Para limpar os artefatos de compilação:

```bash
make clean
```

---

## Uso

```
./simulate <nsets> <bsize> <assoc> <subst> <flag_saida> <arquivo>
```

| Parâmetro    | Descrição                                        |
|-------------|--------------------------------------------------|
| `nsets`     | Número de conjuntos (potência de 2)              |
| `bsize`     | Tamanho do bloco em bytes (potência de 2)        |
| `assoc`     | Grau de associatividade (≥ 1)                    |
| `subst`     | Política de substituição: `R`, `F` ou `L`        |
| `flag_saida`| `0` = saída verbose &nbsp; / &nbsp; `1` = numérica |
| `arquivo`   | Arquivo binário de endereços (big-endian, 32 bits cada) |

### Políticas de substituição

| Código | Política |
|--------|----------|
| `R`    | Random — vítima escolhida aleatoriamente |
| `F`    | FIFO — primeiro a entrar, primeiro a sair |
| `L`    | LRU — menos recentemente usado           |

---

## Exemplos

### Exemplo do enunciado (resultado esperado)
```bash
./simulate 256 4 1 R 1 tests/bin_100.bin
# 100 0.9200 0.0800 1.0000 0.0000 0.0000
```

### Cache 4-way com LRU, saída numérica
```bash
./simulate 64 16 4 L 1 tests/bin_1000.bin
```

### Cache totalmente associativa, saída verbose
```bash
./simulate 1 32 8 F 0 tests/bin_1000.bin
```

---

## Formato de Saída

### `flag_saida = 0` — Verbose

```
==============================
 RELATÓRIO DO SIMULADOR DE CACHE
==============================
Configuração:
  Conjuntos (nsets)   : 256
  Tamanho bloco (B)   : 4 bytes
  Associatividade     : 1-way
  Política subst.     : Random
  Capacidade total    : 1024 bytes
------------------------------
Resultados:
  Total de acessos    : 100
  Hits                : 92
  Misses              : 8
  Misses compulsórios : 8
  Misses de capacidade: 0
  Misses de conflito  : 0
------------------------------
Taxas:
  Taxa de hit         : 92 %
  Taxa de miss        : 8 %
  Taxa miss compuls.  : 100 %
  Taxa miss capac.    : 0 %
  Taxa miss conflito  : 0 %
==============================
```

### `flag_saida = 1` — Padrão numérico

```
100 0.9200 0.0800 1.0000 0.0000 0.0000
```

Ordem: `total_acessos  hit_rate  miss_rate  comp_rate  cap_rate  conf_rate`

---

## Arquivo de Entrada

Formato binário, endereços de **32 bits** em **big-endian**, sem cabeçalho.

Cada endereço ocupa exatamente 4 bytes. O arquivo com N endereços tem `N × 4` bytes.

---

## Detalhes da Implementação

### Decomposição do endereço

```
 31         tag        (offset_bits+index_bits)  index  offset_bits  0
 ┌──────────────────────────────┬────────────────┬──────────────────┐
 │            TAG               │     ÍNDICE     │     OFFSET       │
 └──────────────────────────────┴────────────────┴──────────────────┘
```

- **offset** = `log2(bsize)` bits
- **índice** = `log2(nsets)` bits  
- **tag**    = `32 - offset - índice` bits

### Classificação de misses

A classificação usa a técnica das **duas caches**:

| Cache Real | Cache FA Ideal | Classificação |
|-----------|---------------|---------------|
| miss      | miss (1ª vez) | Compulsório   |
| miss      | miss (bloco já visto, FA cheia) | Capacidade |
| miss      | hit           | Conflito      |

A **cache FA ideal** tem a mesma capacidade total (`nsets × assoc` blocos), com 1 único conjunto, usando LRU como política.

### Consistência de bytes (endianness)

Os endereços no arquivo são lidos em **big-endian** e convertidos para a ordem nativa do host (tipicamente little-endian em x86) antes do processamento.

---

## Requisitos do Sistema

- Sistema operacional: Linux / macOS / Windows (WSL)
- Compilador: `g++ ≥ 7` com suporte a `-std=c++17`
