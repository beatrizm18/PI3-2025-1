#include <stdio.h>			// Bibliotecas gerais
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "esp_err.h"		
#include "esp_spiffs.h"			// API do file system
#include "esp_log.h"			// API p/ mensagens de erro
#include "esp_netif.h"			// Bibliotecas p/ configuração wifi
#include "esp_wifi.h"
#include "hal/adc_types.h"
#include "hal/gpio_types.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "lwip/inet.h"
#include "esp_http_server.h"	// Bibliotecas p/ HTTP server
#include "freertos/FreeRTOS.h"	// Bibliotecas gerais do FreeRTOS
#include "freertos/task.h"
#include "driver/dac_oneshot.h"
#include "esp_adc/adc_oneshot.h"

#define LIMIAR_PARADA 1000 // Limiar de parada da bomba (valor ADC)

// Definições dos pinos

// Saídas binárias
#define 	M102			GPIO_NUM_15
#define 	E104			GPIO_NUM_27
#define 	K1				GPIO_NUM_14
#define 	M1				GPIO_NUM_12
#define 	M106			GPIO_NUM_13


// Entradas binárias
#define		B102			GPIO_NUM_2
#define		S111			GPIO_NUM_4
#define 	S112			GPIO_NUM_5
#define 	B113			GPIO_NUM_18
#define 	B114			GPIO_NUM_19
#define 	S115			GPIO_NUM_22
#define 	S116			GPIO_NUM_23

// Saídas analógicas
#define 	P101			GPIO_NUM_26
#define 	V106			GPIO_NUM_25

// Entradas analógicas
#define 	LIC_B101		ADC_CHANNEL_4
#define 	FIC_B102		ADC_CHANNEL_5
#define 	PIC_B103		ADC_CHANNEL_6
#define 	TIC_B104		ADC_CHANNEL_7

// Parâmetros PID para controle de nível
float Kp_nivel = 2.0f;      
float Ki_nivel = .001f;      
   
// Parâmetros PID para controle de temperatura
float Kp_temp = 1.0f;      
float Ki_temp = .0001f;

// Setpoints
int setpoint_nivel = 1000;  // Nível desejado (em unidades do sensor ultrassônico)
int setpoint_temp = 1000;  // Temperatura desejada (em unidades do sensor de temperatura)

// Variáveis de controle PID temperatur
volatile float erro_nivel, erro_ant_nivel = 0, erro_int_nivel = 0;
volatile float controle_nivel;

// Variáveis de controle PID
volatile float erro_temp, erro_ant_temp = 0, erro_int_temp = 0;
volatile float controle_temp;

static const char* TAG = "PI3";
static bool bomba_ligada = false; // Flag para verificar o estado da bomba
static bool habilitar_controle_nivel = false;
static bool aquecedor_ligado = false; //estaod do aquecedor
static bool habilitar_controle_temperatura = false;

// Handles dos canais do DAC
dac_oneshot_handle_t P101_dac_handle;

// Handles dos canais do ADC
adc_oneshot_unit_handle_t adc1_handle;

// Variáveis globais
// Variaveis globais
static int E104_status = 0;
static int M1_status = 0;
static int M106_status = 0;
int leitura_ultrassom = 0; // Leitura do sensor ultrassônico (nível)
int leitura_temperatura = 0;   // Leitura do sensor de temperatura


// Função para inicializar SPIFFS (sistema de arquivos)
void spiffs_init(void) {
    esp_vfs_spiffs_conf_t fs_config = {
        .base_path = "/storage",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };

    esp_err_t result = esp_vfs_spiffs_register(&fs_config);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar SPIFFS (%s)", esp_err_to_name(result));
    }

    size_t total = 0, used = 0;
    result = esp_spiffs_info(fs_config.partition_label, &total, &used);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao obter informações da partição (%s)", esp_err_to_name(result));
    } else {
        ESP_LOGI(TAG, "Tamanho da partição: total = %d, usado = %d", total, used);
    }
}

static void wifi_init(void) {
    // Inicializa a rede e configura o modo Access Point
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_t *netif = esp_netif_create_default_wifi_ap();

    // Configuração do AP (Ponto de Acesso)
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_AP",           // Nome da rede Wi-Fi
            .password = "senha123",        // Senha do ponto de acesso
            .ssid_len = strlen("ESP32_AP"),
            .channel = 1,                 // Canal Wi-Fi
            .authmode = WIFI_AUTH_WPA2_PSK, // Tipo de autenticação
            .max_connection = 4,          // Número máximo de conexões
            .beacon_interval = 100        // Intervalo de Beacon
        },
    };
    
    esp_netif_dhcpc_stop(netif);                          // Desativa o DHCP

    // Inicializa a configuração do Wi-Fi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);  // Define o modo Access Point

    // Define a configuração do AP
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();
    ESP_LOGI(TAG, "AP Configurado com SSID: ESP32_AP");
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    FILE* f = fopen("/storage/index.html", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file /storage/index.html for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html"); // Define o tipo de conteúdo
    char buf[129]; // Buffer para leitura
    int read_bytes;

    while ((read_bytes = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[read_bytes] = '\0'; // Adiciona o terminador de string
        httpd_resp_send_chunk(req, buf, read_bytes);
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Finaliza a resposta
    return ESP_OK;
}

static esp_err_t style_get_handler(httpd_req_t *req) {
    FILE* f = fopen("/storage/style.css", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file /storage/style.css for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/css"); // Define o tipo de conteúdo
    char buf[129];	// Buffer para leitura
    int read_bytes;

    while ((read_bytes = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[read_bytes] = '\0'; // Adiciona o terminador de string
        httpd_resp_send_chunk(req, buf, read_bytes);
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Finaliza a resposta
    return ESP_OK;
}

static esp_err_t script_get_handler(httpd_req_t *req) {
    FILE* f = fopen("/storage/script.js", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file /storage/script.js for reading");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/javascript"); // Define o tipo de conteúdo
    char buf[129];
    int read_bytes;

    while ((read_bytes = fread(buf, 1, sizeof(buf) - 1, f)) > 0) {
        buf[read_bytes] = '\0';
        httpd_resp_send_chunk(req, buf, read_bytes);
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Finaliza a resposta
    return ESP_OK;
}


static esp_err_t data_get_handler(httpd_req_t *req) {
    char param[32]; // Variável para armazenar a query string
    char sensor[32]; // Variável para armazenar o valor de "tipo"
    char sensor_data[32];
    int adc_val;

    // Obtém a query string da requisição
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        //ESP_LOGD(TAG, "Query string recebida: %s", param);

        // Procura o parâmetro "tipo" e armazena o valor em "command"
        if (httpd_query_key_value(param, "tipo", sensor, sizeof(sensor)) == ESP_OK) {
            //ESP_LOGD(TAG, "Sensor requisitado: %s", sensor);

            // Processa o comando recebido
            if (strcmp(sensor, "sensorDig1") == 0) {
                //ESP_LOGD(TAG, "Sensor digital 1 lido");
                
                if(gpio_get_level(B102) == 1){
					snprintf(sensor_data, sizeof(sensor_data), "ON");
				}else{
					snprintf(sensor_data, sizeof(sensor_data), "OFF");
				}
   
			
            } else if(strcmp(sensor, "sensorAnalog1") == 0) {
				
				//ESP_LOGD(TAG, "Sensor analógico 1 lido");
				
				//adc_oneshot_read(adc1_handle, LIC_B101, &adc_val);
			   	printf("%d;%d\n", bomba_ligada, leitura_ultrassom); 
				
	
				snprintf(sensor_data, sizeof(sensor_data), "%d", leitura_ultrassom);
			
			} else if(strcmp(sensor, "sensorAnalog4") == 0) {
				
				//ESP_LOGD(TAG, "Sensor analógico 4 lido");	
				float temp = (leitura_temperatura * 100)/4096;
				
				printf("%d;%f\n", aquecedor_ligado, temp);
				
				//snprintf(sensor_data, sizeof(sensor_data), "%d", leitura_temperatura);
				snprintf(sensor_data, sizeof(sensor_data), "%f", temp);
			} else {
                ESP_LOGE(TAG, "Comando não reconhecido: %s", sensor);
            }
        } else {
            ESP_LOGE(TAG, "Parâmetro 'tipo' não encontrado na query string");
        }
    } else {
        ESP_LOGE(TAG, "Erro ao obter a query string da requisição");
    }
	
	httpd_resp_send(req, sensor_data, HTTPD_RESP_USE_STRLEN);
	
    return ESP_OK;
}

static esp_err_t command_get_handler(httpd_req_t *req) {
    char param[32]; // Variável para armazenar a query string
    char command[32]; // Variável para armazenar o valor de "tipo"

    // Obtém a query string da requisição
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        //ESP_LOGI(TAG, "Query string recebida: %s", param);

        // Procura o parâmetro "tipo" e armazena o valor em "command"
        if (httpd_query_key_value(param, "tipo", command, sizeof(command)) == ESP_OK) {
            ESP_LOGI(TAG, "Comando recebido: %s", command);
            // Processa o comando recebido
            if (strcmp(command, "command2") == 0) {
                
                //ESP_LOGI(TAG, "Comando 2 executado");
                 E104_status = !E104_status;
                gpio_set_level(E104, E104_status);  
                
                
            } else if (strcmp(command, "command3") == 0) {
                // Código para comando 3
                //ESP_LOGI(TAG, "Comando 3 executado");
                
                
                habilitar_controle_nivel = !habilitar_controle_nivel;
      
            } else if (strcmp(command, "command5") == 0) {
                // Código para comando 5
                //ESP_LOGI(TAG, "Comando 5 executado");
                
                M106_status = !M106_status;
                gpio_set_level(M106, M106_status);
             }
            else if (strcmp(command, "command6") == 0) {
                // Código para comando 5
                //ESP_LOGI(TAG, "Comando 5 executado");
                
                habilitar_controle_temperatura = !habilitar_controle_temperatura;
                
                //M106_status = !M106_status;
                //gpio_set_level(M106, M106_status);    
               
            } else {
                ESP_LOGW(TAG, "Comando não reconhecido: %s", command);
            }
        } else {
            ESP_LOGE(TAG, "Parâmetro 'tipo' não encontrado na query string");
        }
    } else {
        ESP_LOGE(TAG, "Erro ao obter a query string da requisição");
    }

    // Responde ao cliente
    httpd_resp_send(req, "Comando recebido", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static esp_err_t dac_get_handler(httpd_req_t *req) {
    char param[64]; // Buffer para armazenar a string da query
    char canal_str[8], valor_str[8]; // Buffers para armazenar os valores extraídos
    int valor;
    // Obtém a string de query da URL
    if (httpd_req_get_url_query_str(req, param, sizeof(param)) == ESP_OK) {
        
        //ESP_LOGI(TAG, "Query string recebida: %s", param);
        // Extrai o valor do canal do DAC
        if (httpd_query_key_value(param, "tipo", canal_str, sizeof(canal_str)) == ESP_OK) {
			
			//ESP_LOGI(TAG, "Key encontrada: %s", canal_str);
			
			// Extrai o valor do DAC
        	if (httpd_query_key_value(param, "value", valor_str, sizeof(valor_str)) == ESP_OK) {
            	valor = atoi(valor_str); // Converte string para inteiro
            	
            	        
        		//ESP_LOGI("DAC_HANDLER", "Canal: %s, Valor: %d", canal_str, valor);
        
        		// Aqui você pode configurar o DAC do ESP32 com o valor recebido
        		if (strcmp(canal_str, "dac1")) {
         			dac_oneshot_output_voltage(P101_dac_handle, valor);   
     			} else if (strcmp(canal_str, "dac2")) {

        		} 
        	}

		}
	}

    httpd_resp_send(req, "Comando recebido", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


static httpd_handle_t start_webserver(void) {
	// Configuração do Webserver 
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    // Registro de handlers
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler
        };
        httpd_register_uri_handler(server, &root_uri);
  
        httpd_uri_t css_uri = {
    		.uri = "/style.css",
    		.method = HTTP_GET,
    		.handler = style_get_handler,
    		.user_ctx = NULL
		};
		httpd_register_uri_handler(server, &css_uri);

		httpd_uri_t js_uri = {
    		.uri = "/script.js",
    		.method = HTTP_GET,
    		.handler = script_get_handler,
    		.user_ctx = NULL
		};
		httpd_register_uri_handler(server, &js_uri);
		
        httpd_uri_t data_uri = {
            .uri       = "/data",
            .method    = HTTP_GET,
            .handler   = data_get_handler
        };
        httpd_register_uri_handler(server, &data_uri);
		
        httpd_uri_t command_uri = {
            .uri       = "/command",
            .method    = HTTP_GET,
            .handler   = command_get_handler
        };
        httpd_register_uri_handler(server, &command_uri);

		httpd_uri_t dac_uri = {
			.uri	   = "/dac",
			.method	   = HTTP_GET,
			.handler   = dac_get_handler
		};
		httpd_register_uri_handler(server, &dac_uri);
    }
    return server;
}



// Função para calcular o controle PID de nível
float calcular_pid_nivel() {
    erro_nivel = setpoint_nivel - leitura_ultrassom;  // Erro de nível
    erro_int_nivel += erro_nivel;                     // Soma dos erros (termo integral)
//    float erro_der_nivel = erro_nivel - erro_ant_nivel;  // Diferença entre erro atual e erro anterior (termo derivativo)

    controle_nivel = Kp_nivel * erro_nivel + Ki_nivel * erro_int_nivel; //+ Kd_nivel * erro_der_nivel;

    // Limitar o controle a um intervalo válido (0 a 1 para ligar/desligar a bomba)
    if (controle_nivel > 1.0f) {
        controle_nivel = 1.0f;
    } else if (controle_nivel < 0.0f) {
        controle_nivel = 0.0f;
    }

    erro_ant_nivel = erro_nivel;  // Atualiza o erro anterior para o próximo ciclo

    return controle_nivel;
}


// Função para calcular o controle PID de temperatura
float calcular_pid_temp() {
    erro_temp = setpoint_temp - leitura_temperatura;  // Erro de temperatura
    erro_int_temp += erro_temp;                     // Soma dos erros (termo integral)
//    float erro_der_nivel = erro_nivel - erro_ant_nivel;  // Diferença entre erro atual e erro anterior (termo derivativo)

    controle_temp = Kp_temp * erro_temp + Ki_temp * erro_int_temp; //+ Kd_temperatura * erro_der_temperatura;

    // Limitar o controle a um intervalo válido (0 a 1 para ligar/desligar a bomba)
    if (controle_temp > 1.0f) {
        controle_temp = 1.0f;
    } else if (controle_temp < 0.0f) {
        controle_temp = 0.0f;
    }

    erro_ant_temp = erro_temp;  // Atualiza o erro anterior para o próximo ciclo

    return controle_temp;
}


// Função de inicialização do ADC
void adc_init(void) {
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LIC_B101, &config));  // Sensor ultrassônico
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, TIC_B104, &config));  // Sensor temperatura
    
}

// Função para inicializar o DAC
void dac_init(void) {
       dac_oneshot_config_t P101_cfg = {
        .chan_id = DAC_CHAN_1,
    };
    ESP_ERROR_CHECK(dac_oneshot_new_channel(&P101_cfg, &P101_dac_handle));
}

// Função de inicialização do GPIO
void gpio_init(void) {
    gpio_config_t output_config = {
        .pin_bit_mask = (1<<M102) | (1<<E104) | (1<<K1) | (1<<M1) | (1<<M106),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&output_config);

    gpio_config_t input_config = {
        .pin_bit_mask = (1<<B102) | (1<<S111) | (1<<S112) | (1<<B113) | (1<<B114) | (1<<S115) | (1<<S116),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&input_config);
}

void app_main(void) {
    spiffs_init();          // Inicializa o sistema de arquivos
    nvs_flash_init();       // Inicializa a memória não volátil
    wifi_init();            // Inicializa o Wi-Fi
    gpio_init();            // Inicializa os pinos GPIO
    dac_init();             // Inicializa o DAC
    adc_init();             // Inicializa o ADC
    start_webserver();

    while (true) {
        
         // Lê os sensores
        adc_oneshot_read(adc1_handle, LIC_B101, &leitura_ultrassom);  // Leitura do nível
        
        if (habilitar_controle_nivel == true) {
        
        	       
	
	        // Aplica o controle PID de nível
	        float controle_pid_nivel = calcular_pid_nivel();
	
		//	printf("%d;%d\n", bomba_ligada, leitura_ultrassom);
	
	        // Controle de bomba baseado no PID de nível
	        if (controle_pid_nivel > 0.8f && !bomba_ligada) {
	            gpio_set_level(M1, 1);  // Liga a bomba
	            bomba_ligada = true;
	            //printf("Bomba ligada (nível), controle PID de nível: %f\n", controle_pid_nivel);
	        } else if (controle_pid_nivel <= 0.8f && bomba_ligada) {
	            gpio_set_level(M1, 0);  // Desliga a bomba
	            bomba_ligada = false;
	            //printf("Bomba desligada (nível), controle PID de nível: %f\n", controle_pid_nivel);
	        }
        }
        
        
    adc_oneshot_read(adc1_handle, TIC_B104, &leitura_temperatura);  // Leitura da temperatura
    
  		if (habilitar_controle_temperatura == true) {
  
	        // Lê os sensores

	        // Aplica o controle PID de temperatura
	        float controle_pid_temp = calcular_pid_temp();
	
		//	printf("%d;%f\n", aquecedor_ligado, temp);
	
	        // Controle de bomba baseado no PID de temperatura
	        if (controle_pid_temp > 0.8f && !aquecedor_ligado) {
	           gpio_set_level(E104, 1);  // Liga a bomba
	            aquecedor_ligado = true;
	            //printf("Aquecedor ligado (temperatura), controle PID de temperatura: %f\n", controle_pid_nivel);
	        } else if (controle_pid_temp <= 0.8f && aquecedor_ligado) {
	           gpio_set_level(E104, 0);  // Desliga a bomba
	            aquecedor_ligado = false;
	            //printf("Bomba desligada (temperatura), controle PID de temperatura: %f\n", controle_pid_nivel);
	        } 
        }





        // Atraso de 500ms antes de ler novamente
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

