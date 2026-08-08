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
├── assets/        # Assets maioritariamente em png do programa (Logo do bitshark inclusive)
├── build/         # Ficheiros binários (.o)
├── engine/        # Ficheiros do motor de xadrez em si 
├── gui/           # Ficheiros fonte da interface gráfica do programa
├── sfx/           # Sons do jogo
└── README.md      # Informações detalhadas sobre o projeto
```


## Compilação

Para compilar este projeto e jogar contra o bot (que joga sempre com as pretas) , basta fazer :

```
make
```

E será gerado um executável chamado **bshark** . <br>
Executando o seguinte comando , poderá assim jogar contra esta chess engine:

```
./bshark
```


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
