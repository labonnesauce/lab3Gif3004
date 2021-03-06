#include "allocateurMemoire.h"
#include <malloc.h>

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)

#define ALLOC_N_BIG 16
#define ALLOC_N_SMALL 100
#define ALLOC_SMALL_SIZE 1024
#define ESPACE_MEMOIRE 5

static void* espace[ESPACE_MEMOIRE];
static uint8_t libre[ESPACE_MEMOIRE];


// Prépare les buffers nécessaires pour une allocation correspondante aux tailles
// d'images passées en paramètre. Retourne 0 en cas de succès, et -1 si un
// problème (par exemple manque de mémoire) est survenu.
int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie){
    size_t allocate = tailleImageEntree;
    if (allocate < tailleImageSortie) {
        allocate = tailleImageSortie;
    }

    for(int i = 0; i<ESPACE_MEMOIRE; i++){ 
        espace[i] = malloc(allocate);
        libre[i] = 1;
    }
    return 0;
}

// Ces deux fonctions doivent pouvoir s'utiliser exactement comme malloc() et free()
// (dans la limite de la mémoire disponible, bien sûr)
void* tempsreel_malloc(size_t taille){
    for(int i = 0; i < (ESPACE_MEMOIRE - 1); i++){
        if(libre[i] == 1){
            libre[i] = 0;
            return espace[i];
        }
    }
    perror("Erreur leur du tempsreel_malloc \n", taille);
    return NULL;
}

void tempsreel_free(void* ptr){
   for(int i = 0; i< (ESPACE_MEMOIRE-1); i++){ 
        if(epace[i] == ptr){
            libre[i] = 1;
            return;
        }
    }
    perror("Erreur leur du tempsreel_free!\n");
}
