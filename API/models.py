from sqlalchemy import Column, String, Float, DateTime, Enum as SAEnum
from sqlalchemy.sql import func
from database import Base
import enum

class StatusEnum(str, enum.Enum):
    aguardando  = "aguardando"
    diagnostico = "diagnostico"
    manutencao  = "manutencao"
    pronto      = "pronto"

class Pager(Base):
    __tablename__ = "pagers"

    id              = Column(String, primary_key=True, index=True)   # ex: "PAGER-001"
    status          = Column(SAEnum(StatusEnum), default=StatusEnum.aguardando, nullable=False)

    # Localização atual do pager (enviada pelo ESP32)
    lat             = Column(Float, nullable=True)
    lng             = Column(Float, nullable=True)

    # Informações do veículo
    car_plate       = Column(String, nullable=False)   # Placa
    car_model       = Column(String, nullable=False)   # Modelo ex: "Fiat Palio 2019"
    owner_name      = Column(String, nullable=False)   # Nome do cliente
    service_desc    = Column(String, nullable=True)    # Descrição do serviço

    created_at      = Column(DateTime(timezone=True), server_default=func.now())
    updated_at      = Column(DateTime(timezone=True), onupdate=func.now())
    last_seen       = Column(DateTime(timezone=True), nullable=True)  # Último poll do ESP32
