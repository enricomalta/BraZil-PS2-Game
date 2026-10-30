# DIRETRIZES DE DESENVOLVIMENTO — DEEP FOREST / SURVIVAL PS2

Estamos desenvolvendo um jogo survival 3D para PlayStation 2 utilizando Tyra/TyraX.

O projeto já possui:
- Movimentação do jogador.
- Câmera de terceira pessoa.
- Sistema de armas e munição limitada.
- Recarga e efeitos sonoros.
- Sistema de vida e sangue.
- Sangramento com perda de sangue.
- Zumbis com inteligência artificial básica.
- Ataque e dano dos zumbis.
- Colisões.
- HUD.
- Inventário e loot.
- Fome, sede e stamina.
- Equipamentos e interações básicas.
- Mapa existente.

## REGRAS OBRIGATÓRIAS

1. Antes de implementar qualquer mecânica, examine a arquitetura atual do projeto.
2. Não recrie sistemas que já existem.
3. Não substitua sistemas funcionais sem necessidade técnica comprovada.
4. Não altere a câmera, movimentação ou mapa sem necessidade.
5. Utilize as APIs reais do Tyra/TyraX presentes no projeto.
6. Não invente funções ou bibliotecas.
7. Não introduza SDL2, Unity, Unreal ou dependências desnecessárias.
8. Preserve compatibilidade com o compilador e hardware do PS2.
9. Evite alocações dinâmicas frequentes durante o gameplay.
10. Evite processamento desnecessário dentro do Game Loop.
11. Prefira sistemas modulares, reutilizáveis e desacoplados.
12. Preserve os arquivos existentes e não apague funcionalidades.
13. Não implemente várias fases simultaneamente.

## PROCESSO DE IMPLEMENTAÇÃO

Para cada fase:

1. Auditar os sistemas existentes relacionados.
2. Identificar arquivos que precisam ser criados ou modificados.
3. Apresentar um pequeno plano técnico.
4. Implementar somente a fase solicitada.
5. Compilar o projeto.
6. Corrigir erros.
7. Testar o funcionamento.
8. Verificar possíveis impactos na memória e desempenho.
9. Documentar os arquivos modificados.
10. Informar o que foi efetivamente testado e o que ainda precisa de teste.

## OBJETIVO

Evoluir o Deep Forest para um survival de mundo aberto inspirado nas mecânicas de DayZ, mantendo uma implementação viável para o PlayStation 2.

A prioridade é estabilidade, modularidade, desempenho e compatibilidade com hardware real.