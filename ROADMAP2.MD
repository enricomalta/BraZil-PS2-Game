ROADMAP 2 — SURVIVAL GAMEPLAY
Objetivo
Transformar o atual shooter com zumbis em um survival shooter, introduzindo:
- inventário
- itens no mundo
- loot
- fome
- sede
- stamina
- cura
- armas e munições diferentes
- equipamentos
- interação com objetos
- furtividade baseada em som
- sobrevivência
A prioridade continua sendo PS2: sistemas simples, pouca alocação dinâmica e estruturas previsíveis.
FASE 6 — SISTEMA DE INVENTÁRIO
Essa seria a próxima implementação.

Criar um inventário baseado em slots.

Exemplo:
INVENTÁRIO

[ PISTOLA ]
[ MUNIÇÃO 9mm x12 ]
[ BANDAGEM x2 ]
[ COMIDA x1 ]
[ ÁGUA x1 ]
[ ITEM ]
[ ITEM ]

Cada item deverá possuir
ID
Nome
Tipo
Quantidade
Peso
Stack máximo

Tipos inicialmente:
WEAPON
AMMO
FOOD
DRINK
MEDICAL
TOOL
MISC

Não precisamos começar com um inventário visual complexo.

Primeiro fazer a lógica funcionar.
Critério de conclusão
O jogador consegue:
pegar item
↓
item entra no inventário
↓
quantidade aumenta
↓
item pode ser utilizado
↓
quantidade diminui

FASE 7 — LOOT NO MAPA
Agora começamos a transformar o mapa que você já possui em um ambiente de sobrevivência.

Criar objetos interativos:
Caixa
Armário
Mesa
Corpo
Mochila
Carro

O jogador aproxima:
[INTERAGIR]

e recebe itens.

Exemplo:
ARMÁRIO

Munição 9mm x8
Bandagem x1
Água x1

Importante
Não faça loot infinito.

Cada container deve possuir seu próprio inventário.

Isso cria a sensação de exploração.
FASE 8 — FOME E SEDE
Agora entra uma das principais mecânicas de survival.

Criar:
Hunger
Thirst

Por exemplo:
FOME
100 ━━━━━━━━━━

SEDE
100 ━━━━━━━━━━

Os valores diminuem lentamente durante o jogo.

Quando chegarem a determinados níveis:
100–50 = normal

49–25 = efeitos leves

24–10 = efeitos fortes

9–0 = estado crítico

Inicialmente não precisamos matar o jogador imediatamente.

Podemos fazer:
fome crítica
↓
stamina reduzida

sede crítica
↓
stamina reduzida

ambos críticos
↓
perda gradual de vida

Isso cria decisões de sobrevivência.
FASE 9 — STAMINA
Essa é uma mecânica muito importante.

Criar:
Stamina = 100

Corrida:
stamina -= consumo

Parado:
stamina += recuperação

Se chegar a zero:
não consegue correr

E combinar com fome e sede:
fome baixa
↓
recuperação de stamina menor

sede baixa
↓
consumo de stamina maior

Isso começa a criar uma dinâmica interessante.
FASE 10 — SISTEMA MÉDICO
Depois:
Bandagem
Kit médico
Comida
Água

Começamos simples.
Bandagem
Pode recuperar uma quantidade de vida.

Mas podemos evoluir posteriormente para:
sangramento
↓
bandagem
↓
sangramento interrompido

Segunda etapa
Adicionar:
Bleeding

Quando o jogador toma determinados tipos de dano:
Zumbi
Tiro
Queda

pode começar a sangrar.

HUD:
HP       72
SANGRANDO

E a vida vai diminuindo até tratar.

Essa mecânica combina muito bem com o estilo survival.
FASE 11 — SISTEMA DE ARMAS
Agora podemos sair da “arma única”.

Criar uma classe base:
Weapon

e diferentes configurações:
Pistol
Shotgun
Rifle
SMG

Cada arma:
dano
alcance
cadência
munição
carregador
tempo de recarga
ruído
precisão

Exemplo:
PISTOLA

Dano: 35
Magazine: 12
Alcance: 30m
Ruído: médio

SHOTGUN

Dano: alto
Magazine: 6
Alcance: curto
Ruído: muito alto

Isso também alimentará a IA.
FASE 12 — SOM COMO MECÂNICA
Aqui eu faria algo muito importante para o nosso jogo.

Você já tem os sons.

Agora vamos fazer o som possuir gameplay.

Criar um sistema:
NoiseEvent

Exemplo:
Tiro
↓
NoiseEvent
posição = jogador
intensidade = 100
raio = 30m

Zumbis:
recebem NoiseEvent
↓
verificam distância
↓
investigam

Depois:
corrida

produz:
ruído = 20

Andar:
ruído = 5

Agachar:
ruído = 1

Tiro:
ruído = 100

Isso cria uma mecânica de furtividade sem precisar de um sistema gigantesco.
FASE 13 — DETECÇÃO VISUAL
Depois do som, fazemos os zumbis enxergarem.

Criar:
campo de visão
distância máxima
linha de visão

Exemplo:
          ZUMBI
            ↓
       \    |    /
        \   |   /
         \  |  /
          PLAYER

Se existir uma parede:
ZUMBI → parede → PLAYER

não detecta

Se o jogador estiver dentro do campo de visão:
detecta
↓
Chase

Isso combinado com ruído cria uma IA muito mais interessante.
FASE 14 — LOOT ALEATÓRIO
Depois podemos fazer os containers terem tabelas de loot.

Exemplo:
Casa comum

60% comida
40% água
25% bandagem
20% munição
10% arma

Mas o sistema deve utilizar uma seed.

Isso é interessante para o PS2 porque podemos reproduzir a mesma geração quando necessário.
FASE 15 — EQUIPAMENTOS
Depois:
Mochila
Colete
Capacete

A mochila aumenta:
slots

O colete pode reduzir:
dano recebido

O capacete pode reduzir:
dano de tiro na cabeça

Mas eu deixaria isso para depois do inventário estar funcionando.
FASE 16 — INTERAÇÃO COM O MUNDO
Adicionar:
abrir porta
fechar porta
pegar item
abrir caixa
abrir carro
pegar arma
trocar arma

Isso faz o mapa começar a parecer um mundo jogável, e não apenas uma arena.
FASE 17 — ZUMBIS MAIS COMPLETOS
Agora podemos evoluir a IA atual.

Estados:
IDLE
PATROL
INVESTIGATE
SEARCH
CHASE
ATTACK
STAGGER
DEAD

Exemplo:
Jogador dispara
       ↓
zumbi ouve
       ↓
INVESTIGATE
       ↓
chega ao local
       ↓
não encontra jogador
       ↓
SEARCH
       ↓
encontra jogador
       ↓
CHASE
       ↓
ATTACK

Isso é muito mais interessante que simplesmente:
player → zombie
zombie → player

FASE 18 — PRIMEIRO LOOP DE SOBREVIVÊNCIA
Quando essas mecânicas estiverem prontas, teremos:
ENTRAR NO MAPA
      ↓
EXPLORAR
      ↓
ENCONTRAR LOOT
      ↓
GERENCIAR INVENTÁRIO
      ↓
FOME / SEDE
      ↓
ENCONTRAR ÁGUA / COMIDA
      ↓
ENCONTRAR MUNIÇÃO
      ↓
EVITAR ZUMBIS
      ↓
COMBATER QUANDO NECESSÁRIO
      ↓
CURAR FERIMENTOS
      ↓
CONTINUAR EXPLORANDO

Aí sim teremos o começo de um verdadeiro survival.
Ordem que eu recomendo para o Copilot
Não mande implementar as 13 fases de uma vez.

Faça:
6 → Inventário
↓
7 → Loot
↓
8 → Fome/Sede
↓
9 → Stamina
↓
10 → Medicina
↓
11 → Armas
↓
12 → Sistema de ruído
↓
13 → Visão dos zumbis
↓
14 → Loot procedural
↓
15 → Equipamentos
↓
16 → Interações
↓
17 → IA avançada
↓
18 → Loop completo

E uma regra que eu colocaria no prompt do Copilot:
Não avance para a próxima fase até a fase atual compilar, executar e ser testada. Não reescreva sistemas existentes que já funcionam; estenda-os de forma compatível. Toda nova mecânica deve considerar memória, CPU e limitações do PlayStation 2.

E tem uma coisa que eu mudaria no projeto agora
Antes de começar inventário, eu criaria uma pequena camada de Item + Inventory + Interactable.

Porque praticamente tudo que vamos adicionar daqui para frente vai depender disso:
             Item
              │
       ┌──────┼───────┐
       ↓      ↓       ↓
     Food    Ammo   Medical
       │
       ↓
   Inventory
       │
       ↓
   Interactable
       │
 ┌─────┼─────┐
 ↓     ↓     ↓
Caixa  Corpo  Armário

## Status da implementação atual

### Implementado no prototipo
- Fase 6: inventário fixo de 8 slots, expansível para 12 com mochila, itens empilháveis e uso/equipamento.
- Fase 7: pickups e duas caixas com três itens cada; o loot de cada caixa é finito e determinístico.
- Fases 8 e 9: fome, sede e stamina com barras na HUD; correr consome stamina e os estados críticos afetam a sobrevivência.
- Fase 10: bandagem e kit médico curam e interrompem sangramento.
- Fase 12: passos, corrida e tiros publicam eventos de ruido para os zumbis.
- Fase 13: deteccao visual considera distancia e linha de visao bloqueada por objetos.
- Fase 15: mochila amplia slots; colete e capacete reduzem dano.
- Fase 17: zumbis investigam ruido, procuram, perseguem, atacam, cambaleiam e morrem; patrulham enquanto ociosos.
- Fase 18: o loop basico de explorar, coletar, gerenciar recursos, combater e curar esta conectado.

### Ainda parcial
- Fase 11: a pistola 9 mm usa carregadores reais e munição carregada bala a bala; shotgun, rifle e SMG ainda não existem.
- Fase 14: caixas usam uma sequência determinística de loot, mas ainda não há tabelas configuráveis por tipo de local/container.
- Fase 16: coleta de itens e abertura de caixas funcionam; portas e carros ainda não são interativos.

### Controles do prototipo
- Interagir: usar o comando de interacao do objeto destacado.
- Triângulo: abrir inventário; segurado com um carregador selecionado, inserir munição bala a bala.
- Direcional para cima/baixo: selecionar slot.
- X: usar ou equipar o item selecionado.
- L1: trocar para o carregador carregado selecionado/disponivel.
- Círculo: fechar o inventário.
- A HUD do inventario mostra nomes e quantidades, sem imagens de itens.