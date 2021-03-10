#!/bin/bash

# Ce script assume :
#   - Qu'il est exécuté dans le même répertoire que les fichiers exécutables
#   - Que les vidéos sont situés dans des dossiers *p (par exemple 160p) dans le même répertoire

sudo rm /dev/shm/mem*         
sudo ./decodeur 240p/02_Sintel.ulv /mem1 &
sleep 0.5
sudo ./compositeur /mem1 &
wait;

