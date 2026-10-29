Auditoria inicial — Deep Forest e survival-ps2
A auditoria inicial foi concluída. A implementação atual da Fase 1 está registrada ao final deste documento.

Diagnóstico do Deep Forest
O projeto atual já é um jogo para PlayStation 2 integrado à Tyra: o ponto de entrada cria Tyra::Engine e TerrainGame em main.cpp. O ciclo de jogo está em terrain_game.cpp, e o executável existente, deep-forest.elf, é ELF de 32 bits para MIPS.

A organização relevante é:

src/: entrada e implementação do jogo.
src/gen/ e inc/: arquivos gerados pelo TyraX, muitos marcados para não serem editados diretamente.
src/scripts/: scripts do projeto; example_interaction.cpp é um exemplo editável.
objects/: objetos editáveis por arquivo.
deep-forest.tyra: manifesto, cena, preferências e controles.
Makefile: saída deep-forest.elf; usa /tyra/engine e /tyra/Makefile.base por padrão.
Scripts C++
Há dois mecanismos documentados em script.hpp:

TYRA_SCRIPT: registra um script global; seu update() é chamado pelo loop.
TYRA_OBJECT_SCRIPT: registra um componente que pode ser anexado a um objeto no editor em Properties > Scripts.
As anexações por objeto são geradas em object_scripts.gen.cpp. O driver cria uma instância por anexação ao carregar a cena e chama onStart, onUpdate e onUsed. Portanto, não se deve editar esse arquivo gerado diretamente.

Movimento, câmera e sistemas existentes
O jogo já tem jogador nativo Tyra. game_collision.gen.cpp lê gamepad, orienta a câmera e implementa movimento, corrida, colisão e limites; também há caminho de terceira pessoa.
A posição e direção da câmera são calculadas em TerrainGame::updatePlayer() em terrain_game.cpp, e passadas ao renderer por CameraInfo3D.
A cena já contém o objeto player-1, com velocidades de caminhada e corrida e configurações de câmera em 03b7b92d790d3bb7.json.
Há colisão de cenário/objetos e consultas de varredura; a implementação também usa Tyra::BroadPhase em game_collision.gen.cpp.
Modelos estáticos e animados são carregados e renderizados pelo runtime em terrain_game.cpp e game_scene.gen.cpp. O sistema suporta animações de modelos, mas o jogador atual não tem clips configurados.
O áudio usa engine->audio.adpcm, com controle de canais e reverberação em game_scene.gen.cpp.
O jogo já tem manejo de carregamento/liberação de recursos por cena. Scripts de objeto usam new/delete ao carregar e descarregar cenas; isso deve ser considerado no orçamento de memória do PS2.
Teclado: o mapa em deep-forest.tyra já contém bindings WASD, mas a preferência keyboardMouse está desativada. main.cpp também define loadUsbKbdMouse como false. A ativação precisa seguir a configuração do editor, não introduzir SDL no executável do PS2.

Versão da engine e build: pendências
O manifesto informa editorVersion: 2.0.0 e formatVersion: 117, mas isso não identifica a versão da engine Tyra. O Makefile depende de uma instalação externa e não fixa sua versão nem o compilador. Não vou inferir esses dados.

O README documenta Build & Run por F5 ou tyrax-editor.exe --build <pasta> --run. Há um ELF MIPS preexistente, e run.ps1 encaminha o executável ao PCSX2. Na auditoria inicial, tyrax-editor.exe e make.exe não estavam disponíveis no PATH; nesta sessão, o build foi validado pelo toolchain Docker configurado no projeto.

O que survival-ps2 oferece
README.md e CMakeLists.txt descrevem um protótipo C++17 para PC, com SDL2 baixada pelo CMake. A separação em core, gameplay e platform/pc é uma referência útil, mas ainda não existe implementação em platform/ps2.

O protótipo já demonstra estruturas de jogador, arma e zumbi em Player.cpp, Weapon.cpp e Zombie.cpp. Porém, Game.cpp renderiza uma arena 2.5D com SDL: o disparo escolhe o zumbi mais próximo, sem direção de mira ou raycast. A IA cobre estados básicos de investigação e perseguição, mas ainda não implementa ataque nem o ciclo completo de morte previsto no roadmap.

Conclusão: usar esse código como referência para regras e organização; não copiar SDL, CMake ou APIs de PC para o executável Tyra/PS2.

Implementação inicial da Fase 1 (2026-10-29)

O movimento, a câmera, a orientação de mira, o gamepad e as colisões continuam sob controle do jogador nativo Tyra. Não foi criado um segundo controlador, evitando divergência entre a posição do gameplay e a posição usada pelo motor.

Arquivos criados:

- inc/gameplay/player.hpp: estado independente com posição, vetor de orientação, vida, dano e restauração.
- src/scripts/survival_player.cpp: registra um script global; sincroniza posição/orientação a partir do ScriptContext após o movimento nativo e bloqueia a entrada quando a vida chega a zero. survivalPlayer() expõe o estado aos sistemas de gameplay futuros sem alocação por frame.

O estado de vida começa em 100. Dano não positivo é recusado; dano letal limita a vida a zero. Essa interface ainda não tem uma fonte de dano nesta etapa; combate e ataques pertencem às fases seguintes.

Critérios ainda pendentes:

- `player-1` é um controlador Tyra do tipo player (type 6), não um mesh visível em jogo; ele não referencia um modelo estático ou animado. O marcador/gizmo que aparece no editor não é o avatar renderizado. Por escolha do usuário, a câmera permanece em primeira pessoa e o avatar visível aguarda um modelo real.
- Agachamento não foi adicionado: o controlador encontrado não expõe essa ação e a preferência de teclado/mouse segue desativada.
- Vida ainda não é exibida em HUD; o HUD está previsto para a Fase 4.
- A compilação do ELF foi confirmada nesta sessão via Docker. A execução no PCSX2 ainda não foi feita; o caminho configurado no script do projeto não aponta para uma instalação local encontrada.

Validação nesta sessão: cl /c /std:c++14 compilou o script e um teste C++ isolado com um ScriptContext mínimo de teste. A tentativa de link desse executável de teste não passou porque a instalação local do Visual Studio não tem kernel32.lib (LNK1104). O teste isolado não substitui a compilação com as declarações reais da Tyra nem a execução no PCSX2.

A versão exata da engine Tyra continua não identificada. O manifesto informa editorVersion: 2.0.0, e o ELF existente é MIPS, mas isso não determina a versão da engine.

Fase 2 e início da Fase 3

- Adicionadas as ações remapeáveis Fire (R1) e Reload (L1) ao fim do mapa de input, preservando os índices anteriores, e marcadas como `role: custom` para sobreviver à regeneração do build oficial.
- A arma provisória começa com 8 disparos no carregador, 24 de reserva e intervalo mínimo de 0,35 s. A recarga transfere apenas a quantidade disponível e não excede a capacidade.
- A arma provisória é um modelo low-poly original anexado à câmera FPP; recua ao disparar e inclina durante a recarga. Segurar R1 dispara respeitando o intervalo; L1 recarrega. Disparo, acerto e recarga dão feedback de vibração. A HUD da Fase 4 agora apresenta carregador e reserva em segmentos, além da vida.
- A esfera de teste foi substituída por um modelo humanoide low-poly original. O raycast seleciona o zumbi atingido mais próximo, aplica 34 de dano e remove cada inimigo após três acertos. As caixas da arena bloqueiam mira/disparos por teste ray/AABB alinhado aos eixos; oclusão por modelos e caixas rotacionadas ainda não é verificada.
- `Zombie` agora possui os estados Idle, Investigate, Chase, Attack e Dead. A cena contém três instâncias iniciais; o runtime mantém estado independente em uma tabela fixa para até oito inimigos, sem alocações por frame. Tiros publicam ruído; cada zumbi investiga ruídos audíveis, detecta o jogador a curta distância, persegue e ataca a cada 1,2 s, causando 12 de dano.
- O controle da câmera continua no controlador Tyra existente. O deadzone direito padrão foi elevado de 0,2 para 0,35 para reduzir drift do DualShock; o combate não solicita mais trepidação da câmera.
- O estado do jogador, da arma, do zumbi e do ruído é reiniciado quando a cena é carregada novamente. A cadência e a IA usam `g_frameDt`.
- HUD ganhou um retículo central branco que fica verde quando o raycast central encontra o zumbi dentro do alcance útil. A mesma função de interseção é compartilhada pelo indicador e pelo disparo para evitar que a mira confirme um alvo que o tiro não acertaria.
- O alcance continua em 60 unidades; o modelo de teste foi afastado para 24 unidades para validar disparos além da distância anterior. O log de acerto agora informa a distância medida.

O usuário confirmou que a versão com três zumbis funcionou no PCSX2. A HUD da Fase 4 exibe vida, carregador (8 segmentos) e reserva; derrota bloqueia o jogador, apresenta mensagem e permite reiniciar a cena com Confirm (X).

Atualização solicitada pelo usuário: removidas as quatro paredes da arena, mantendo somente as duas caixas de madeira para testes. Os três zumbis foram reposicionados em pontos separados. A IA usa visão por segmento contra as caixas alinhadas aos eixos: quando perde a visão, encerra Chase/Attack em vez de perseguir a posição atual do jogador. Em Idle, cada zumbi percorre pontos curtos determinísticos ao redor de sua posição inicial. Chase, Investigate e patrulha movem o zumbi com colisão XZ por expansão dos AABBs das caixas e deslizamento por eixo; navegação global por caminho ainda não existe. O alcance de ataque foi ampliado para 2,2 unidades; o jogador recebe 12 de dano a cada 1,2 s enquanto estiver ao alcance e visível. O som do zumbi toca ao iniciar a perseguição e em cada golpe que causa dano.

Validação desta entrega: `tyrax-editor.exe --build <pasta> --docker` terminou com `Build OK`, gerou sete textos e três barras de HUD, compilou a cena de oito objetos (incluindo apenas as duas caixas da arena) e atualizou `bin/deep-forest.elf` (4.888.812 bytes). Não houve erros de compilação ou link; permanecem avisos de ABI das bibliotecas. A execução desta versão de IA/arena no PCSX2 ainda está pendente.

A Fase 1 não pode ser declarada concluída ainda: `player-1` continua no modo walk/FPP e não referencia um modelo humano. Crouch permanece adiado; jump continua sendo a ação de pulo. A Fase 2 e a implementação multi-zumbi anterior da Fase 3 foram validadas pelo usuário no PCSX2. A Fase 4 está em andamento: HUD, derrota/reinício, disparos bloqueados por cobertura, IA e novos sons compilam; ainda falta validar a nova IA/arena e os sons no PCSX2, além de adicionar surgimento dinâmico/ondas de inimigos.

Integração de áudio: os cinco arquivos fornecidos foram preservados em `sound/` e convertidos para cópias mono PCM16/22,05 kHz em `res/sfx/`. O manifesto declara os efeitos na ordem disparo, recarga, vazio, passos e zumbi, agrupados no banco `survival-sfx`; o combate ativa esse banco ao carregar a cena. `survival_audio.cpp` reproduz os sons em canais SFX disponíveis, priorizando disparo e recarga sobre passos. O som do zumbi toca ao avistar o jogador e em cada golpe que causa dano; os passos são acionados pelo movimento do jogador.

Validação de áudio: o build `tyrax-editor.exe --build <pasta> --docker` terminou com `Build OK`, gerou `SND_COUNT = 5`, um banco com os cinco índices e atualizou `bin/deep-forest.elf` (4.894.604 bytes). Os avisos existentes de ABI do linker permanecem; nenhum erro de compilação ou link foi reportado. A execução continua pendente de teste no PCSX2.