#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
#include <hardware/pwm.h>
#include "hardware/i2c.h"
#include "hardware/clocks.h"   
#include "animacao_matriz.pio.h" // Biblioteca PIO para controle de LEDs WS2818B 

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define GREEN_LED 11
#define RED_LED 13
#define MATRIZ_PIN 7            // Pino GPIO conectado aos LEDs WS2818B
#define LED_COUNT 25            // Número de LEDs na matriz
#define BUZZER_A 21
#define BUTTON_A 5

PIO pio;
uint sm;

volatile uint8_t current_state = 0; // Estado atual do semáforo
volatile bool mode = false; // Modo noturno do semáforo

// Matriz para armazenar os desenhos da matriz de LEDs
uint padrao_led[4][LED_COUNT] = {
    {0, 0, 0, 0, 0,
     0, 1, 1, 1, 0,
     0, 1, 1, 1, 0,
     0, 1, 1, 1, 0,
     0, 0, 0, 0, 0,
    }, // Semáforo Verde
    {0, 0, 0, 0, 0,
     0, 2, 2, 2, 0,
     0, 2, 2, 2, 0,
     0, 2, 2, 2, 0,
     0, 0, 0, 0, 0,
    }, // Semáforo Amarelo
    {0, 0, 0, 0, 0,
     0, 3, 3, 3, 0,
     0, 3, 3, 3, 0,
     0, 3, 3, 3, 0,
     0, 0, 0, 0, 0,
    }, // Semáforo Vermelh0
    {0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,
    }, // Desligado
};

// Ordem da matriz de LEDS, útil para poder visualizar na matriz do código e escrever na ordem correta do hardware
int ordem[LED_COUNT] = {0, 1, 2, 3, 4, 9, 8, 7, 6, 5, 10, 11, 12, 13, 14, 19, 18, 17, 16, 15, 20, 21, 22, 23, 24};  


// Rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(unsigned r, unsigned g, unsigned b){
    return (g << 24) | (r << 16) | (b << 8);
}

// Rotina para desenhar o padrão de LED
void display_desenho(uint8_t desenho){
    uint32_t valor_led;
    for (int i = 0; i < LED_COUNT; i++){
        // Define a cor do LED de acordo com o padrão
        if (padrao_led[desenho][ordem[24 - i]] == 1){
            valor_led = matrix_rgb(0, 10, 0); // Verde
        } else if (padrao_led[desenho][ordem[24 - i]] == 2){
            valor_led = matrix_rgb(10, 10, 0); // Amarelo
        } else if (padrao_led[desenho][ordem[24 - i]] == 3){
            valor_led = matrix_rgb(10, 0, 0); // Vermelho
        } else{
            valor_led = matrix_rgb(0, 0, 0); // Desliga o LED
        }
        // Atualiza o LED
        pio_sm_put_blocking(pio, sm, valor_led);
    }
}



void vLedTask(){
    // Inicializa os leds
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
    gpio_init(RED_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);
    
    while (true){
        if (mode){
            // Modo noturno, pisca os LEDs lentamente
            gpio_put(GREEN_LED, true); // Liga o LED verde
            gpio_put(RED_LED, true); // Liga o LED vermelho
            vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
            gpio_put(GREEN_LED, false); // Desliga o LED verde
            gpio_put(RED_LED, false); // Desliga o LED vermelho
            vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo
        } 
        else {

            if (current_state == 0){
                gpio_put(GREEN_LED, true); // Liga o LED verde
                gpio_put(RED_LED, false); // Desliga o LED vermelho
                vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5 segundos
                current_state = 1; // Muda o estado para vermelho
            }
            else if (current_state == 1){
                gpio_put(GREEN_LED, true); // Liga o LED verde
                gpio_put(RED_LED, true); // Liga o LED vermelho
                vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5 segundos
                current_state = 2; // Muda o estado para vermelho
            } else{
                gpio_put(GREEN_LED, false); // Desliga o LED verde
                gpio_put(RED_LED, true); // Liga o LED vermelho
                vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5 segundos
                current_state = 0; // Muda o estado para verde
            }
        }
    }
}

void vMatrizTask(){
    // Configuração do PIO
    pio = pio0; 
    uint offset = pio_add_program(pio, &animacao_matriz_program);
    sm = pio_claim_unused_sm(pio, true);
    animacao_matriz_program_init(pio, sm, offset, MATRIZ_PIN);

    while (true){
        if (mode){
            // Modo noturno, pisca os LEDs lentamente
            display_desenho(1); // Atualiza o padrão de LED
            vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1000ms
            display_desenho(3); // Atualiza o padrão de LED
            vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1000ms
        } else{
            display_desenho(current_state); // Atualiza o padrão de LED
            vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5 segundos
        }
    }
}

void vBuzzerTask(){
    // Configuração do buzzer
    gpio_set_function(BUZZER_A, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_A); // Obtém o slice correspondente
    pwm_set_clkdiv(slice_num, 125); // Define o divisor de clock
    pwm_set_wrap(slice_num, 1000);  // Define o valor máximo do PWM
    pwm_set_enabled(slice_num, true);

    while (true){
        if (mode){
            // Modo noturno, beep lento a cada 2 segundos
            pwm_set_gpio_level(BUZZER_A, 100); // Liga o buzzer
            vTaskDelay(pdMS_TO_TICKS(200)); // Aguarda 300ms
            pwm_set_gpio_level(BUZZER_A, 0); // Desliga o buzzer
            vTaskDelay(pdMS_TO_TICKS(1800)); // Aguarda 1.7s
        } else{
            if (current_state == 0){
                // Um beep curto por segundo
                pwm_set_gpio_level(BUZZER_A, 100); // Define o nível do PWM para o buzzer
                vTaskDelay(pdMS_TO_TICKS(200)); // Aguarda 200ms
                pwm_set_gpio_level(BUZZER_A, 0); // Desliga o buzzer
                vTaskDelay(pdMS_TO_TICKS(800)); // Aguarda 900ms
            }
            else if (current_state == 1){
                // Um beep rápido intermitente
                pwm_set_gpio_level(BUZZER_A, 100); // Liga o buzzer
                vTaskDelay(pdMS_TO_TICKS(100)); // Aguarda 100ms
                pwm_set_gpio_level(BUZZER_A, 0); // Desliga o buzzer
                vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms
            } else{
                // Tom contínuo curto 500 ms ligado e 1.5 desligados
                pwm_set_gpio_level(BUZZER_A, 100); // Liga o buzzer
                vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms
                pwm_set_gpio_level(BUZZER_A, 0); // Desliga o buzzer
                vTaskDelay(pdMS_TO_TICKS(1500)); // Aguarda 1.5s
            }
        }
    }
}

void vSwitchModeTask(){
    // Configuração do botão
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    while (true){
        if (gpio_get(BUTTON_A) == 0){
            mode = !mode; // Alterna o modo
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void vDisplayTask()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);


    int contador = 0;
    bool cor = true;

    char semaforocolor[3][10] = {"Verde", "Amarelo", "Vermelho"}; // Array para armazenar os nomes das cores

    while (true)
    {
        if (mode){
            ssd1306_fill(&ssd, !cor); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Semaforo", 30, 6); // Desenha uma string
            ssd1306_draw_string(&ssd, "Inteligente", 16, 18);  // Desenha uma string
            ssd1306_line(&ssd, 3, 27, 123, 27, cor);      // Desenha uma linha

            ssd1306_draw_string(&ssd, "Modo Noturno", 16, 32); // Desenha uma string
            ssd1306_draw_string(&ssd, "Ativado", 32, 44); // Desenha uma string
            ssd1306_send_data(&ssd);                           // Atualiza o display
            vTaskDelay(pdMS_TO_TICKS(2000));
        } else{
            ssd1306_fill(&ssd, !cor);                          // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
            ssd1306_draw_string(&ssd, "Semaforo", 30, 6); // Desenha uma string
            ssd1306_draw_string(&ssd, "Inteligente", 16, 18);  // Desenha uma string
            ssd1306_line(&ssd, 3, 27, 123, 27, cor);      // Desenha uma linha
            ssd1306_draw_string(&ssd, "Estado atual:", 8, 32); // Desenha uma string
            ssd1306_draw_string(&ssd, semaforocolor[current_state], 30, 44); // Desenha uma string

            ssd1306_send_data(&ssd);                           // Atualiza o display
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == botaoB){
        reset_usb_boot(0, 0);
    }
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B



    stdio_init_all();

    xTaskCreate(vLedTask, "Led Semaforo Task", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatrizTask, "Matriz Semaforo Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer Semaforo Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vSwitchModeTask, "Botao Semaforo Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display Semaforo Task", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
