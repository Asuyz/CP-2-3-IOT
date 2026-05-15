from fastapi import FastAPI, Depends, HTTPException, status
from sqlalchemy.orm import Session
from sqlalchemy.sql import func
from typing import List
from datetime import datetime

from database import engine, get_db, Base
import models, schemas

# Cria as tabelas no banco se não existirem
Base.metadata.create_all(bind=engine)

app = FastAPI(
    title="PitStop Pager API",
    description="API para gerenciamento de pagers IoT em oficinas mecânicas",
    version="1.0.0"
)

# ─────────────────────────────────────────────────────────────────────────────
# ROTAS DO ESP32 (Pager)
# ─────────────────────────────────────────────────────────────────────────────

@app.get(
    "/pager/{pager_id}",
    response_model=schemas.PagerResponse,
    summary="[ESP32] Consulta status e dados do pager",
    tags=["ESP32"]
)
def get_pager(pager_id: str, db: Session = Depends(get_db)):
    """
    Chamado pelo ESP32 periodicamente (polling).
    Retorna status atual, coordenadas e informações do veículo.
    Atualiza o campo `last_seen` a cada chamada.
    """
    pager = db.query(models.Pager).filter(models.Pager.id == pager_id).first()

    if not pager:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Pager '{pager_id}' não encontrado."
        )

    # Atualiza o last_seen a cada poll do ESP32
    pager.last_seen = datetime.utcnow()
    db.commit()
    db.refresh(pager)

    return pager


@app.post(
    "/pager/{pager_id}/location",
    response_model=schemas.PagerResponse,
    summary="[ESP32] Atualiza coordenadas GPS do pager",
    tags=["ESP32"]
)
def update_location(
    pager_id: str,
    body: schemas.PagerLocationUpdate,
    db: Session = Depends(get_db)
):
    """
    Chamado pelo ESP32 quando detecta mudança de status.
    Envia latitude e longitude atuais do pager (posição do carro no pátio).
    """
    pager = db.query(models.Pager).filter(models.Pager.id == pager_id).first()

    if not pager:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Pager '{pager_id}' não encontrado."
        )

    pager.lat        = body.lat
    pager.lng        = body.lng
    pager.last_seen  = datetime.utcnow()
    pager.updated_at = datetime.utcnow()

    db.commit()
    db.refresh(pager)

    return pager


# ─────────────────────────────────────────────────────────────────────────────
# ROTAS DO PAINEL DA OFICINA (Mecânico)
# ─────────────────────────────────────────────────────────────────────────────

@app.post(
    "/pager",
    response_model=schemas.PagerResponse,
    status_code=status.HTTP_201_CREATED,
    summary="[Oficina] Registra novo pager/veículo",
    tags=["Oficina"]
)
def create_pager(body: schemas.PagerCreate, db: Session = Depends(get_db)):
    """
    Chamado pelo atendente quando o carro chega na oficina.
    Cria o registro do pager com os dados do veículo e cliente.
    Status inicial: aguardando.
    """
    existing = db.query(models.Pager).filter(models.Pager.id == body.id).first()
    if existing:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail=f"Pager '{body.id}' já está em uso. Finalize o serviço anterior."
        )

    pager = models.Pager(
        id           = body.id,
        status       = models.StatusEnum.aguardando,
        car_plate    = body.car_plate,
        car_model    = body.car_model,
        owner_name   = body.owner_name,
        service_desc = body.service_desc
    )

    db.add(pager)
    db.commit()
    db.refresh(pager)

    return pager


@app.patch(
    "/pager/{pager_id}/status",
    response_model=schemas.PagerResponse,
    summary="[Oficina] Atualiza status do serviço",
    tags=["Oficina"]
)
def update_status(
    pager_id: str,
    body: schemas.PagerStatusUpdate,
    db: Session = Depends(get_db)
):
    """
    Chamado pelo mecânico via painel quando o status do serviço muda.
    O ESP32 detectará a mudança no próximo polling e atualizará LED/display.
    """
    pager = db.query(models.Pager).filter(models.Pager.id == pager_id).first()

    if not pager:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Pager '{pager_id}' não encontrado."
        )

    pager.status     = body.status
    pager.updated_at = datetime.utcnow()

    db.commit()
    db.refresh(pager)

    return pager


@app.get(
    "/pagers",
    response_model=List[schemas.PagerResponse],
    summary="[Oficina] Lista todos os pagers ativos",
    tags=["Oficina"]
)
def list_pagers(db: Session = Depends(get_db)):
    """
    Retorna todos os pagers cadastrados.
    Usado pelo painel Kanban da oficina.
    """
    return db.query(models.Pager).order_by(models.Pager.created_at.desc()).all()


@app.delete(
    "/pager/{pager_id}",
    status_code=status.HTTP_204_NO_CONTENT,
    summary="[Oficina] Finaliza e libera o pager",
    tags=["Oficina"]
)
def delete_pager(pager_id: str, db: Session = Depends(get_db)):
    """
    Chamado quando o cliente retira o carro e o pager é devolvido.
    Remove o registro para que o pager possa ser reutilizado.
    """
    pager = db.query(models.Pager).filter(models.Pager.id == pager_id).first()

    if not pager:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Pager '{pager_id}' não encontrado."
        )

    db.delete(pager)
    db.commit()
