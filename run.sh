#!/bin/bash

echo "Docker-Container wird gestartet!"

CONTAINER_ID=$(docker create --platform linux/amd64 carl_os make build-x86_64)

echo "Kopiere Dateien in Container..."
docker cp src $CONTAINER_ID:/root/env/
docker cp targets $CONTAINER_ID:/root/env/
docker cp Makefile $CONTAINER_ID:/root/env/


echo "Starte Build..."
docker start -a $CONTAINER_ID
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
  echo "Build erfolgreich! Kopiere Artefakte zurück..."

  docker cp $CONTAINER_ID:/root/env/dist .
  docker rm $CONTAINER_ID > /dev/null
  echo "Befehl im Docker Container erfolgreich ausgeführt."
  
  # Erstelle virtuelle Festplatte falls nicht vorhanden
  if [ ! -f disk.img ]; then
    echo "Erstelle virtuelle Festplatte (10MB)..."
    dd if=/dev/zero of=disk.img bs=1M count=10
  fi
 
  qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso -drive file=disk.img,format=raw,index=0,media=disk
else
  docker rm $CONTAINER_ID > /dev/null
  echo "Fehler beim Ausführen des Befehls im Docker Container."
  exit 1
fi

