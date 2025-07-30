#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "string.h"
#include <time.h>

#include "mpu6050.h"
#include "logger.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"
#include "hardware/pwm.h"

#include "ff.h"
#include "diskio.h"
#include "f_util.h"
#include "hw_config.h"
#include "my_debug.h"
#include "rtc.h"
#include "sd_card.h"

// Definição dos pinos I2C para o MPU6050
#define I2C_PORT i2c0                 
#define I2C_SDA 0
#define I2C_SCL 1

// Definição dos pinos I2C para o display OLED
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define ENDERECO_DISP 0x3C            

// GPIO utilizada
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12 
#define BUTTON_A 5
#define BUTTON_B 6
#define JOYSTICK_BUTTON 22
#define LED_MATRIX_PIN 7
#define BUZZER_A 21
#define BUZZER_B 10

// Configurações da I2C do display
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define endereco 0x3C
bool cor = true;
ssd1306_t ssd;

// Structs do MPU6050
mpu6050_raw_data_t mpu_raw_data;
mpu6050_data_t mpu_data;
logger_file_t logger_file;
mpu6050_filtered_t mpu_data_kf;

// Structs do cartão SD
sdcard_cmds_t sdcard_cmds;

// Configurações para o PWM
uint wrap = 2000;
uint clkdiv = 25;

// Enums para os feedbacks visuais/sonoros
enum led_states_t led_state;
enum buzzer_states_t buzzer_state;

// -> Funções Auxiliares =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Função para configurar o PWM e iniciar com 0% de DC
void set_pwm(uint gpio, uint wrap){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice_num, clkdiv);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); 
    pwm_set_gpio_level(gpio, 0);
}

// -> ISR dos Botões =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Tratamento de interrupções 
int display_page = 1;
int num_pages = 5;
uint32_t last_isr_time = 0;
void gpio_irq_handler(uint gpio, uint32_t events){
    uint32_t current_isr_time = to_us_since_boot(get_absolute_time());
    if(current_isr_time-last_isr_time > 200000){ // Debounce
        last_isr_time = current_isr_time;
        
        // Montar/desmontar o cartão SD
        if(gpio==BUTTON_A) {
            if(sdcard_cmds.is_mounted) sdcard_cmds.run_unmount = true;
            if(!sdcard_cmds.is_mounted) sdcard_cmds.run_mount = true;
        }
        
        // Iniciar/parar captura de dados
        if(gpio==BUTTON_B){
            if(!sdcard_cmds.save_data) sdcard_cmds.handle_filename = true;
            sdcard_cmds.save_data = !sdcard_cmds.save_data;
            if(!sdcard_cmds.save_data){
                led_state = INIT_MOUNT_SD;
                buzzer_state = STOP_SAVES;
            }
        }
    }
}

int main(){
    stdio_init_all();
    sleep_ms(5000);

    // Inicialização da I2C do MPU6050
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Declara os pinos como I2C na Binary Info
    bi_decl(bi_2pins_with_func(I2C_SDA, I2C_SCL, GPIO_FUNC_I2C));
    mpu6050_reset();

    // Iniciando o display
    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT_DISP);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);

    // Iniciando os botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    // Iniciando o LED RGB
    set_pwm(LED_RED, wrap);
    set_pwm(LED_GREEN, wrap);
    set_pwm(LED_BLUE, wrap);
    led_state = INIT_MOUNT_SD;
    handle_rgb_led(led_state, wrap);

    // Inicializando os Buzzers
    set_pwm(BUZZER_A, wrap);
    set_pwm(BUZZER_B, wrap);
    buzzer_state = IDLE;
    handle_buzzer(&buzzer_state, wrap);

    sdcard_cmds.run_mount = false;
    sdcard_cmds.run_unmount = false;
    sdcard_cmds.is_mounted = false;
    sdcard_cmds.save_data = false;
    sdcard_cmds.handle_filename = false;

    while (true){
        // buzzer_state = IDLE;
        handle_buzzer(&buzzer_state, wrap);
        handle_rgb_led(led_state, wrap);

        // Atualiza o nome do arquivo, quando necessário
        if(sdcard_cmds.handle_filename){
            handle_filename(&logger_file);
            sdcard_cmds.handle_filename = false;
            buzzer_state = INIT_SAVES;
            handle_buzzer(&buzzer_state, wrap);
        }

        // Salva os dados, quando necessário
        if(sdcard_cmds.save_data){
            // Alertas sonoros e visuais
            led_state = SAVE_READ_SD;
            handle_rgb_led(led_state, wrap);

            // Lê os dados do MPU6050
            mpu6050_read_raw(&mpu_raw_data);
            // Processa os dados brutos
            mpu6050_proccess_data(mpu_raw_data, &mpu_data);
            // Aplica o filtro de Kalmann
            mpu6050_kalmann_filter(mpu_data, &mpu_data_kf);
            // Debug dos dados
            mpu6050_debug_data(mpu_data, mpu_data_kf);
            // Salva os dados no cartão SD
            save_imu_data(&logger_file, mpu_data, mpu_data_kf);
        }

        else{ // Montagem/desmontagem do SD Card
            if(sdcard_cmds.run_mount){
                printf("[run_mount] Montando o SD...\n");
                // Alertas sonoros e visuais
                led_state = INIT_MOUNT_SD;
                handle_rgb_led(led_state, wrap);
                buzzer_state = MOUNT;
                handle_buzzer(&buzzer_state, wrap);

                // Tenta montar e retorna sucesso/erro
                if(!run_mount()){
                    led_state = ERROR;
                    handle_rgb_led(led_state, wrap);
                    printf("[ERRO - run_mount] Falha ao montar o SD!\n\n");
                    sdcard_cmds.is_mounted = false;
                    sdcard_cmds.run_mount = false;
                }
                else{
                    printf("[run_mount] SD montado!\n\n");
                    sdcard_cmds.is_mounted = true;
                    sdcard_cmds.run_mount = false;
                    led_state = READY_FOR_SAVE;
                    handle_rgb_led(led_state, wrap);
                }
            }
            if(sdcard_cmds.run_unmount){
                printf("[run_unmount] Desmontando o SD...\n");
                led_state = UNMOUNT;
                handle_rgb_led(led_state, wrap);
                buzzer_state = B_UNMOUNT;
                handle_buzzer(&buzzer_state, wrap);

                run_unmount();
                sdcard_cmds.is_mounted = false;
                sdcard_cmds.run_unmount = false;
                printf("[run_unmount] SD desmontado!\n\n");
            }
        }

        // Display OLED
        char str_index[13];
        sprintf(str_index, "COLETA: %d", logger_file.index);

        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd, 0, 0, 128, 64, cor, !cor);
        ssd1306_rect(&ssd, 0, 0, 128, 12, cor, cor); // Fundo preenchido
        // LINHA 1 - STATUS
        ssd1306_draw_string(&ssd, "6DoF-Logger", 4, 3, true);
        switch(led_state){
            case INIT_MOUNT_SD:
                ssd1306_draw_string(&ssd, "INICIALIZANDO", 4, 18, false);
                ssd1306_draw_string(&ssd, "MONTE COM", 4, 34, false);
                ssd1306_draw_string(&ssd, "BOTAO A", 4, 50, false);
                break;
            case READY_FOR_SAVE:
                ssd1306_draw_string(&ssd, "SD MONTADO", 4, 18, false);
                ssd1306_draw_string(&ssd, "PRONTO PARA", 4, 28, false);
                ssd1306_draw_string(&ssd, "GRAVAR DADOS", 4, 38, false);
                ssd1306_draw_string(&ssd, "COM BOTAO B", 4, 48, false);
                break;
            case SAVE_READ_SD:
                ssd1306_draw_string(&ssd, "GRAVANDO DADOS", 4, 14, false);
                ssd1306_draw_string(&ssd, logger_file.filename, 4, 24, false);
                ssd1306_draw_string(&ssd, str_index, 4, 34, false);
                ssd1306_draw_string(&ssd, "PARE COM ", 4, 44, false);
                ssd1306_draw_string(&ssd, "BOTAO B", 4, 54, false);
                break;
            case UNMOUNT:
                ssd1306_draw_string(&ssd, "DESMONTADO", 4, 18, false);
                ssd1306_draw_string(&ssd, "REMOVA O SD", 4, 28, false);
                ssd1306_draw_string(&ssd, "OU MONTE DENOVO", 4, 38, false);
                ssd1306_draw_string(&ssd, "COM BOTAO A", 4, 48, false);
                break;
            case ERROR:
                ssd1306_draw_string(&ssd, "ERRO DETECTADO", 4, 14, false);
                ssd1306_draw_string(&ssd, "VERIFIQUE O", 4, 24, false);
                ssd1306_draw_string(&ssd, "CARTAO SD E", 4, 34, false);
                ssd1306_draw_string(&ssd, "TENTE DENOVO", 4, 44, false);
                break;
        }



        ssd1306_send_data(&ssd);
        sleep_ms(100);
    }
}
