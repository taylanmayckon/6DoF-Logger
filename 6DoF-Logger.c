#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "string.h"
#include <time.h>

#include "mpu6050.h"
#include "logger.h"
#include "hardware/i2c.h"
#include "hardware/rtc.h"

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

// Structs do MPU6050
mpu6050_raw_data_t mpu_raw_data;
mpu6050_data_t mpu_data;
logger_file_t logger_file;

// Structs do cartão SD
sdcard_cmds_t sdcard_cmds;

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
            sdcard_cmds.save_data = !sdcard_cmds.save_data;
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

    // Iniciando os botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    sdcard_cmds.run_mount = false;
    sdcard_cmds.run_unmount = false;
    sdcard_cmds.is_mounted = false;
    sdcard_cmds.save_data = false;

    strcpy(logger_file.filename, "imu_data1.csv");
    logger_file.index = 0;

    while (true){
        if(sdcard_cmds.save_data){
            // Lê os dados do MPU6050
            mpu6050_read_raw(&mpu_raw_data);
            // Processa os dados brutos
            mpu6050_proccess_data(mpu_raw_data, &mpu_data);
            // Debug dos dados
            mpu6050_debug_data(mpu_data);
            // Salva os dados no cartão SD
            save_imu_data(&logger_file, mpu_data);
        }
        else{
            if(sdcard_cmds.run_mount){
                printf("[run_mount] Montando o SD...\n");
                run_mount();
                sdcard_cmds.is_mounted = true;
                sdcard_cmds.run_mount = false;
                printf("[run_mount] SD montado!\n");
            }
            if(sdcard_cmds.run_unmount){
                printf("[run_unmount] Desmontando o SD...\n");
                run_unmount();
                sdcard_cmds.is_mounted = false;
                sdcard_cmds.run_unmount = false;
                printf("[run_unmount] SD desmontado!\n");
            }
        }

        sleep_ms(100);
    }
}
