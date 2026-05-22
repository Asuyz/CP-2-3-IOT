# 🏁 PitStop Pager

> Sistema IoT para acompanhamento de veículos na oficina — o pager no carro consulta a API, mostra o status no display e envia a localização via GPS.

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![Plataforma](https://img.shields.io/badge/plataforma-IoT%20%2B%20API-blue)
![Hardware](https://img.shields.io/badge/hardware-ESP32-orange)

---

## 📖 Sobre o Projeto

O **PitStop Pager** é um projeto IoT para oficinas mecânicas: um ESP32 fica dentro do carro, faz polling na API para obter o status do serviço e exibe as informações no display OLED. Quando necessário, ele também envia a posição GPS do veículo para a API.

---

## ✨ Funcionalidades

### API (FastAPI)

- Cadastro de pager/veículo
- Atualização de status do serviço
- Consulta do pager pelo ESP32 (polling)
- Atualização de localização (lat/lng)
- Listagem e remoção de pagers

### Pager (ESP32)

- Wi‑Fi + polling periódico da API
- Display OLED com status e dados do veículo
- LED RGB indicando o status
- Buzzer de alerta
- Envio de localização via GPS (NEO‑6M)

---

## 🔄 Fluxo do Sistema

```
Atendente cadastra o pager na API
        │
        ▼
ESP32 faz polling periódico
        │
        ├── Atualiza LED/Display
        ▼
ESP32 envia GPS para a API
```

---

## 🚦 Status do Serviço

| Status         | LED     | Descrição                              |
| -------------- | ------- | -------------------------------------- |
| ⚪ Aguardando  | Branco  | Carro recebido, aguardando atendimento |
| 🟡 Diagnóstico | Amarelo | Avaliação inicial do veículo           |
| 🔵 Manutenção  | Azul    | Serviço em execução                    |
| 🟢 Pronto      | Verde   | Serviço concluído                      |

---

## 🛠️ Stack Técnico

### Hardware

- **ESP32**
- **Módulo GPS NEO‑6M**
- **Display OLED SSD1306**
- **LED RGB**
- **Buzzer**

### Backend

- **Python + FastAPI**
- **SQLAlchemy**
- **SQLite**
- **Uvicorn**

### Simulação

- **Wokwi** (wokwi.toml + chip GPS simulado)

---

## 📁 Estrutura do Repositório

```
CP-2-3-IOT/
├── API/                 # Servidor FastAPI e banco SQLite
│   ├── main.py
│   ├── models.py
│   ├── schemas.py
│   ├── database.py
│   ├── requirements.txt
│   └── pitstop.db
├── pitstop/             # Firmware ESP32 (Arduino)
│   ├── pitstop.ino
│   └── build/           # Artefatos de compilação
├── wokwi.toml           # Configuração da simulação Wokwi
├── wokwi-project.txt    # Link do projeto Wokwi
├── gps-neo6m.chip.*     # Chip GPS simulado
├── diagram.json         # Diagrama do circuito (Wokwi)
├── libraries.txt        # Dependências Arduino
└── README.md
```

---

## 🚀 Como Executar

### 1. Subir a API

```bash
cd API
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn main:app --host 0.0.0.0 --port 8000
```

### 2. Criar um pager (necessário para o ESP32)

```bash
curl -X POST http://localhost:8000/pager \
  -H "Content-Type: application/json" \
  -d '{"id":"PAGER-001","car_plate":"ABC-1234","car_model":"Fiat Palio 2019","owner_name":"João Silva","service_desc":"Troca de óleo"}'
```

### 3. Rodar o firmware

- Ajuste `API_BASE_URL` em `pitstop/pitstop.ino`:
  - Wokwi: `http://host.wokwi.internal:8000`
  - ESP32 real: `http://<IP-da-maquina>:8000`
- Compile e grave no ESP32 pela Arduino IDE ou rode no Wokwi (VS Code + extensão).

---

## ⚙️ Configuração do Firmware

- `WIFI_SSID` e `WIFI_PASSWORD`
- `API_BASE_URL`
- `PAGER_ID`
- `POLL_INTERVAL`
- `LOCATION_INTERVAL`

---

## 👥 Equipe

#### Feito Por:

- [Asuyz](https://github.com/Asuyz)
- [RuralGiovane](https://github.com/RuralGiovane)
- [roque-arantes](https://github.com/roque-arantes)
- [orlando-IDA](https://github.com/orlando-IDA)
- [Gabriel Lourenço Martins](https://github.com/ggabmartins)

---

<p align="center">Feito com muito café e graxa de motor ☕🔧</p>
