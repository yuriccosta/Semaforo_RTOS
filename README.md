# Semáforo Inteligente com Modo Noturno e Acessibilidade

## Objetivo Geral

Desenvolver um semáforo inteligente com acessibilidade, que seja capaz de **traduzir o estado das luzes em sons**, facilitando a compreensão por pessoas com deficiência visual. Além disso, o sistema conta com um **modo noturno**, adaptando seu funcionamento à ausência de luz ambiente.

---

## Descrição Funcional

O sistema utiliza o **FreeRTOS** como sistema operacional em tempo real para gerenciar suas tarefas paralelas, proporcionando controle eficiente sobre os periféricos e estados do semáforo.

### Tarefas implementadas:

- **`vLedTask`**  
  Responsável por atualizar o estado do semáforo e mudar as cores do LED RGB.  
  - Alterna o estado a cada 5 segundos (vermelho → verde → amarelo).  
  - No **modo noturno**, pisca a luz **amarela**: acende por 1 segundo e apaga por 1 segundo, em loop.

- **`vMatrizTask`**  
  Atualiza o desenho da **matriz de LEDs** com base no estado atual.  
  - Reflete as cores verde, amarela ou vermelha.  
  - No **modo noturno**, pisca com luz **amarela** de forma alternada (1s acesa, 1s apagada).

- **`vBuzzerTask`**  
  Emite sons que representam o estado do semáforo:  
  - Verde: som por 200ms e silêncio por 800ms.  
  - Amarelo: som por 100ms e silêncio por 500ms.  
  - Vermelho: som por 500ms e silêncio por 1500ms.  
  - **Modo noturno**: som por 1000ms e silêncio por 1000ms.

- **`vSwitchModeTask`**  
  Monitora um botão físico para alternar entre o **modo normal** e o **modo noturno**.  
  - Verifica o estado do botão a cada 200ms.  
  - Atualiza uma flag global `mode`.

- **`vDisplayTask`**  
  Atualiza o **display OLED** a cada 5 segundos com mensagens como:  
  - "Modo Noturno Ativo"  
  - Ou a cor atual do semáforo: "VERDE", "AMARELO", "VERMELHO".

---

## Periféricos Utilizados (BitDogLab / RP2040)

| Periférico     | Função                                                                 |
|----------------|------------------------------------------------------------------------|
| **Display OLED**  | Exibe o estado atual ou se o modo noturno está ativo.                  |
| **Matriz de LEDs**| Representa graficamente o semáforo com as cores verde, amarelo e vermelho. |
| **Buzzer**        | Emite sons com diferentes durações para cada estado do semáforo.       |
| **LED RGB**       | Mostra visualmente o estado do semáforo (verde, amarelo ou vermelho).  |
| **Botão**         | Alterna entre o modo normal e o modo noturno.                         |

---

## Variáveis Globais e Estruturas Importantes

| Nome               | Descrição                                                                 |
|--------------------|--------------------------------------------------------------------------|
| `vLedTask`         | Tarefa que atualiza o estado do semáforo e o LED RGB.                    |
| `vMatrizTask`      | Tarefa que desenha a cor correspondente na matriz de LEDs.               |
| `vBuzzerTask`      | Tarefa que emite sons conforme o estado do semáforo.                     |
| `vSwitchModeTask`  | Tarefa que alterna entre os modos de operação através do botão.          |
| `vDisplayTask`     | Tarefa que atualiza o texto exibido no display OLED.                     |
| `padrao_led`       | Matriz que armazena os padrões de desenho para a matriz de LEDs.         |
| `uint8_t current_state` | Variável que guarda o estado atual do semáforo (0 = verde, 1 = amarelo, 2 = vermelho). |
| `bool mode`        | Flag que indica se o **modo noturno** está ativado (`true`) ou não (`false`). |

---
