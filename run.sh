echo "Docker-Container wird gestartet!"

docker run --rm -it -v "$(pwd)":/root/env carl_os /bin/bash -c "make build-x86_64"

if [ $? -eq 0 ]; then
  echo "Befehl im Docker Container erfolgreich ausgeführt."
else
  echo "Fehler beim Ausführen des Befehls im Docker Container."
  exit 1
fi

qemu-system-x86_64 -cdrom dist/x86_64/kernel.iso