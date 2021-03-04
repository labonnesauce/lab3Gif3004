#include "commMemoirePartagee.h"
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <sys/mman.h>
#include <pthread.h>

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas du lecteur)
int initMemoirePartageeLecteur(const char* identifiant, struct memPartage *zone){
    struct stat fileInfo;
    fstat(zone->fd, &fileInfo);

    if(fileInfo.st_size){ //­>0
        if(zone->header->frameWriter == 0){
            sched_yield();
        }else{

        }
    }

    shm_open(identifiant, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); //peut etre enlever Write mode
}

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas de l'écrivain)
int initMemoirePartageeEcrivain(const char* identifiant, struct memPartage *zone, size_t taille, struct memPartageHeader* headerInfos){
    shm_open(identifiant, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR); //peut etre enlever Read mode
    ftruncate(zone->fd, zone->tailleDonnees);

    mmap(NULL, zone->tailleDonnees, PROT_WRITE, MAP_SHARED, zone->fd, 4); //addresse? offset de 4 ou 0?
}

// Appelé par le lecteur pour se mettre en attente d'un résultat
int attenteLecteur(struct memPartage *zone){

    if(zone->copieCompteur == 0){
        sched_yield();
    }else{
        pthread_mutex_lock(&zone->header->mutex);
        zone->header->frameReader++;
    }
    return 0;
}

// Fonction spéciale similaire à attenteLecteur, mais asynchrone : cette fonction ne bloque jamais.
// Cela est utile pour le compositeur, qui ne doit pas bloquer l'entièreté des flux si un seul est plus lent.
int attenteLecteurAsync(struct memPartage *zone){
    if(zone->copieCompteur == 0){
        sched_yield();
    }else{
        pthread_mutex_lock(&(zone->header->mutex));
        zone->header->frameReader++;
    }
    return 0;

}

// Appelé par l'écrivain pour se mettre en attente de la lecture du résultat précédent par un lecteur
int attenteEcrivain(struct memPartage *zone){
    if(zone->copieCompteur == zone->header->frameWriter){
        sched_yield();
    }else{
        pthread_mutex_lock(&(zone->header->mutex));
        zone->header->frameReader++;
    }
    return 0;
}