# ROADMAP 3 — SURVIVAL AVANÇADO
## DEEP FOREST — Especificação técnica e funcional

**Plataforma:** PlayStation 2  
**Engine:** Tyra / TyraX  
**Linguagem principal:** C++  
**Objetivo:** Evoluir o Deep Forest para um jogo de sobrevivência 3D inspirado nas mecânicas de DayZ, com sistemas interligados, persistência de mundo e desempenho compatível com o hardware do PS2.

---

# 1. ESTADO ATUAL DO PROJETO

Antes de iniciar este roadmap, considere que o projeto já possui os seguintes sistemas funcionais:

- Movimentação do jogador.
- Câmera em terceira pessoa.
- Sistema de armas.
- Munição limitada.
- Recarga de armas.
- Efeitos sonoros de disparo, recarga e arma vazia.
- Sistema de vida.
- Sistema de sangue.
- Sangramento com redução progressiva do sangue.
- Sistema de dano recebido.
- Sistema de morte do jogador.
- Zumbis com inteligência artificial básica.
- Estados de comportamento dos zumbis.
- Detecção e perseguição do jogador.
- Ataques de curta distância.
- Colisões.
- HUD.
- Inventário.
- Loot.
- Fome.
- Sede.
- Stamina.
- Equipamentos.
- Interações básicas com objetos.
- Mapa existente.

**IMPORTANTE SOBRE O SISTEMA DE SANGUE:**

O sistema de sangue já existe. Não recriar a mecânica de sangramento nem substituir o sistema atual de vida e sangue.

As novas mecânicas devem utilizar os sistemas existentes e expandi-los somente quando necessário.

O sangue representa a quantidade de sangue disponível no organismo. A perda de sangue pode provocar consequências como fraqueza, desmaio e morte, conforme as regras que forem implementadas.

---

# 2. REGRAS GERAIS DE DESENVOLVIMENTO

Estas regras são obrigatórias para todas as fases.

## 2.1 Auditoria antes da implementação

Antes de escrever código:

1. Examine a estrutura completa do projeto.
2. Identifique o Game Loop.
3. Identifique o sistema de atualização de entidades.
4. Identifique como o jogador é representado.
5. Identifique o funcionamento do inventário.
6. Identifique o sistema de itens e equipamentos.
7. Identifique o sistema de vida, sangue, fome, sede e stamina.
8. Identifique como são carregados os modelos 3D.
9. Identifique como funcionam as colisões.
10. Identifique como os sons são carregados e reproduzidos.
11. Identifique como funciona o HUD.
12. Identifique os recursos disponíveis no Tyra/TyraX.

Não presumir que uma API existe sem verificar sua implementação.

## 2.2 Arquitetura

- Utilizar os sistemas já existentes sempre que possível.
- Criar módulos independentes para novas funcionalidades.
- Evitar classes excessivamente grandes.
- Separar lógica de gameplay, apresentação visual e armazenamento.
- Evitar dependências circulares.
- Centralizar configurações de gameplay.
- Utilizar estruturas de dados adequadas ao PS2.
- Evitar processamento redundante.
- Evitar alocações dinâmicas frequentes durante o gameplay.
- Utilizar limites máximos para entidades, objetos e eventos.
- Evitar criar sistemas paralelos que façam a mesma coisa.

## 2.3 Compatibilidade com PlayStation 2

O projeto precisa funcionar no hardware original.

Considerar:

- Memória RAM limitada.
- Capacidade de processamento do Emotion Engine.
- Capacidade gráfica da Graphics Synthesizer.
- Quantidade de polígonos por cena.
- Quantidade de entidades simultâneas.
- Quantidade de texturas carregadas.
- Consumo de memória de áudio.
- Frequência de atualização dos sistemas.
- Compatibilidade com o compilador utilizado.
- Compatibilidade com o Tyra/TyraX e PS2SDK.

Evitar implementar mecânicas que dependam de recursos inexistentes no ambiente atual.

## 2.4 Critérios de conclusão

Uma fase só poderá ser considerada concluída quando:

1. A funcionalidade estiver implementada.
2. Estiver integrada aos sistemas existentes.
3. Possuir tratamento de erros e estados inválidos.
4. Possuir feedback visual ou sonoro quando necessário.
5. Compilar sem erros.
6. Não apresentar erros críticos durante a execução.
7. Passar pelos testes funcionais.
8. Não quebrar funcionalidades anteriores.
9. Ter seu consumo de memória e processamento avaliado.
10. Ter documentação atualizada.

Não utilizar implementações fictícias, placeholders ou funcionalidades que apenas aparentem funcionar.

---

# 3. SISTEMAS DE SOBREVIVÊNCIA

# FASE 19 — TEMPERATURA CORPORAL

## Objetivo

Implementar um sistema de temperatura corporal que seja influenciado pelo clima, roupas, condições ambientais e proximidade de fontes de calor.

O sistema deve interagir diretamente com fome, sede, stamina, saúde e equipamentos.

## 19.1 Estado de temperatura

Criar um sistema responsável por controlar a temperatura corporal do jogador.

A temperatura deverá ser representada internamente por um valor normalizado de 0 a 100.

Referências iniciais:

- 90–100: temperatura confortável.
- 70–89: temperatura levemente baixa.
- 40–69: frio moderado.
- 20–39: frio intenso.
- 1–19: hipotermia crítica.
- 0: condição extrema.

Esses valores deverão ser configuráveis.

## 19.2 Fatores ambientais

A temperatura deverá ser influenciada por:

- Temperatura ambiente.
- Chuva.
- Intensidade da chuva.
- Tempo de exposição.
- Vento, caso o projeto possua suporte.
- Proximidade de fogueiras.
- Presença de abrigos.
- Horário do dia.
- Condições das roupas.

## 19.3 Sistema de roupas

Cada peça de roupa poderá possuir:

- Isolamento térmico.
- Resistência à chuva.
- Capacidade de retenção de água.
- Nível de umidade.
- Durabilidade.
- Peso.
- Tipo de equipamento.

As roupas deverão ser classificadas por categorias:

- Cabeça.
- Torso.
- Pernas.
- Pés.
- Mãos.

As peças equipadas deverão contribuir para a proteção térmica do personagem.

## 19.4 Umidade das roupas

As roupas deverão possuir umidade entre 0 e 100.

Comportamentos:

- A chuva aumenta a umidade.
- Ambientes cobertos reduzem a exposição à chuva.
- Fontes de calor podem acelerar a secagem.
- Roupas molhadas oferecem menos isolamento térmico.
- Cada peça pode secar em velocidades diferentes.

## 19.5 Consequências do frio

Quando a temperatura estiver baixa:

- A recuperação de stamina poderá ser reduzida.
- O consumo de energia poderá aumentar.
- A movimentação poderá sofrer penalidades.
- A exposição prolongada poderá provocar perda de saúde.
- O jogador poderá receber alertas de hipotermia.

## 19.6 HUD

Exibir:

- Indicador de temperatura.
- Estado atual do personagem.
- Alertas quando a temperatura atingir níveis perigosos.

O indicador deverá utilizar o padrão visual do HUD atual.

## 19.7 Critérios de aceitação

- A temperatura muda com o ambiente.
- A chuva interfere na temperatura.
- Roupas oferecem proteção diferente.
- Roupas molhadas perdem eficiência.
- Abrigos reduzem a exposição.
- Fogueiras aquecem o jogador.
- Frio intenso causa consequências reais.
- Os valores são persistentes quando o sistema de salvamento estiver disponível.

---

# FASE 20 — DOENÇAS E INFECÇÕES

## Objetivo

Criar um sistema de doenças que torne a sobrevivência mais complexa e exija cuidados com alimentação, água e ferimentos.

Não criar um sistema puramente visual. As doenças devem possuir efeitos reais sobre o personagem.

## 20.1 Sistema de doenças

Criar uma estrutura capaz de representar diferentes condições médicas.

Cada doença deverá possuir:

- Identificador.
- Nome.
- Tipo.
- Gravidade.
- Tempo de duração.
- Tempo de incubação, quando aplicável.
- Sintomas.
- Efeitos sobre o personagem.
- Possíveis tratamentos.
- Condições de recuperação.

## 20.2 Doenças iniciais

Implementar pelo menos:

**Intoxicação alimentar**
- Pode ocorrer ao consumir alimentos estragados.
- Pode provocar perda de saúde.
- Pode aumentar a sede.
- Pode reduzir stamina.

**Infecção intestinal**
- Pode ocorrer ao consumir água contaminada.
- Pode provocar fraqueza.
- Pode aumentar o consumo de água.
- Pode causar perda progressiva de saúde em casos graves.

**Infecção de ferimentos**
- Pode ocorrer após ferimentos sem tratamento adequado.
- Deve possuir evolução gradual.
- Pode provocar perda de saúde.
- Pode exigir medicamento específico.

**Hipotermia**
- Integrar com o sistema de temperatura.
- Pode provocar fraqueza.
- Pode reduzir a capacidade de movimentação.
- Pode provocar perda de saúde em estado crítico.

## 20.3 Sistema de sintomas

Os sintomas devem ser aplicados de acordo com a doença.

Exemplos:

- Tosse.
- Vômito.
- Fraqueza.
- Tremores.
- Perda de saúde.
- Aumento da sede.
- Redução de stamina.

Os efeitos não precisam ser todos implementados na primeira versão, mas cada doença deve ter comportamento funcional e identificável.

## 20.4 Medicamentos

Criar medicamentos com propriedades específicas.

Cada medicamento deverá possuir:

- Tipo de tratamento.
- Doenças que pode tratar.
- Intensidade do tratamento.
- Tempo de ação.
- Quantidade de usos.
- Requisitos de utilização.

Não permitir que qualquer medicamento cure todas as doenças.

## 20.5 Integração

O sistema deverá utilizar:

- Inventário.
- Itens médicos.
- Água.
- Alimentos.
- Sistema de saúde.
- Sistema de sangue.
- HUD.

## 20.6 Critérios de aceitação

- É possível adquirir doenças.
- Cada doença possui efeitos próprios.
- Os sintomas aparecem durante o gameplay.
- Medicamentos adequados tratam as condições.
- Medicamentos inadequados não produzem cura indevida.
- Doenças podem evoluir se não forem tratadas.
- A recuperação ocorre conforme as regras definidas.

---

# FASE 21 — FOGUEIRA E COZIMENTO

## Objetivo

Implementar um sistema completo de fogueiras para aquecimento, iluminação e preparação de alimentos.

## 21.1 Coleta de materiais

Permitir coletar:

- Gravetos.
- Galhos.
- Madeira.
- Materiais inflamáveis.

Os materiais deverão ser representados como itens reais do inventário.

## 21.2 Construção da fogueira

O jogador deverá conseguir:

1. Selecionar os materiais.
2. Combinar os materiais necessários.
3. Criar um kit de fogueira.
4. Posicionar a fogueira no mundo.
5. Acender a fogueira.
6. Alimentar o fogo com madeira.
7. Apagar o fogo.

A posição da fogueira deverá respeitar colisões e limitações do terreno.

## 21.3 Estados da fogueira

Implementar os seguintes estados:

- Construída.
- Apagada.
- Acendendo.
- Acesa.
- Queimando.
- Sem combustível.
- Extinta.

Cada estado deverá possuir comportamento próprio.

## 21.4 Combustível

A fogueira deverá possuir:

- Quantidade de combustível.
- Consumo progressivo.
- Intensidade do fogo.
- Tempo estimado de funcionamento.

Adicionar madeira aumenta o combustível.

Quando o combustível acabar, o fogo deverá diminuir e apagar.

## 21.5 Cozimento

Criar um sistema de preparação de alimentos.

Estados dos alimentos:

- Cru.
- Cozinhando.
- Assado.
- Queimado.

O cozimento deverá depender do tempo de exposição ao calor.

Não permitir que o jogador transforme instantaneamente qualquer alimento em comida pronta.

## 21.6 Propriedades dos alimentos

Cada alimento deverá possuir:

- Valor nutricional.
- Quantidade de hidratação, quando aplicável.
- Estado de preparação.
- Risco de contaminação.
- Tempo de deterioração, quando implementado.

Alimentos queimados poderão ter propriedades inferiores.

## 21.7 Interação com o ambiente

A fogueira deverá:

- Emitir luz.
- Emitir calor.
- Reproduzir sons.
- Produzir fumaça, se tecnicamente viável.
- Atrair zumbis conforme a intensidade do ruído e a distância.

## 21.8 Critérios de aceitação

- O jogador coleta materiais.
- Constrói e posiciona uma fogueira.
- Consegue acender e apagar.
- O combustível diminui com o tempo.
- Alimentos podem ser cozidos.
- O tempo de cozimento influencia o resultado.
- A fogueira aquece o jogador.
- A fogueira produz ruído detectável pelos zumbis.

---

# FASE 22 — ÁGUA E PURIFICAÇÃO

## Objetivo

Expandir o sistema de sede com fontes de água, recipientes, contaminação e purificação.

## 22.1 Fontes de água

Implementar suporte para:

- Poços.
- Rios.
- Lagos.
- Garrafas encontradas.
- Recipientes abandonados.

As fontes deverão ser configuráveis por localização no mapa.

## 22.2 Recipientes

Cada recipiente deverá possuir:

- Capacidade máxima.
- Quantidade atual.
- Tipo de líquido.
- Nível de contaminação.
- Estado de limpeza.

Permitir encher, esvaziar e consumir líquidos.

## 22.3 Qualidade da água

Criar estados:

- Limpa.
- Suspeita.
- Contaminada.
- Purificada.

A água encontrada no mundo não deverá ser automaticamente considerada potável.

## 22.4 Purificação

Implementar métodos:

- Fervura.
- Tabletes purificadores.
- Outros métodos, caso existam itens correspondentes.

A fervura deverá exigir fogueira e tempo de preparação.

## 22.5 Consumo

Permitir:

- Beber diretamente de fontes adequadas.
- Beber de recipientes.
- Escolher quantidade de consumo, quando aplicável.
- Interromper o consumo.

O consumo deverá atualizar a sede do personagem.

## 22.6 Critérios de aceitação

- Fontes de água funcionam.
- Recipientes armazenam quantidades reais.
- Água possui estados de qualidade.
- Purificação altera a qualidade.
- Água contaminada pode causar doenças.
- Consumo atualiza corretamente a sede.

---

# 4. MUNDO E AMBIENTE

# FASE 23 — CLIMA DINÂMICO

## Objetivo

Criar um sistema de clima que altere progressivamente as condições ambientais do mundo.

## 23.1 Estados climáticos

Implementar:

- Céu limpo.
- Nublado.
- Chuva fraca.
- Chuva forte.
- Neblina.
- Tempestade.

## 23.2 Transições

O clima deverá mudar ao longo do tempo.

As transições deverão ser graduais, sem alterações visuais abruptas.

Implementar:

- Duração dos estados.
- Intensidade.
- Probabilidade de mudança.
- Intervalos de transição.
- Configurações de clima inicial.

## 23.3 Chuva

A chuva deverá:

- Possuir representação visual.
- Emitir sons ambientais.
- Molhar roupas.
- Influenciar a temperatura.
- Alterar a visibilidade.
- Interagir com fogueiras.

Se a chuva apagar fogueiras, deverá existir uma regra de resistência ou proteção.

## 23.4 Neblina

A neblina deverá:

- Reduzir a distância de visão.
- Influenciar a detecção de zumbis.
- Alterar a aparência do ambiente.
- Possuir intensidade variável.

## 23.5 Tempestades

Implementar, quando viável:

- Chuva intensa.
- Sons de trovão.
- Iluminação de relâmpagos.
- Redução de visibilidade.

## 23.6 Otimização

Não utilizar efeitos excessivamente pesados.

Priorizar efeitos simples, partículas limitadas, texturas e iluminação compatíveis com o PS2.

## 23.7 Critérios de aceitação

- O clima muda com o tempo.
- Os estados possuem diferenças visuais e sonoras.
- A chuva afeta roupas e temperatura.
- A neblina afeta visibilidade.
- As fogueiras interagem com a chuva.
- O desempenho permanece estável.

---

# FASE 24 — CICLO DE DIA E NOITE

## Objetivo

Implementar um relógio interno do mundo, com transição entre dia, tarde, noite e amanhecer.

## 24.1 Relógio

Criar um sistema de horário interno.

Controlar:

- Horas.
- Minutos.
- Duração de um dia completo.
- Velocidade do tempo.
- Horário inicial.

A duração do ciclo deverá ser configurável.

## 24.2 Iluminação

A iluminação do mapa deverá mudar gradualmente conforme o horário.

Implementar:

- Amanhecer.
- Manhã.
- Tarde.
- Pôr do sol.
- Noite.
- Madrugada.

Utilizar os recursos de iluminação disponíveis no Tyra/TyraX.

## 24.3 Sistema de lanterna

Implementar uma lanterna funcional.

A lanterna deverá:

- Ser encontrada como item.
- Poder ser equipada.
- Poder ser ligada e desligada.
- Consumir pilhas.
- Possuir alcance de iluminação.
- Possuir intensidade configurável.

## 24.4 Pilhas

Criar itens de bateria.

A lanterna deverá consumir energia gradualmente.

Quando a bateria acabar, a lanterna deverá apagar.

Permitir substituir a bateria utilizando o inventário.

## 24.5 Zumbis

Integrar o horário e iluminação com a IA.

Exemplos:

- Zumbis podem detectar jogadores por visão e som.
- A distância de detecção visual poderá variar conforme a iluminação.
- A iluminação da lanterna poderá facilitar a detecção do jogador.

Não tornar os zumbis completamente cegos durante a noite.

## 24.6 Critérios de aceitação

- O horário avança corretamente.
- A iluminação muda gradualmente.
- A lanterna funciona.
- As pilhas possuem duração.
- A bateria pode ser substituída.
- A iluminação influencia a detecção dos zumbis.

---

# FASE 25 — HORDAS E EVENTOS

## Objetivo

Expandir o sistema atual de zumbis para permitir concentrações de inimigos e eventos dinâmicos no mapa.

## 25.1 Hordas

Criar grupos de zumbis que funcionem como unidades de concentração.

Cada horda deverá possuir:

- Quantidade de zumbis.
- Posição inicial.
- Área de atuação.
- Estado.
- Regras de movimentação.
- Distância de ativação.

## 25.2 Comportamento

As hordas deverão:

- Permanecer em áreas específicas.
- Reagir a ruídos.
- Investigar sons.
- Perseguir jogadores.
- Atacar quando estiverem próximos.
- Retornar a uma área de referência quando aplicável.

## 25.3 Eventos de ruído

Utilizar o sistema de ruído existente.

Eventos capazes de atrair zumbis:

- Disparos.
- Explosões, caso existam.
- Fogueiras.
- Portas e objetos barulhentos.
- Corrida.
- Eventos ambientais.

## 25.4 Eventos especiais

Criar eventos configuráveis:

- Concentração de zumbis em uma região.
- Área com loot especial.
- Local de alto risco.
- Presença de zumbi mais resistente.
- Evento de ruído no mapa.

## 25.5 Limites de desempenho

- Definir quantidade máxima de zumbis ativos.
- Controlar a distância de ativação.
- Evitar atualizar IA distante desnecessariamente.
- Utilizar reaproveitamento de entidades quando possível.
- Evitar criar hordas ilimitadas.

## 25.6 Critérios de aceitação

- Hordas aparecem em locais definidos.
- Reagem a eventos de ruído.
- Podem perseguir o jogador.
- Possuem limites de quantidade.
- Não causam quedas excessivas de desempenho.

---

# 5. RECURSOS E ATIVIDADES

# FASE 26 — CRAFTING AVANÇADO

## Objetivo

Criar um sistema de fabricação de itens por combinação de materiais.

O crafting deverá ser integrado ao inventário, loot e equipamentos.

## 26.1 Sistema de receitas

Cada receita deverá possuir:

- Identificador.
- Nome.
- Lista de materiais necessários.
- Quantidade de cada material.
- Requisitos adicionais.
- Resultado.
- Quantidade produzida.
- Tempo de fabricação, quando aplicável.

## 26.2 Receitas iniciais

Implementar receitas funcionais para:

- Bandagem improvisada.
- Tocha.
- Ferramenta improvisada.
- Mochila improvisada.
- Kit de fogueira.
- Materiais de reparo.

## 26.3 Validação

Antes de fabricar:

- Verificar se o jogador possui os materiais.
- Verificar quantidades.
- Verificar espaço no inventário.
- Verificar requisitos.
- Impedir fabricação quando faltarem recursos.

## 26.4 Consumo de materiais

Após a fabricação:

- Remover os materiais utilizados.
- Criar o item resultante.
- Atualizar o inventário.
- Atualizar a interface.
- Tratar falhas sem perder itens indevidamente.

## 26.5 Interface

Criar uma interface de crafting que apresente:

- Receitas disponíveis.
- Materiais necessários.
- Quantidades disponíveis.
- Resultado.
- Indicação de requisitos não atendidos.

## 26.6 Critérios de aceitação

- Receitas podem ser consultadas.
- Materiais são validados.
- Fabricação consome os recursos corretos.
- Itens resultantes são adicionados.
- Não há duplicação ou perda indevida de itens.

---

# FASE 27 — CAÇA E ANIMAIS

## Objetivo

Adicionar animais ao mundo, criando novas fontes de alimento e recursos.

## 27.1 Animais iniciais

Começar com uma implementação simples:

- Veado.
- Coelho.
- Javali.

A quantidade de espécies deverá ser configurável.

## 27.2 Sistema de entidade

Cada animal deverá possuir:

- Identificador.
- Posição.
- Rotação.
- Vida.
- Velocidade.
- Estado atual.
- Área de movimentação.
- Distância de detecção.
- Distância de fuga.

## 27.3 Inteligência artificial

Estados:

- Idle.
- Wander.
- Graze.
- Alert.
- Flee.
- Dead.

Comportamentos:

- Permanecer parado.
- Caminhar pela região.
- Procurar alimento.
- Detectar o jogador.
- Fugir quando ameaçado.
- Reagir a disparos.
- Morrer ao receber dano.

## 27.4 Caça

O jogador deverá conseguir caçar utilizando armas disponíveis.

Após a morte:

- O animal deverá gerar um corpo ou entidade de loot.
- O jogador poderá coletar carne.
- Poderá obter materiais adicionais.
- Os itens deverão ser adicionados ao inventário.

## 27.5 Recursos

Implementar itens como:

- Carne crua.
- Pele.
- Materiais de origem animal.

## 27.6 Critérios de aceitação

- Animais aparecem em áreas configuradas.
- Possuem movimentação.
- Detectam ameaças.
- Fogem do jogador.
- Podem ser caçados.
- Produzem recursos coletáveis.

---

# FASE 28 — PESCA

## Objetivo

Implementar a pesca como atividade de sobrevivência.

## 28.1 Equipamentos

Itens necessários:

- Vara de pesca.
- Linha.
- Anzol.
- Isca.

Os requisitos deverão depender da receita ou configuração do equipamento.

## 28.2 Pontos de pesca

Definir regiões específicas do mapa.

O jogador somente poderá pescar quando estiver dentro de uma área válida e possuir os equipamentos necessários.

## 28.3 Processo de pesca

Implementar:

1. Verificação do equipamento.
2. Verificação da região.
3. Preparação da vara.
4. Lançamento da linha.
5. Tempo de espera.
6. Evento de captura.
7. Resultado.
8. Atualização do inventário.

## 28.4 Resultados

Possíveis resultados:

- Peixe capturado.
- Falha na captura.
- Perda de isca.
- Perda de equipamento, quando configurado.

## 28.5 Alimentos

O peixe deverá ser integrado ao sistema de alimentos.

Permitir:

- Consumir cru, conforme regras de risco.
- Cozinhar.
- Queimar.
- Armazenar no inventário.

## 28.6 Critérios de aceitação

- A pesca exige equipamento.
- Somente funciona em regiões válidas.
- Possui tempo de espera.
- Produz resultados variáveis.
- Peixes podem ser preparados e consumidos.

---

# FASE 29 — AGRICULTURA

## Objetivo

Permitir que o jogador produza alimentos através de plantações.

## 29.1 Preparação do terreno

Criar interação com pontos de plantio.

Permitir preparar o terreno e verificar se a área é adequada.

## 29.2 Sementes

Criar itens de sementes com:

- Tipo de planta.
- Tempo de crescimento.
- Necessidade de água.
- Quantidade produzida.
- Estado de desenvolvimento.

## 29.3 Ciclo de crescimento

Estados:

- Terreno preparado.
- Semente plantada.
- Germinação.
- Crescimento.
- Maturação.
- Colheita.
- Planta morta, quando aplicável.

## 29.4 Água

A plantação deverá precisar de água.

Permitir utilizar recipientes para regar.

## 29.5 Colheita

Ao atingir a maturidade:

- O jogador poderá colher.
- A plantação produzirá alimentos.
- Os alimentos irão para o inventário.
- O terreno poderá ser reutilizado.

## 29.6 Persistência

O crescimento deverá utilizar tempo de jogo e possuir suporte para continuar corretamente após salvar e carregar a partida.

## 29.7 Critérios de aceitação

- O terreno pode ser preparado.
- Sementes podem ser plantadas.
- A água influencia o crescimento.
- O crescimento possui estágios.
- A colheita gera itens.
- O sistema não exige processamento contínuo de todas as plantações.

---

# 6. EQUIPAMENTOS E PROGRESSÃO

# FASE 30 — PESO E CAPACIDADE DE CARGA

## Objetivo

Implementar um sistema de peso que limite a quantidade de itens carregados e influencie a movimentação.

## 30.1 Peso dos itens

Cada item deverá possuir:

- Peso individual.
- Quantidade.
- Peso total acumulado.
- Possibilidade de empilhamento.

## 30.2 Capacidade

O personagem deverá possuir capacidade de carga.

O limite poderá aumentar com mochilas e equipamentos específicos.

## 30.3 Penalidades

Conforme o peso carregado:

- Redução da velocidade.
- Aumento do consumo de stamina.
- Redução da eficiência de corrida.
- Recuperação mais lenta de stamina.

## 30.4 Inventário

Exibir:

- Peso atual.
- Peso máximo.
- Percentual de ocupação.
- Estado de sobrecarga.

Impedir que o jogador ultrapasse limites definidos, salvo exceções configuradas.

## 30.5 Critérios de aceitação

- Cada item possui peso.
- O inventário calcula peso corretamente.
- Equipamentos alteram a capacidade.
- O peso influencia a movimentação.
- A interface apresenta informações corretas.

---

# FASE 31 — DURABILIDADE DE EQUIPAMENTOS

## Objetivo

Adicionar desgaste progressivo aos equipamentos.

## 31.1 Durabilidade

Cada item compatível deverá possuir durabilidade entre 0 e 100.

## 31.2 Equipamentos afetados

- Armas.
- Roupas.
- Mochilas.
- Ferramentas.
- Equipamentos de sobrevivência.

## 31.3 Desgaste

Exemplos:

- Armas perdem condição após disparos.
- Roupas perdem condição após danos.
- Ferramentas perdem condição após utilização.
- Mochilas podem ser danificadas durante ataques.

## 31.4 Estados

- Novo.
- Bom.
- Danificado.
- Muito danificado.
- Inutilizado.

## 31.5 Consequências

A condição deverá influenciar o funcionamento dos equipamentos.

Exemplos:

- Armas danificadas podem apresentar menor confiabilidade.
- Roupas danificadas podem perder isolamento.
- Mochilas danificadas podem perder capacidade, caso configurado.

## 31.6 Critérios de aceitação

- Durabilidade diminui conforme utilização.
- Equipamentos apresentam condições diferentes.
- Itens danificados possuem consequências reais.
- A interface exibe a condição.

---

# FASE 32 — MANUTENÇÃO DE ARMAS

## Objetivo

Criar um sistema de manutenção e reparo de armas.

## 32.1 Condição das armas

Integrar com a durabilidade da Fase 31.

Cada arma deverá possuir:

- Condição.
- Desgaste.
- Possíveis falhas.
- Compatibilidade com materiais de reparo.

## 32.2 Falhas

Implementar falhas proporcionais à condição da arma.

Exemplos:

- Falha de disparo.
- Travamento.
- Necessidade de manutenção.

As falhas deverão ser controladas e não podem tornar o sistema aleatório ou injusto.

## 32.3 Reparos

Criar materiais de manutenção.

O jogador deverá:

- Selecionar a arma.
- Verificar os materiais necessários.
- Executar o reparo.
- Recuperar parte da durabilidade.

## 32.4 Critérios de aceitação

- Armas perdem condição.
- Condição influencia confiabilidade.
- Falhas possuem tratamento.
- Reparos consomem materiais.
- Armas podem recuperar durabilidade.

---

# 7. CONSTRUÇÃO E INTERAÇÃO COM O MUNDO

# FASE 33 — BARRICADAS

## Objetivo

Permitir bloquear entradas e dificultar o acesso de zumbis a determinados ambientes.

## 33.1 Objetos

Implementar suporte para:

- Portas.
- Janelas.
- Entradas.
- Passagens.

## 33.2 Construção

O jogador deverá utilizar materiais para criar barricadas.

Exemplos:

- Tábuas.
- Pregos.
- Ferramentas.

## 33.3 Comportamento

As barricadas deverão:

- Possuir vida.
- Bloquear passagem.
- Receber dano.
- Ser destruídas.
- Possuir estado visual.
- Interagir com colisões.

## 33.4 Zumbis

Os zumbis deverão reconhecer obstáculos barricados.

Quando não conseguirem atravessar, poderão atacar a barricada.

## 33.5 Critérios de aceitação

- O jogador constrói barricadas.
- Os obstáculos bloqueiam passagem.
- Zumbis conseguem atacar.
- As barricadas recebem dano.
- Podem ser destruídas.
- As colisões acompanham o estado do objeto.

---

# FASE 34 — CONSTRUÇÃO DE BASES

## Objetivo

Permitir que o jogador construa estruturas para proteção e armazenamento.

## 34.1 Construções iniciais

Implementar:

- Caixas de armazenamento.
- Cercas.
- Paredes.
- Portas.
- Portões.
- Barracas ou abrigos simples.

## 34.2 Sistema de construção

Cada estrutura deverá possuir:

- Identificador.
- Materiais necessários.
- Posição.
- Rotação.
- Vida.
- Estado de construção.
- Colisão.
- Possibilidade de interação.

## 34.3 Construção progressiva

Estruturas maiores poderão exigir diferentes etapas.

Exemplo:

1. Preparação do local.
2. Posicionamento da estrutura.
3. Consumo dos materiais.
4. Construção.
5. Estrutura concluída.

## 34.4 Armazenamento

Caixas deverão possuir inventários independentes.

O jogador poderá:

- Abrir.
- Guardar itens.
- Retirar itens.
- Transferir quantidades.
- Fechar.

## 34.5 Limites

Definir quantidade máxima de estruturas e objetos construídos.

## 34.6 Critérios de aceitação

- Estruturas podem ser construídas.
- Materiais são consumidos.
- Colisões funcionam.
- Portas e portões podem ser utilizados.
- Caixas armazenam itens.
- As construções podem ser salvas.

---

# FASE 35 — VEÍCULOS

## Objetivo

Adicionar veículos funcionais para exploração do mapa.

## 35.1 Primeira implementação

Começar com um único veículo simples.

Não implementar vários modelos simultaneamente.

## 35.2 Estados do veículo

- Estacionado.
- Disponível.
- Ocupado.
- Sem combustível.
- Danificado.
- Inutilizado.

## 35.3 Componentes

O veículo deverá possuir:

- Motor.
- Combustível.
- Vida geral.
- Pneus.
- Velocidade.
- Capacidade de transporte.

## 35.4 Combustível

Implementar:

- Capacidade máxima.
- Quantidade atual.
- Consumo por distância ou tempo de funcionamento.
- Abastecimento.
- Impossibilidade de funcionar sem combustível.

## 35.5 Componentes danificados

Permitir que componentes recebam danos.

Exemplos:

- Pneus danificados.
- Motor danificado.
- Veículo sem condições de funcionamento.

## 35.6 Interação

O jogador deverá conseguir:

- Aproximar-se.
- Entrar.
- Sair.
- Ligar.
- Desligar.
- Dirigir.
- Abastecer.
- Verificar condições.

## 35.7 Limitações

Implementar física simplificada compatível com o PS2.

Não criar simulação física complexa desnecessária.

## 35.8 Critérios de aceitação

- O jogador entra e sai do veículo.
- O veículo pode ser conduzido.
- Combustível é consumido.
- Componentes recebem danos.
- O veículo pode ser abastecido e reparado.
- A colisão funciona corretamente.

---

# FASE 36 — ARMADILHAS

## Objetivo

Permitir criar armadilhas para animais e zumbis.

## 36.1 Tipos iniciais

- Armadilha para pequenos animais.
- Armadilha de captura.
- Armadilha de dano.
- Armadilha improvisada.

## 36.2 Estados

- Construída.
- Preparada.
- Ativada.
- Disparada.
- Danificada.
- Desmontada.

## 36.3 Funcionamento

Cada armadilha deverá possuir:

- Tipo.
- Área de ativação.
- Alvo compatível.
- Dano ou resultado.
- Tempo de funcionamento.
- Durabilidade.

## 36.4 Integração

- Inventário.
- Crafting.
- Animais.
- Zumbis.
- Sistema de dano.
- Persistência.

## 36.5 Critérios de aceitação

- Armadilhas podem ser construídas.
- Podem ser posicionadas.
- Detectam entidades compatíveis.
- Produzem resultados.
- Podem ser recuperadas ou danificadas.

---

# FASE 37 — INTERAÇÃO AMBIENTAL AVANÇADA

## Objetivo

Expandir as possibilidades de interação com o mapa.

## 37.1 Elementos interativos

Implementar suporte para:

- Portas.
- Janelas.
- Portões.
- Armários.
- Gavetas.
- Veículos.
- Cadáveres.
- Caixas.
- Abrigos.
- Objetos do ambiente.

## 37.2 Estados

Objetos interativos poderão possuir:

- Fechado.
- Aberto.
- Bloqueado.
- Danificado.
- Destruído.

## 37.3 Interações

O jogador deverá conseguir:

- Abrir e fechar.
- Inspecionar.
- Vasculhar.
- Coletar.
- Utilizar.
- Bloquear.
- Desbloquear.
- Destruir, quando aplicável.

## 37.4 Feedback

Toda interação deverá apresentar feedback adequado.

Exemplos:

- Som de porta.
- Animação ou alteração visual.
- Atualização de colisão.
- Mensagem contextual no HUD.
- Alteração do inventário.

## 37.5 Cadáveres

Permitir vasculhar cadáveres de zumbis e animais.

Os cadáveres deverão possuir inventários próprios quando necessário.

## 37.6 Critérios de aceitação

- Objetos possuem estados reais.
- Interações alteram o mundo.
- Colisões acompanham alterações.
- Loot pode ser coletado.
- Eventos de som são gerados quando apropriado.

---

# 8. PERSISTÊNCIA DO MUNDO

# FASE 38 — SISTEMA DE SALVAMENTO E PERSISTÊNCIA

## Objetivo

Criar um sistema confiável de salvamento para preservar o estado do jogador e do mundo entre sessões.

Esta fase também deverá preparar a arquitetura para uma futura integração com servidor externo.

## 38.1 Dados do jogador

Salvar:

- Posição.
- Rotação.
- Vida.
- Sangue.
- Sangramento e condições médicas.
- Fome.
- Sede.
- Stamina.
- Temperatura.
- Doenças.
- Inventário.
- Equipamentos.
- Munição.
- Durabilidade.
- Peso carregado.

## 38.2 Dados do mundo

Salvar:

- Estado das portas.
- Estado das janelas.
- Barricadas.
- Construções.
- Caixas.
- Itens armazenados.
- Veículos.
- Combustível.
- Componentes danificados.
- Plantações.
- Armadilhas.
- Pontos de loot.
- Estado de eventos importantes.

## 38.3 Formato

Criar um formato de armazenamento adequado ao PS2.

Considerar:

- Memory Card.
- Arquivos locais.
- Estrutura compacta.
- Versão do arquivo.
- Validação dos dados.
- Proteção contra corrupção.

Não utilizar um formato complexo sem necessidade.

## 38.4 Salvamento

Implementar:

- Novo jogo.
- Carregar jogo.
- Salvar jogo.
- Continuar jogo.
- Verificação de dados.
- Tratamento de falhas.

## 38.5 Segurança dos dados

O sistema deverá:

- Validar os dados antes de carregar.
- Evitar sobrescrever arquivos válidos com dados incompletos.
- Utilizar versão de save.
- Detectar arquivos inválidos.
- Possuir tratamento de falhas durante o salvamento.

## 38.6 Persistência temporal

Sistemas como plantações, combustível, condições de itens e doenças deverão considerar corretamente a passagem do tempo.

Definir claramente quais sistemas continuam evoluindo enquanto o jogador está fora do jogo.

Não simular todos os processos de forma desnecessária.

## 38.7 Preparação para multiplayer

Não implementar multiplayer nesta fase.

Entretanto, evitar que os sistemas dependam exclusivamente de variáveis globais ou de um único jogador.

Separar, quando apropriado:

- Estado do jogador.
- Estado do mundo.
- Estado das entidades.
- Estado dos objetos interativos.
- Persistência.

Essa separação facilitará uma futura integração com servidor externo.

## 38.8 Critérios de aceitação

- O jogo pode salvar.
- O jogo pode carregar.
- O inventário é preservado.
- A posição do jogador é preservada.
- Os atributos são preservados.
- Construções são preservadas.
- Veículos são preservados.
- Objetos armazenados são preservados.
- Dados inválidos são tratados.
- Não há duplicação de itens ao carregar.
- A compatibilidade entre versões de save é considerada.

---

# 9. INTEGRAÇÃO ENTRE OS SISTEMAS

As mecânicas não devem funcionar isoladamente.

O objetivo é construir um ecossistema de sobrevivência.

Exemplos obrigatórios de integração:

### Chuva

Chuva → molha roupas → reduz isolamento → diminui temperatura → reduz stamina → aumenta risco de hipotermia.

### Fogueira

Coleta de madeira → crafting → construção de fogueira → consumo de combustível → aquecimento → secagem de roupas → cozimento de alimentos.

### Água

Encontrar fonte de água → coletar água → verificar contaminação → purificar → consumir → recuperar sede.

### Doenças

Consumir alimento estragado → adquirir intoxicação → apresentar sintomas → perder saúde → utilizar medicamento → iniciar recuperação.

### Caça

Encontrar animal → detectar jogador → fugir → caçar → coletar carne → cozinhar → consumir → recuperar fome.

### Construção

Coletar materiais → fabricar ferramentas → construir barricada → bloquear passagem → zumbis atacam → barricada recebe dano.

### Peso

Coletar itens → aumentar peso → reduzir mobilidade → consumir mais stamina → administrar inventário.

### Dia e noite

Passagem do tempo → mudança de iluminação → redução da visibilidade → uso de lanterna → aumento da exposição a zumbis.

Essas integrações devem ser implementadas nas fases correspondentes, sem esperar que todos os sistemas estejam prontos.

---

# 10. SISTEMA DE TESTES

Cada fase deverá possuir testes funcionais.

O Copilot deverá criar ou atualizar testes automatizados quando a arquitetura permitir.

Também deverá executar testes práticos no jogo.

## Testes obrigatórios

1. Testar comportamento normal.
2. Testar valores mínimos.
3. Testar valores máximos.
4. Testar inventário cheio.
5. Testar ausência de materiais.
6. Testar utilização repetida.
7. Testar interrupção de ações.
8. Testar transições de estados.
9. Testar interação entre sistemas.
10. Testar salvamento e carregamento quando aplicável.

## Testes de desempenho

Verificar:

- Quantidade de entidades simultâneas.
- Frequência de atualização.
- Consumo de memória.
- Alocações dinâmicas.
- Quantidade de objetos no mapa.
- Quantidade de sons simultâneos.
- Impacto de partículas.
- Impacto de efeitos climáticos.

Quando possível, registrar FPS, memória utilizada e limites observados.

Não afirmar que algo foi testado no PS2 real se somente foi executado no PC ou PCSX2.

---

# 11. ORDEM DE EXECUÇÃO

Implementar as fases nesta ordem:

1. Fase 19 — Temperatura corporal.
2. Fase 20 — Doenças e infecções.
3. Fase 21 — Fogueira e cozimento.
4. Fase 22 — Água e purificação.
5. Fase 23 — Clima dinâmico.
6. Fase 24 — Ciclo de dia e noite.
7. Fase 25 — Hordas e eventos.
8. Fase 26 — Crafting avançado.
9. Fase 27 — Caça e animais.
10. Fase 28 — Pesca.
11. Fase 29 — Agricultura.
12. Fase 30 — Peso e capacidade.
13. Fase 31 — Durabilidade.
14. Fase 32 — Manutenção de armas.
15. Fase 33 — Barricadas.
16. Fase 34 — Construção de bases.
17. Fase 35 — Veículos.
18. Fase 36 — Armadilhas.
19. Fase 37 — Interação ambiental.
20. Fase 38 — Persistência do mundo.

A ordem poderá ser ajustada somente quando existir uma dependência técnica comprovada. Qualquer mudança deverá ser explicada antes da implementação.

---

# 12. INSTRUÇÕES DE TRABALHO PARA O COPILOT

## IMPORTANTE: NÃO IMPLEMENTAR TODAS AS FASES DE UMA VEZ.

Este documento é o planejamento completo do desenvolvimento.

O trabalho deverá acontecer individualmente, uma fase por vez.

### PRIMEIRA ETAPA

Ao receber este documento:

1. Analise o projeto atual.
2. Verifique quais sistemas já existem.
3. Verifique se há funcionalidades parcialmente implementadas.
4. Identifique as dependências da Fase 19.
5. Identifique os arquivos que precisarão ser modificados.
6. Apresente uma proposta de arquitetura para a Fase 19.
7. Informe possíveis riscos de compatibilidade com o PS2.

Não comece a implementar todas as fases.

Apresente primeiro o resultado da auditoria e o plano técnico da Fase 19.

### APÓS A APROVAÇÃO

Quando autorizado:

1. Implemente a Fase 19 completamente.
2. Integre-a aos sistemas existentes.
3. Execute os testes.
4. Compile o projeto.
5. Corrija os problemas encontrados.
6. Documente o resultado.
7. Apresente os arquivos criados e modificados.
8. Informe o que foi testado e em qual ambiente.
9. Informe limitações conhecidas.
10. Aguarde autorização para iniciar a Fase 20.

### REGRA DE CONCLUSÃO

Não marcar uma fase como concluída apenas porque o código compila.

A fase precisa funcionar durante o gameplay, estar integrada, apresentar feedback adequado e passar pelos critérios de aceitação definidos neste documento.

Se algum recurso não puder ser implementado por limitação real do Tyra/TyraX ou do PS2, explicar a limitação e apresentar uma alternativa tecnicamente viável.

**OBJETIVO FINAL:**

Construir um jogo de sobrevivência completo, com exploração, gerenciamento de recursos, combate, zumbis, clima, doenças, crafting, caça, construção, veículos e persistência de mundo, mantendo a identidade visual e técnica do Deep Forest e respeitando as limitações do PlayStation 2.