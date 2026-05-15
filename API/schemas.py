from pydantic import BaseModel, Field
from typing import Optional
from datetime import datetime
from models import StatusEnum

# ── Criar pager (mecânico registra o carro) ──────────────────────────────────
class PagerCreate(BaseModel):
    id:           str         = Field(..., example="PAGER-001")
    car_plate:    str         = Field(..., example="ABC-1234")
    car_model:    str         = Field(..., example="Fiat Palio 2019")
    owner_name:   str         = Field(..., example="João Silva")
    service_desc: Optional[str] = Field(None, example="Troca de óleo e filtro")

# ── Atualizar status (mecânico atualiza pelo painel) ─────────────────────────
class PagerStatusUpdate(BaseModel):
    status: StatusEnum = Field(..., example="manutencao")

# ── Atualizar localização (ESP32 envia GPS) ───────────────────────────────────
class PagerLocationUpdate(BaseModel):
    lat: float = Field(..., example=-23.5505)
    lng: float = Field(..., example=-46.6333)

# ── Resposta completa do pager (retornada no GET) ─────────────────────────────
class PagerResponse(BaseModel):
    id:           str
    status:       StatusEnum
    lat:          Optional[float]
    lng:          Optional[float]
    car_plate:    str
    car_model:    str
    owner_name:   str
    service_desc: Optional[str]
    created_at:   Optional[datetime]
    updated_at:   Optional[datetime]
    last_seen:    Optional[datetime]

    class Config:
        from_attributes = True
