# PROJETO INTEGRADOR 3

## ETAPA 1:

### INTRODUÇÃO
Este projeto tem como o objetivo o desenvolvimento de um sistema embarcado completo para o monitoramento e controle de variáveis físicas na bancada MPS PA Compact Workstation, da Festo. A solução proposta será capaz de controlar, em tempo real, as variáveis de temperatura e nível, utilizando sensores e atuadores já integrados  à bancada. Todo o sistema de controle será implementado do zero, com código desenvolvido pela própria equipe, utilizando o microcontrolador MSP430F5529.
	A arquitetura do sistema será composta por sensores analógicos e digitais conectados às entradas do microcontrolador, que serão responsáveis pela leitura contínua dos sinais de processo. Após a conversão e tratamento dos sinais, os valores medidos serão comparados com seus respectivos setpoints, definidos manualmente por meio de botões ou menus. A partir dessa comparação, o sistema executará algoritmos de controle PID que gerarão as ações de controle necessárias para acionar os elementos finais: a bomba (nível) e o aquecedor (temperatura).
	Para garantir a visibilidade e a interação com o sistema, será utilizado um módulo Wi-Fi, que transmitirá os dados em tempo real para um computador. Por meio dessa conexão, será possível visualizar as grandezas medidas, os valores de referência, os sinais de controle calculados, além de mensagens de alerta e o status atual da operação, permitindo maior mobilidade e facilidade no acompanhamento do sistema. O controle também poderá ser pausado, reiniciado ou ajustado por meio de comandos simples na interface, permitindo testes com diferentes configurações e perturbações externas.
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


### JUSTICATIVA DA ESCOLHA DO MSP430


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


 	
 	

	
