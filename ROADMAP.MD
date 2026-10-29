# ROADMAP TÉCNICO — SURVIVAL PS2 / DEEP FOREST

## 1. Objetivo

Transformar o projeto Deep Forest existente, desenvolvido sobre a engine Tyra para PlayStation 2, na base de um jogo original de sobrevivência com zumbis.

O jogo terá como referência o combate tático, a furtividade, a escassez de munição e o uso de cobertura de The Last of Us Factions, mas deverá ter identidade, código, personagens, cenários e recursos próprios.

A estratégia de desenvolvimento é utilizar o ambiente de desenvolvimento atual para construir, testar e depurar o jogo, mantendo a compatibilidade com o PlayStation 2 como requisito central.

O desenvolvimento deve ocorrer em etapas pequenas, compiláveis e verificáveis.

## 2. Regras obrigatórias antes de modificar o código

1. Faça uma auditoria completa da estrutura atual do projeto.
2. Identifique a versão e a arquitetura da engine Tyra utilizadas pelo Deep Forest.
3. Leia o README, os arquivos de configuração, o sistema de build e os exemplos existentes.
4. Localize o ponto de entrada, o game loop, os sistemas de input, renderização, câmera, modelos, colisões e áudio.
5. Identifique como os arquivos da pasta Sources são compilados e como os scripts são registrados ou instanciados.
6. Descubra se a aba Scripts do editor gera arquivos automaticamente e quais arquivos são gerenciados pelo editor.
7. Não presuma que o projeto utiliza APIs de outras engines.
8. Não introduza SDL2, OpenGL, Unity, Unreal, SDL GameController ou bibliotecas exclusivas de PC no executável do PS2.
9. Não substitua o sistema de build existente por CMake sem comprovar que isso é necessário.
10. Não apague, renomeie ou sobrescreva arquivos existentes sem justificativa e sem preservar o comportamento atual.
11. Não invente métodos, classes ou APIs do Tyra. Consulte as declarações reais e os exemplos presentes no projeto.
12. Não implemente todas as fases de uma vez.
13. Preserve o funcionamento original do Deep Forest durante a integração.
14. Antes de cada alteração relevante, informe os arquivos que serão criados ou modificados e o motivo.

Comece exclusivamente pela auditoria. Não implemente gameplay antes de apresentar o diagnóstico.

## 3. Arquitetura desejada

Separar a lógica de gameplay dos detalhes específicos de renderização e entrada sempre que a arquitetura existente permitir.

Organização conceitual desejada:

- Core: inicialização, estado da partida e ciclo de atualização.
- Gameplay: Player, Weapon, Zombie, combate, saúde e regras.
- World: cenário, colisões, obstáculos e pontos de interesse.
- AI: percepção, estados e comportamento dos inimigos.
- Platform: integração com a engine Tyra e o hardware do PS2.
- UI: HUD, munição, vida e mensagens de estado.
- Assets: modelos, texturas, animações e áudio.

Essa organização é uma referência lógica, não uma ordem para criar pastas arbitrariamente. Adapte os diretórios aos padrões reais do Deep Forest e às convenções do projeto.

O gameplay não deve depender desnecessariamente de detalhes internos do editor visual. Se a engine exige scripts ou componentes específicos para instanciar objetos, respeite esse mecanismo.

## 4. Fase 0 — Auditoria técnica

Investigue e documente:

- Estrutura completa dos arquivos relevantes.
- Ponto de entrada do programa.
- Ciclo principal do jogo.
- API de input e controles.
- API de câmera.
- API de transformação e posicionamento.
- Carregamento e renderização de modelos.
- Sistema de atualização dos objetos.
- Colisões disponíveis.
- Sistema de animações.
- Sistema de áudio.
- Gerenciamento de memória.
- Registro de scripts e objetos.
- Comandos reais de compilação.
- Localização do executável produzido.
- Método de execução no emulador ou console.

Entregue um relatório com os caminhos dos arquivos que comprovam cada descoberta.

Ao final, apresente um plano de integração específico para a versão real da engine encontrada.

Não prossiga para a Fase 1 até confirmar que a auditoria foi concluída e que o projeto original ainda compila.

## 5. Fase 1 — Player

Implemente o jogador usando as APIs reais da engine.

Requisitos:

- Classe Player ou equivalente.
- Posição e orientação.
- Velocidade de caminhada.
- Corrida.
- Agachamento, caso seja possível integrar sem comprometer a primeira versão.
- Controle por gamepad.
- Suporte a teclado durante os testes no PC, se o ambiente atual permitir.
- Estado de vida.
- Atualização independente da renderização sempre que viável.
- Limites iniciais de movimentação e colisões.
- Modelo provisório já disponível no projeto, se houver.

Utilize inicialmente um personagem simples. Não crie um sistema complexo de animações antes de validar a movimentação.

Critérios de conclusão:

- O projeto compila.
- O jogador aparece na cena.
- O controle movimenta o personagem.
- A rotação corresponde à direção pretendida.
- O jogador não atravessa limites essenciais da arena, caso o sistema de colisão já esteja integrado.
- A implementação não quebra os recursos originais do Deep Forest.

## 6. Fase 2 — Câmera e combate

Implemente:

- Câmera em terceira pessoa.
- Posicionamento sobre o ombro, se o renderer e o sistema de câmera permitirem.
- Controle da direção de mira.
- Uma arma provisória.
- Munição no carregador.
- Recarga.
- Intervalo mínimo entre disparos.
- Detecção de acertos.
- Aplicação de dano.
- Alcance máximo.
- Feedback visual ou textual de disparo e acerto.

Prefira raycast ou outra solução de detecção compatível com a engine. Se não houver raycast, avalie uma implementação simples e adequada ao PS2.

Não crie balística avançada, física complexa ou efeitos gráficos pesados nesta fase.

Critérios de conclusão:

- O jogador consegue mirar e disparar.
- A munição diminui corretamente.
- A recarga restaura a munição.
- Os disparos respeitam o intervalo configurado.
- Os acertos são detectados de maneira consistente.

## 7. Fase 3 — Zombie e IA

Crie uma classe Zombie ou equivalente, separada do Player.

Cada zumbi deve ter:

- Posição.
- Vida.
- Velocidade.
- Estado atual.
- Alcance de percepção.
- Alcance de ataque.
- Tempo entre ataques.
- Estado de morte.

Implemente uma máquina de estados finitos:

- Idle: permanece parado ou executa uma patrulha simples.
- Investigate: vai até uma posição onde percebeu um ruído.
- Chase: persegue o jogador quando o detecta.
- Attack: ataca quando está próximo.
- Dead: deixa de executar a IA e não recebe novos danos.

Os tiros devem produzir eventos de ruído com posição e intensidade. O zumbi deve investigar o local do ruído antes de necessariamente localizar o jogador.

Evite alocações dinâmicas contínuas durante a atualização dos inimigos. Prefira estruturas previsíveis e adequadas à memória do PS2.

Critérios de conclusão:

- É possível instanciar vários zumbis.
- Os inimigos podem ser eliminados.
- Os tiros atraem inimigos próximos.
- A IA muda de estado conforme a situação.
- O jogador recebe dano quando um inimigo ataca.
- A morte do jogador e dos inimigos é tratada corretamente.

## 8. Fase 4 — Arena de sobrevivência

Crie uma pequena arena de teste com:

- Piso.
- Paredes.
- Caixas e obstáculos.
- Pontos de surgimento dos inimigos.
- Colisões básicas.
- Recursos limitados.
- HUD de vida e munição.
- Condição de derrota e reinício.

Priorize geometria simples e modelos low-poly.

A cobertura deve funcionar inicialmente como bloqueio de visão ou de disparos quando a infraestrutura necessária estiver pronta. Não simule uma cobertura funcional apenas por meio de elementos visuais.

## 9. Fase 5 — Portabilidade e desempenho

A arquitetura deve continuar adequada ao PlayStation 2.

Verifique:

- Compatibilidade com o compilador e o padrão C++ efetivamente usados pelo projeto.
- Consumo de RAM.
- Alocações durante o game loop.
- Número de inimigos simultâneos.
- Custo de colisões e consultas de percepção.
- Quantidade de polígonos.
- Texturas e formatos suportados.
- Uso de áudio e animações.
- Desempenho no PCSX2 e, quando possível, no console real.

Não considere a portabilidade concluída apenas porque o projeto compila no Windows.

## 10. Testes e entregas por etapa

Para cada fase:

1. Informe o que será implementado.
2. Liste os arquivos criados e modificados.
3. Implemente somente o escopo aprovado.
4. Compile usando o comando real do projeto.
5. Corrija os erros introduzidos.
6. Informe o resultado dos testes executados.
7. Diferencie testes realmente realizados de testes ainda pendentes.
8. Preserve as alterações preexistentes do usuário.
9. Registre decisões técnicas e limitações em documentação.
10. Só avance depois que os critérios de conclusão forem atendidos.

Se uma API necessária não existir, pare e apresente alternativas fundamentadas no código encontrado.

## 11. Primeira resposta esperada do agente

Neste primeiro momento, não escreva o jogo inteiro.

Entregue somente:

- Diagnóstico da estrutura do Deep Forest.
- Identificação do mecanismo de scripts C++.
- Explicação de como um objeto ou script é instanciado.
- Comando de compilação identificado no projeto.
- Plano de implementação da Fase 1.
- Lista exata dos arquivos que precisariam ser criados ou modificados na Fase 1.

Aguarde a conclusão dessa análise antes de modificar os arquivos.

Objetivo final: desenvolver um survival shooter original, com câmera em terceira pessoa, combate tático, furtividade, armas, zumbis e recursos limitados, mantendo a engine Tyra e o PlayStation 2 como alvos prioritários.