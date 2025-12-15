#!/bin/bash

echo "Starting build in Docker container..."

CONTAINER_ID=$(docker create --platform linux/amd64 carl_os make build-x86_64)

echo "copying source files to container..."
docker cp src $CONTAINER_ID:/root/env/
docker cp targets $CONTAINER_ID:/root/env/
docker cp Makefile $CONTAINER_ID:/root/env/

echo "Starte Build..."
docker start -a $CONTAINER_ID
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
  echo "Build successful..."

  docker cp $CONTAINER_ID:/root/env/dist .
  docker rm $CONTAINER_ID >/dev/null

  if [ ! -f disk.img ]; then
    echo "Creating Drive(10MB)..."
    dd if=/dev/zero of=disk.img bs=1M count=10
  fi

  qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso -drive file=disk.img,format=raw,index=0,media=disk
else
  docker rm $CONTAINER_ID >/dev/null
  echo "Error"
  exit 1
fi
