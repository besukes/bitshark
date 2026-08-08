<div align="center">
  <img src="assets/logo/bitshark.png"
     width = 25% alt = "Bitshark">

  <h3>BITSHARK</h3>
   An average chess engine.
  <br>
</div>
  
## Overview

**Bitshark** é uma pequena chess engine feita em C para o jogo clássico de xadrez, que depois vai ser modificada para ser um BOT no modo história do jogo CChess, feito pelo mesmo criador desta engine.
<br>
Este projeto foi iniciado no dia **18 de julho de 2026** por um estudante de Engenharia Informática na UMinho.

## Tools

* Linguagem: **C**

## Features

* Search
  * NegaMax search
  * Alpha-Beta pruning
  * Quiescence Search
  * Move ordering
* Evaluation
  * Piece Square Tables
  * Basic mobility score
  * Incremental evaluation
* Transposition table (Currently implementing)


## Estrutura do Projeto
```bash
.
├── elib/          # Headers (.h) deste motor
├── esrc/          # Ficheiros fonte do jogo 
└── README.md      # Informações detalhadas sobre o projeto
```


## Compilação

Para compilar este projeto e jogar contra o bot (que joga sempre com as pretas) , basta fazer :

```
make
```

E será gerado um executável chamado **bshark** e , executando o seguinte comando :

```
./bshark
```

Poderá jogar contra esta chess engine.


## Créditos
Esta engine foi criada por Alberto Silva.

```⢰⠢⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠈⢧⠨⠢⡀⠀⠀⠀⠀⠀⠀⠲⢒⠒⠦⠤⣀⡀⠀⠀⠀⠀⢀⣠⣤⠴⠒⠋⠙⡄
⠀⠸⠀⠐⢱⠀⠀⠀⠀⠀⠀⢀⣈⠦⠤⠀⠒⠊⠉⠉⠀⠈⠁⣀⠭⠀⢁⣀⣀⣹
⠀⠀⠖⠀⠃⠓⢤⠖⠒⠚⠉⠀⠀⡀⠀⠀⠀⢀⣰⢴⠶⠐⢈⣴⣿⠟⠿⢻⣿⡞
⠀⢰⠀⠀⠀⡤⢤⣄⠀⠀⠤⠤⠀⠀⣐⠄⠀⠼⡏⡧⠄⢢⡾⠋⠀⠀⢠⣎⡟⠀
⢠⠃⣀⠴⠋⠀⠀⠈⠑⠢⢄⣀⣠⠊⠀⢀⠎⠀⠡⠅⠀⠻⢿⣤⣤⡴⣾⣴⣿⠀
⡬⠚⠁⠀⠀⠀⠀⠀⠀⠀⠀⡼⠃⣠⠶⣇⣀⣀⠀⠀⠀⠀⠀⣀⣌⡠⠤⠘⠋⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣔⡡⠚⠁⠀⠀⠀⠈⠉⠉⠉⠻⡍⠙⡄⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠑⠃
```
