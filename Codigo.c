#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#define pin35 35
#define pin32 32
#define pin27 23 

// Dados da sua rede Wi-Fi
const char* ssid = "Rafael";
const char* password = "12345678";

// Servidor web na porta 80
WebServer server(80);

// Umidades
int umidade1 = 0, umidade2 = 0;

// Variáveis para irrigação forçada
unsigned long tempoForcado = 0;
const unsigned long duracaoForcada = 2000; 
bool irrigacaoForcada = false;

// Variáveis para o controle do ciclo automático
const long duracaoIrrigacaoAuto = 2000; 
const long duracaoEsperaAuto = 60000;   
bool cicloAutoAtivo = false;
unsigned long tempoInicioIrrigacaoAuto = 0;


void atualizarUmidade() {
  umidade1 = map(analogRead(pin35), 1550, 4095, 100, 0);
  umidade2 = map(analogRead(pin32), 1550, 4095, 100, 0);

  umidade1 = constrain(umidade1, 0, 100);
  umidade2 = constrain(umidade2, 0, 100);
}

String gerarPaginaHTML() {
  return R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Umidade do Solo</title>
      <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
          font-family: Arial, sans-serif;
          display: flex;
          flex-direction: column;
          justify-content: center;
          align-items: center;
          min-height: 100vh;
          background-color: #f0f0f0;
          padding: 20px 0;
        }
        .container {
          display: flex;
          gap: 20px;
          flex-wrap: wrap;
          justify-content: center;
          width: 100%;
        }
        .card {
          width: 90vw;
          max-width: 200px;
          aspect-ratio: 1 / 1;
          background-color: lightblue;
          border-radius: 20px;
          box-shadow: 0 0 15px rgba(0, 0, 0, 0.2);
          display: flex;
          flex-direction: column;
          justify-content: center;
          align-items: center;
          text-align: center;
          padding: 20px;
        }
        h1 {
          font-size: 1.3em;
          margin-bottom: 15px;
        }
        .valor {
          font-size: 1.8em;
          color: black;
        }
        .status-container {
          width: 100%;
          display: flex;
          flex-direction: column;
          align-items: center;
          margin-top: 20px;
          gap: 20px;
        }
        .status-card {
          width: 90vw;
          max-width: 420px; /* Largura maior para caber o texto */
          padding: 20px;
          background-color: #e0e0e0;
          border-radius: 20px;
          box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);
          text-align: center;
        }
        .status-card .valor {
          color: #333;
          font-weight: bold;
          font-size: 1.5em; /* Fonte um pouco menor para o status */
        }
        .botao-wrapper {
          display: flex;
          flex-direction: column;
          align-items: center;
          min-width: 250px;
        }
        #statusForcar {
          margin-top: 10px;
          font-size: 1em;
          text-align: center;
          color: black;
          min-height: 1.5em;
        }
        button {
          padding: 10px 20px;
          font-size: 1em;
          border: none;
          border-radius: 10px;
          background-color: black;
          color: white;
          cursor: pointer;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <div class="card">
          <h1>Sensor 1</h1>
          <div class="valor" id="valor1">--%</div>
        </div>
        <div class="card">
          <h1>Sensor 2</h1>
          <div class="valor" id="valor2">--%</div>
        </div>
      </div>
      
      <div class="status-container">
        <div class="status-card">
            <h1>Status Automático</h1>
            <div class="valor" id="statusAutomatico">Ocioso</div>
        </div>
        <div class="status-card">
            <div class="botao-wrapper">
              <button onclick="forcarIrrigacao()">Forçar Irrigação</button>
              <div id="statusForcar"></div>
            </div>
        </div>
      </div>

      <script>
        // --- SCRIPT MODIFICADO ---
        setInterval(() => {
          // Busca umidade 1
          fetch("/sensor1")
            .then(resp => resp.text())
            .then(dado => document.getElementById("valor1").textContent = dado + "%");

          // Busca umidade 2
          fetch("/sensor2")
            .then(resp => resp.text())
            .then(dado => document.getElementById("valor2").textContent = dado + "%");

          // --- ADICIONADO --- Busca o status do sistema automático
          fetch("/status")
            .then(resp => resp.text())
            .then(dado => document.getElementById("statusAutomatico").textContent = dado);

        }, 1000); // Atualiza tudo a cada 1 segundo

        function forcarIrrigacao() {
          const botao = document.querySelector("button");
          const status = document.getElementById("statusForcar");

          botao.disabled = true;
          botao.style.opacity = 0.5;
          botao.style.cursor = "not-allowed";

          fetch("/forcar")
            .then(() => {
              let segundos = 2; // Duração da irrigação forçada
              status.textContent = `Irrigando por ${segundos} segundos...`;

              const intervalo = setInterval(() => {
                segundos--;
                if (segundos > 0) {
                  status.textContent = `Irrigando por ${segundos} segundos...`;
                } else {

                  clearInterval(intervalo);
                  status.textContent = "Finalizado.";

                  botao.disabled = false;
                  botao.style.opacity = 1;
                  botao.style.cursor = "pointer";

                  setTimeout(() => {
                    status.textContent = "";
                  }, 3000);
                }
              }, 1000);
            });
        }
      </script>
    </body>
    </html>
  )rawliteral";
}


void setup() {
  Serial.begin(115200);
  pinMode(pin27, OUTPUT);
  digitalWrite(pin27, HIGH); 
  
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("umidade")) {
    Serial.println("Acesse no navegador: http://umidade.local");
  }

  server.on("/", []() {
    server.send(200, "text/html", gerarPaginaHTML());
  });

  server.on("/sensor1", []() {
    server.send(200, "text/plain", String(umidade1));
  });

  server.on("/sensor2", []() {
    server.send(200, "text/plain", String(umidade2));
  });

  server.on("/forcar", []() {
    irrigacaoForcada = true;
    tempoForcado = millis();
    server.send(200, "text/plain", "Irrigação forçada ativada");
  });

  server.on("/status", []() {
    String status = "Ocioso"; 
    
    if (cicloAutoAtivo) {
      unsigned long tempoDecorrido = millis() - tempoInicioIrrigacaoAuto;
      
      if (tempoDecorrido < duracaoIrrigacaoAuto) {
        status = "Irrigando...";
      } else {
        long tempoTotalCiclo = duracaoIrrigacaoAuto + duracaoEsperaAuto;
        long tempoRestanteMs = tempoTotalCiclo - tempoDecorrido;
        int segundosRestantes = tempoRestanteMs / 1000;
        status = "Próxima verificação em: " + String(segundosRestantes) + "s";
      }
    }
    server.send(200, "text/plain", status);
  });

  server.begin();
  Serial.println("Servidor web iniciado");
}


void loop() {
  atualizarUmidade();

  // Lógica principal de irrigação
  if (irrigacaoForcada == true) {
    digitalWrite(pin27, LOW); // Liga a bomba
    if (millis() - tempoForcado > duracaoForcada) {
      irrigacaoForcada = false;
      digitalWrite(pin27, HIGH); 
    }
  } else {
    // Bloco da irrigação AUTOMÁTICA
    if ((umidade1 < 50 || umidade2 < 50) && !cicloAutoAtivo) {
      cicloAutoAtivo = true;
      tempoInicioIrrigacaoAuto = millis();
      digitalWrite(pin27, LOW); 
    }

    if (cicloAutoAtivo) {
      unsigned long tempoDecorrido = millis() - tempoInicioIrrigacaoAuto;
      if (tempoDecorrido >= duracaoIrrigacaoAuto) {
        digitalWrite(pin27, HIGH); 
      }
      if (tempoDecorrido >= (duracaoIrrigacaoAuto + duracaoEsperaAuto)) {
        cicloAutoAtivo = false; 
      }
    }
    
    if ((umidade1 >= 50 && umidade2 >= 50) && !cicloAutoAtivo) {
        digitalWrite(pin27, HIGH); 
    }
  }
  
  server.handleClient();
}