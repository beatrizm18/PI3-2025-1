# PROJETO INTEGRADOR 3

## ETAPA 1:

### INTRODUÇÃO
Este projeto tem como o objetivo o desenvolvimento de um sistema embarcado completo para o monitoramento e controle de variáveis físicas na bancada MPS PA Compact Workstation, da Festo. A solução proposta será capaz de controlar, em tempo real, as variáveis de temperatura e nível, utilizando sensores e atuadores já integrados  à bancada. Todo o sistema de controle será implementado do zero, com código desenvolvido pela própria equipe, utilizando o microcontrolador MSP430F5529.

A arquitetura do sistema será composta por sensores analógicos e digitais conectados às entradas do microcontrolador, que serão responsáveis pela leitura contínua dos sinais de processo. Após a conversão e tratamento dos sinais, os valores medidos serão comparados com seus respectivos setpoints, definidos manualmente por meio de botões ou menus. A partir dessa comparação, o sistema executará algoritmos de controle PID que gerarão as ações de controle necessárias para acionar os elementos finais: a bomba (nível) e o aquecedor (temperatura).

Para garantir a visibilidade e a interação com o sistema, será utilizado um display, onde será possível visualizar as grandezas medidas, os valores de referência, os sinais de controle calculados, além de mensagens de alerta e o status atual da operação, permitindo maior mobilidade e facilidade no acompanhamento do sistema. O controle também poderá ser pausado, reiniciado ou ajustado por meio de comandos simples na interface, permitindo testes com diferentes configurações e perturbações externas.

A inclusão da lógica de controle no projeto representa o coração da aplicação. Será desenvolvida toda a estrutura de controle, incluindo: 
Leitura e filtragem dos sinais dos sensores analógicos e digitais;
Conversão dos sinais para unidades físicas reais;
Cálculo das variáveis de erro em relação aos setpoints;
Aplicação do algoritmo PID com ajuste fino dos parâmetros Kp, Ki e Kd;
Geração de sinais de saída compatíveis com os atuadores utilizados (ex: PWM para válvula, sinal contínuo para o aquecedor);
Implementação de proteções contra valores extremos ou operação insegura;
Validação do controle com testes físicos em bancada, em condições variadas.
O sistema será modular, permitindo a ativação ou desativação de cada controle individualmente (por exemplo, testar o controle de nível isolado antes de integrá-lo ao controle de temperatura. Esse desenvolvimento modular facilita os testes, a depuração e possíveis expansões futuras. 

Além da programação embarcada, o projeto também contempla a simulação do comportamento do sistema por meio de modelos matemáticos, de modo a prever o desempenho do controle antes da implementação. As simulações serão realizadas com base em respostas ao degrau, identificação de constantes de tempo e modelagem de planta para cada variável controlada. Esses modelos auxiliarão no ajuste inicial dos parâmetros PID, que serão posteriormente refinados empiricamente com base nos testes em laboratório. 
	
Com isso, o projeto entregará uma solução funcional de controle embarcado, implementada de forma autônoma e adaptada à estrutura real da bancada. A expectativa é obter um controle estável, responsivo e confiável, que permita atuar sobre variáveis reais de processo com precisão e repetibilidade, servindo como uma base para futuras melhorias e integrações com sistemas de supervisão.

______________________________________________________________________________________________

### JUSTICATIVA DA ESCOLHA DO MSP430 
O MSP430 é uma família de microcontroladores desenvolvida pela Texas Instruments, reconhecida por sua alta eficiência energética e arquitetura otimizada para aplicações embarcadas de baixo consumo. Uma das principais razões para a escolha é a eficiência, que o torna adequado para projetos de monitoramento e sensores.
 
A arquitetura do MSP430 foi projetada para maximizar o desempenho em aplicações analógicas e de controle com consumo mínimo. É ideal em sistemas que operam de forma contínua, mas com ciclos intermitentes de processamento.

Além disso, o MSP430 possui conversores analógico-digital (ADC) de alta resolução, timers com capacidade de captura/compare e interfaces seriais integradas (como UART, SPI e I2C), o que permite o controle preciso de sensores analógicos e digitais sem a necessidade de circuitos externos complexos. Isso reduz o custo e a complexidade do projeto, mantendo sua robustez e confiabilidade.

Outro ponto relevante é a ampla documentação técnica e suporte, que facilitam o desenvolvimento de aplicações. A Texas Instruments disponibiliza ferramentas gratuitas, como o Code Composer Studio (CCS) que será utilizado no desenvolvimento do projeto e diversas bibliotecas de exemplos, o que agiliza a implementação e a validação dos sistemas desenvolvidos.

Dessa forma, o MSP430 se mostra uma excelente escolha para o projeto, obtendo medições precisas, baixo consumo de energia e operação confiável, como o analisador de parâmetros. Sua simplicidade, eficiência e recursos integrados tornam este microcontrolador ideal para aplicações que demandam um controle preciso e estável dos sinais adquiridos.

_______________________________________________________________________________________________

### SENSORES E ATUADORES DA BANCADA FESTO - MPS PA COMPACT WORKSTATION

### Lista de Sonsores
	* Sensor de nível ultrassônico - B101
 	* Sensoores capaciitivo de proximidade - B113/B114
  	* Interruptores de nível tipo boia - S111/ S112/ S117
   	* Sensor de vazão - B102
    	* Sensor de pressão - B103
     	* Sensor de temperatura - B104

### Lista de Atuadores
	* Bomba centrífuga - P101
 	* Válvula proporcional para controle de vazão - V106
  	* Válvula esférica - V102
   	* Aquecedor - E104

_____________________________________________________________________________________________
 	
| **Descrição**                          | **Referência Datasheet** |
|----------------------------------------|---------------------------|
| Sensor de nível ultrassônico           | B101                      |
| Sensores capacitivo de proximidade     | B113 / B114               |
| Interruptores de nível tipo boia       | S111 / S112               |
| Sensor de vazão                        | B102                      |
| Sensor de pressão                      | B103                      |
| Sensor de temperatura                  | B104                      |

___________________________________________________________________________________________________________________

| #  | Designator                | Description                                     | Quantity | Footprint         |
|----|---------------------------|-------------------------------------------------|----------|-------------------|
| 1  | C13, C14                  | CAP CER 100nF 50V X7R 0805                      | 2        | 0805C             |
| 2  | C1, C2, C3, C4, C5, C6, C9, C10, C11, C12 | CAP CER 10nF 50V X7R 0805         | 10       | 0805C             |
| 3  | R1, R2, R3, R4, R5, R6, R8, R9, R10, R11, R12 | RES 10K OHM 5% 1/8W 0805      | 11       | 0805R             |
| 4  | J1                        | BRB Matekts connector, 1x2, 180°                | 1        | BRB_1X2_M         |
| 5  | IC3, IC4, IC5, IC6, IC7, IC8, IC9, IC10, IC11, IC12, C13, C14 | Integrated Circuit PCB17C    | 12       | DIP762W60P254L854H530Q4N |
| 6  | IC1, IC2                  | Integrated Circuit LM324N/NOPB                 | 2        | DIP794W56P254L910H530Q4N |
| 7  | P3                        | Connector D15T08 Female, 180°, 2 Lines, 7&6 Columns, 26 Pins | 1 | ds1039             |
| 8  | P2                        | Connector D17T08 Female, 180°, 2 Lines, 12 Columns, 24 Pins | 1 | ds1039             |
| 9  | U1                        | IC MCU 32BIT ESP32 DEVKIT V1                   | 1        | ESP32-DEVKIT-V1   |
| 10 | M1                        | LM2596 4 PINS                                   | 1        | MOD_LM2596        |
| 11 | R26, R27, R35, R36        | RES 100 OHM 5% 1/8W                             | 4        | RESISTOR          |
| 12 | R13, R14, R15, R16        | RES 10K OHM 5% 1/8W                             | 4        | RESISTOR          |
| 13 | R28, R29, R30, R31        | RES 33K OHM 5% 1/8W                             | 4        | RESISTOR          |
| 14 | R32, R33, R34             | RES 47K OHM 5% 1/8W                             | 3        | RESISTOR          |
| 15 | R17, R18, R19, R20        | RES 68K OHM 5% 1/8W                             | 4        | RESISTOR          |
| 16 | D1, D2, D3, D4            | Schottky Diode                                  | 4        | SOT95P230X110-3N  |

