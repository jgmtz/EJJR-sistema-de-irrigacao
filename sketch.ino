#define SENSOR_SOLO 32
#define SENSOR_NIVEL 25
#define BOMBA_PIN 26

const int umidade_meta = 70;

void setup() {
  Serial.begin(115200);

  pinMode(SENSOR_NIVEL, INPUT_PULLUP);
  pinMode(BOMBA_PIN, OUTPUT);
  digitalWrite(BOMBA_PIN, LOW);

  Serial.println("Sistema de Irrigação Inteligente Iniciado!");
  Serial.print("Meta de Umidade: ");
  Serial.print(umidade_meta);
  Serial.println("%");
}

void loop() {
  int umidadeSolo = analogRead(SENSOR_SOLO);
  int estadoBotao = digitalRead(SENSOR_NIVEL);

  bool aguaDisponivel = (estadoBotao == LOW);

  int umidade_percent = map(umidadeSolo, 4095, 0, 0, 100);

  Serial.println("------------------------------");
  Serial.print("Umidade do Solo: ");
  Serial.print(umidade_percent);
  Serial.println("%");
  Serial.print("Nível de Água: ");
  Serial.println(aguaDisponivel ? "Reservatório OK" : "Reservatório Vazio");

  if (!aguaDisponivel) {
    Serial.println("⚠️ Reservatório sem água!");
  } else {
    if (umidade_percent < umidade_meta) {
      Serial.println("🌱 Solo seco detectado. Iniciando irrigação...");
      digitalWrite(BOMBA_PIN, HIGH);
      delay(5000);
      digitalWrite(BOMBA_PIN, LOW);
      Serial.println("✅ Irrigação concluída.");
    } else {
      Serial.println("✅ Solo com umidade adequada. Irrigação não necessária.");
    }
  }

  delay(10000);
}
