#include "allocateurMemoire.h"
#include <malloc.h>

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)


static void* espaceBig[ALLOC_N_BIG];
static uint8_t libreBig[ALLOC_N_BIG];

static void* espaceSmall[ALLOC_N_SMALL];
static uint8_t libreSmall[ALLOC_N_SMALL];


// Prépare les buffers nécessaires pour une allocation correspondante aux tailles
// d'images passées en paramètre. Retourne 0 en cas de succès, et -1 si un
// problème (par exemple manque de mémoire) est survenu.
int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie){
    size_t allocate = tailleImageEntree;
    if (allocate < tailleImageSortie) {
        allocate = tailleImageSortie;
    }

    for(int i = 0; i<ALLOC_N_BIG; i++){ 
        espaceBig[i] = malloc(allocate);
        libreBig[i] = 1;
    }

    for(int i=0; i<ALLOC_N_SMALL; i++) {
        espaceSmall[i] = malloc(ALLOC_SMALL_SIZE);
        libreSmall[i] = 1;
    }
    int lock = mlockall(MCL_CURRENT);
    if(lock!=0) {
        perror("Erreur de mlock \n");
        return -1;
    }
    return 0;
}

// Ces deux fonctions doivent pouvoir s'utiliser exactement comme malloc() et free()
// (dans la limite de la mémoire disponible, bien sûr)
void* tempsreel_malloc(size_t taille){
    if(taille > ALLOC_SMALL_SIZE) {
        for(int i = 0; i < (ALLOC_N_BIG); i++){
            if(libreBig[i] == 1){
                libreBig[i] = 0;
                return espaceBig[i];
            }
        }
    }
    else {
        for(int i = 0; i < (ALLOC_N_SMALL); i++){
            if(libreSmall[i] == 1){
                libreSmall[i] = 0;
                return espaceSmall[i];
            }
        }
    }

    perror("Erreur leur du tempsreel_malloc \n");
    return NULL;
}

void tempsreel_free(void* ptr){
   for(int i = 0; i< ALLOC_N_BIG; i++){ 
        if(espaceBig[i] == ptr){
            libreBig[i] = 1;
            return;
        }
    }

    for(int i = 0; i< ALLOC_N_SMALL; i++){ 
        if(espaceSmall[i] == ptr){
            libreSmall[i] = 1;
            return;
        }
    }
    perror("Erreur leur du tempsreel_free!\n");
}
