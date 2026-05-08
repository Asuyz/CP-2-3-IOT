# 🏁 PitStop Pager

> Sistema inteligente de acompanhamento de veículos em oficinas mecânicas — o pager fica no carro, o cliente acompanha tudo em tempo real pelo celular.

![Status](https://img.shields.io/badge/status-em%20desenvolvimento-yellow)
![Plataforma](https://img.shields.io/badge/plataforma-IoT%20%2B%20Web-blue)
![Hardware](https://img.shields.io/badge/hardware-ESP32-orange)
![Licença](https://img.shields.io/badge/licença-MIT-green)

---

## 📖 Sobre o Projeto

O **PitStop Pager** resolve um problema real e universal: a ansiedade de deixar o carro na oficina sem saber o que está acontecendo com ele.

Inspirado no pager de restaurantes, o sistema funciona de forma simples — um dispositivo IoT é colocado dentro do veículo ao chegar na oficina. A partir daí, o mecânico atualiza os status do serviço por um painel web, e o cliente acompanha tudo em tempo real pelo site, incluindo a localização exata do carro no pátio via GPS.

Para tornar a experiência ainda mais envolvente, o sistema conta com um mecanismo de **gamificação com pontos e recompensas**, incentivando o cliente a buscar o veículo rapidamente assim que o serviço for concluído — o que libera vagas no pátio mais rápido e aumenta a eficiência da oficina.

---

## ✨ Funcionalidades

### Para o cliente
- Acompanhamento do status do serviço em tempo real pelo site
- Localização do carro no pátio da oficina via GPS
- Notificação assim que o serviço for concluído
- Aprovação de serviços extras com um toque
- Sistema de pontos e recompensas gamificados
- Histórico completo de serviços realizados

### Para a oficina
- Painel Kanban para atualização de status dos veículos
- Visão geral de todos os carros em atendimento
- Solicitação de aprovação de serviço extra ao cliente em tempo real
- Dados de comportamento e fluxo de veículos

### O pager físico (no carro)
- **GPS** — localização do veículo no pátio
- **LED RGB** — cor muda conforme o status do serviço
- **Vibração** — alerta físico ao mudar de status
- **Wi-Fi** — conectado ao gateway da oficina via ESP32

---

## 🎮 Gamificação

O sistema de pontos incentiva comportamentos que beneficiam tanto o cliente quanto a oficina:

| Ação | Recompensa |
|------|-----------|
| Buscar o carro em menos de 5 min após aviso | Multiplicador **1.2x** nos pontos |
| Buscar em menos de 15 min | Multiplicador **1.1x** |
| Avaliar o serviço no site | +50 pontos |
| Indicar um amigo | +200 pontos |
| 3ª visita no mesmo mês | Multiplicador **1.5x** em tudo |
| Aprovar serviço extra em menos de 2 min | +30 pontos |

### Ranks do cliente

| Rank | Pontos | Benefícios |
|------|--------|-----------|
| 🔩 Parafuso | 0 — 499 | Acesso ao sistema |
| 🔧 Mecânico | 500 — 1499 | Fila prioritária |
| ⚙️ Engrenagem | 1500 — 2999 | Desconto em peças |
| 🏎️ Piloto | 3000 — 4999 | Lavagem grátis |
| 🏁 Campeão | 5000+ | Todos os benefícios |

Ao finalizar o serviço, um **contador regressivo de 5 minutos** é exibido no site do cliente para acionar o multiplicador de pontos — criando urgência positiva sem pressão.

---

## 🔄 Fluxo do Sistema

```
Cliente chega na oficina
        │
        ▼
Atendente instala o pager no carro
        │
        ▼
Cliente vai embora com o link do site
        │
        ▼
Mecânico atualiza status no painel (Kanban)
        │
        ▼
Cliente acompanha em tempo real pelo site
        │
        ▼
Serviço finalizado → site notifica + contador de 5 min inicia
        │
        ▼
Cliente chega → abre o site → GPS mostra onde está o carro
        │
        ▼
Pager é retirado → pontos creditados na conta do cliente
```

---

## 🚦 Status do Serviço

| Status | LED | Descrição |
|--------|-----|-----------|
| ⚪ Aguardando na fila | Branco | Carro recebido, aguardando atendimento |
| 🟡 Em diagnóstico | Amarelo | Avaliação inicial do veículo |
| 🔵 Manutenção em andamento | Azul | Serviço em execução |
| 🔴 Aprovação necessária | Vermelho piscando | Problema extra encontrado, aguarda OK do cliente |
| 🟢 Pronto para retirada | Verde | Serviço concluído |

---

## 🛠️ Stack Técnico

### Hardware
- **ESP32** — microcontrolador principal do pager
- **Módulo GPS NEO-6M** — localização do veículo
- **LED RGB** — sinalização visual de status
- **Motor de vibração** — alerta físico
- **Bateria LiPo** — autonomia de longa duração

### Backend
- **Node.js** com Express
- **MQTT** (Mosquitto broker) — comunicação com o pager
- **WebSocket** — atualização em tempo real no site do cliente
- **PostgreSQL** — banco de dados de ordens de serviço e pontos

### Frontend
- **Site do cliente** — acompanhamento em tempo real + mapa do pátio
- **Painel do mecânico** — Kanban de veículos em atendimento
- **Mapa do pátio** — visualização da posição GPS do carro

### Protocolo de comunicação

```
Pager (ESP32 + GPS)
        │ Wi-Fi / LoRa
        ▼
Gateway da oficina (Raspberry Pi)
        │ MQTT
        ▼
Backend (Node.js)
        │ WebSocket
        ▼
Site do cliente (navegador)
```

---

## 📁 Estrutura do Repositório

```
pitstop-pager/
├── firmware/               # Código do ESP32 (Arduino / MicroPython)
│   ├── src/
│   │   ├── main.ino
│   │   ├── gps.ino
│   │   ├── led.ino
│   │   └── mqtt.ino
│   └── README.md
│
├── backend/                # Servidor Node.js
│   ├── src/
│   │   ├── server.js
│   │   ├── mqtt/
│   │   ├── websocket/
│   │   ├── routes/
│   │   └── models/
│   ├── .env.example
│   └── package.json
│
├── frontend/
│   ├── client/             # Site do cliente
│   └── dashboard/          # Painel do mecânico
│
├── docs/                   # Diagramas e documentação
│   ├── arquitetura.png
│   ├── fluxo.png
│   └── componentes.md
│
└── README.md
```

---

## 🚀 Como Executar

### Pré-requisitos
- Node.js 18+
- Mosquitto (broker MQTT)
- PostgreSQL
- Arduino IDE (para o firmware)

### 1. Clone o repositório

```bash
git clone https://github.com/seu-usuario/pitstop-pager.git
cd pitstop-pager
```

### 2. Configure o backend

```bash
cd backend
cp .env.example .env
# Edite o .env com suas configurações
npm install
npm run dev
```

### 3. Configure o broker MQTT

```bash
mosquitto -c mosquitto.conf
```

### 4. Suba o frontend

```bash
cd frontend/client
npm install
npm run dev
```

### 5. Grave o firmware no ESP32

Abra o arquivo `firmware/src/main.ino` na Arduino IDE, configure o Wi-Fi e o broker MQTT no arquivo de configuração e grave no ESP32.

---

## 🔌 Variáveis de Ambiente

```env
# Servidor
PORT=3000

# Banco de dados
DATABASE_URL=postgresql://usuario:senha@localhost:5432/pitstop

# MQTT
MQTT_HOST=localhost
MQTT_PORT=1883

# JWT
JWT_SECRET=sua_chave_secreta
```

---

## 👥 Equipe

#### Feito Por: 

* [Asuyz](https://github.com/Asuyz)
* [RuralGiovane](https://github.com/RuralGiovane)
* [roque-arantes](https://github.com/roque-arantes) 
* [orlando-IDA](https://github.com/orlando-IDA)


---

## 📄 Licença

Este projeto está licenciado sob a licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

---

<p align="center">Feito com muito café e graxa de motor ☕🔧</p>
